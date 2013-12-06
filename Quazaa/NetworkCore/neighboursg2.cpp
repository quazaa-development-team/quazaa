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

#include <QElapsedTimer>

#include "neighboursg2.h"
#include "hostcache.h"
#include "g2node.h"
#include "g2packet.h"
#include "network.h"
#include "hubhorizon.h"

#include "quazaasettings.h"
#include "quazaaglobals.h"
#include "commonfunctions.h"

#include "Discovery/discovery.h"

#include "debug_new.h"

CNeighboursG2::CNeighboursG2(QObject* parent) :
	CNeighboursConnections(parent),
	m_nNextKHL(30),
	m_nLNIWait(0),
	m_bNeedLNI(false),
	m_nUpdateWait(0),
	m_nSecsTrying(0),
	m_tLastModeChange(0),
	m_nHubBalanceWait(0),
	m_nPeriodsLow(0),
	m_nPeriodsHigh(0)
{
}
CNeighboursG2::~CNeighboursG2()
{
}

void CNeighboursG2::connectNode()
{
	if(quazaaSettings.Gnutella2.ClientMode < 2)
	{
		m_nClientMode = G2_LEAF;
	}
	else
	{
		m_nClientMode = G2_HUB;
	}

	m_nSecsTrying = m_nHubBalanceWait = m_nPeriodsLow = m_nPeriodsHigh = 0;
	m_bNeedLNI = false;
	m_nLNIWait = quazaaSettings.Gnutella2.LNIMinimumUpdate;
	m_tLastModeChange = common::getTNowUTC();

	CNeighboursConnections::connectNode();

	// Only query service if we're not already querying and we actually need fresh hosts.
	if ( !discoveryManager.isActive( Discovery::stGWC ) &&
		 ( hostCache.isEmpty() || !hostCache.hasConnectable() ) )
	{
		discoveryManager.queryService( CNetworkType( dpG2 ) );
	}

	HubHorizonPool.setup();
}

void CNeighboursG2::maintain()
{
	ASSUME_LOCK(m_pSection);

	quint32 nNodes = m_nHubsConnectedG2 + m_nLeavesConnectedG2;

	CNeighboursConnections::maintain();

	if(m_nHubsConnectedG2 + m_nLeavesConnectedG2 != nNodes)
	{
		m_bNeedLNI = true;
	}

	// Note: For speed testing... the atomicint class seems to have a lot of overhead, still...
	/*static quint64 tMutex = 0, tAtomic = 0;
	quint32 nSizeM;
	quint32 nSizeA;

	QElapsedTimer oETM;
	oETM.start();
	for ( quint32 i = 0; i < 10000000; ++i )
		nSizeM = hostCache.count();
	tMutex += oETM.elapsed();

	QElapsedTimer oETA;
	oETA.start();
	for ( quint32 i = 0; i < 10000000; ++i )
		nSizeA = hostCache.countAtomic();
	tAtomic += oETA.elapsed();

	Q_ASSERT( nSizeM == nSizeA );

	qDebug() << "  Total Mutex time: " << QString::number( tMutex  ).toLocal8Bit().data()
			 << " Total Atomic time: " << QString::number( tAtomic ).toLocal8Bit().data();*/

	// TODO: Test whether already active checking is required
	if ( !m_nHubsConnectedG2 && !discoveryManager.isActive( Discovery::stGWC )
		 && ( hostCache.isEmpty() || !hostCache.hasConnectable() ) && m_nUnknownInitiated == 0 )
	{
		qDebug() << "GWC query: Active:" << discoveryManager.isActive(Discovery::stGWC)
				 << ", empty cache:" << hostCache.isEmpty()
				 << ", has connectable:" << hostCache.hasConnectable()
				 << ", has unknown initiated:" << (m_nUnknownInitiated != 0);
		discoveryManager.queryService( CNetworkType( dpG2 ) );
	}

	if(m_nNextKHL == 0)
	{
		dispatchKHL();
		m_nNextKHL = quazaaSettings.Gnutella2.KHLPeriod;
	}
	else
	{
		m_nNextKHL--;
	}

	if(m_nLNIWait == 0)
	{
		if(m_bNeedLNI)
		{
			m_bNeedLNI = false;
			m_nLNIWait = quazaaSettings.Gnutella2.LNIMinimumUpdate;

			foreach(CNeighbour * pNode, m_lNodes)
			{
				if(pNode->m_nProtocol == dpG2 && pNode->m_nState == nsConnected)
				{
					((CG2Node*)pNode)->sendLNI();
				}
			}
		}
	}
	else
	{
		--m_nLNIWait;
	}

	if(m_nHubsConnectedG2 == 0)
	{
		m_nSecsTrying++;

		if(m_nSecsTrying / 60 > 10 && quazaaSettings.Gnutella2.ClientMode == 0)
		{
			switchG2ClientMode(G2_HUB);
			m_nSecsTrying = 0;
		}
	}
	else
	{
		m_nSecsTrying = 0;
	}

	if ( (quazaaSettings.Gnutella2.ClientMode == G2_LEAF) && (m_nClientMode != G2_LEAF) ) {
		systemLog.postLog(LogSeverity::Notice, "Switching to G2 LEAF mode (Manual switch)");
		switchG2ClientMode(G2_LEAF);
	} else if ( (quazaaSettings.Gnutella2.ClientMode == G2_HUB) && (m_nClientMode != G2_HUB) ) {
		systemLog.postLog(LogSeverity::Notice, "Switching to G2 HUB mode (Manual switch)");
		switchG2ClientMode(G2_HUB);
	}

	if(time(0) - m_tLastModeChange > quazaaSettings.Gnutella2.HubBalanceGrace)
	{
		if(m_nHubBalanceWait == 0)
		{
			hubBalancing();
			m_nHubBalanceWait = quazaaSettings.Gnutella2.HubBalancePeriod;
		}
		else
		{
			m_nHubBalanceWait--;
		}
	}

	if ( isG2Hub() && Network.getLocalAddress().isValid()
		 && !discoveryManager.isActive( Discovery::stGWC )
		 && m_nUpdateWait-- == 0 )
	{
		if ( m_nLeavesConnectedG2 < 0.7 * quazaaSettings.Gnutella2.NumLeafs ) // if we have less than 70% leaves (no reason to update GWC if we are already full of leaves)
		{
			discoveryManager.updateService(CNetworkType(dpG2));
		}
		m_nUpdateWait = quazaaSettings.Discovery.AccessThrottle * 60;
	}
}

void CNeighboursG2::dispatchKHL()
{
	ASSUME_LOCK( m_pSection );

	if( !m_nHubsConnectedG2 && !m_nLeavesConnectedG2 )
	{
		return;
	}

	G2Packet* pKHL = G2Packet::newPacket( "KHL" );

	const quint32 tNow = common::getTNowUTC();

	pKHL->writePacket( "TS", 4 )->writeIntLE<quint32>( tNow );

	foreach ( CNeighbour * pNode, m_lNodes )
	{
		if ( pNode->m_nProtocol != dpG2 )
		{
			continue;
		}

		if ( pNode->m_nState == nsConnected && ((CG2Node*)pNode)->m_nType == G2_HUB )
		{
			if ( pNode->m_oAddress.protocol() == QAbstractSocket::IPv4Protocol )
			{
				pKHL->writePacket( "NH", 6 )->writeHostAddress( pNode->m_oAddress );
			}
			else
			{
				pKHL->writePacket( "NH", 18 )->writeHostAddress( pNode->m_oAddress );
			}
		}
	}

	hostCache.m_pSection.lock();

	quint32 nCount = quazaaSettings.Gnutella2.KHLHubCount;

	for ( G2HostCacheIterator itHost = hostCache.m_lHosts.begin(); // nCount == ( nCount > 0 )
		  nCount && itHost != hostCache.m_lHosts.end(); ++itHost )  // as nCount is a quint32
	{
		if ( !(*itHost) )
		{
			continue;
		}

		if ( !(*itHost)->failures() &&
			 tNow - (*itHost)->timestamp() < quazaaSettings.Gnutella2.HostCurrent )
		{
			if ( (*itHost)->address().protocol() == QAbstractSocket::IPv4Protocol )
			{
				pKHL->writePacket( "CH", 10 )->writeHostAddress( (*itHost)->address() );
				pKHL->writeIntLE<quint32>( (*itHost)->timestamp() );
			}
			else
			{
				pKHL->writePacket( "CH", 22 )->writeHostAddress( (*itHost)->address() );
				pKHL->writeIntLE<quint32>( (*itHost)->timestamp() );
			}
			--nCount;
		}
	}

	hostCache.m_pSection.unlock();

//#if ENABLE_HOST_CACHE_BENCHMARKING
//	hostCache.m_nLockWaitTime
//	hostCache.m_nWorkTime
//#endif

	foreach ( CNeighbour * pNode, m_lNodes )
	{
		if ( pNode->m_nState == nsConnected && pNode->m_nProtocol == dpG2 )
		{
			((CG2Node*)pNode)->sendPacket( pKHL, false, false );
		}
	}

	pKHL->release();
}

bool CNeighboursG2::switchG2ClientMode(G2NodeType nRequestedMode)
{
	if(!m_bActive)
	{
		return false;
	}

	if(m_nClientMode == nRequestedMode)
	{
		return false;
	}

	m_nPeriodsLow = m_nPeriodsHigh = 0;
	m_tLastModeChange = time(0);

	foreach(CNeighbour * pNode, m_lNodes)
	{
		if(pNode->m_nProtocol == dpG2)
		{
			pNode->close();
		}
	}

	m_nClientMode = nRequestedMode;
	m_nUpdateWait = 0;

	systemLog.postLog( LogSeverity::Notice, Components::G2,
					   "Hub Balancing: Switched to %s mode.", ( isG2Hub() ? "HUB" : "LEAF" ) );

	return true;
}

bool CNeighboursG2::needMoreG2(G2NodeType nType)
{
	if(nType == G2_HUB)   // Need hubs?
	{
		if(isG2Hub())   // If we are a hub.
		{
			return (m_nHubsConnectedG2 < quazaaSettings.Gnutella2.NumPeers);
		}
		else    // If we are a leaf.
		{
			return (m_nLeavesConnectedG2 < quazaaSettings.Gnutella2.NumHubs);
		}
	}
	else // Need leaves?
	{
		if(isG2Hub())      // If we are a hub.
		{
			return (m_nLeavesConnectedG2 < quazaaSettings.Gnutella2.NumLeafs);
		}
	}

	return false;
}

void CNeighboursG2::hubBalancing()
{
	// NOT TESTED
	ASSUME_LOCK(m_pSection);

	if(m_nHubsConnectedG2 == 0)
	{
		return;
	}

	if(quazaaSettings.Gnutella2.ClientMode != 0)
	{
		return;
	}

	bool bHasQHubs = false;

	if(m_nClientMode == G2_LEAF)
	{
		// we're a leaf
		// TODO: Check capabilities

		if( Network.isFirewalled() )
			return;

		quint32 nLeaves = 0, nCapacity = 0;

		foreach(CNeighbour * pNode, m_lNodes)
		{
			if(pNode->m_nState == nsConnected && pNode->m_nProtocol == dpG2 && ((CG2Node*)pNode)->m_nType == G2_HUB)
			{
				nLeaves += ((CG2Node*)pNode)->m_nLeafCount;
				nCapacity += ((CG2Node*)pNode)->m_nLeafMax;
				bHasQHubs |= ((CG2Node*)pNode)->m_bG2Core;
			}
		}

		if( !bHasQHubs )
		{
			// Switch to G2 Hub mode if there are no other Quazaa hubs
			systemLog.postLog(LogSeverity::Notice, "Switching to G2 HUB mode (no Quazaa hubs)");
			switchG2ClientMode(G2_HUB);
			return;
		}

		if(nLeaves * 100 / nCapacity > quazaaSettings.Gnutella2.HubBalanceHigh)
		{
			m_nPeriodsHigh++;

			if(m_nPeriodsHigh >= quazaaSettings.Gnutella2.HubBalanceHighTime)
			{
				systemLog.postLog(LogSeverity::Notice, "Switching to G2 HUB mode");
				switchG2ClientMode(G2_HUB);
				return;
			}
		}
		else
		{
			m_nPeriodsHigh = m_nPeriodsLow = 0;
		}
	}
	else
	{
		// We're a hub.
		quint32 nLeaves = 0, nCapacity = 0;

		foreach(CNeighbour * pNode, m_lNodes)
		{
			if(pNode->m_nState == nsConnected && pNode->m_nProtocol == dpG2 && ((CG2Node*)pNode)->m_nType == G2_HUB)
			{
				nLeaves += ((CG2Node*)pNode)->m_nLeafCount;
				nCapacity += ((CG2Node*)pNode)->m_nLeafMax;
				bHasQHubs |= ((CG2Node*)pNode)->m_bG2Core;
			}
		}

		nLeaves += m_nLeavesConnectedG2;
		nCapacity += quazaaSettings.Gnutella2.NumLeafs;

		if(nLeaves * 100 / nCapacity < quazaaSettings.Gnutella2.HubBalanceLow && bHasQHubs) // Downgrade if there are other Quazaa hubs
		{
			m_nPeriodsLow++;

			if(m_nPeriodsLow >= quazaaSettings.Gnutella2.HubBalanceLowTime)
			{
				systemLog.postLog(LogSeverity::Notice, "Switching to G2 LEAF mode.");
				switchG2ClientMode(G2_LEAF);
				return;
			}
		}
		else
		{
			m_nPeriodsHigh = m_nPeriodsLow = 0;
		}

	}
}

G2Packet* CNeighboursG2::createQueryAck(QUuid oGUID, bool bWithHubs, CNeighbour* pExcept, bool bDone)
{
	G2Packet* pPacket = G2Packet::newPacket("QA", true);

	pPacket->writePacket("TS", 4)->writeIntLE<quint32>( common::getTNowUTC() );
	pPacket->writePacket("FR", (Network.m_oAddress.protocol() == QAbstractSocket::IPv4Protocol ? 6 : 18))->writeHostAddress(Network.m_oAddress);
	pPacket->writePacket("RA", 4)->writeIntLE<quint32>(30 + 30 * m_nHubsConnectedG2);
	pPacket->writePacket("V", 4)->writeString(CQuazaaGlobals::VENDOR_CODE(), false);

	if(bDone)
	{
		pPacket->writePacket("D", (Network.m_oAddress.protocol() == QAbstractSocket::IPv4Protocol ? 8 : 20))->writeHostAddress(Network.m_oAddress);

		if(bWithHubs)
		{
			pPacket->writeIntLE<quint16>(m_nLeavesConnectedG2);

			foreach(CNeighbour * pNode, m_lNodes)
			{
				if(pNode->m_nProtocol == dpG2 && pNode->m_nState == nsConnected && ((CG2Node*)pNode)->m_nType == G2_HUB && pNode != pExcept)
				{
					pPacket->writePacket("D", (pNode->m_oAddress.protocol() == QAbstractSocket::IPv4Protocol ? 8 : 20))->writeHostAddress(pNode->m_oAddress);
					pPacket->writeIntLE<quint16>(((CG2Node*)pNode)->m_nLeafCount);
				}
			}

			/*int nCount = */HubHorizonPool.addHorizonHubs(pPacket);

			// TODO Add hubs from HostCache
			/*if( nCount < 10 )
			{
				HostCache.m_pSection.lock();

				foreach( CHostCacheHost* pHost, HostCache.m_lHosts )
				{

				}

				HostCache.m_pSection.unlock();
			}*/
		}
		else
		{
			pPacket->writeIntLE<quint16>(0);
		}
	}

	pPacket->writeByte(0);
	pPacket->writeGUID(oGUID);

	return pPacket;
}

