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

#include "handshakes.h"
#include "network.h"
#include "handshake.h"
#include "ratecontroller.h"
#include "neighbours.h"
#include "securitymanager.h"

#include <QTimer>

#include "debug_new.h"

Handshakes handshakes;
CThread handshakesThread;

Handshakes::Handshakes( QObject* parent ) :
	QTcpServer( parent )
{
	m_nAccepted = 0;
	m_bActive = false;
}

Handshakes::~Handshakes()
{
	if ( m_bActive )
	{
		stop();
	}
}

void Handshakes::listen()
{
	QMutexLocker l( &m_pSection );

	if ( m_bActive )
	{
		return;
	}

	m_nAccepted = 0;
	m_bActive = true;

	handshakesThread.start( "Handshakes", &m_pSection, this );
}
void Handshakes::stop()
{
	m_pSection.lock();
	m_bActive = false;

	handshakesThread.exit( 0 );

	Q_ASSERT( m_lHandshakes.isEmpty() );

	m_pSection.unlock();
}

void Handshakes::incomingConnection( qintptr handle )
{
	QMutexLocker l( &m_pSection );

	Handshake* pNew = new Handshake();
	m_lHandshakes.insert( pNew );
	pNew->acceptFrom( handle );
	pNew->moveToThread( &handshakesThread );
	m_pController->addSocket( pNew );
	m_nAccepted++;

	qDebug() << pNew->address().toStringWithPort() << " - Handshakes::incomingConnection() - New incomming connection initiated.";

	if ( securityManager.isDenied( pNew->address() ) )
	{
		pNew->close();
		pNew->deleteLater();

		qDebug() << pNew->address().toStringWithPort() << " Incomming connection denied.";
	}
}

void Handshakes::onTimer()
{
	QMutexLocker l( &m_pSection );

	quint32 tNow = time( NULL );

	foreach ( Handshake * pHs, m_lHandshakes )
	{
		pHs->onTimer( tNow );
	}
}

void Handshakes::removeHandshake( Handshake* pHs )
{
	ASSUME_LOCK( handshakes.m_pSection );

	m_lHandshakes.remove( pHs );
	if ( m_pController )
	{
		m_pController->removeSocket( pHs );
	}
}

void Handshakes::processNeighbour( Handshake* pHs )
{
	removeHandshake( pHs );
	neighbours.onAccept( pHs );
}

void Handshakes::setupThread()
{
	m_pController = new RateController( &m_pSection );

	m_pController->moveToThread( &handshakesThread ); // should not be necesarry

	m_pController->setDownloadLimit( 4096 );
	m_pController->setUploadLimit( 4096 );

	bool bOK = QTcpServer::listen( QHostAddress::Any, networkG2.localAddress().port() );

	if ( bOK )
	{
		systemLog.postLog( LogSeverity::Notice, Component::G2,
						   tr( "Handshakes: listening on port %1."
							 ).arg( networkG2.localAddress().port() ) );
	}
	else
	{
		systemLog.postLog( LogSeverity::Error, Component::G2,
						   "Handshakes: cannot listen on port %d, incoming connections will be unavailable.",
						   networkG2.localAddress().port() );
	}

	m_pTimer = new QTimer( this );
	connect( m_pTimer, &QTimer::timeout, this, &Handshakes::onTimer );
	m_pTimer->start( 1000 );
}
void Handshakes::cleanupThread()
{
	if ( isListening() )
	{
		close();

		for ( QSet<Handshake*>::iterator itHs = m_lHandshakes.begin(); itHs != m_lHandshakes.end(); )
		{
			Handshake* pHs = *itHs;

			pHs->close();

			m_pController->removeSocket( pHs );
			itHs = m_lHandshakes.erase( itHs );

			delete pHs;
		}

		delete m_pController;
		delete m_pTimer;
	}
}

