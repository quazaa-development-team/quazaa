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

#include "handshake.h"
#include "handshakes.h"
#include "network.h"
#include "neighbours.h"
#include "neighbour.h"
#include "g2node.h"

#include <QTcpSocket>
#include <QFile>

#include "quazaaglobals.h"

#include "debug_new.h"

Handshake::Handshake( QObject* parent )
	: NetworkConnection( parent )
{
}
Handshake::~Handshake()
{
	ASSUME_LOCK( handshakes.m_pSection );

	handshakes.removeHandshake( this );
}

void Handshake::onTimer( quint32 tNow )
{
	if ( tNow - m_tConnected > 15 )
	{
		systemLog.postLog( LogSeverity::Debug, Component::Network,
						   tr( "Timed out handshaking with  %1"
							   ).arg( address().toString() ) );
		close();
	}
}
void Handshake::onRead()
{
	QMutexLocker l( &handshakes.m_pSection );

	//qDebug() << "CHandshake::OnRead()";

	if ( bytesAvailable() < 8 )
	{
		return;
	}

	if ( peek( 8 ).startsWith( "GNUTELLA" ) )
	{
#if LOG_CONNECTIONS
		systemLog.postLog( LogSeverity::Debug, Component::Network,
						   QString( "Incoming connection from %1 is Gnutella Neighbour connection"
								  ).arg( m_pSocket->peerAddress().toString() ) );
#endif
		qDebug() << m_oAddress.toStringWithPort() << " - Handshake::onRead() - Recieved messige starts with \"GNUTELLA\"";
		handshakes.processNeighbour( this );
		delete this;
	}
	else if ( peek( 5 ).startsWith( "GET /" ) )
	{
		if ( peek( bytesAvailable() ).indexOf( "\r\n\r\n" ) != -1 )
		{
#if LOG_CONNECTIONS
			systemLog.postLog( LogSeverity::Debug, Component::Network,
							   QString( "Incoming connection from %1 is a Web request"
									  ).arg( m_pSocket->peerAddress().toString() ) );
#endif
			onWebRequest();
		}
	}
	else
	{
		systemLog.postLog( LogSeverity::Debug, Component::Network,
						   QString( "Closing connection with %1 - unknown protocol"
								  ).arg( address().toString() ) );

		QByteArray baResp;
		baResp += "HTTP/1.1 501 Not Implemented\r\n";
		baResp += "Server: " + QuazaaGlobals::USER_AGENT_STRING() + "\r\n";
		baResp += "\r\n";

		write( baResp );
		close( true );
	}
}

void Handshake::onConnectNode()
{

}
void Handshake::onDisconnectNode()
{
	handshakes.m_pSection.lock();
	delete this;
	handshakes.m_pSection.unlock();
}
void Handshake::onError( QAbstractSocket::SocketError e )
{
	Q_UNUSED( e );
	handshakes.m_pSection.lock();
	delete this;
	handshakes.m_pSection.unlock();
}

void Handshake::onStateChange( QAbstractSocket::SocketState s )
{
	Q_UNUSED( s );
}

void Handshake::onWebRequest()
{
	QString sRequest;

	sRequest = read( bytesAvailable() );
	sRequest = sRequest.left( sRequest.indexOf( "\r\n\r\n" ) );

	QStringList arrLines = sRequest.split( "\r\n" );

	if ( arrLines[0].startsWith( "GET / HTTP" ) )
	{
		QByteArray baResp;

		baResp += "HTTP/1.1 200 OK\r\n";
		baResp += "Server: " + QuazaaGlobals::USER_AGENT_STRING() + "\r\n";
		baResp += "Connection: close\r\n";
		baResp += "Content-Type: text/html; charset=utf-8\r\n";


		QByteArray baHtml;
		baHtml += "<!DOCTYPE html><html><head><title>Quazaa " + QuazaaGlobals::APPLICATION_VERSION_STRING() +
				  " : Node Information</title>";
		baHtml += "<link href=\"/res/favicon.ico\" rel=\"shortcut icon\" type=\"image/vnd.microsoft.icon\" />";
		baHtml += "<meta name=\"robots\" content=\"noindex,nofollow\"></head>";
		baHtml += "<body style=\"color:white;background-color:rgb(12,26,43);padding-left:12px;padding-right:12px;\">";
		baHtml += "<div style=\"box-shadow:0px 0px 8px rgb(189,189,189);border-radius:6px;margin-top:28px;margin-bottom:15px;margin-left:15px;margin-right:15px;\">";
		baHtml += "<div style=\"height:130px;border:1px solid rgb(197,197,197);border-bottom:0px;border-radius:6px 6px 0px 0px;background:url('/res/header_background.png') no-repeat scroll left top rgb(0,102,152);\">";
		baHtml += "<a href=\"http://quazaa.sf.net\"><img src=\"/res/QuazaaLogo.png\"></a></div>";
		baHtml += "<div style=\"color:rgb(51,51,51);background-color:white;border:1px solid rgb(197,197,197);border-top:0px;border-radius:0px 0px 6px 6px;padding:14px 20px 12px;\">";
		baHtml += "This node is powered by <a href=\"http://quazaa.sf.net\">Quazaa " +
				  QuazaaGlobals::APPLICATION_VERSION_STRING() + "</a>.</div></div>";
		baHtml += "<div style=\"border:1px solid rgb(197,197,197);box-shadow:0px 0px 8px rgb(189,189,189);border-radius:6px;margin-bottom:15px;margin-left:15px;margin-right:15px;\">";
		baHtml += "<div style=\"text-align:center;color:white;background-color:rgb(34,34,34);border-radius:6px 6px 0px 0px;padding:14px 20px 12px;\">";
		baHtml += "What is <a href=\"http://quazaa.sf.net\">Quazaa</a>?</div>";
		baHtml += "<div style=\"color:rgb(51,51,51);background-color:white;border:1px solid rgb(197,197,197);border-top:0px;border-radius:0px 0px 6px 6px;padding:14px 20px 12px;\">";
		baHtml += "Quazaa is a cross-platform multi-network peer-to-peer (P2P) file sharing client which will support G2, Ares, eDonkey2000 (eMule), HTTP, FTP and BitTorrent. Quazaa is free, <a href=\"http://opensource.org/\">open source</a> software and contains no spyware or adware.</div></div>";
		baHtml += "<div style=\"text-align:center;color:rgb(51,51,51);background-color:white;box-shadow:0px 0px 8px rgb(189,189,189);border-radius:6px;padding:14px 20px 12px;margin-top:15px;margin-left:15px;margin-right:15px;\">";
		baHtml += "This node is currently connected to the following nodes:<table width=\"100%\" cellspacing=\"0\">";
		baHtml += "<b><tr><th>Address</th><th>Time</th><th>Mode</th><th>Leaves</th><th>Client</th></tr></b>";

		neighbours.m_pSection.lock();

		quint32 tNow = time( 0 );

		for ( QList<Neighbour*>::iterator it = neighbours.begin(); it != neighbours.end(); it++ )
		{
			if ( ( *it )->m_nState != nsConnected )
			{
				continue;
			}

			baHtml += "<tr><td style=\"text-align:center;\"><a href=\"http://" + ( *it )->address().toStringWithPort() + "\">" +
					  ( *it )->address().toStringWithPort() + "</a></td>";

			quint32 tConnected = tNow - ( *it )->connectTime();
			baHtml += "<td style=\"text-align:center;\">" + QString().sprintf( "%.2u:%.2u:%.2u", tConnected / 3600,
																			   tConnected % 3600 / 60, ( tConnected % 3600 ) % 60 ) + "</td>";

			if ( ( *it )->m_nProtocol == DiscoveryProtocol::G2 )
			{
				G2Node* pG2 = static_cast<G2Node*>( *it );

				baHtml += "<td style=\"text-align:center;\">" + QString( ( pG2->m_nType == G2_HUB ? "G2 Hub" : "G2 Leaf" ) ) + "</td>";

				baHtml += "<td style=\"text-align:center;\">";
				if ( pG2->m_nType == G2_HUB )
				{
					baHtml += QString::number( pG2->m_nLeafCount ) + "/" + QString::number( pG2->m_nLeafMax );
				}
				else
				{
					baHtml += "&nbsp;";
				}
				baHtml += "</td>";
			}
			else
			{
				baHtml += "<td style=\"text-align:center;\">&nbsp;</td><td style=\"text-align:center;\">&nbsp;</td>";
			}
			baHtml += "<td style=\"text-align:center;\">" + ( *it )->m_sUserAgent + "</td>";
			baHtml += "</tr>";
		}

		neighbours.m_pSection.unlock();

		baHtml += "</table></div></body></html>";

		baResp += "Content-length: " + QString( baHtml.length() ) + "\r\n";
		baResp += "\r\n";

		write( baResp );
		write( baHtml );
	}
	else
	{
		QByteArray baResp;

		QString sPath = arrLines[0].mid( 4, arrLines[0].length() - 12 ).trimmed();

		bool bFound = false;

		if ( sPath.startsWith( "/res" ) ) // redirect /res prefixed URIs to the resource system, anything not prefixed will go to uploads
		{
			sPath = ":/Resource/Web" + sPath.mid( 4 );

			QFile f( sPath );

			if ( f.exists() )
			{
				if ( f.open( QIODevice::ReadOnly ) )
				{
					bFound = true;

					baResp += "HTTP/1.1 200 OK\r\n";
					baResp += "Server: " + QuazaaGlobals::USER_AGENT_STRING() + "\r\n";
					baResp += "Connection: close\r\n";
					baResp += "Content-Length: " + QString::number( f.size() ) + "\r\n";
					baResp += "\r\n";

					baResp += f.readAll();
					f.close();

					write( baResp );
				}
			}
		}

		// TODO: Handle P2P uploads here

		if ( !bFound )
		{
			baResp += "HTTP/1.1 404 Not found\r\n";
			baResp += "Server: " + QuazaaGlobals::USER_AGENT_STRING() + "\r\n";
			baResp += "Connection: close\r\n";
			baResp += "Content-Type: text/plain\r\n";
			baResp += "\r\n";
			baResp += "The requested file was not found on this server.\r\n";
#ifdef _DEBUG
			baResp += sPath;
#endif
			write( baResp );
		}
	}
	close( true );

}
