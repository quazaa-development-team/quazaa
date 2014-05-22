/*
** g2node.h
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

#ifndef G2NODE_H
#define G2NODE_H

#include "neighbour.h"
#include <QElapsedTimer>
#include <QQueue>
#include <QHash>

class G2Packet;
class QueryHashTable;
class HubHorizonGroup;

/**
 * @brief The CG2Node class represents a G2 node connected to us.
 */
class G2Node : public Neighbour
{
	Q_OBJECT

public:
	bool            m_bG2Core;
	bool            m_bCachedKeys;
	G2NodeType      m_nType;

	quint16         m_nLeafCount;
	quint16         m_nLeafMax;

	bool            m_bAcceptDeflate;

	quint32         m_tKeyRequest;

//	quint32         m_tLastHAWIn;       // Time when HAW packet recievied
//	quint32         m_nCountHAWIn;      // Number of HAW packets recievied
//	bool            m_bSendQHT;
//	quint32         m_tLastQHT;

	quint32         m_nHAWWait;

	QQueue<G2Packet*>   m_lSendQueue;

	QueryHashTable*    m_pRemoteTable;
	QueryHashTable*    m_pLocalTable;
	HubHorizonGroup*   m_pHubGroup;

	QHash<quint32, quint32> m_lRABan;       // list of banned return addresses

private:
	/**
	 * @brief m_bHandshakeMessage1Recieved indicates that we have recieved HS message #1 from the
	 * other party and are now waiting for message #3.
	 */
	bool m_bHandshakeMessage1Recieved;

	/**
	 * @brief m_bHandshakeMessage2Recieved indicates that we have recieved HS message #2 from the
	 * other party.
	 */
	bool m_bHandshakeMessage2Recieved;

public:
	G2Node( QObject* parent = NULL );
	virtual ~G2Node();

	void sendPacket( G2Packet* pPacket, bool bBuffered = false, bool bRelease = false );

	void sendLNI();
	void sendHAW();

	void onTimer( quint32 tNow );

	inline void attachTo( NetworkConnection* pOther );

protected:
	/**
	 * @brief initiateHandshake writes the initial G2 handshake message and sends it to the node.
	 * In this case we are the connection initiator. This handles handshake part #1.
	 */
	void initiateHandshake();

	/**
	 * @brief parseIncomingHandshake parses an initial G2 handshake message from the Node.
	 * In this case, the other node is the connection initiator. This handles handshake part #1.
	 */
	void handleIncomingHandshake();

	/**
	 * @brief parseHandshakeResponse parses the reply message to a handshake initiated by us.
	 * In this case we are the connection initiator. This handles handshake part #2.
	 */
	void handleHandshakeResponse();

	/**
	 * @brief parseHandShakeAccept parses the third handshake message.
	 * In this case, the other node is the connection initiator. This handles handshake part #3.
	 */
	void handleHandshakeAccept();

	/**
	 * @brief readUserAgentSecurity reads and checks the user agent information from the Handshake
	 * @return true if the connection has been terminated; false otherwise
	 */
	bool readUserAgentSecurity( const QString& sHandShake );

	/**
	 * @brief send_ConnectError closes the connection with sReason (Error).
	 * @param sReason : the error code and message
	 * @param bXTry : whether to append an X-Try header
	 */
	void send_ConnectError( QString sReason, bool bXTry );

	/**
	 * @brief send_ConnectOK sends handshake message #2 or #3 to other party. This acknowledges the
	 * established connection to the other party.
	 *
	 * @param bHandshakeStep2  <code>true</code> if this is step #2 of the handshake;
	 * <br><code>false</code> if this is step #3
	 * @param bDeflated        Whether or not to negociate connection deflation.
	 */
	void send_ConnectOK( bool bHandshakeStep2, bool bDeflate = false );
	void sendStartups();

	void onPacket( G2Packet* pPacket );
	void onPing( G2Packet* pPacket );
	void onPong( G2Packet* pPacket );
	void onLNI( G2Packet* pPacket );
	void onKHL( G2Packet* pPacket );
	void onQHT( G2Packet* pPacket );
	void onQKR( G2Packet* pPacket );
	void onQKA( G2Packet* pPacket );
	void onQA( G2Packet* pPacket );
	void onQH2( G2Packet* pPacket );
	void onHaw( G2Packet* pPacket );
	void onQuery( G2Packet* pPacket );

	qint64 writeToNetwork( qint64 nBytes );

	inline bool hasData();

signals:
	void nodeStateChanged();

public slots:
	/**
	 * @brief onConnectNode is to be triggered once the TCP socket of this connection emits the
	 * QTcpSocket::connected signal.
	 */
	void onConnectNode();

	/**
	 * @brief onRead handles newly available bytes for a network connection.
	 */
	void onRead();

	void onError( QAbstractSocket::SocketError e );

	friend class CNetwork;
};

void G2Node::attachTo( NetworkConnection* pOther )
{
	Neighbour::attachTo( pOther );
}

bool G2Node::hasData()
{
	if ( !m_lSendQueue.isEmpty() )
	{
		return true;
	}

	return Neighbour::hasData();
}

#endif // G2NODE_H
