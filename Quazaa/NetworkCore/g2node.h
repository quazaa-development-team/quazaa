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
class CQueryHashTable;
class CHubHorizonGroup;

class CG2Node : public CNeighbour
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
	quint32         m_tLastHAWIn;			// Time when HAW packet recievied
	quint32         m_nCountHAWIn;			// Number of HAW packets recievied

	bool            m_bSendQHT;
	quint32         m_tLastQHT;

	quint32         m_nHAWWait;

	QQueue<G2Packet*>   m_lSendQueue;

	CQueryHashTable*    m_pRemoteTable;
	CQueryHashTable*    m_pLocalTable;
	CHubHorizonGroup*   m_pHubGroup;

	QHash<quint32, quint32> m_lRABan;       // list of banned return addresses

public:
	CG2Node(QObject* parent = NULL);
	virtual ~CG2Node();

	void attachTo(CNetworkConnection* pOther)
	{
		CNeighbour::attachTo(pOther);
	}

	void sendPacket(G2Packet* pPacket, bool bBuffered = false, bool bRelease = false);

protected:
	void parseOutgoingHandshake();
	void parseIncomingHandshake();

	void send_ConnectError(QString sReason);
	void send_ConnectOK(bool bReply, bool bDeflated = false);
	void sendStartups();

public:
	void onTimer(quint32 tNow);
signals:
	void nodeStateChanged();
public slots:
	void onConnectNode();
	void onRead();

public:
	void sendLNI();
	void sendHAW();
protected:
	void onPacket(G2Packet* pPacket);
	void onPing(G2Packet* pPacket);
	void onPong(G2Packet* pPacket);
	void onLNI(G2Packet* pPacket);
	void onKHL(G2Packet* pPacket);
	void onQHT(G2Packet* pPacket);
	void onQKR(G2Packet* pPacket);
	void onQKA(G2Packet* pPacket);
	void onQA(G2Packet* pPacket);
	void onQH2(G2Packet* pPacket);
	void onQuery(G2Packet* pPacket);
	void onHaw(G2Packet* pPacket);

protected:
	qint64 writeToNetwork(qint64 nBytes);
	bool hasData()
	{
		if ( !m_lSendQueue.isEmpty() )
		{
			return true;
		}

		return CNeighbour::hasData();
	}

	friend class CNetwork;
};

#endif // G2NODE_H
