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

#include "Security/securitymanager.h"

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
		disconnectNode();
	}
}

void CNeighboursBase::connectNode()
{
	ASSUME_LOCK(m_pSection);

	connect( &securityManager.m_oSanity, SIGNAL( beginSanityCheck() ),
			 this, SLOT( sanityCheck() ), Qt::UniqueConnection );

	connect( this, SIGNAL( sanityCheckPerformed() ),
			 &securityManager.m_oSanity, SLOT( sanityCheckPerformed() ), Qt::UniqueConnection );

	m_bActive = true;
}
void CNeighboursBase::disconnectNode()
{
	ASSUME_LOCK(m_pSection);

	m_bActive = false;
}

void CNeighboursBase::addNode(CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	m_lNodes.append(pNode);
	m_lNodesByAddr.insert(pNode->m_oAddress, pNode);
	m_lNodesByPtr.insert(pNode);

	emit neighbourAdded(pNode);
}
void CNeighboursBase::removeNode(CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	m_lNodes.removeAll(pNode);
	m_lNodesByAddr.remove(pNode->m_oAddress);
	m_lNodesByPtr.remove(pNode);

	emit neighbourRemoved(pNode);
}

CNeighbour* CNeighboursBase::find(const QHostAddress& oAddress, DiscoveryProtocol::Protocol nProtocol)
{
	ASSUME_LOCK(m_pSection);

	if(m_lNodesByAddr.contains(oAddress))
	{
		foreach(CNeighbour * pRet, m_lNodesByAddr.values(oAddress))
		{
			if(pRet->m_nProtocol == nProtocol || nProtocol == DiscoveryProtocol::None)
			{
				return pRet;
			}
		}
	}
	return 0;
}
bool CNeighboursBase::neighbourExists(const CNeighbour* pNode)
{
	ASSUME_LOCK(m_pSection);

	return m_lNodesByPtr.contains(const_cast<CNeighbour * const&>(pNode));
}

void CNeighboursBase::maintain()
{
	ASSUME_LOCK(m_pSection);

	quint32 tNow = time(0);

	foreach(CNeighbour * pNode, m_lNodes)
	{
		pNode->onTimer(tNow);
	}
}

void CNeighboursBase::sanityCheck()
{
	//qDebug() << "[Neighbours] Started sanity checking.";
	securityManager.m_oSanity.lockForRead();
	//qDebug() << "Got sanity lock. Waiting for Neighbours.";

	uint nCount = 0;

	if ( m_pSection.tryLock() ) // obtain Neighbours lock second in order to minimize lockdown time
	{
		//qDebug() << "Neighbours lock.";
		for ( int i = 0; i < m_lNodes.size(); ++i )
		{
			if ( securityManager.m_oSanity.isNewlyDenied( m_lNodes[i]->address() ) )
			{
				++nCount;
				m_lNodes[i]->close(); // this also removes the node from Neighbours
			}
		}

		//qDebug() << "Neighbours unlock.";
		m_pSection.unlock();
	}
	else
	{
		securityManager.m_oSanity.unlock();
		qDebug() << "[Neighbours] Didn't get Neighbours lock. Trying again later.";
		QMetaObject::invokeMethod( this, "sanityCheck", Qt::QueuedConnection );
		return;
	}

	//qDebug() << "Sanity unlock.";
	securityManager.m_oSanity.unlock();

	emit sanityCheckPerformed();

	qDebug() << QString( "[Neighbours] Finished sanity checking. %1 host(s) removed."
						 ).arg( nCount ).toLocal8Bit().data();
}

