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
#include "network.h"

#include "neighboursbase.h"
#include "neighbour.h"

#include "debug_new.h"

NeighboursBase::NeighboursBase( QObject* parent ) :
	QObject( parent ),
	m_bActive( false )
{
}
NeighboursBase::~NeighboursBase()
{
	if ( m_bActive )
	{
		disconnectNode();
	}
}

void NeighboursBase::connectNode()
{
	ASSUME_LOCK( m_pSection );

	connect( &securityManager.m_oSanity, SIGNAL( beginSanityCheck() ),
			 this, SLOT( sanityCheck() ), Qt::UniqueConnection );

	connect( this, SIGNAL( sanityCheckPerformed() ),
			 &securityManager.m_oSanity, SLOT( sanityCheckPerformed() ), Qt::UniqueConnection );

	connect( &networkG2, &NetworkG2::localAddressChanged,
			 this,  &NeighboursBase::localAddressChanged );

	m_bActive = true;
}
void NeighboursBase::disconnectNode()
{
	ASSUME_LOCK( m_pSection );

	m_bActive = false;
}

void NeighboursBase::addNode( Neighbour* pNode )
{
	ASSUME_LOCK( m_pSection );

	m_lNodes.append( pNode );
	m_lNodesByAddr.insert( pNode->address(), pNode );
	m_lNodesByPtr.insert( pNode );

	emit neighbourAdded( pNode );
}
void NeighboursBase::removeNode( Neighbour* pNode )
{
	ASSUME_LOCK( m_pSection );

	m_lNodes.removeAll( pNode );
	m_lNodesByAddr.remove( pNode->address() );
	m_lNodesByPtr.remove( pNode );

	emit neighbourRemoved( pNode );
}

Neighbour* NeighboursBase::find( const QHostAddress& oAddress, DiscoveryProtocol::Protocol nProtocol )
{
	ASSUME_LOCK( m_pSection );

	if ( m_lNodesByAddr.contains( oAddress ) )
	{
		foreach ( Neighbour * pRet, m_lNodesByAddr.values( oAddress ) )
		{
			if ( pRet->m_nProtocol == nProtocol || nProtocol == DiscoveryProtocol::None )
			{
				return pRet;
			}
		}
	}
	return 0;
}
bool NeighboursBase::neighbourExists( const Neighbour* pNode )
{
	ASSUME_LOCK( m_pSection );

	return m_lNodesByPtr.contains( const_cast<Neighbour* const&>( pNode ) );
}

void NeighboursBase::maintain()
{
	ASSUME_LOCK( m_pSection );

	quint32 tNow = time( NULL );

	foreach ( Neighbour * pNode, m_lNodes )
	{
		pNode->onTimer( tNow );
	}
}

void NeighboursBase::sanityCheck()
{
	securityManager.m_oSanity.lockForRead();

	uint nCount = 0;

	if ( m_pSection.tryLock() ) // obtain Neighbours lock second in order to minimize lockdown time
	{
		for ( int i = 0; i < m_lNodes.size(); ++i )
		{
			if ( securityManager.m_oSanity.isNewlyDenied( m_lNodes[i]->address() ) )
			{
				++nCount;
				m_lNodes[i]->close(); // this also removes the node from Neighbours
			}
		}

		m_pSection.unlock();
	}
	else
	{
		securityManager.m_oSanity.unlock();
//		qDebug() << "[Neighbours] Didn't get Neighbours lock for sanity check. Trying again later.";
		QMetaObject::invokeMethod( this, "sanityCheck", Qt::QueuedConnection );
		return;
	}

	securityManager.m_oSanity.unlock();

	emit sanityCheckPerformed();
}

/**
 * @brief NeighboursBase::localAddressChanged Cleans up unwanted self-connections.
 */
void NeighboursBase::localAddressChanged()
{
	qDebug() << "*** localAddressChanged()";
	networkG2.m_pSection.lock();

	if ( m_pSection.tryLock() )
	{
		qDebug() << "*** localAddressChanged() - local address: "
				 << networkG2.localAddress().toStringWithPort().toLocal8Bit();
		for ( int i = 0; i < m_lNodes.size(); ++i )
		{

			qDebug() << "*** localAddressChanged() - testing: "
					 << m_lNodes[i]->address().toStringWithPort().toLocal8Bit();

			if ( m_lNodes[i]->address() == networkG2.localAddress() )
			{
				qDebug() << "*** localAddressChanged() - SELF-CONNECTION DETECTED";
				m_lNodes[i]->close(); // this also removes the node from Neighbours
			}
		}

		m_pSection.unlock();
	}
	else
	{
		// couldn't get a lock, trying again later to minimize lockdown time and risk of deadlock
		QMetaObject::invokeMethod( this, "localAddressChanged", Qt::QueuedConnection );


		qDebug() << "*** localAddressChanged() - trying again later";
	}

	networkG2.m_pSection.unlock();
}

