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

#include "neighboursrouting.h"
#include "Hashes/hash.h"
#include "query.h"
#include "neighbour.h"
#include "g2node.h"
#include "g2packet.h"
#include "queryhashtable.h"

#include "debug_new.h"

CNeighboursRouting::CNeighboursRouting(QObject* parent) :
	CNeighboursBase(parent)
{
}
CNeighboursRouting::~CNeighboursRouting()
{
}

void CNeighboursRouting::routeQuery(CQueryPtr pQuery, G2Packet *pPacket, CNeighbour* pFrom, bool bToHubs)
{
	quint32 tNow = time(0);
	quint32 nCount = 0, nHubs = 0, nLeaves = 0;

	foreach(CNeighbour* pNode, m_lNodes)
	{
		if( pNode != pFrom && pNode->m_nState == nsConnected && pNode->m_nProtocol == dpG2 && tNow - pNode->m_tConnected > 30 )
		{
			CG2Node* pG2 = static_cast<CG2Node*>(pNode);

			if( !bToHubs && pG2->m_nType == G2_HUB )
			{
				continue;
			}

			if( pG2->m_pRemoteTable != 0 && pG2->m_pRemoteTable->m_bLive )
			{
				if( !pG2->m_pRemoteTable->checkQuery(pQuery) )
				{
					continue;
				}
			}
			else if( pG2->m_nType == G2_LEAF )
			{
				continue;
			}

			pG2->sendPacket(pPacket, true, false);
			nCount++;

			if( pG2->m_nType == G2_HUB )
				nHubs++;
			else
				nLeaves++;
		}
	}

#if LOG_QUERY_FORWARDING
	qDebug() << "G2 Query forwarded to " << nCount << "nodes (hubs:" << nHubs << "leaves:" << nLeaves << ")";
#endif
}

