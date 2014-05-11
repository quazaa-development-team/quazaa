/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2013.
** This file is part of QUAZAA (quazaa.sourceforge.net)
**
** Quazaa is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Quazaa is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with Quazaa; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "searchmanager.h"
#include "managedsearch.h"
#include "queryhit.h"
#include "g2packet.h"
#include <QMutexLocker>
#include "g2hostcache.h"
#include "g2hostcachehost.h"
#include "network.h"
#include <QMetaType>
#include "securitymanager.h"

#include "quazaasettings.h"
#include "commonfunctions.h"

#include "debug_new.h"

SearchManager searchManager;

const quint32 PacketsPerSec = 8;

SearchManager::SearchManager( QObject* parent ) :
	QObject( parent )
{
	m_nPruneCounter = 0;
	m_nCookie = 0;

	qRegisterMetaType<QueryHitSharedPtr>( "QueryHitSharedPtr" );
}

void SearchManager::add( ManagedSearch* pSearch )
{
	QMutexLocker l( &m_pSection );

	Q_ASSERT( !m_lSearches.contains( pSearch->m_oGUID ) );
	m_lSearches.insert( pSearch->m_oGUID, pSearch );
	if ( pSearch->thread() != searchManager.thread() )
	{
		pSearch->moveToThread( searchManager.thread() );
	}
}

void SearchManager::remove( ManagedSearch* pSearch )
{
	QMutexLocker l( &m_pSection );

	Q_ASSERT( m_lSearches.contains( pSearch->m_oGUID ) );
	m_lSearches.remove( pSearch->m_oGUID );
}

/**
 * @brief CSearchManager::find allows to find a managed search by its GUID.
 * @param oGUID
 * @return A ptr to the ManagedSearch; NULL if the search could not be found.
 */
ManagedSearch* SearchManager::find( QUuid& oGUID )
{
	return m_lSearches.value( oGUID, NULL );
}

void SearchManager::onTimer()
{
	QMutexLocker l( &m_pSection );

	quint32 nSearches = m_lSearches.size();

	foreach ( ManagedSearch * pSearch, m_lSearches )
	{
		pSearch->sendHits();
		if ( pSearch->m_bPaused )
		{
			--nSearches;
		}
	}

	if ( !nSearches )
	{
		return;
	}

	const QDateTime tNowDT = common::getDateTimeUTC();

	++m_nPruneCounter;
	if ( !( m_nPruneCounter % 30 ) )
	{
		hostCache.pruneByQueryAck( tNowDT.toTime_t() );
	}

	quint32 nPacketsLeft = PacketsPerSec;
	quint32 nPacketsPerSearch = qMin( 4u, nPacketsLeft / nSearches + 1u );


	quint32 nExecuted = 0;

	foreach ( ManagedSearch * pSearch, m_lSearches )
	{
		if ( pSearch->m_bPaused )
		{
			continue;
		}

		if ( pSearch->m_nCookie != m_nCookie )
		{
			quint32 nPacket = qMin( nPacketsPerSearch, nPacketsLeft );
			pSearch->execute( tNowDT, &nPacket );
			pSearch->m_nCookie = m_nCookie;
			nPacketsLeft -= ( ( nPacketsLeft < nPacketsPerSearch ) ?
							  ( nPacketsLeft - nPacket ) : ( nPacketsPerSearch - nPacket ) );
			++nExecuted;

			if ( !nPacketsLeft )
			{
				break;
			}
		}
	}

	if ( nPacketsLeft || nExecuted == nSearches )
	{
		++m_nCookie;
	}
}

/**
 * @brief CSearchManager::OnQueryAcknowledge
 * @param pPacket
 * @param oSender: sender
 * @param oGUID will be overwritten with the search GUID contained in the packet.
 * @return true: the caller must route the packet; false otherwise
 */
bool SearchManager::onQueryAcknowledge( G2Packet* pPacket, const EndPoint& oSender, QUuid& oGUID )
{
	// Note: The query acknowledgement packet is used to inform a search client that a target hub
	// has received its query and is processing it. It also provides information for the search
	// client's hub cache, expanding knowledge of the network and ensuring hubs are not searched
	// more than once in a given query.

	if ( !pPacket->m_bCompound )
	{
		return false;
	}

	pPacket->skipCompound();			// skip children to get search GUID
	if ( pPacket->getRemaining() < 16 )	// must be at least 16 bytes for GUID
	{
		return false;
	}

	oGUID = pPacket->readGUID();		// Read search GUID

	QMutexLocker l( &m_pSection );

	if ( ManagedSearch* pSearch = find( oGUID ) )	// is it our Query Ack?
	{
		// YES, this is ours, let's parse the packet and process it

		EndPoint oQAOrigin = oSender;
		QList<EndPoint>           lDoneHubs;      // already searched Hubs
		QHash<EndPoint, quint32>  lSuggestedHubs; // possible suggestions for searching

		quint32 tRetryAfter = 0; // number of seconds to wait before retrying the Hub
		qint64  tAdjust     = 0; // offset between the other's UTC and ours

		QString sVendor;
		bool bLikelyFoxy = false;
		const quint32 tNow = common::getTNowUTC();

		// counters
		quint32 nSearchedHubs = 0, nSearchedLeaves = 0, nSuggestedHubs = 0;

		pPacket->m_nPosition = 0;   // reset position within the packet

		char szType[9];         // child packet name
		quint32 nLength = 0;    // child packet length
		quint32 nNext   = 0;    // next child packet starting position

		while ( pPacket->readPacket( &szType[0], nLength ) )
		{
			nNext = pPacket->m_nPosition + nLength;

			// Officially defined children (g2dn):
			// /QA/D  - a hub that has now been searched and should not be queried again
			// /QA/S  - a hub that might later be searched if this has not already been done
			// /QA/TS - UTC timestamp of the packet generation time
			// /QA/RA - the amount of time that needs to pass at least before retrying the sender
			// /QA/FR - for forwarded queries only: original sender of te packet

			// Extensions defined by Quazaa:
			// /QA/V  - 4 letter vendor code

			if ( strcmp( "D", szType ) == 0 && nLength >= 4 )
			{
				// hub that has now been searched

				EndPoint oSearchedHub;
				if ( nLength >= 16 )
				{
					// IPv6
					Q_IPV6ADDR nIP;
					pPacket->read( &nIP, 16 );
					oSearchedHub.setAddress( nIP );

					if ( nLength >= 18 )
					{
						quint16 nPort = pPacket->readIntLE<quint16>();
						oSearchedHub.setPort( nPort );
					}

					if ( nLength >= 20 )
					{
						nSearchedLeaves += pPacket->readIntLE<quint16>();
					}
				}
				else
				{
					// IPv4
					quint32 nIP = pPacket->readIntBE<quint32>();
					oSearchedHub.setAddress( nIP );

					if ( nLength >= 6 )
					{
						quint16 nPort = pPacket->readIntLE<quint16>();
						oSearchedHub.setPort( nPort );
					}

					if ( nLength >= 8 )
					{
						nSearchedLeaves += pPacket->readIntLE<quint16>();
					}
				}

				lDoneHubs.append( oSearchedHub );

				++nSearchedHubs;
			}
			else if ( strcmp( "S", szType ) == 0 && nLength >= 6 )
			{
				// hub suggestion

				EndPoint suggestion;
				pPacket->readHostAddress( &suggestion, !( nLength >= 18 ) );
				const quint32 tTimeStamp = ( nLength >= ( !suggestion.protocol() ? 10u : 22u ) ) ?
										   pPacket->readIntLE<quint32>() + tAdjust : tNow - 60;

				lSuggestedHubs.insert( suggestion, tTimeStamp );

				++nSuggestedHubs;
			}
			else if ( strcmp( "TS", szType ) == 0 && nLength >= 4 )
			{
				// UTC timestamp of the packet generation time

				// TODO: this child packet should be read before parsing the other packets
				tAdjust = tNow - pPacket->readIntLE<quint32>();
			}
			else if ( strcmp( "RA", szType ) == 0 && nLength >= 2 )
			{
				// the amount of time that needs to pass at least before retrying the sender
				// TODO: make sure the packet order of the children does not matter anymore
				//       (FR and RA packet)

				if ( nLength >= 4 )
				{
					tRetryAfter = pPacket->readIntLE<quint32>();
				}
				else if ( nLength >= 2 )
				{
					tRetryAfter = pPacket->readIntLE<quint16>();
				}

				hostCache.lock();
				SharedG2HostPtr pHost = hostCache.get( oQAOrigin );
				if ( !pHost.isNull() )
				{
					pHost->setRetryAfter( tNow + tRetryAfter );
				}
				hostCache.unlock();
			}
			else if ( strcmp( "FR", szType ) == 0 && nLength >= 4 )
			{
				// for forwarded queries only: original sender of the packet

				if ( nLength >= 16 )
				{
					Q_IPV6ADDR ip;
					pPacket->read( &ip, 16 );
					oQAOrigin.setAddress( ip );
				}
				else
				{
					quint32 nFromIp = pPacket->readIntBE<quint32>();
					oQAOrigin.setAddress( nFromIp );
				}
			}
			else if ( strcmp( "V", szType ) == 0 && nLength >= 4 )
			{
				sVendor = pPacket->readString( 4 );
			}
			else
			{
				// this handles extensions and future developments unknown to us
				systemLog.postLog( LogSeverity::Debug, Component::G2,
								   tr( "Caught unknown child of QA packet: " ) + szType );
			}

			pPacket->m_nPosition = nNext;
		}

		// we already know QA GUID

		bLikelyFoxy = ( nSearchedHubs > 20 && nSuggestedHubs < 6 && tRetryAfter == 300 );
		systemLog.postLog( LogSeverity::Debug, Component::G2,
						   "Processing query acknowledge from %s (time adjust %+d seconds): %d hubs, %d leaves, %d suggested hubs, retry after %d seconds, %s, likely foxy: %s).",
						   oQAOrigin.toString().toLocal8Bit().constData(), int( tAdjust ), nSearchedHubs,
						   nSearchedLeaves, nSuggestedHubs, tRetryAfter,
						   ( sVendor.isEmpty() ? "unknown" : qPrintable( sVendor ) ), ( bLikelyFoxy ? "yes" : "no" ) );

		// add/update searched Hubs to the cache
		if ( Q_LIKELY( !bLikelyFoxy ) )
		{
			for ( QList<EndPoint>::iterator itHub = lDoneHubs.begin();
				  itHub != lDoneHubs.end(); ++itHub )
			{
				if ( ( *itHub ).port() && !securityManager.isDenied( *itHub ) )
				{
					hostCache.add( *itHub, tNow );
				}
			}

			for ( QHash<EndPoint, quint32>::iterator itHub = lSuggestedHubs.begin();
				  itHub != lSuggestedHubs.end(); ++itHub )
			{
				if ( itHub.key().port() && !securityManager.isDenied( itHub.key() ) )
				{
					hostCache.add( itHub.key(), itHub.value() );
				}
			}
		}
		else
		{
			EndPoint oBan = oSender;

			if ( networkG2.isFirewalled() && !lDoneHubs.isEmpty() )
			{
				oBan = lDoneHubs[0]; // 1st done hub, Shareaza does not include /FR child packet
			}

			securityManager.ban( oBan, Security::RuleTime::SixHours, true,
								 tr( "[AUTO] Likely Foxy client" ), true
#if SECURITY_LOG_BAN_SOURCES
								 , QString( "searchmanager.cpp line 307" )
#endif // SECURITY_LOG_BAN_SOURCES
							   );

			// remove suggested Hubs from host cache
			for ( QList<EndPoint>::iterator itHub = lDoneHubs.begin(); itHub != lDoneHubs.end(); ++itHub )
			{
				hostCache.remove( *itHub );
			}
		}

		pSearch->m_nHubs += nSearchedHubs;
		pSearch->m_nLeaves += nSearchedLeaves;

		const QDateTime tNowDT = common::getDateTimeUTC();
		pSearch->onHostAcknowledge( oQAOrigin, tNowDT );

		for ( int i = 0; i < lDoneHubs.size(); ++i )
		{
			pSearch->onHostAcknowledge( lDoneHubs[i], tNowDT );
		}

		emit pSearch->statsUpdated();

		return false;
	}

	// not our ack - tell caller to route it
	return true;
}

/**
 * @brief CSearchManager::onQueryHit handles a newly arriving QueryHit.
 * @param pPacket
 * @param pHitInfo
 * @return true if the packet is to be routed; false otherwise (e.g. own search)
 */
bool SearchManager::onQueryHit( G2Packet* pPacket, QueryHitInfo* pHitInfo )
{
	Q_ASSERT( pPacket );
	Q_ASSERT( pHitInfo );

	QMutexLocker l( &m_pSection );

	if ( ManagedSearch* pSearch = find( pHitInfo->m_oGUID ) )
	{
		// our search

		QueryHit* pHit = QueryHit::readPacket( pPacket, pHitInfo );

		if ( pHit )
		{
			pSearch->onQueryHit( pHit );
		}

		return false;
	}

	return true;
}

