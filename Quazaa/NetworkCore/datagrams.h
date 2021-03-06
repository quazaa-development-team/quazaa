/*
** datagrams.h
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

#ifndef DATAGRAMS_H
#define DATAGRAMS_H

#include "types.h"
#include <QMutex>
#include <QUdpSocket>
#include <QHash>
#include <QLinkedList>
#include <QTimer>
#include <QTime>

#include "queryhit.h"
#include "networkconnection.h"

class G2Packet;

class DatagramWatcher
{
public:
	virtual      ~DatagramWatcher();
	virtual void onSuccess(void* pParam) = 0;
	virtual void onFailure(void* pParam) = 0;
};

class DatagramOut;
class DatagramIn;
class CBuffer;
class QHostAddress;

class CDatagrams : public QObject
{
	Q_OBJECT

public:
	QMutex      m_pSection;
protected:
	quint32 m_nUploadLimit;

	QUdpSocket* m_pSocket;

	bool m_bFirewalled;

	QTimer*       m_tSender;

	QHash<quint16, DatagramOut*>     m_SendCacheMap;    // To quicky find the sequence of packets.
	QLinkedList<DatagramOut*>		 m_SendCache;		// A LIFO queue, last is oldest.
	QLinkedList<DatagramOut*>		 m_FreeDatagramOut;
	quint16                          m_nSequence;

	QHash < QHostAddress,
		  QHash<quint32, DatagramIn*>
		  >                     m_RecvCache;            // For searching by ip & sequence.
	QLinkedList<DatagramIn*>    m_RecvCacheTime;        // A list ordered by recieve time, last is oldest.

	QLinkedList <
		QPair<CEndPoint, char*>
				>               m_AckCache;

	QLinkedList<DatagramIn*> m_FreeDatagramIn;		// A list of free incoming packets.
	QLinkedList<CBuffer*>	 m_FreeBuffer;		// A list of free buffers.

	CBuffer*    	m_pRecvBuffer;
	QHostAddress*   m_pHostAddress;
	quint16         m_nPort;

	bool            m_bActive;

	TCPBandwidthMeter m_mInput;
	TCPBandwidthMeter m_mOutput;

	quint32			m_nDiscarded;
	quint32			m_nInFrags;
	quint32			m_nOutFrags;

public:
	CDatagrams();
	~CDatagrams();

	void listen();
	void disconnectNode();

	void sendPacket(CEndPoint& oAddr, G2Packet* pPacket, bool bAck = false, DatagramWatcher* pWatcher = 0, void* pParam = 0);

	void removeOldIn(bool bForce = false);
	void remove(DatagramIn* pDatagramIn, bool bReclaim = false);
	void remove(DatagramOut* pDatagramOut);
	void onReceiveGND();
	void onAcknowledgeGND();

	void onPacket(CEndPoint addr, G2Packet* pPacket);
	void onPing(CEndPoint& addr, G2Packet* pPacket);
	void onPong(CEndPoint& addr, G2Packet* pPacket);
	void onCRAWLR(CEndPoint& addr, G2Packet* pPacket);
	void onQKR(CEndPoint& addr, G2Packet* pPacket);
	void onQKA(CEndPoint& addr, G2Packet* pPacket);
	void onQA(CEndPoint& addr, G2Packet* pPacket);
	void onQH2(CEndPoint& addr, G2Packet* pPacket);
	void onQuery(CEndPoint& addr, G2Packet* pPacket);

	inline quint32 downloadSpeed();
	inline quint32 uploadSpeed();
	inline bool isFirewalled();
	inline bool isListening();

public slots:
	void onDatagram();
	void flushSendCache();
	void __FlushSendCache();

signals:
	void sendQueueUpdated();

	friend class CNetwork;
};

#pragma pack(push, 1)
typedef struct
{
	char     szTag[3];
	quint8   nFlags;
	quint16  nSequence;
	quint8   nPart;
	quint8   nCount;
} GND_HEADER;

#pragma pack(pop)

quint32 CDatagrams::downloadSpeed()
{
	return m_mInput.AvgUsage();
}
quint32 CDatagrams::uploadSpeed()
{
	return m_mOutput.AvgUsage();
}
bool CDatagrams::isFirewalled()
{
	return m_bFirewalled;
}
bool CDatagrams::isListening()
{
	return (m_bActive && m_pSocket && m_pSocket->isValid());
}

extern CDatagrams Datagrams;

#endif // DATAGRAMS_H
