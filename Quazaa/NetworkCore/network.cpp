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

#include "network.h"

#include "thread.h"
#include "g2packet.h"
#include "datagrams.h"
#include <QTimer>
#include "g2node.h"
#include "handshakes.h"
#include "neighbours.h"

#include "quazaasettings.h"

#include "queryhashmaster.h"
#include "searchmanager.h"
#include "sharemanager.h"

#include "geoiplist.h"

#include "debug_new.h"

NetworkG2 networkG2;
CThread networkThread;

NetworkG2::NetworkG2( QObject* parent ) :
	QObject( parent )
{
	m_oAddress.setPort( quazaaSettings.Connection.Port );

	m_pSecondTimer     = 0;
	m_tCleanRoutesNext = 60;
	m_bSharesReady     = false;
}
NetworkG2::~NetworkG2()
{
	stop();
}

/**
 * @brief start Starts the G2 network thread and connects to the network if the network has not
 * already been started.
 */
void NetworkG2::start()
{
	QMutexLocker l( &m_pSection );

	if ( m_bActive )
	{
		systemLog.postLog( LogSeverity::Debug, "Network already started" );
		return;
	}

	m_bActive = true;
	m_oAddress.setPort( quazaaSettings.Connection.Port );

	handshakes.listen();

	m_oRoutingTable.clear();

	connect( &shareManager, &ShareManager::sharesReady,
			 this, &NetworkG2::onSharesReady, Qt::UniqueConnection );

	networkThread.start( "Network", &m_pSection, this );

	datagrams.moveToThread( &networkThread );
	searchManager.moveToThread( &networkThread );
	neighbours.moveToThread( &networkThread );
	neighbours.connectNode();
}

/**
 * @brief NetworkG2::stop Disconnects from the G2 network and stops the network thread if it is
 * currently active.
 */
void NetworkG2::stop()
{
	m_pSection.lock();
	if ( m_bActive )
	{
		m_bActive = false;
		networkThread.exit( 0 );
	}
	m_pSection.unlock();
}
void NetworkG2::setupThread()
{
	Q_ASSERT( m_pSecondTimer == 0 );

	m_pSecondTimer = new QTimer();
	connect( m_pSecondTimer, SIGNAL( timeout() ), this, SLOT( onSecondTimer() ) );
	m_pSecondTimer->start( 1000 );

	datagrams.listen();
	handshakes.listen();

	m_bSharesReady = shareManager.sharesAreReady();
}
void NetworkG2::cleanupThread()
{
	qDebug() << "Starting Cleanup...";

	m_pSecondTimer->stop();
	delete m_pSecondTimer;
	m_pSecondTimer = 0;

	qDebug() << "Shutting down Handshakes...";
	handshakes.stop();
	qDebug() << "Shutting down Datagrams...";
	datagrams.disconnectNode();
	qDebug() << "Shutting down Neighbours...";
	neighbours.disconnectNode();

	moveToThread( qApp->thread() );

	qDebug() << "Cleanup complete.";
}

void NetworkG2::onSecondTimer()
{
	if ( !m_pSection.tryLock( 150 ) )
	{
		systemLog.postLog( LogSeverity::Warning, tr( "WARNING: Network core overloaded!" ) );
		return;
	}

	if ( !m_bActive )
	{
		m_pSection.unlock();
		return;
	}

	if ( m_tCleanRoutesNext > 0 )
	{
		--m_tCleanRoutesNext;
	}
	else
	{
		//m_oRoutingTable.Dump();
		m_oRoutingTable.expireOldRoutes();
		m_tCleanRoutesNext = 60;
	}

	if ( !queryHashMaster.isValid() )
	{
		queryHashMaster.build();
	}

	neighbours.maintain();

	searchManager.onTimer();

	m_pSection.unlock();

	emit datagrams.sendQueueUpdated();
}

bool NetworkG2::isListening() const
{
	return handshakes.isListening() && datagrams.isListening();
}

bool NetworkG2::isFirewalled() const
{
	return datagrams.isFirewalled() || handshakes.isFirewalled();
}

bool NetworkG2::acquireLocalAddress( const QString& sHeader )
{
	EndPoint hostAddr( sHeader, m_oAddress.port() );

	if ( hostAddr.isValid() )
	{
		if ( hostAddr != m_oAddress )
		{
			m_oAddress = hostAddr;
			qDebug() << "*** localAddressChanged emitted";
			emit localAddressChanged();
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool NetworkG2::isConnectedTo(const EndPoint& /*addr*/ ) const
{
	return false;
}

bool NetworkG2::routePacket(const QUuid& pTargetGUID, G2Packet* pPacket, bool bLockNeighbours, bool bBuffered )
{
	G2Node* pNode = 0;
	EndPoint pAddr;

	if ( m_oRoutingTable.find( pTargetGUID, &pNode, &pAddr ) )
	{
		if ( pNode )
		{
			if ( bLockNeighbours )
			{
				neighbours.m_pSection.lock();
			}

			if ( neighbours.neighbourExists( pNode ) )
			{
				pNode->sendPacket( pPacket, bBuffered, false );
#if LOG_ROUTED_PACKETS
				systemLog.postLog( LogSeverity::Debug,
								   QString( "CNetwork::RoutePacket %1 Packet: %2 routed to neighbour: %3" ).arg( pTargetGUID.toString() ).arg(
									   pPacket->GetType() ).arg( pNode->m_oAddress.toString().toLocal8Bit().constData() ) );
#endif
			}

			if ( bLockNeighbours )
			{
				neighbours.m_pSection.unlock();
			}
			return true;
		}
		else if ( pAddr.isValid() )
		{
			datagrams.sendPacket( pPacket, pAddr, true );

#if LOG_ROUTED_PACKETS
			systemLog.postLog( LogSeverity::Debug,
							   QString( "CNetwork::RoutePacket %1 Packet: %2 routed to remote node: %3" ).arg( pTargetGUID.toString() ).arg(
								   pPacket->GetType() ).arg( pAddr.toString().toLocal8Bit().constData() ) );
#endif
			return true;
		}

#if LOG_ROUTED_PACKETS
		systemLog.postLog( LogSeverity::Debug, QString( "CNetwork::RoutePacket - No node and no address!" ) );
#endif
	}

#if LOG_ROUTED_PACKETS
	systemLog.postLog( LogSeverity::Debug,
					   QString( "CNetwork::RoutePacket %1 Packet: %2 DROPPED!" ).arg( pTargetGUID.toString() ).arg( pPacket->GetType() ) );
#endif
	return false;
}
bool NetworkG2::routePacket( G2Packet* pPacket, G2Node* pNbr )
{
	QUuid pGUID;

	if ( pPacket->getTo( pGUID ) && pGUID != quazaaSettings.Profile.GUID ) // No and address != my address
	{
		G2Node* pNode = 0;
		EndPoint pAddr;

		if ( m_oRoutingTable.find( pGUID, &pNode, &pAddr ) )
		{
			bool bForwardTCP = false;
			bool bForwardUDP = false;

			if ( pNbr )
			{
				if ( pNbr->m_nType == G2_LEAF ) // if received from leaf - can forward anywhere
				{
					bForwardTCP = bForwardUDP = true;
				}
				else    // if received from a hub - can be forwarded to leaf
				{
					if ( pNode && pNode->m_nType == G2_LEAF )
					{
						bForwardTCP = true;
					}
				}
			}
			else    // received from udp - do not forward via udp
			{
				bForwardTCP = true;
			}

			if ( pNode && bForwardTCP )
			{
				pNode->sendPacket( pPacket, true, false );
				return true;
			}
			else if ( pAddr.isValid() && bForwardUDP )
			{
				datagrams.sendPacket( pPacket, pAddr, true );
				return true;
			}
			// drop
		}
		return true;
	}
	return false;
}

void NetworkG2::connectToNode( const EndPoint& /*addr*/ )
{
	// TODO: Verify network is connected before attempting connection and create connection if it is not
	/*CG2Node* pNew = Neighbours.ConnectTo(addr);

	if(pNew)
	{
		pNew->moveToThread(&NetworkThread);
	}*/
}

void NetworkG2::onSharesReady()
{
	m_bSharesReady = true;
}

