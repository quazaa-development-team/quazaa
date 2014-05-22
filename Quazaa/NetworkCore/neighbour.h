/*
** neighbour.h
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

#ifndef NEIGHBOUR_H
#define NEIGHBOUR_H

#include "compressedconnection.h"
#include "types.h"

class Neighbour : public CompressedConnection
{
	Q_OBJECT

public:
	DiscoveryProtocol::Protocol m_nProtocol;

	QString         m_sHandshake;

	quint32         m_tLastPacketIn;
	quint32         m_tLastPacketOut;
	quint32         m_nPacketsIn;
	quint32         m_nPacketsOut;
	QString         m_sUserAgent;

	quint32         m_tLastPingOut;
	quint32         m_nPingsWaiting;
	QElapsedTimer   m_tRTTTimer;
	qint64          m_tRTT;
	quint32         m_tLastQuery;

	NodeState       m_nState;

	bool            m_bAutomatic;


public:
	Neighbour( QObject* parent = NULL );
	virtual ~Neighbour();

	virtual void connectTo( EndPoint oAddress )
	{
		systemLog.postLog( LogSeverity::Information, Component::Network,
						   "Initiating neighbour connection to %s...",
						   qPrintable( oAddress.toString() ) );
		m_nState = nsConnecting;
		NetworkConnection::connectTo( oAddress );
	}
	void attachTo( NetworkConnection* pOther )
	{
		qDebug() << "new state: handshaking";
		m_nState = nsHandshaking;
		CompressedConnection::attachTo( pOther );
	}

	virtual void onTimer( quint32 tNow );
	void close( bool bDelayed = false );

signals:

public slots:
	void onDisconnectNode();

	/**
	 * @brief onRead handles newly available bytes for a network connection.
	 */
	virtual void onRead() = 0;
	virtual void onError( QAbstractSocket::SocketError e );

};

#endif // NEIGHBOUR_H
