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

#include "neighboursconnections.h"
#include "ratecontroller.h"
#include "g2node.h"
#include "g2hostcache.h"
#include "g2hostcachehost.h"
#include "quazaasettings.h"
#include "network.h"
#include "geoiplist.h"
#include "neighbours.h"
#include "thread.h"
#include "commonfunctions.h"
#include "securitymanager.h"

#include "debug_new.h"

NeighboursConnections::NeighboursConnections( QObject* parent ) :
	NeighboursRouting( parent ),
	m_pController( 0 ),
	m_nHubsConnectedG2( 0 ),
	m_nLeavesConnectedG2( 0 ),
	m_nUnknownInitiated( 0 ),
	m_nUnknownIncoming( 0 )
{
}
NeighboursConnections::~NeighboursConnections()
{
}

void NeighboursConnections::connectNode()
{
	QMutexLocker l( &m_pSection );

	Q_ASSERT( !m_pController );

	m_pController = new RateController( &m_pSection );
	m_pController->setDownloadLimit( quazaaSettings.Connection.InSpeed );
	m_pController->setUploadLimit( quazaaSettings.Connection.OutSpeed );
	m_pController->moveToThread( &networkThread );

	m_nHubsConnectedG2 = m_nLeavesConnectedG2 = 0;

	NeighboursRouting::connectNode();
}
void NeighboursConnections::disconnectNode()
{
	QMutexLocker l( &m_pSection );

	while ( !m_lNodes.isEmpty() )
	{
		Neighbour* pCurr = m_lNodes.takeFirst();
		delete pCurr;
	}

	delete m_pController;
	m_pController = 0;

	NeighboursRouting::disconnectNode();
}

void NeighboursConnections::addNode( Neighbour* pNode )
{
	ASSUME_LOCK( m_pSection );

	m_pController->addSocket( pNode );

	NeighboursRouting::addNode( pNode );
}

void NeighboursConnections::removeNode( Neighbour* pNode )
{
	ASSUME_LOCK( m_pSection );

	m_pController->removeSocket( pNode );

	NeighboursRouting::removeNode( pNode );
}

Neighbour* NeighboursConnections::randomNode( DiscoveryProtocol::Protocol nProtocol, int nType, Neighbour* pNodeExcept )
{
	QList<Neighbour*> lNodeList;

	for ( QList<Neighbour*>::iterator i = m_lNodes.begin(); i != m_lNodes.end(); i++ )
	{
		if ( ( *i )->m_nState == nsConnected && ( *i )->m_nProtocol == nProtocol )
		{
			if ( ( nProtocol == DiscoveryProtocol::G2 ) && ( ( ( G2Node* )( *i ) )->m_nType == nType )
				 && ( ( *i ) != pNodeExcept ) )
			{
				lNodeList.append( ( *i ) );
			}
		}
	}

	if ( lNodeList.isEmpty() )
	{
		return 0;
	}

	int nIndex = common::getRandomNum( 0, lNodeList.size() - 1 );

	return lNodeList.at( nIndex );
}

void NeighboursConnections::disconnectYoungest( DiscoveryProtocol::Protocol nProtocol, int nType, bool bCore )
{
	Neighbour* pNode = 0;

	bool bKeepManual = true;

	time_t tNow = time( 0 );

	while ( true )
	{
		for ( QList<Neighbour*>::const_iterator i = m_lNodes.begin(); i != m_lNodes.end(); i++ )
		{
			if ( ( *i )->m_nState == nsConnected && ( *i )->m_nProtocol == nProtocol )
			{
				if ( bKeepManual && !( *i )->m_bAutomatic && tNow - ( *i )->connectTime() < 120 )
				{
					continue;
				}

				if ( nProtocol == DiscoveryProtocol::G2 )
				{
					if ( ( ( G2Node* )( *i ) )->m_nType != nType // if node type is not requested type
						 || ( !bCore && ( ( G2Node* )( *i ) )->m_bG2Core ) ) // or we don't want to disconnect "our" nodes
					{
						continue;
					}
				}

				if ( !pNode )
				{
					pNode = ( *i );
				}
				else
				{
					if ( ( *i )->connectTime() > pNode->connectTime() )
					{
						pNode = ( *i );
					}
				}
			}
		}

		if ( pNode )
		{
			// we found a node to disconnect
			pNode->close();
			break;
		}
		else if ( bKeepManual )
		{
			// no node to disconnect, try manually connected nodes as well
			bKeepManual = false;
		}
		else
		{
			// nothing to do here...
			break;
		}
	}
}

void NeighboursConnections::maintain()
{
	ASSUME_LOCK( m_pSection );

	NeighboursRouting::maintain();

	quint32 nHubsG2 = 0, nLeavesG2 = 0;
	quint32 nCoreHubsG2 = 0, nCoreLeavesG2 = 0;
	quint32 nUnknown = 0;

	m_nUnknownInitiated = m_nUnknownIncoming = 0;

	foreach ( Neighbour * pNode, m_lNodes )
	{
		if ( pNode->m_nState == nsConnected )
		{
			switch ( pNode->m_nProtocol )
			{
			case DiscoveryProtocol::G2:
				switch ( ( ( G2Node* )pNode )->m_nType )
				{
				case G2_UNKNOWN:
					nUnknown++;
					if ( pNode->initiatedByUs() )
					{
						m_nUnknownInitiated++;
					}
					else
					{
						m_nUnknownIncoming++;
					}

					break;
				case G2_HUB:
					nHubsG2++;
					if ( ( ( G2Node* )pNode )->m_bG2Core )
					{
						nCoreHubsG2++;
					}
					break;
				case G2_LEAF:
					nLeavesG2++;
					if ( ( ( G2Node* )pNode )->m_bG2Core )
					{
						nCoreLeavesG2++;
					}
				}
				break;
			default:
				nUnknown++;
				if ( pNode->initiatedByUs() )
				{
					m_nUnknownInitiated++;
				}
				else
				{
					m_nUnknownIncoming++;
				}

				break;
			}
		}
		else
		{
			nUnknown++;

			if ( pNode->initiatedByUs() )
			{
				m_nUnknownInitiated++;
			}
			else
			{
				m_nUnknownIncoming++;
			}
		}
	}

	m_nHubsConnectedG2 = nHubsG2;
	m_nLeavesConnectedG2 = nLeavesG2;

	if ( !neighbours.isG2Hub() )
	{
		if ( nHubsG2 > quazaaSettings.Gnutella2.NumHubs )
		{
			int nToDisconnect = nHubsG2 - quazaaSettings.Gnutella2.NumHubs;

			for ( ; nToDisconnect; --nToDisconnect )
			{
				disconnectYoungest( DiscoveryProtocol::G2, G2_HUB, ( 100 * nCoreHubsG2 / nHubsG2 ) > 50 );
			}
		}
		else if ( nHubsG2 < quazaaSettings.Gnutella2.NumHubs )
		{
			qint32 nAttempt = qint32( ( quazaaSettings.Gnutella2.NumHubs - nHubsG2 ) * quazaaSettings.Gnutella.ConnectFactor );
			nAttempt = qMin( nAttempt, 8 ) - nUnknown;

			const quint32 tNow = common::getTNowUTC();
			bool bCountry = true;
			int  nCountry = 0;
			QSet<SharedG2HostPtr> oExcept;

			hostCache.lock();
			for ( ; nAttempt > 0; --nAttempt )
			{
				// nowe polaczenie
				SharedG2HostPtr pHost;
				QString sCountry;
				sCountry = bCountry ? ( quazaaSettings.Connection.PreferredCountries.size() ?
										quazaaSettings.Connection.PreferredCountries.at( nCountry ) :
										geoIP.findCountryCode( networkG2.m_oAddress ) ) : "ZZ";
				pHost = hostCache.getConnectable( oExcept, sCountry );

				if ( pHost )
				{
					if ( !neighbours.find( pHost->address() ) )
					{
						// Banned hosts are not added to the Host Cache and new bans are handled by
						// the sanity check mechanism.
						/*if ( securityManager.isDenied( pHost->address() ) )
						{
							hostCache.remove( pHost );
							continue;
						}*/
						connectTo( pHost->address(), DiscoveryProtocol::G2 );
						pHost->setLastConnect( tNow );
					}
					else
					{
						oExcept.insert( pHost );
						nAttempt++;
					}
				}
				else
				{
					if ( !bCountry )
					{
						break;
					}
					else
					{
						if ( quazaaSettings.Connection.PreferredCountries.size() )
						{
							nCountry++;
							if ( nCountry >= quazaaSettings.Connection.PreferredCountries.size() )
							{
								bCountry = false;
							}
							nAttempt++;
							continue;
						}
						bCountry = false;
						nAttempt++;
					}
				}
			}
			hostCache.unlock();
		}
	}
	else
	{
		if ( nHubsG2 > quazaaSettings.Gnutella2.NumPeers )
		{
			// rozlaczyc hub
			int nToDisconnect = nHubsG2 - quazaaSettings.Gnutella2.NumPeers;

			for ( ; nToDisconnect; --nToDisconnect )
			{
				disconnectYoungest( DiscoveryProtocol::G2, G2_HUB, ( 100 * nCoreHubsG2 / nHubsG2 ) > 50 );
			}
		}
		else if ( nHubsG2 < quazaaSettings.Gnutella2.NumPeers )
		{
			const quint32 tNow = common::getTNowUTC();
			qint32 nAttempt = qint32( ( quazaaSettings.Gnutella2.NumPeers - nHubsG2 ) * quazaaSettings.Gnutella.ConnectFactor );
			nAttempt = qMin( nAttempt, 8 ) - nUnknown;
			QSet<SharedG2HostPtr> oExcept;

			hostCache.lock();
			for ( ; nAttempt > 0; --nAttempt )
			{
				// nowe polaczenie
				SharedG2HostPtr pHost = hostCache.getConnectable( oExcept );

				if ( pHost )
				{
					if ( !neighbours.find( pHost->address() ) )
					{
						// Banned hosts are not added to the Host Cache and new bans are handled by
						// the sanity check mechanism.
						/*if ( securityManager.isDenied( pHost->address() ) )
						{
							hostCache.remove( pHost );
							continue;
						}*/

						connectTo( pHost->address(), DiscoveryProtocol::G2 );
						pHost->setLastConnect( tNow );
					}
					else
					{
						oExcept.insert( pHost );
						++nAttempt;
					}
				}
				else
				{
					break;
				}
			}
			hostCache.unlock();
		}

		if ( nLeavesG2 > quazaaSettings.Gnutella2.NumLeafs )
		{
			int nToDisconnect = nLeavesG2 - quazaaSettings.Gnutella2.NumLeafs;

			for ( ; nToDisconnect; --nToDisconnect )
			{
				disconnectYoungest( DiscoveryProtocol::G2, G2_LEAF, ( 100 * nCoreLeavesG2 / nLeavesG2 ) > 50 );
			}
		}
	}
}

quint32 NeighboursConnections::downloadSpeed()
{
	return m_pController ? m_pController->downloadSpeed() : 0;
}

quint32 NeighboursConnections::uploadSpeed()
{
	return m_pController ? m_pController->uploadSpeed() : 0;
}

Neighbour* NeighboursConnections::onAccept( NetworkConnection* pConn )
{
	// TODO: Make new Neighbour deriviate for handshaking with Gnutella clients

#if LOG_CONNECTIONS
	systemLog.postLog( LogSeverity::Debug, "CNeighboursConnections::onAccept" );
#endif

	if ( !m_bActive )
	{
		pConn->close();
		return NULL;
	}

	if ( !m_pSection.tryLock( 50 ) )
	{
		systemLog.postLog( LogSeverity::Debug,
						   tr( "Not accepting incoming connection. Neighbours overloaded" ) );
		pConn->close();
		return NULL;
	}

	G2Node* pNew = new G2Node();
	pNew->attachTo( pConn );
	addNode( pNew );
	pNew->moveToThread( &networkThread );

	qDebug() << pNew->address().toStringWithPort() << " - NeighboursConnections::onAccept()- Transformed handshake into G2 neighbour connection";

	m_pSection.unlock();

	return pNew;
}

Neighbour* NeighboursConnections::connectTo( EndPoint oAddress, DiscoveryProtocol::Protocol nProtocol, bool bAutomatic )
{
	ASSUME_LOCK( m_pSection );

	Neighbour* pNode = 0;

	switch ( nProtocol )
	{
	case DiscoveryProtocol::G2:
		pNode = new G2Node();
		break;
	default:
		Q_ASSERT_X( 0, "CNeighbours::ConnectTo", "Unknown protocol" );
	}

	pNode->m_bAutomatic = bAutomatic;
	pNode->connectTo( oAddress );
	pNode->moveToThread( &networkThread );
	addNode( pNode );
	return pNode;
}

