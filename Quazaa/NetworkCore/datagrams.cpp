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

#include "datagrams.h"
#include "network.h"
#include "neighbours.h"
#include "datagramfrags.h"
#include "g2node.h"
#include "g2packet.h"
#include "searchmanager.h"
#include "Hashes/hash.h"
#include "queryhit.h"
#include "systemlog.h"
#include "querykeys.h"
#include "query.h"

#include "g2hostcache.h"
#include "securitymanager.h"

#include "quazaaglobals.h"
#include "quazaasettings.h"

#include "thread.h"
#include "buffer.h"

#include "debug_new.h"

CDatagrams Datagrams;

CDatagrams::CDatagrams()
{
	m_nUploadLimit = 32768; // TODO: Upload limiting.

	m_pRecvBuffer = new Buffer();
	m_pHostAddress = new QHostAddress();
	m_nSequence = 0;

	m_bActive = false;

	m_pSocket = 0;
	m_tSender = 0;
	m_bFirewalled = true;

	m_nInFrags = 0;
	m_nOutFrags = 0;
}

CDatagrams::~CDatagrams()
{
	if(m_bActive)
	{
		disconnectNode();
	}

	if(m_pSocket)
	{
		delete m_pSocket;
	}

	if(m_tSender)
	{
		delete m_tSender;
	}

	if(m_pRecvBuffer)
	{
		delete m_pRecvBuffer;
	}
	if(m_pHostAddress)
	{
		delete m_pHostAddress;
	}
}

void CDatagrams::listen()
{
	QMutexLocker l(&m_pSection);

	if(m_bActive)
	{
		systemLog.postLog(LogSeverity::Debug, Components::Network, QString("CDatagrams::Listen - already listening"));
		return;
	}

	Q_ASSERT(m_pSocket == 0);

	m_pSocket = new QUdpSocket(this);

	CEndPoint addr = networkG2.getLocalAddress();
	if(m_pSocket->bind(addr.port()))
	{
		systemLog.postLog(LogSeverity::Debug, Components::Network, QString("Datagrams listening on %1").arg(m_pSocket->localPort()));
		m_nDiscarded = 0;

		for(int i = 0; i < quazaaSettings.Gnutella2.UdpBuffers; i++)
		{
			m_FreeBuffer.append(new Buffer(1024));
		}

		for(int i = 0; i < quazaaSettings.Gnutella2.UdpInFrames; i++)
		{
			m_FreeDatagramIn.append(new DatagramIn);
		}

		for(int i = 0; i < quazaaSettings.Gnutella2.UdpOutFrames; i++)
		{
			m_FreeDatagramOut.append(new DatagramOut);
		}

		connect(this, SIGNAL(sendQueueUpdated()), this, SLOT(flushSendCache()), Qt::QueuedConnection);
		connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onDatagram()), Qt::QueuedConnection);

		m_bActive = true;
	}
	else
	{
		systemLog.postLog(LogSeverity::Debug, Components::Network, QString("Can't bind UDP socket! UDP communication disabled!"));
		disconnectNode();
	}

	m_bFirewalled = true;
}

void CDatagrams::disconnectNode()
{
	QMutexLocker l(&m_pSection);

	m_bActive = false;

	if(m_pSocket)
	{
		m_pSocket->close();
		delete m_pSocket;
		m_pSocket = 0;
	}

	disconnect(SIGNAL(sendQueueUpdated()));

	while(!m_AckCache.isEmpty())
	{
		QPair<CEndPoint, char*> oAck = m_AckCache.takeFirst();
		delete [] oAck.second;
	}

	while(!m_SendCache.isEmpty())
	{
		remove(m_SendCache.first());
	}

	while(!m_RecvCacheTime.isEmpty())
	{
		remove(m_RecvCacheTime.first());
	}

	while(!m_FreeDatagramIn.isEmpty())
	{
		delete m_FreeDatagramIn.takeFirst();
	}

	while(!m_FreeDatagramOut.isEmpty())
	{
		delete m_FreeDatagramOut.takeFirst();
	}

	while(!m_FreeBuffer.isEmpty())
	{
		delete m_FreeBuffer.takeFirst();
	}
}

void CDatagrams::onDatagram()
{
	if ( !m_bActive )
	{
		return;
	}

	//while(m_pSocket->hasPendingDatagrams())
	{
		qint64 nSize = m_pSocket->pendingDatagramSize();
		m_pRecvBuffer->resize( nSize );
		qint64 nReadSize = m_pSocket->readDatagram( m_pRecvBuffer->data(), nSize, m_pHostAddress, &m_nPort );

		m_mInput.Add( nReadSize );

		if ( nReadSize < 8 )
		{
		//	continue;
			return;
		}

		// TODO: Ask brov about this. Should we reply with something like: "You are banned!"?
		CEndPoint addr(*m_pHostAddress, m_nPort);
		// Don't continue processing packets from hosts that are banned.
		if ( securityManager.isDenied(addr) )
		{
			systemLog.postLog( LogSeverity::Security, Components::Network,
							   tr( "Dropped packet from banned host: %1."
								   ).arg( m_pHostAddress->toString() ) );
		//	continue;
			return;
		}

		m_nInFrags++;

		GND_HEADER* pHeader = (GND_HEADER*)m_pRecvBuffer->data();
		if ( strncmp( (char*)&pHeader->szTag, "GND", 3 ) == 0 &&
			 pHeader->nPart > 0 && (pHeader->nCount == 0 || pHeader->nPart <= pHeader->nCount ) )
		{
			if ( pHeader->nCount == 0 )
			{
				// ACK
				onAcknowledgeGND();
			}
			else
			{
				// DG
				onReceiveGND();
			}
		}
	}
}

void CDatagrams::onReceiveGND()
{
	GND_HEADER* pHeader = (GND_HEADER*)m_pRecvBuffer->data();
	QHostAddress nIp = *m_pHostAddress;
	quint32 nSeq = ((pHeader->nSequence << 16) & 0xFFFF0000) + (m_nPort & 0x0000FFFF);

#ifdef DEBUG_UDP
	systemLog.postLog(LogSeverity::Debug, "Received GND from %s:%u nSequence = %u nPart = %u nCount = %u", m_pHostAddress->toString().toLocal8Bit().constData(), m_nPort, pHeader->nSequence, pHeader->nPart, pHeader->nCount);
#endif

	DatagramIn* pDatagramIn = 0;

	if(m_RecvCache.contains(nIp) && m_RecvCache[nIp].contains(nSeq))
	{
		pDatagramIn = m_RecvCache[nIp][nSeq];

		// To give a chance for bigger packages ;)
		if(pDatagramIn->m_nLeft)
		{
			pDatagramIn->m_tStarted = time(0);
		}
	}
	else
	{
		QMutexLocker l(&m_pSection);

		if(!m_FreeDatagramIn.isEmpty())
		{
			pDatagramIn = m_FreeDatagramIn.takeFirst();
		}
		else
		{
			if(m_FreeDatagramIn.isEmpty())
			{
				removeOldIn(true);
				if(m_FreeDatagramIn.isEmpty())
				{
#ifdef DEBUG_UDP
					systemLog.postLog(LogSeverity::Debug, QString("UDP in frames exhausted"));
#endif
					m_nDiscarded++;
					return;
				}
			}

			pDatagramIn = m_FreeDatagramIn.takeFirst();
		}

		if(m_FreeBuffer.size() < pHeader->nCount)
		{
			m_nDiscarded++;
			m_FreeDatagramIn.append(pDatagramIn);
			removeOldIn(false);
			return;
		}

		pDatagramIn->create(CEndPoint(*m_pHostAddress, m_nPort), pHeader->nFlags, pHeader->nSequence, pHeader->nCount);

		for(int i = 0; i < pHeader->nCount; i++)
		{
			Q_ASSERT(pDatagramIn->m_pBuffer[i] == 0);
			pDatagramIn->m_pBuffer[i] = m_FreeBuffer.takeFirst();
		}

		m_RecvCache[nIp][nSeq] = pDatagramIn;
		m_RecvCacheTime.prepend(pDatagramIn);
	}

	// It is here, in case if we did not have free datagrams
	// ACK = I've received a datagram, and if you have received and rejected it, do not send ACK-a
	if(pHeader->nFlags & 0x02)
	{
		GND_HEADER* pAck = new GND_HEADER;

		memcpy(pAck, pHeader, sizeof(GND_HEADER));
		pAck->nCount = 0;
		pAck->nFlags = 0;

#ifdef DEBUG_UDP
		systemLog.postLog(LogSeverity::Debug, "Sending UDP ACK to %s:%u", m_pHostAddress->toString().toLocal8Bit().constData(), m_nPort);
#endif

		//m_pSocket->writeDatagram((char*)&oAck, sizeof(GND_HEADER), *m_pHostAddress, m_nPort);
		//m_mOutput.Add(sizeof(GND_HEADER));
		m_AckCache.append(qMakePair(CEndPoint(*m_pHostAddress, m_nPort), reinterpret_cast<char*>(pAck)));
		if( m_AckCache.count() == 1 )
			QMetaObject::invokeMethod(this, "flushSendCache", Qt::QueuedConnection);
	}

	if(pDatagramIn->add(pHeader->nPart, m_pRecvBuffer->data() + sizeof(GND_HEADER), m_pRecvBuffer->size() - sizeof(GND_HEADER)))
	{

		G2Packet* pPacket = 0;
		try
		{
			CEndPoint addr(*m_pHostAddress, m_nPort);
			pPacket = pDatagramIn->toG2Packet();
			if(pPacket)
			{
				onPacket(addr, pPacket);
			}
		}
		catch(...)
		{

		}
		if(pPacket)
		{
			pPacket->release();
		}

		m_pSection.lock();
		remove(pDatagramIn, true);
		m_pSection.unlock();
	}
}

void CDatagrams::onAcknowledgeGND()
{
	GND_HEADER* pHeader = (GND_HEADER*)m_pRecvBuffer->data();

#ifdef DEBUG_UDP
	systemLog.postLog(LogSeverity::Debug, "UDP received GND ACK from %s seq %u part %u", m_pHostAddress->toString().toLocal8Bit().constData(), pHeader->nSequence, pHeader->nPart);
#endif

	if(!m_SendCacheMap.contains(pHeader->nSequence))
	{
		return;
	}

	DatagramOut* pDatagramOut = m_SendCacheMap.value(pHeader->nSequence);

	if(pDatagramOut->acknowledge(pHeader->nPart))
	{
		m_pSection.lock();
		remove(pDatagramOut);
		m_pSection.unlock();
	}
}

void CDatagrams::remove(DatagramIn* pDatagramIn, bool bReclaim)
{
	ASSUME_LOCK(m_pSection);
	for(int i = 0; i < pDatagramIn->m_nCount; i++)
	{
		if(pDatagramIn->m_pBuffer[i])
		{
			m_FreeBuffer.append(pDatagramIn->m_pBuffer[i]);
			pDatagramIn->m_pBuffer[i]->clear();
			pDatagramIn->m_pBuffer[i] = 0;
		}
	}

	if(bReclaim)
	{
		return;
	}

	if(m_RecvCache.contains(pDatagramIn->m_oAddress))
	{
		quint32 nSeq = ((pDatagramIn->m_nSequence << 16) & 0xFFFF0000) + (pDatagramIn->m_oAddress.port() & 0x0000FFFF);
		Q_ASSERT(pDatagramIn == m_RecvCache[pDatagramIn->m_oAddress][nSeq]);
		m_RecvCache[pDatagramIn->m_oAddress].remove(nSeq);
		if(m_RecvCache[pDatagramIn->m_oAddress].isEmpty())
		{
			m_RecvCache.remove(pDatagramIn->m_oAddress);
		}

		QLinkedList<DatagramIn*>::iterator itFrame = m_RecvCacheTime.end();
		while( itFrame != m_RecvCacheTime.begin() )
		{
			--itFrame;

			if( *itFrame == pDatagramIn )
			{
				m_RecvCacheTime.erase(itFrame);
				break;
			}
		}

		m_FreeDatagramIn.append(pDatagramIn);
	}
}

// Removes a package from the cache collection.
void CDatagrams::removeOldIn(bool bForce)
{
	quint32 tNow = time(0);
	bool bRemoved = false;

	while(!m_RecvCacheTime.isEmpty() && (tNow - m_RecvCacheTime.back()->m_tStarted > quazaaSettings.Gnutella2.UdpInExpire || m_RecvCacheTime.back()->m_nLeft == 0))
	{
		remove(m_RecvCacheTime.back());
		bRemoved = true;
	}

	if(bForce && !bRemoved && !m_RecvCacheTime.isEmpty())
	{
		for(QLinkedList<DatagramIn*>::iterator itPacket = m_RecvCacheTime.begin(); itPacket != m_RecvCacheTime.end(); ++itPacket)
		{
			if((*itPacket)->m_nLeft == 0)
			{
				remove(*itPacket);
				break;
			}
		}
	}
}

void CDatagrams::remove(DatagramOut* pDatagramOut)
{
	ASSUME_LOCK(m_pSection);

	m_SendCacheMap.remove(pDatagramOut->m_nSequence);

	QLinkedList<DatagramOut*>::iterator itFrame = m_SendCache.end();
	while( itFrame != m_SendCache.begin() )
	{
		--itFrame;

		if( *itFrame == pDatagramOut )
		{
			m_SendCache.erase(itFrame);
			break;
		}
	}

	m_FreeDatagramOut.append(pDatagramOut);

	if(pDatagramOut->m_pBuffer)
	{
		m_FreeBuffer.append(pDatagramOut->m_pBuffer);
		pDatagramOut->m_pBuffer->clear();
		pDatagramOut->m_pBuffer = 0;
	}
}

void CDatagrams::flushSendCache()
{
	QMutexLocker l(&m_pSection);

	__FlushSendCache();
}

void CDatagrams::__FlushSendCache()
{
	if(!m_bActive)
	{
		return;
	}

	//QMutexLocker l(&m_pSection);
	ASSUME_LOCK(Datagrams.m_pSection);

	quint32 tNow = time(0);

	qint64 nToWrite = qint64(m_nUploadLimit) - qint64(m_mOutput.Usage());

	static TCPBandwidthMeter meter;

	// TODO: Maybe make it dynamic? So bad routers are automatically detected and settings adjusted?
	qint64 nMaxPPS = quazaaSettings.Connection.UDPOutLimitPPS - meter.Usage();

	if( nMaxPPS <= 0 )
	{
		systemLog.postLog( LogSeverity::Debug, Components::Network,
						   "UDP: PPS limit reached, ACKS: %d, Packets: %d, Average PPS: %u / %u",
						   m_AckCache.size(), m_SendCache.size(), meter.AvgUsage(), meter.Usage() );
		return;
	}

	while( nToWrite > 0 && !m_AckCache.isEmpty() && nMaxPPS > 0)
	{
		QPair< CEndPoint, char* > oAck = m_AckCache.takeFirst();
		m_pSocket->writeDatagram(oAck.second, sizeof(GND_HEADER), oAck.first, oAck.first.port());
		m_mOutput.Add(sizeof(GND_HEADER));
		nToWrite -= sizeof(GND_HEADER);
		delete (GND_HEADER*)oAck.second;
		--nMaxPPS;
		meter.Add(1);
	}

	QHostAddress nLastHost;

	// it can write slightly more than limit allows... that's ok
	while(nToWrite > 0 && !m_SendCache.isEmpty() && nMaxPPS > 0)
	{
		bool bSent = false;

		char* pPacket;
		quint32 nPacket;

		for(QLinkedList<DatagramOut*>::iterator itPacket = m_SendCache.begin(); itPacket != m_SendCache.end(); ++itPacket)
		{
			DatagramOut* pDatagramOut = *itPacket;

			if(pDatagramOut->m_oAddress == nLastHost)
			{
				continue;
			}

			// TODO: Check the firewall's UDP state. Could do 3 UDP states.
			if(pDatagramOut->getPacket(tNow, &pPacket, &nPacket, pDatagramOut->m_bAck && m_nInFrags > 0))
			{
#ifdef DEBUG_UDP
				systemLog.postLog(LogSeverity::Debug, "UDP sending to %s seq %u part %u count %u", pDatagramOut->m_oAddress.toString().toLocal8Bit().constData(), pDatagramOut->m_nSequence, ((GND_HEADER*)pPacket)->nPart, pDatagramOut->m_nCount);
#endif

				m_pSocket->writeDatagram(pPacket, nPacket, pDatagramOut->m_oAddress, pDatagramOut->m_oAddress.port());
				m_nOutFrags++;

				nLastHost = pDatagramOut->m_oAddress;

				if(nToWrite >= nPacket)
				{
					nToWrite -= nPacket;
				}
				else
				{
					nToWrite = 0;
				}

				m_mOutput.Add(nPacket);

				if(!pDatagramOut->m_bAck)
				{
					remove(pDatagramOut);
				}

				nMaxPPS--;
				meter.Add(1);

				bSent = true;

				break;
			}
		}

		if(m_SendCache.isEmpty() || !bSent)
		{
			break;
		}
	}

	while(!m_SendCache.isEmpty() && tNow - m_SendCache.back()->m_tSent > quazaaSettings.Gnutella2.UdpOutExpire)
	{
		remove(m_SendCache.back());
	}
}

void CDatagrams::sendPacket(CEndPoint oAddr, G2Packet* pPacket, bool bAck, DatagramWatcher* pWatcher, void* pParam)
{
	if(!m_bActive)
	{
		return;
	}

	QMutexLocker l(&m_pSection);

	Q_UNUSED(pWatcher);
	Q_UNUSED(pParam);

	if(m_FreeDatagramOut.isEmpty())
	{
		systemLog.postLog(LogSeverity::Debug, Components::Network, QString("UDP out frames exhausted"));

		if( !bAck ) // if caller does not want ACK, drop the packet here
			return; // TODO: needs more testing

		remove(m_SendCache.last());

	}

	if(m_FreeBuffer.isEmpty())
	{
		removeOldIn(false);

		if(m_FreeBuffer.isEmpty())
		{
			systemLog.postLog(LogSeverity::Debug, Components::Network, QString("UDP out discarded, out of buffers"));
			return;
		}
	}

	DatagramOut* pDatagramOut = m_FreeDatagramOut.takeFirst();
	pDatagramOut->create(oAddr, pPacket, m_nSequence++, m_FreeBuffer.takeFirst(), (bAck && (m_nInFrags > 0))); // to prevent net spam when unable to receive datagrams

	m_SendCache.prepend(pDatagramOut);
	m_SendCacheMap[pDatagramOut->m_nSequence] = pDatagramOut;

	// TODO: Notify the listener if we have one.

#ifdef DEBUG_UDP
	systemLog.postLog(LogSeverity::Debug, "UDP queued for %s seq %u parts %u", oAddr.toString().toLocal8Bit().constData(), pDatagramOut->m_nSequence, pDatagramOut->m_nCount);
#endif

	//emit SendQueueUpdated();
	__FlushSendCache();
}

void CDatagrams::onPacket(CEndPoint addr, G2Packet* pPacket)
{
	try
	{
		if(pPacket->isType("PI"))
		{
			onPing(addr, pPacket);
		}
		else if(pPacket->isType("PO"))
		{
			onPong(addr, pPacket);
		}
		else if(pPacket->isType("CRAWLR"))
		{
			onCRAWLR(addr, pPacket);
		}
		else if(pPacket->isType("QKR"))
		{
			onQKR(addr, pPacket);
		}
		else if(pPacket->isType("QKA"))
		{
			onQKA(addr, pPacket);
		}
		else if(pPacket->isType("QA"))
		{
			onQA(addr, pPacket);
		}
		else if(pPacket->isType("QH2"))
		{
			onQH2(addr, pPacket);
		}
		else if(pPacket->isType("Q2"))
		{
			onQuery(addr, pPacket);
		}
		else
		{
			//systemLog.postLog(LogSeverity::Debug, QString("G2 UDP recieved unknown packet %1").arg(pPacket->GetType()));
			//qDebug() << "UDP RECEIVED unknown packet " << pPacket->GetType();
		}
	}
	catch(...)
	{
		systemLog.postLog(LogSeverity::Debug, Components::Network, QString("malformed packet"));
		//qDebug() << "malformed packet";
	}
}

void CDatagrams::onPing(CEndPoint& addr, G2Packet* pPacket)
{
	Q_UNUSED(pPacket);

	G2Packet* pNew = G2Packet::newPacket("PO", false);
	sendPacket(addr, pNew, false);
	pNew->release();
}

void CDatagrams::onPong(CEndPoint& addr, G2Packet* pPacket)
{
	if(pPacket->m_bCompound)
	{
		char szType[9];
		quint32 nLength = 0, nNext = 0;

		while(pPacket->readPacket(&szType[0], nLength))
		{
			nNext = pPacket->m_nPosition + nLength;

			if(strcmp("RELAY", szType) == 0)
			{
				if(!networkG2.isConnectedTo(addr))
				{
					m_bFirewalled = false;
				}
			}

			pPacket->m_nPosition = nNext;
		}
	}
}

void CDatagrams::onCRAWLR(CEndPoint& addr, G2Packet* pPacket)
{
	QMutexLocker l2(&Neighbours.m_pSection);

//	bool bRLeaf = false;
//	bool bRNick = false;
//	bool bRGPS = false;
//	bool bRExt = false;

	if(!pPacket->m_bCompound)
	{
		return;
	}

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while(pPacket->readPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("RLEAF", szType) == 0)
		{
//			bRLeaf = true;
		}
		else if(strcmp("RNAME", szType) == 0)
		{
//			bRNick = true;
		}
		else if(strcmp("RGPS", szType) == 0)
		{
//			bRGPS = true;
		}
		else if(strcmp("REXT", szType) == 0)
		{
//			bRExt = true;
		}

		pPacket->m_nPosition = nNext;
	}

	G2Packet* pCA = G2Packet::newPacket("CRAWLA", true);

	G2Packet* pTmp = G2Packet::newPacket("SELF", true);
	if(Neighbours.isG2Hub())
	{
		pTmp->writePacket("HUB", 0);
	}
	else
	{
		pTmp->writePacket("LEAF", 0);
	}
	pTmp->writePacket("NA", ((networkG2.m_oAddress.protocol() == 0) ? 6 : 18))->writeHostAddress(networkG2.m_oAddress);
	pTmp->writePacket("CV", CQuazaaGlobals::USER_AGENT_STRING().toUtf8().size())->writeString(CQuazaaGlobals::USER_AGENT_STRING(), false);
	pTmp->writePacket("V", 4)->writeString(CQuazaaGlobals::VENDOR_CODE(), false);;
	quint16 nLeaves = Neighbours.m_nLeavesConnectedG2;
	pTmp->writePacket("HS", 2)->writeIntLE(nLeaves);
	if(!quazaaSettings.Profile.GnutellaScreenName.isEmpty())
	{
		pTmp->writePacket("NAME", quazaaSettings.Profile.GnutellaScreenName.left(255).toUtf8().size())->writeString(quazaaSettings.Profile.GnutellaScreenName.left(255));
	}

	pCA->writePacket(pTmp);
	pTmp->release();

	for(QList<CNeighbour*>::iterator itNode = Neighbours.begin(); itNode != Neighbours.end(); ++itNode)
	{
		if((*itNode)->m_nProtocol != DiscoveryProtocol::G2)
		{
			continue;
		}

		CG2Node* pNode = (CG2Node*) * itNode;
		if(pNode->m_nState == nsConnected)
		{
			if(pNode->m_nType == G2_HUB)
			{
				G2Packet* pNH = G2Packet::newPacket("NH");
				pNH->writePacket("NA", ((pNode->m_oAddress.protocol() == 0) ? 6 : 18))->writeHostAddress(pNode->m_oAddress);
				pNH->writePacket("HS", 2)->writeIntLE(pNode->m_nLeafCount);
				pCA->writePacket(pNH);
				pNH->release();
			}
			else if(pNode->m_nType == G2_LEAF)
			{
				G2Packet* pNL = G2Packet::newPacket("NL");
				pNL->writePacket("NA", ((pNode->m_oAddress.protocol() == 0) ? 6 : 18))->writeHostAddress(pNode->m_oAddress);
				pCA->writePacket(pNL);
				pNL->release();
			}
		}
	}

	sendPacket(addr, pCA, true);

	pCA->release();
}

void CDatagrams::onQKR(CEndPoint& addr, G2Packet* pPacket)
{
	if(!Neighbours.isG2Hub())
	{
		return;
	}

	CEndPoint oRequestedAddress = addr;
	CEndPoint oSendingAddress = addr;

	if(pPacket->m_bCompound)
	{
		char szType[9];
		quint32 nLength = 0, nNext = 0;

		while(pPacket->readPacket(&szType[0], nLength))
		{
			nNext = pPacket->m_nPosition + nLength;

			if(strcmp("SNA", szType) == 0 && nLength >= 4)
			{
				if(nLength >= 16)
				{
					Q_IPV6ADDR ip;
					pPacket->read(&ip, 16);
					oSendingAddress.setAddress(ip);
				}
				else
				{
					quint32	nIp = pPacket->readIntBE<quint32>();
					oSendingAddress.setAddress(nIp);
				}
			}
			else if(strcmp("RNA", szType) == 0 && nLength >= 6)
			{
				if(nLength >= 18)
				{
					pPacket->readHostAddress(&oRequestedAddress, false);
				}
				else
				{
					pPacket->readHostAddress(&oRequestedAddress);
				}
			}
			pPacket->m_nPosition = nNext;
		}
	}

	if(!oRequestedAddress.port() || oRequestedAddress.isFirewalled())
	{
		return;
	}

	G2Packet* pAns = G2Packet::newPacket("QKA", true);
	quint32 nKey = QueryKeys.create(oRequestedAddress);
	pAns->writePacket("QK", 4);
	pAns->writeIntLE<quint32>(nKey);
	G2Packet* pSNA = G2Packet::newPacket("SNA");
	pSNA->writeHostAddress(oSendingAddress);
	pAns->writePacket(pSNA);
	pSNA->release();

	sendPacket(oRequestedAddress, pAns, false);
	pAns->release();

#if LOG_QUERY_HANDLING
	systemLog.postLog(LogSeverity::Debug, Components::G2, "Node %s asked for a query key (0x%08x) for node %s", qPrintable(addr.toStringWithPort()), nKey, qPrintable(oRequestedAddress.toStringWithPort()));
#endif // LOG_QUERY_HANDLING
}

void CDatagrams::onQKA(CEndPoint& addr, G2Packet* pPacket)
{
	if ( !pPacket->m_bCompound )
	{
		return;
	}

	quint32 nKey = 0;
	QHostAddress nKeyHost;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while(pPacket->readPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("QK", szType) == 0 && nLength >= 4)
		{
			nKey = pPacket->readIntLE<quint32>();
		}
		else if(strcmp("SNA", szType) == 0 && nLength >= 4)
		{
			if(nLength >= 16)
			{
				Q_IPV6ADDR ip;
				pPacket->read(&ip, 16);
				nKeyHost.setAddress(ip);
			}
			else
			{
				quint32 nIp = pPacket->readIntBE<quint32>();
				nKeyHost.setAddress(nIp);
			}
		}
		pPacket->m_nPosition = nNext;
	}

	{
		const quint32 tNow = common::getTNowUTC();
		hostCache.addKey( addr, tNow, NULL, nKey, tNow );
	}

#if LOG_QUERY_HANDLING
	systemLog.postLog(LogSeverity::Debug, QString("Got a query key for %1 = 0x%2").arg(addr.toString().toLocal8Bit().constData()).arg(nKey));
	//qDebug("Got a query key for %s = 0x%x", addr.toString().toLocal8Bit().constData(), nKey);
#endif // LOG_QUERY_HANDLING

	if(Neighbours.isG2Hub() && !nKeyHost.isNull() && nKeyHost != ((QHostAddress)networkG2.m_oAddress))
	{
		G2Packet* pQNA = G2Packet::newPacket("QNA");
		pQNA->writeHostAddress(addr);
		pPacket->prependPacket(pQNA);

		Neighbours.m_pSection.lock();
		CNeighbour* pNode = Neighbours.find(nKeyHost, DiscoveryProtocol::G2);
		if( pNode )
		{
			((CG2Node*)pNode)->sendPacket(pPacket, true, false);
		}
		Neighbours.m_pSection.unlock();
	}
}

void CDatagrams::onQA(CEndPoint& addr, G2Packet* pPacket)
{
	const quint32 tAck = 0;
	const quint32 tNow = common::getTNowUTC();
	hostCache.addAck( addr, tNow, tAck, tNow );

	QUuid oGuid;

	// Hubs are only supposed to route UDP /QA - we'll drop it if we're in leaf mode
	if ( SearchManager.onQueryAcknowledge( pPacket, addr, oGuid ) && Neighbours.isG2Hub() )
	{
		// Add from address
		G2Packet* pFR = G2Packet::newPacket( "FR" );
		pFR->writeHostAddress( addr );
		pPacket->addOrReplaceChild( "FR", pFR );

		networkG2.m_pSection.lock();
		networkG2.routePacket( oGuid, pPacket, true, false );
		networkG2.m_pSection.unlock();
	}
}

void CDatagrams::onQH2(CEndPoint& addr, G2Packet* pPacket)
{
	if ( !pPacket->m_bCompound )
	{
		return;
	}

	QueryHitInfo* pInfo = QueryHit::readInfo( pPacket, &addr );

	if ( pInfo )
	{
		if ( securityManager.isVendorBlocked( pInfo->m_sVendor ) ) // Block foxy client search results. We can't download from them any way.
		{
			securityManager.ban( pInfo->m_oNodeAddress, Security::RuleTime::SixHours, true,
								 QString( "[AUTO] Vendor blocked (%1)" ).arg( pInfo->m_sVendor ),
								 true
#if SECURITY_LOG_BAN_SOURCES
								 , QString( "datagrams.cpp line 982" )
#endif // SECURITY_LOG_BAN_SOURCES
								 );
		}
		else
		{
			if ( SearchManager.onQueryHit( pPacket, pInfo ) &&
				 Neighbours.isG2Hub() && pInfo->m_nHops < 7 )
			{
				pPacket->m_pBuffer[pPacket->m_nLength - 17]++;

				networkG2.m_pSection.lock();

				if(pInfo->m_oNodeAddress == pInfo->m_oSenderAddress)
				{
					// hits node address matches sender address
					networkG2.m_oRoutingTable.add(pInfo->m_oNodeGUID, pInfo->m_oSenderAddress);
				}
				else if(!pInfo->m_lNeighbouringHubs.isEmpty())
				{
					// hits address does not match sender address (probably forwarded by a hub)
					// and there are neighbouring hubs available, use them instead (sender address can be used instead...)
					networkG2.m_oRoutingTable.add(pInfo->m_oNodeGUID, pInfo->m_lNeighbouringHubs[0], false);
				}

				networkG2.routePacket(pInfo->m_oGUID, pPacket, true);

				networkG2.m_pSection.unlock();
			}
		}
	}
}

void CDatagrams::onQuery(CEndPoint &addr, G2Packet *pPacket)
{
	CQueryPtr pQuery = CQuery::fromPacket(pPacket, &addr);

	if(pQuery.isNull())
	{
#if LOG_QUERY_HANDLING
		qDebug() << "Received malformed query from" << qPrintable(addr.toStringWithPort());
#endif // LOG_QUERY_HANDLING
		return;
	}

	if( !Neighbours.isG2Hub() )
	{
		// Stop receiving queries from others
		// We are here because we just downgraded to leaf mode
		// Shareaza should not retry with QK == 0
		// TODO: test this
#if LOG_QUERY_HANDLING
		systemLog.postLog(LogSeverity::Debug, Components::G2, "Sending null query key to %s because we're not a hub.", qPrintable(addr.toStringWithPort()));
#endif // LOG_QUERY_HANDLING

		G2Packet* pQKA = G2Packet::newPacket("QKA", true);
		pQKA->writePacket("QK", 4)->writeIntLE<quint32>(0);

		if( addr != pQuery->m_oEndpoint )
		{
			pQKA->writePacket("SNA", (pQuery->m_oEndpoint.protocol() == QAbstractSocket::IPv6Protocol ? 18 : 6))->writeHostAddress(pQuery->m_oEndpoint);
		}

		sendPacket(pQuery->m_oEndpoint, pQKA);
		pQKA->release();

		return;
	}

	if(!QueryKeys.check(pQuery->m_oEndpoint, pQuery->m_nQueryKey))
	{
#if LOG_QUERY_HANDLING
		systemLog.postLog(LogSeverity::Debug, Components::G2, "Issuing query key correction for %s.", qPrintable(addr.toStringWithPort()));
#endif // LOG_QUERY_HANDLING

		G2Packet* pQKA = G2Packet::newPacket("QKA", true);
		pQKA->writePacket("QK", 4)->writeIntLE<quint32>(QueryKeys.create(pQuery->m_oEndpoint));

		if( addr != pQuery->m_oEndpoint )
		{
			pQKA->writePacket("SNA", (pQuery->m_oEndpoint.protocol() == QAbstractSocket::IPv6Protocol ? 18 : 6))->writeHostAddress(pQuery->m_oEndpoint);
		}
		sendPacket(addr, pPacket);
		pQKA->release();

		return;
	}

	if( !networkG2.m_oRoutingTable.add(pQuery->m_oGUID, pQuery->m_oEndpoint) )
	{
#if LOG_QUERY_HANDLING
		qDebug() << "Query already processed, ignoring";
#endif // LOG_QUERY_HANDLING
		G2Packet* pQA = Neighbours.createQueryAck(pQuery->m_oGUID, false, 0, false);
		sendPacket(pQuery->m_oEndpoint, pQA, true);
		pQA->release();
		return;
	}

#if LOG_QUERY_HANDLING
	qDebug() << "[datagrams.cpp line 1076] Processing query from: " << qPrintable(addr.toStringWithPort());
#endif // LOG_QUERY_HANDLING

	// just in case
	if( pQuery->m_oEndpoint == networkG2.m_oAddress )
	{
		systemLog.postLog( LogSeverity::Error, Components::Network,
						   "Q2 received via UDP and return address points to us, changing return address to source %s",
						   qPrintable( addr.toStringWithPort() ) );
		G2Packet* pUDP = G2Packet::newPacket("UDP");
		pUDP->writeHostAddress(addr);
		pUDP->writeIntLE<quint32>(0);
		pPacket->addOrReplaceChild("UDP", pUDP);
	}

	Neighbours.m_pSection.lock();
	G2Packet* pQA = Neighbours.createQueryAck(pQuery->m_oGUID);
	sendPacket(pQuery->m_oEndpoint, pQA, true);
	pQA->release();

	Neighbours.routeQuery(pQuery, pPacket);
	Neighbours.m_pSection.unlock();

	// local search
}
