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
#include "hostcache.h"
#include "network.h"
#include <QMetaType>
#include "Security/securitymanager.h"

#include "quazaasettings.h"
#include "commonfunctions.h"

#include "debug_new.h"

CSearchManager SearchManager;

const quint32 PacketsPerSec = 8;

CSearchManager::CSearchManager(QObject* parent) :
	QObject(parent)
{
	m_nPruneCounter = 0;
	m_nCookie = 0;

	qRegisterMetaType<QueryHitSharedPtr>("QueryHitSharedPtr");
}

void CSearchManager::Add(CManagedSearch* pSearch)
{
	QMutexLocker l(&m_pSection);

	Q_ASSERT(!m_lSearches.contains(pSearch->m_oGUID));
	m_lSearches.insert(pSearch->m_oGUID, pSearch);
	if(pSearch->thread() != SearchManager.thread())
	{
		pSearch->moveToThread(SearchManager.thread());
	}
}

void CSearchManager::Remove(CManagedSearch* pSearch)
{
	QMutexLocker l(&m_pSection);

	Q_ASSERT(m_lSearches.contains(pSearch->m_oGUID));
	m_lSearches.remove(pSearch->m_oGUID);
}

CManagedSearch* CSearchManager::Find(QUuid& oGUID)
{
	return m_lSearches.value(oGUID, 0);
}

void CSearchManager::OnTimer()
{
	QMutexLocker l( &m_pSection );

	quint32 nSearches = m_lSearches.size();

	foreach ( CManagedSearch* pSearch, m_lSearches )
	{
		pSearch->SendHits();
		if ( pSearch->m_bPaused )
			--nSearches;
	}

	if ( !nSearches )
		return;

	const QDateTime tNowDT = common::getDateTimeUTC();

	++m_nPruneCounter;
	if ( !(m_nPruneCounter % 30) )
	{
		hostCache.pruneByQueryAck( tNowDT.toTime_t() );
	}

	quint32 nPacketsLeft = PacketsPerSec;
	quint32 nPacketsPerSearch = qMin( 4u, nPacketsLeft / nSearches + 1u );


	quint32 nExecuted = 0;

	foreach ( CManagedSearch* pSearch, m_lSearches )
	{
		if ( pSearch->m_bPaused )
			continue;

		if ( pSearch->m_nCookie != m_nCookie )
		{
			quint32 nPacket = qMin( nPacketsPerSearch, nPacketsLeft );
			pSearch->Execute( tNowDT, &nPacket );
			pSearch->m_nCookie = m_nCookie;
			nPacketsLeft -= ( (nPacketsLeft < nPacketsPerSearch) ?
							  (nPacketsLeft - nPacket) : (nPacketsPerSearch - nPacket) );
			++nExecuted;

			if ( !nPacketsLeft )
				break;
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
bool CSearchManager::OnQueryAcknowledge(G2Packet* pPacket, const CEndPoint& oSender, QUuid& oGUID)
{
	// Note: The query acknowledgement packet is used to inform a search client that a target hub
	// has received its query and is processing it. It also provides information for the search
	// client's hub cache, expanding knowledge of the network and ensuring hubs are not searched
	// more than once in a given query.

	if ( !pPacket->m_bCompound )
	{
		return false;
	}

	pPacket->SkipCompound();            // skip children to get search GUID
	if ( pPacket->GetRemaining() < 16 ) // must be at least 16 bytes for GUID
	{
		return false;
	}

	oGUID = pPacket->ReadGUID();        // Read search GUID

	QMutexLocker l( &m_pSection );

	if ( CManagedSearch* pSearch = Find( oGUID ) )	// is it our Query Ack?
	{
		// YES, this is ours, let's parse the packet and process it

		CEndPoint oQAOrigin = oSender;
		QList<CEndPoint>           lDoneHubs;      // already searched Hubs
		QHash<CEndPoint, quint32>  lSuggestedHubs; // possible suggestions for searching

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

		while ( pPacket->ReadPacket( &szType[0], nLength ) )
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

			if ( strcmp( "D", szType) == 0 && nLength >= 4 )
			{
				// hub that has now been searched

				CEndPoint oSearchedHub;
				if ( nLength >= 16 )
				{
					// IPv6
					Q_IPV6ADDR nIP;
					pPacket->Read( &nIP, 16 );
					oSearchedHub.setAddress( nIP );

					if ( nLength >= 18 )
					{
						quint16 nPort = pPacket->ReadIntLE<quint16>();
						oSearchedHub.setPort(nPort);
					}

					if ( nLength >= 20 )
					{
						nSearchedLeaves += pPacket->ReadIntLE<quint16>();
					}
				}
				else
				{
					// IPv4
					quint32 nIP = pPacket->ReadIntBE<quint32>();
					oSearchedHub.setAddress( nIP );

					if ( nLength >= 6 )
					{
						quint16 nPort = pPacket->ReadIntLE<quint16>();
						oSearchedHub.setPort(nPort);
					}

					if ( nLength >= 8 )
					{
						nSearchedLeaves += pPacket->ReadIntLE<quint16>();
					}
				}

				lDoneHubs.append( oSearchedHub );

				++nSearchedHubs;
			}
			else if ( strcmp( "S", szType ) == 0 && nLength >= 6 )
			{
				// hub suggestion

				CEndPoint suggestion;
				pPacket->ReadHostAddress( &suggestion, !( nLength >= 18 ) );
				const quint32 tTimeStamp = ( nLength >= (suggestion.protocol() == 0 ? 10u : 22u) ) ?
											 pPacket->ReadIntLE<quint32>() + tAdjust : tNow - 60;

				lSuggestedHubs.insert(suggestion, tTimeStamp);

				++nSuggestedHubs;
			}
			else if ( strcmp("TS", szType) == 0 && nLength >= 4 )
			{
				// UTC timestamp of the packet generation time

				// TODO: this child packet should be read before parsing the other packets
				tAdjust = tNow - pPacket->ReadIntLE<quint32>();
			}
			else if ( strcmp("RA", szType) == 0 && nLength >= 2 )
			{
				// the amount of time that needs to pass at least before retrying the sender
				// TODO: make sure the packet order of the children does not matter anymore
				//       (FR and RA packet)

				if ( nLength >= 4 )
				{
					tRetryAfter = pPacket->ReadIntLE<quint32>();
				}
				else if ( nLength >= 2 )
				{
					tRetryAfter = pPacket->ReadIntLE<quint16>();
				}

				hostCache.m_pSection.lock();
				CG2HostCacheHost* pHost = hostCache.get( oQAOrigin );
				if ( pHost )
				{
					pHost->setRetryAfter( tNow + tRetryAfter );
				}
				hostCache.m_pSection.unlock();
			}
			else if ( strcmp("FR", szType) == 0 && nLength >= 4 )
			{
				// for forwarded queries only: original sender of the packet

				if ( nLength >= 16 )
				{
					Q_IPV6ADDR ip;
					pPacket->Read( &ip, 16 );
					oQAOrigin.setAddress( ip );
				}
				else
				{
					quint32 nFromIp = pPacket->ReadIntBE<quint32>();
					oQAOrigin.setAddress( nFromIp );
				}
			}
			else if ( strcmp("V", szType) == 0 && nLength >= 4 )
			{
				sVendor = pPacket->ReadString( 4 );
			}
			else
			{
				// this handles extensions and future developments unknown to us
				systemLog.postLog( LogSeverity::Debug, Components::G2,
								   tr( "Caught unknown child of QA packet: " ) + szType );
			}

			pPacket->m_nPosition = nNext;
		}

		// we already know QA GUID

		bLikelyFoxy = (nSearchedHubs > 20 && nSuggestedHubs < 6 && tRetryAfter == 300);
		systemLog.postLog( LogSeverity::Debug, Components::G2,
						   "Processing query acknowledge from %s (time adjust %+d seconds): %d hubs, %d leaves, %d suggested hubs, retry after %d seconds, %s, likely foxy: %s).",
						   oQAOrigin.toString().toLocal8Bit().constData(), int( tAdjust ), nSearchedHubs,
						   nSearchedLeaves, nSuggestedHubs, tRetryAfter,
						   ( sVendor.isEmpty() ? "unknown" : qPrintable( sVendor ) ), (bLikelyFoxy ? "yes" : "no") );

		// Add Hubs to the cache
		if ( Q_LIKELY(!bLikelyFoxy) )
		{
			for ( QList<CEndPoint>::iterator itHub = lDoneHubs.begin(); itHub != lDoneHubs.end(); ++itHub )
			{
				if ( (*itHub).port() )
					hostCache.add(*itHub, tNow);
			}

			for ( QHash<CEndPoint, quint32>::iterator itHub = lSuggestedHubs.begin(); itHub != lSuggestedHubs.end(); ++itHub )
			{
				if ( itHub.key().port() )
					hostCache.add(itHub.key(), itHub.value());
			}
		}
		else
		{
			CEndPoint oBan = oSender;

			if ( Network.IsFirewalled() && !lDoneHubs.isEmpty() )
			{
				oBan = lDoneHubs[0]; // 1st done hub, Shareaza does not include /FR child packet
			}

			securityManager.ban( oBan, Security::ban6Hours, true,
								 tr( "[AUTO] Likely Foxy client" )
#ifdef _DEBUG
								 , QString( "searchmanager.cpp line 307" )
#endif
								 );

			// remove suggested Hubs from host cache
			for ( QList<CEndPoint>::iterator itHub = lDoneHubs.begin(); itHub != lDoneHubs.end(); ++itHub )
			{
				hostCache.remove( *itHub );
			}
		}

		pSearch->m_nHubs += nSearchedHubs;
		pSearch->m_nLeaves += nSearchedLeaves;

		const QDateTime tNowDT = common::getDateTimeUTC();
		pSearch->OnHostAcknowledge( oQAOrigin, tNowDT );

		for ( int i = 0; i < lDoneHubs.size(); ++i )
		{
			pSearch->OnHostAcknowledge( lDoneHubs[i], tNowDT );
		}

		emit pSearch->StatsUpdated();

		return false;
	}

	// not our ack - tell caller to route it
	return true;
}

bool CSearchManager::OnQueryHit(G2Packet* pPacket, QueryHitInfo* pHitInfo)
{
	QMutexLocker l(&m_pSection);

	if(CManagedSearch* pSearch = Find(pHitInfo->m_oGUID))
	{
		// our search

		CQueryHit* pHit = CQueryHit::ReadPacket(pPacket, pHitInfo);

		if(pHit)
		{
			pSearch->OnQueryHit(pHit);
		}

		return false;
	}

	return true;
}

