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

#include "neighbour.h"
#include "neighbours.h"
#include "quazaasettings.h"
#include "g2hostcache.h"
#include <QTcpSocket>
#include "securitymanager.h"

#include "debug_new.h"

Neighbour::Neighbour( QObject* parent ) :
	CompressedConnection( parent ),
	m_nProtocol( DiscoveryProtocol::None ),
	m_tLastPacketIn( 0 ),
	m_tLastPacketOut( 0 ),
	m_nPacketsIn( 0 ),
	m_nPacketsOut( 0 ),
	m_tLastPingOut( 0 ),
	m_nPingsWaiting( 0 ),
	m_tRTT( 0 ),
	m_tLastQuery( 0 ),
	m_nState( nsClosed ),
	m_bAutomatic( true )
{
}

Neighbour::~Neighbour()
{
	ASSUME_LOCK( neighbours.m_pSection );
	neighbours.removeNode( this );
}

void Neighbour::onTimer( quint32 tNow )
{
	if ( m_nState < nsConnected )
	{
		if ( tNow - m_tConnected > quazaaSettings.Connection.TimeoutConnect )
		{
			if ( m_bInitiated ) // we initiatied the connection
			{
				hostCache.onFailure( m_oAddress );
			}

			if ( m_nState == nsConnecting )
				systemLog.postLog( LogSeverity::Information,  Component::Network,
								   qPrintable( tr( "Timed out connecting to %s." ) ),
								   qPrintable( m_oAddress.toStringWithPort() ) );
			else
				systemLog.postLog( LogSeverity::Information,  Component::Network,
								   qPrintable( tr( "Timed out handshaking with %s." ) ),
								   qPrintable( m_oAddress.toStringWithPort() ) );

			close();
			return;
		}
	}
	else if ( m_nState == nsConnected )
	{
		if ( tNow - m_tLastPacketIn > quazaaSettings.Connection.TimeoutTraffic )
		{
			systemLog.postLog( LogSeverity::Error,
							   tr( "Closing connection to %1 due to lack of traffic." ).arg( m_oAddress.toString() ) );
			systemLog.postLog( LogSeverity::Debug,
							   QString( "Conn %1, Packet %2, bytes avail %3, net bytes avail %4, ping %5" ).arg( tNow - m_tConnected ).arg(
								   tNow - m_tLastPacketIn ).arg( bytesAvailable() ).arg( networkBytesAvailable() ).arg( tNow - m_tLastPingOut ) );
			close();
			return;
		}

		if ( m_nPingsWaiting > 0 && tNow - m_tLastPingOut > quazaaSettings.Gnutella2.PingTimeout
			 && tNow - m_tLastPacketIn > quazaaSettings.Connection.TimeoutTraffic )
		{
			systemLog.postLog( LogSeverity::Debug,
							   QString( "Closing connection with %1 ping timed out" ).arg( m_oAddress.toString() ) );
			close();
			return;
		}
	}
}

void Neighbour::close( bool bDelayed )
{
	m_nState = nsClosing;
	CompressedConnection::close( bDelayed );
}

void Neighbour::onDisconnectNode()
{
	neighbours.m_pSection.lock();
	delete this;
	neighbours.m_pSection.unlock();
}

void Neighbour::onError( QAbstractSocket::SocketError e )
{
	if ( e == QAbstractSocket::RemoteHostClosedError )
	{
		if ( m_nState == nsHandshaking )
		{
			systemLog.postLog( LogSeverity::Information, Component::Network,
							   QString( "Neighbour %1 dropped connection during handshake."
										).arg( m_oAddress.toString() ) );

			if ( m_bInitiated )
			{
				// for some bad clients that drop connections too early
				securityManager.ban( m_oAddress, Security::RuleTime::FiveMinutes, true,
									 QString( "[AUTO] Dropped handshake. User Agent: " )
									 + ( m_sUserAgent.isEmpty() ? "unknown" : m_sUserAgent )
									 + " Country: " + m_oAddress.countryName(), true
#if SECURITY_LOG_BAN_SOURCES
									 , QString( "neighbour.cpp line 133" )
#endif // SECURITY_LOG_BAN_SOURCES
									 );
			}
		}
		else
		{
			systemLog.postLog( LogSeverity::Information, Component::Network,
							   QString( "Neighbour %1 dropped connection unexpectedly."
										).arg( m_oAddress.toString() ) );
		}
	}
	else
	{
		systemLog.postLog( LogSeverity::Error, Component::Network,
						   "Neighbour %s dropped connection unexpectedly (socket error: %s).",
						   qPrintable( m_oAddress.toStringWithPort() ),
						   qPrintable( errorString() ) );

		if ( m_bInitiated )
		{
			hostCache.onFailure( m_oAddress );
		}
	}

	neighbours.m_pSection.lock();
	delete this;
	neighbours.m_pSection.unlock();
}

