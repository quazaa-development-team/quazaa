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
	virtual void onSuccess( void* pParam ) = 0;
	virtual void onFailure( void* pParam ) = 0;
};

class DatagramOut;
class DatagramIn;
class Buffer;
class QHostAddress;

/**
 * @brief The Datagrams class handles sending and recieving UDP datagrams.
 */
class Datagrams : public QObject
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
		  >                     m_RecvCache;            // For searching by IP & sequence.
	QLinkedList<DatagramIn*>    m_RecvCacheTime;        // A list ordered by recieve time, last is oldest.

	QLinkedList <
	QPair<EndPoint, char*>
	>               m_AckCache;

	QLinkedList<DatagramIn*> m_FreeDatagramIn;		// A list of free incoming packets.
	QLinkedList<Buffer*>	 m_FreeBuffer;		// A list of free buffers.

	Buffer*    	m_pRecvBuffer;
	QHostAddress*   m_pHostAddress;             // the sender's host address
	quint16         m_nPort;                    // the sender's port

	bool            m_bActive;

	TCPBandwidthMeter m_mInput;
	TCPBandwidthMeter m_mOutput;

	quint32			m_nDiscarded;
	quint32			m_nInFrags;
	quint32			m_nOutFrags;

public:
	Datagrams();
	~Datagrams();

	void listen();
	void disconnectNode();

	void sendPacket( G2Packet* pPacket, const EndPoint& oAddr, bool bAck = false,
					 DatagramWatcher* pWatcher = NULL, void* pParam = NULL );

	void removeOldIn( bool bForce = false );
	void remove( DatagramIn* pDatagramIn, bool bReclaim = false );
	void remove( DatagramOut* pDatagramOut );
	void onReceiveGND();
	void onAcknowledgeGND();

	void onPacket( G2Packet* pPacket, const EndPoint& addr );
	void onPing  ( G2Packet* pPacket, const EndPoint& addr );
	void onPong  ( G2Packet* pPacket, const EndPoint& addr );
	void onCRAWLR( G2Packet* pPacket, const EndPoint& addr );
	void onQKR   ( G2Packet* pPacket, const EndPoint& addr );
	void onQKA   ( G2Packet* pPacket, const EndPoint& oHost );
	void onQA    ( G2Packet* pPacket, const EndPoint& addr );
	void onQH2   ( G2Packet* pPacket, const EndPoint& addr );
	void onQuery ( G2Packet* pPacket, const EndPoint& addr );

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

#pragma pack(push, 1) // instructs the compiler to not to use padding when declaring the struct
/**
 * @typedef GND_HEADER defines the UDP reliability protocol control header used in Gnutella2.
 */
typedef struct
{
	/**
	 * @brief szTag contains a three byte encoding protocol identifier, in our case "GND" for
	 * "GNutella Datagram". If this signature is not present the packet should not be decoded as a
	 * Gnutella2 reliability layer transmission.
	 */
	char     szTag[3];

	/**
	 * @brief nFlags contains flags which modify the content of the packet.
	 * See: http://g2.doxu.org/index.php/UDP_Transceiver#Encoding
	 */
	quint8   nFlags;

	/**
	 * @brief nSequence contains the sequence number of the packet. Sequence numbers on consecutive
	 * packets need not be increasing (although that is convenient), they must only be different.
	 * If a packet is fragmented, all of its fragments will have the same sequence number.
	 * Byte order is unimportant here.
	 */
	quint16  nSequence;

	/**
	 * @brief nPart contains the fragment part number (1 <= nPart <= nCount)
	 */
	quint8   nPart;

	/**
	 * @brief nCount contains the number of fragment parts in this packet. On a transmission, this
	 * value will be non-zero (all packets must have at least one fragment). If nCount is zero, this
	 * is an acknowledgement.
	 */
	quint8   nCount;
} GND_HEADER;

#pragma pack(pop)

quint32 Datagrams::downloadSpeed()
{
	return m_mInput.avgUsage();
}
quint32 Datagrams::uploadSpeed()
{
	return m_mOutput.avgUsage();
}
bool Datagrams::isFirewalled()
{
	return m_bFirewalled;
}
bool Datagrams::isListening()
{
	return ( m_bActive && m_pSocket && m_pSocket->isValid() );
}

extern Datagrams datagrams;

#endif // DATAGRAMS_H
