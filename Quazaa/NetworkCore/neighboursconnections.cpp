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
#include "quazaasettings.h"
#include "network.h"
#include "geoiplist.h"
#include "neighbours.h"
#include "thread.h"
#include "commonfunctions.h"
#include "securitymanager.h"

#include "debug_new.h"

CNeighboursConnections::CNeighboursConnections(QObject* parent) :
	CNeighboursRouting(parent),
	m_pController(0),
	m_nHubsConnectedG2(0),
	m_nLeavesConnectedG2(0),
	m_nUnknownInitiated(0),
	m_nUnknownIncoming(0)
{
}
CNeighboursConnections::~CNeighboursConnections()
{
}

void CNeighboursConnections::connectNode()
{
	QMutexLocker l(&m_pSection);

	Q_ASSERT(m_pController == 0);

	m_pController = new CRateController(&m_pSection);
	m_pController->setDownloadLimit(quazaaSettings.Connection.InSpeed);
	m_pController->setUploadLimit(quazaaSettings.Connection.OutSpeed);
	m_pController->moveToThread(&NetworkThread);

	m_nHubsConnectedG2 = m_nLeavesConnectedG2 = 0;

	CNeighboursRouting::connectNode();
}
void CNeighboursConnections::disconnectNode()
{
	QMutexLocker l(&m_pSection);

	while(!m_lNodes.isEmpty())
	{
		CNeighbour* pCurr = m_lNodes.takeFirst();
		delete pCurr;
	}

	delete m_pController;
	m_pController = 0;

	CNeighboursRouting::disconnectNode();
}

void CNeighboursConnections::addNode(CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	m_pController->addSocket(pNode);

	CNeighboursRouting::addNode(pNode);
}

void CNeighboursConnections::removeNode(CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	m_pController->removeSocket(pNode);

	CNeighboursRouting::removeNode(pNode);
}

CNeighbour* CNeighboursConnections::randomNode(DiscoveryProtocol nProtocol, int nType, CNeighbour* pNodeExcept)
{
	QList<CNeighbour*> lNodeList;

	for(QList<CNeighbour*>::iterator i = m_lNodes.begin(); i != m_lNodes.end(); i++)
	{
		if((*i)->m_nState == nsConnected && (*i)->m_nProtocol == nProtocol)
		{
			if(( nProtocol == dpG2 ) && ( ((CG2Node*)(*i))->m_nType == nType ) && ( (*i) != pNodeExcept ))
			{
				lNodeList.append((*i));
			}
		}
	}

	if(lNodeList.isEmpty())
	{
		return 0;
	}

	int nIndex = common::getRandomNum(0, lNodeList.size() - 1);

	return lNodeList.at(nIndex);
}

void CNeighboursConnections::disconnectYoungest(DiscoveryProtocol nProtocol, int nType, bool bCore)
{
	CNeighbour* pNode = 0;

	bool bKeepManual = true;

	time_t tNow = time(0);

	while(1)
	{
		for(QList<CNeighbour*>::const_iterator i = m_lNodes.begin(); i != m_lNodes.end(); i++)
		{
			if((*i)->m_nState == nsConnected && (*i)->m_nProtocol == nProtocol)
			{
				if( bKeepManual && !(*i)->m_bAutomatic && tNow - (*i)->m_tConnected < 120 )
					continue;

				if( nProtocol == dpG2 )
				{
					if( ((CG2Node*)(*i))->m_nType != nType // if node type is not requested type
						|| (!bCore && ((CG2Node*)(*i))->m_bG2Core) ) // or we don't want to disconnect "our" nodes
					{
						continue;
					}
				}

				if(pNode == 0)
				{
					pNode = (*i);
				}
				else
				{
					if((*i)->m_tConnected > pNode->m_tConnected)
					{
						pNode = (*i);
					}
				}
			}
		}

		if(pNode)
		{
			// we found a node to disconnect
			pNode->close();
			break;
		}
		else if(bKeepManual)
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

void CNeighboursConnections::maintain()
{
	ASSUME_LOCK(m_pSection);

	CNeighboursRouting::maintain();

	quint32 nHubsG2 = 0, nLeavesG2 = 0;
	quint32 nCoreHubsG2 = 0, nCoreLeavesG2 = 0;
	quint32 nUnknown = 0;

	m_nUnknownInitiated = m_nUnknownIncoming = 0;

	foreach(CNeighbour * pNode, m_lNodes)
	{
		if(pNode->m_nState == nsConnected)
		{
			switch(pNode->m_nProtocol)
			{
			case dpG2:
				switch(((CG2Node*)pNode)->m_nType)
				{
				case G2_UNKNOWN:
					nUnknown++;
					if( pNode->m_bInitiated )
						m_nUnknownInitiated++;
					else
						m_nUnknownIncoming++;

					break;
				case G2_HUB:
					nHubsG2++;
					if(((CG2Node*)pNode)->m_bG2Core)
					{
						nCoreHubsG2++;
					}
					break;
				case G2_LEAF:
					nLeavesG2++;
					if(((CG2Node*)pNode)->m_bG2Core)
					{
						nCoreLeavesG2++;
					}
				}
				break;
			default:
				nUnknown++;
				if( pNode->m_bInitiated )
					m_nUnknownInitiated++;
				else
					m_nUnknownIncoming++;

				break;
			}
		}
		else
		{
			nUnknown++;

			if( pNode->m_bInitiated )
				m_nUnknownInitiated++;
			else
				m_nUnknownIncoming++;
		}
	}

	m_nHubsConnectedG2 = nHubsG2;
	m_nLeavesConnectedG2 = nLeavesG2;

	if(!Neighbours.isG2Hub())
	{
		if(nHubsG2 > quazaaSettings.Gnutella2.NumHubs)
		{
			int nToDisconnect = nHubsG2 - quazaaSettings.Gnutella2.NumHubs;

			for(; nToDisconnect; nToDisconnect--)
			{
				disconnectYoungest(dpG2, G2_HUB, (100 * nCoreHubsG2 / nHubsG2) > 50);
			}
		}
		else if(nHubsG2 < quazaaSettings.Gnutella2.NumHubs)
		{
			qint32 nAttempt = qint32((quazaaSettings.Gnutella2.NumHubs - nHubsG2) * quazaaSettings.Gnutella.ConnectFactor);
			nAttempt = qMin(nAttempt, 8) - nUnknown;

			const quint32 tNow = common::getTNowUTC();
			bool bCountry = true;
			int  nCountry = 0;
			QSet<SharedG2HostPtr> oExcept;

			QMutexLocker l(&hostCache.m_pSection);

			for ( ; nAttempt > 0; --nAttempt )
			{
				// nowe polaczenie
				SharedG2HostPtr pHost;
				QString sCountry;
				sCountry = bCountry ? ( quazaaSettings.Connection.PreferredCountries.size() ?
										quazaaSettings.Connection.PreferredCountries.at(nCountry) :
										geoIP.findCountryCode(Network.m_oAddress) ) : "ZZ";
				pHost = hostCache.getConnectable( oExcept, sCountry );

				if ( pHost )
				{
					if ( !Neighbours.find( pHost->address() ) )
					{
						// Banned hosts are not added to the Host Cache and new bans are handled by
						// the sanity check mechanism.
						/*if ( securityManager.isDenied( pHost->address() ) )
						{
							hostCache.remove( pHost );
							continue;
						}*/
						connectTo( pHost->address(), dpG2 );
						pHost->setLastConnect( tNow );
					}
					else
					{
						oExcept.insert(pHost);
						nAttempt++;
					}
				}
				else
				{
					if(!bCountry)
					{
						break;
					}
					else
					{
						if(quazaaSettings.Connection.PreferredCountries.size())
						{
							nCountry++;
							if(nCountry >= quazaaSettings.Connection.PreferredCountries.size())
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
		}
	}
	else
	{
		if(nHubsG2 > quazaaSettings.Gnutella2.NumPeers)
		{
			// rozlaczyc hub
			int nToDisconnect = nHubsG2 - quazaaSettings.Gnutella2.NumPeers;

			for(; nToDisconnect; nToDisconnect--)
			{
				disconnectYoungest(dpG2, G2_HUB, (100 * nCoreHubsG2 / nHubsG2) > 50);
			}
		}
		else if(nHubsG2 < quazaaSettings.Gnutella2.NumPeers)
		{
			const quint32 tNow = common::getTNowUTC();
			qint32 nAttempt = qint32((quazaaSettings.Gnutella2.NumPeers - nHubsG2) * quazaaSettings.Gnutella.ConnectFactor);
			nAttempt = qMin(nAttempt, 8) - nUnknown;
			QSet<SharedG2HostPtr> oExcept;

			QMutexLocker l(&hostCache.m_pSection);

			for ( ; nAttempt > 0; --nAttempt )
			{
				// nowe polaczenie
				SharedG2HostPtr pHost = hostCache.getConnectable( oExcept );

				if ( pHost )
				{
					if( !Neighbours.find( pHost->address() ) )
					{
						// Banned hosts are not added to the Host Cache and new bans are handled by
						// the sanity check mechanism.
						/*if ( securityManager.isDenied( pHost->address() ) )
						{
							hostCache.remove( pHost );
							continue;
						}*/

						connectTo( pHost->address(), dpG2 );
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
		}

		if(nLeavesG2 > quazaaSettings.Gnutella2.NumLeafs)
		{
			int nToDisconnect = nLeavesG2 - quazaaSettings.Gnutella2.NumLeafs;

			for(; nToDisconnect; nToDisconnect--)
			{
				disconnectYoungest(dpG2, G2_LEAF, (100 * nCoreLeavesG2 / nLeavesG2) > 50);
			}
		}
	}
}

quint32 CNeighboursConnections::downloadSpeed()
{
	return m_pController ? m_pController->downloadSpeed() : 0;
}

quint32 CNeighboursConnections::uploadSpeed()
{
	return m_pController ? m_pController->uploadSpeed() : 0;
}

CNeighbour* CNeighboursConnections::onAccept(CNetworkConnection* pConn)
{
	// TODO: Make new CNeighbour deriviate for handshaking with Gnutella clients

#if LOG_CONNECTIONS
	systemLog.postLog(LogSeverity::Debug, "CNeighboursConnections::onAccept");
#endif

	if(!m_bActive)
	{
		pConn->close();
		return 0;
	}

	if(!m_pSection.tryLock(50))
	{
		systemLog.postLog(LogSeverity::Debug, "Not accepting incoming connection. Neighbours overloaded");
		pConn->close();
		return 0;
	}

	CG2Node* pNew = new CG2Node();
	pNew->attachTo(pConn);
	addNode(pNew);
	pNew->moveToThread(&NetworkThread);

	m_pSection.unlock();

	return pNew;
}

CNeighbour* CNeighboursConnections::connectTo(CEndPoint& oAddress, DiscoveryProtocol nProtocol, bool bAutomatic)
{
	ASSUME_LOCK(m_pSection);

	CNeighbour* pNode = 0;

	switch(nProtocol)
	{
	case dpG2:
		pNode = new CG2Node();
		break;
	default:
		Q_ASSERT_X(0, "CNeighbours::ConnectTo", "Unknown protocol");
	}

	pNode->m_bAutomatic = bAutomatic;
	pNode->connectTo(oAddress);
	pNode->moveToThread(&NetworkThread);
	addNode(pNode);
	return pNode;
}

