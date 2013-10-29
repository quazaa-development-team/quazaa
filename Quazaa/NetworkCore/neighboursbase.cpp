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

#include "neighboursbase.h"
#include "neighbour.h"
#include "debug_new.h"

CNeighboursBase::CNeighboursBase(QObject* parent) :
	QObject(parent),
	m_bActive(false)
{
}
CNeighboursBase::~CNeighboursBase()
{
	if(m_bActive)
	{
		Disconnect();
	}
}

void CNeighboursBase::Connect()
{
	ASSUME_LOCK(m_pSection);

	m_bActive = true;
}
void CNeighboursBase::Disconnect()
{
	ASSUME_LOCK(m_pSection);

	m_bActive = false;
}

void CNeighboursBase::AddNode(CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	m_lNodes.append(pNode);
	m_lNodesByAddr.insert(pNode->m_oAddress, pNode);
	m_lNodesByPtr.insert(pNode);

	emit NeighbourAdded(pNode);
}
void CNeighboursBase::RemoveNode(CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	m_lNodes.removeAll(pNode);
	m_lNodesByAddr.remove(pNode->m_oAddress);
	m_lNodesByPtr.remove(pNode);

	emit NeighbourRemoved(pNode);
}

CNeighbour* CNeighboursBase::Find(const QHostAddress& oAddress, DiscoveryProtocol nProtocol)
{
	ASSUME_LOCK(m_pSection);

	if(m_lNodesByAddr.contains(oAddress))
	{
		foreach(CNeighbour * pRet, m_lNodesByAddr.values(oAddress))
		{
			if(pRet->m_nProtocol == nProtocol || nProtocol == dpNull)
			{
				return pRet;
			}
		}
	}
	return 0;
}
bool CNeighboursBase::NeighbourExists(const CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	return m_lNodesByPtr.contains(const_cast<CNeighbour * const&>(pNode));
}

void CNeighboursBase::Maintain()
{
	ASSUME_LOCK(m_pSection);

	quint32 tNow = time(0);

	foreach(CNeighbour * pNode, m_lNodes)
	{
		pNode->OnTimer(tNow);
	}
}

