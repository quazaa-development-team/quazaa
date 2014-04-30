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

#include "chatsession.h"
#include "chatcore.h"
#include "quazaasettings.h"
#include "widgetprivatemessage.h"
#include "winmain.h"

#ifdef _DEBUG
#include "debug_new.h"
#endif

ChatSession::ChatSession( QObject* parent ) :
	NetworkConnection( parent )
{
	m_nState = csNull;
	m_nProtocol = DiscoveryProtocol::None;
	m_pWidget = 0;

	m_bShareaza = false;
}
ChatSession::~ChatSession()
{
	chatCore.remove( this );
}

// called from GUI thread
void ChatSession::connectNode()
{
	mainWindow->OpenChat( this );
	m_nState = csConnecting;
	chatCore.add( this );

	if ( thread() != &chatThread )
	{
		moveToThread( &chatThread );
	}
}

void ChatSession::onTimer( quint32 tNow )
{
	if ( m_nState < csConnected )
	{
		if ( tNow - m_tConnected > quazaaSettings.Connection.TimeoutConnect )
		{
			m_nState = csClosed;
			NetworkConnection::close();
			emit systemMessage( "Timed out connecting to remote host" );
		}
	}
	else if ( m_nState == csConnected )
	{

	}
}

void ChatSession::onConnectNode()
{

}
void ChatSession::onDisconnectNode()
{
	m_nState = csClosed;
	emit systemMessage( "Connection lost" );
}
void ChatSession::onRead()
{

}
void ChatSession::onError( QAbstractSocket::SocketError e )
{
	Q_UNUSED( e );
}

void ChatSession::onStateChange( QAbstractSocket::SocketState s )
{
	Q_UNUSED( s );
}

void ChatSession::setupWidget( CWidgetPrivateMessage* pWg )
{
	m_pWidget = pWg;

	connect( this, SIGNAL( guidChanged( QUuid ) ), m_pWidget, SLOT( OnGUIDChanged( QUuid ) ) );
	connect( this, SIGNAL( nickChanged( QString ) ), m_pWidget, SLOT( OnNickChanged( QString ) ) );
	connect( this, SIGNAL( incomingMessage( QString, bool ) ), m_pWidget, SLOT( OnIncomingMessage( QString, bool ) ) );
	connect( this, SIGNAL( systemMessage( QString ) ), m_pWidget, SLOT( OnSystemMessage( QString ) ) );
	connect( m_pWidget, SIGNAL( SendMessageS( QString, bool ) ), this, SLOT( sendMessage( QString, bool ) ) );
	connect( m_pWidget, SIGNAL( SendMessageS( QTextDocument*, bool ) ), this, SLOT( sendMessage( QTextDocument*, bool ) ) );
	connect( m_pWidget, SIGNAL( destroyed() ), this, SLOT( deleteLater() ) );
}

