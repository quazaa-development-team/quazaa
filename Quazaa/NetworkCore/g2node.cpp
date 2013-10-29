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

#include "g2node.h"
#include "network.h"
#include "neighbours.h"
#include "g2packet.h"
#include "parser.h"
#include "datagrams.h"
#include "searchmanager.h"
#include "Hashes/hash.h"
#include "query.h"
#include "queryhit.h"
#include "queryhashtable.h"
#include "queryhashmaster.h"
#include "hubhorizon.h"
#include "Security/securitymanager.h"

#include "HostCache/hostcache.h"

#include "quazaasettings.h"
#include "quazaaglobals.h"

#include "debug_new.h"

//#define _DISABLE_COMPRESSION

CG2Node::CG2Node(QObject* parent) :
	CNeighbour(parent),
	m_pHubGroup(new CHubHorizonGroup)
{

	m_nProtocol = dpG2;

	m_nType = G2_UNKNOWN;

	m_nLeafCount = m_nLeafMax = 0;
	m_bAcceptDeflate = false;
	m_tLastQuery = 0;
	m_tKeyRequest = 0;
	m_bCachedKeys = false;
	m_bG2Core = false;

	m_pLocalTable = 0;
	m_pRemoteTable = 0;

	m_nHAWWait = 0;
}

CG2Node::~CG2Node()
{
	Network.m_oRoutingTable.Remove(this);

	while(m_lSendQueue.size())
	{
		m_lSendQueue.dequeue()->Release();
	}

	if(m_pLocalTable)
	{
		delete m_pLocalTable;
	}

	if(m_pRemoteTable)
	{
		delete m_pRemoteTable;
	}

	delete m_pHubGroup;
}

void CG2Node::SendPacket(G2Packet* pPacket, bool bBuffered, bool bRelease)
{
	ASSUME_LOCK(Neighbours.m_pSection);

	m_nPacketsOut++;

	if(bBuffered)
	{
		while(m_lSendQueue.size() > 128)
		{
			m_lSendQueue.dequeue()->Release();
		}

		pPacket->AddRef();
		m_lSendQueue.enqueue(pPacket);
	}
	else
	{
		pPacket->ToBuffer(GetOutputBuffer());
	}

	if(bRelease)
	{
		pPacket->Release();
	}

	emit readyToTransfer();
}

void CG2Node::OnConnect()
{
	//QMutexLocker l(&Neighbours.m_pSection);

	systemLog.postLog ( LogSeverity::Information, Components::G2,
						"Connection with %s established, handshaking...",
						qPrintable( m_oAddress.toString() ) );

	m_nState = nsHandshaking;
	emit NodeStateChanged();

	QByteArray sHs;

	sHs += "GNUTELLA CONNECT/0.6\r\n";
	sHs += "Accept: application/x-gnutella2\r\n";
	sHs += "User-Agent: " + CQuazaaGlobals::USER_AGENT_STRING() + "\r\n";
	sHs += "Remote-IP: " + m_oAddress.toString() + "\r\n";
	sHs += "Listen-IP: " + Network.GetLocalAddress().toStringWithPort() + "\r\n";
	if(Neighbours.IsG2Hub())
	{
		sHs += "X-Ultrapeer: True\r\n";
		sHs += "X-Hub: True\r\n";
	}
	else
	{
		sHs += "X-Ultrapeer: False\r\n";
		sHs += "X-Hub: False\r\n";
	}
#ifndef _DISABLE_COMPRESSION
	sHs += "Accept-Encoding: deflate\r\n";
#endif

	sHs += "\r\n";

	//qDebug() << "Handshake send:\n" << sHs;

	m_sHandshake += "Handshake out:\n" + sHs;

	Write(sHs);
}

void CG2Node::OnRead()
{

	QMutexLocker l(&Neighbours.m_pSection);

	//qDebug() << "CG2Node::OnRead";
	if(m_nState == nsHandshaking)
	{
		if(Peek(bytesAvailable()).indexOf("\r\n\r\n") != -1)
		{
			if(m_bInitiated)
			{
				ParseOutgoingHandshake();
			}
			else
			{
				ParseIncomingHandshake();
			}
		}
	}
	else if(m_nState == nsConnected)
	{

		G2Packet* pPacket = 0;
		try
		{
			while((pPacket = G2Packet::ReadBuffer(GetInputBuffer())))
			{
				m_tLastPacketIn = time(0);
				m_nPacketsIn++;

				OnPacket(pPacket);

				pPacket->Release();
			}
		}
		catch(...)
		{
			if(pPacket)
			{
				systemLog.postLog(LogSeverity::Debug, Components::G2, QString("%1").arg(pPacket->Dump()));
				pPacket->Release();
			}

			systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Packet error - %1").arg(m_oAddress.toString()));
			Close();
		}
	}

}

void CG2Node::OnTimer(quint32 tNow)
{

	CNeighbour::OnTimer(tNow);

	if(m_nState == nsConnected)
	{
		if(m_nPingsWaiting == 0 && (tNow - m_tLastPacketIn >= 30 || tNow - m_tLastPingOut >= quazaaSettings.Gnutella2.PingRate))
		{
			// If we got the last packet at least 30 seconds ago
			// or our last ping at least 2 minutes ago
			// and do not expect a response to an earlier ping
			// then we send keep-alive ping, on the occasion of the RTT measurement
			G2Packet* pPacket = G2Packet::New("PI", false);
			SendPacket(pPacket, false, true); // Unbuffered, we can accurately measure the RTT
			m_nPingsWaiting++;
			m_tLastPingOut = tNow;
			m_tRTTTimer.start();
		}

		/*if( m_tKeyRequest > 0 && tNow - m_tKeyRequest > 90 )
		{
			qDebug() << "Closing connection with " << m_oAddress.toString().toLocal8Bit() << "QueryKey wait timeout reached";
			m_nState = nsClosing;
			emit NodeStateChanged();
			Close();
			return;
		}*/

		if ( ( m_nType == G2_HUB && tNow - m_tConnected > 30 ) &&
				(( m_pLocalTable != 0 && m_pLocalTable->m_nCookie != QueryHashMaster.m_nCookie &&
				   tNow - m_pLocalTable->m_nCookie > 60 ) ||
				 ( QueryHashMaster.m_nCookie - m_pLocalTable->m_nCookie > 60 ||
				   !m_pLocalTable->m_bLive ))
		   )
		{
			if(m_pLocalTable->PatchTo(&QueryHashMaster, this))
			{
				systemLog.postLog(LogSeverity::Notice, Components::G2, tr("Sending query routing table to %1 (%2 bits, %3 entries, %4 bytes, %5% full)").arg(m_oAddress.toString().toLocal8Bit().constData()).arg(m_pLocalTable->m_nBits).arg(m_pLocalTable->m_nHash).arg(m_pLocalTable->m_nHash / 8).arg(m_pLocalTable->GetPercent()));
			}
		}

		if(m_nType == G2_HUB && Neighbours.IsG2Hub())
		{
			if(m_nHAWWait == 0)
			{
				SendHAW();
				m_nHAWWait = quazaaSettings.Gnutella2.HAWPeriod;
			}
			else
			{
				m_nHAWWait--;
			}
		}

		// Anti-DDoS
		// cleaning table
		if(m_lRABan.size() >= 1000)
		{
			systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Clearing bans on hub %1").arg(m_oAddress.toString()));
			//qDebug() << "Clearing bans on hub " << m_oAddress.toString();
			for(QHash<quint32, quint32>::iterator itBan = m_lRABan.begin(); itBan != m_lRABan.end();)
			{
				if(tNow - itBan.value() > 60)   // 1 minute
				{
					itBan = m_lRABan.erase(itBan);
				}
				else
				{
					++itBan;
				}
			}
			systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Still active bans: %1").arg(m_lRABan.size()));
			//qDebug() << "Still active bans: " << m_lRABan.size();
		}

	}
}

void CG2Node::ParseIncomingHandshake()
{
	//QMutexLocker l(&Neighbours.m_pSection);

	qint32 nIndex = Peek(bytesAvailable()).indexOf("\r\n\r\n");
	QString sHs = Read(nIndex + 4);

	//qDebug() << "Handshake receive:\n" << sHs;

	m_sHandshake += "Handshake in:\n" + sHs;

	if(m_sUserAgent.isEmpty())
	{
		m_sUserAgent = Parser::GetHeaderValue(sHs, "User-Agent");
	}

	if(m_sUserAgent.isEmpty())
	{
		Send_ConnectError("503 Anonymous clients are not allowed here");
		return;
	}

	if( securityManager.isAgentBlocked(m_sUserAgent) )
	{
		Send_ConnectError("403 Access Denied, sorry");
		securityManager.ban( m_oAddress, Security::ban6Hours, true,
							 QString( "[AUTO] UA Blocked (%1)" ).arg( m_sUserAgent )
#ifdef _DEBUG
							 , QString( "g2node.cpp line 320" )
#endif
							 );
		return;
	}

	if(sHs.startsWith("GNUTELLA CONNECT/0.6"))
	{
		QString sAccept = Parser::GetHeaderValue(sHs, "Accept");
		bool bAcceptG2 = sAccept.contains("application/x-gnutella2");

		if(!bAcceptG2)
		{
			Send_ConnectError("503 Required network not accepted");
			return;
		}

#ifndef _DISABLE_COMPRESSION
		m_bAcceptDeflate = false;
		QString sAcceptEnc = Parser::GetHeaderValue(sHs, "Accept-Encoding");
		if(sAcceptEnc.contains("deflate") && Neighbours.IsG2Hub())
		{
			m_bAcceptDeflate = true;
		}
#endif

		QString sUltra = Parser::GetHeaderValue(sHs, "X-Ultrapeer").toLower();
		//QString sUltraNeeded = Parser::GetHeaderValue(sHs, "X-Ultrapeer-Needed").toLower();
		if(sUltra.isEmpty())
		{
			sUltra = Parser::GetHeaderValue(sHs, "X-Hub").toLower();

			if( sUltra.isEmpty() )
			{
				Send_ConnectError("503 No hub mode specified");
				return;
			}
		}

		QString sRemoteIP = Parser::GetHeaderValue(sHs, "Remote-IP");
		if(!sRemoteIP.isEmpty())
		{
			Network.AcquireLocalAddress(sRemoteIP);
		}
		else
		{
			Send_ConnectError("503 Remote-IP header missing");
			return;
		}

		bool bUltra = (sUltra == "true");

		if(bUltra)
		{
			if(!Neighbours.NeedMoreG2(G2_HUB))
			{
				Send_ConnectError("503 Maximum hub connections reached");
				return;
			}
			m_nType = G2_HUB;
		}
		else
		{
			if( securityManager.isClientBad(m_sUserAgent) )
			{
				Send_ConnectError("403 Your client is too old or otherwise bad. Please upgrade.");
				return;
			}

			if(!Neighbours.NeedMoreG2(G2_LEAF))
			{
				Send_ConnectError("503 Maximum leaf connections reached");
				return;
			}
			m_nType = G2_LEAF;
		}

		Send_ConnectOK(false, m_bAcceptDeflate);

	}
	else if(sHs.contains(" 200 OK"))
	{
		QString sContentType = Parser::GetHeaderValue(sHs, "Content-Type");
		bool bG2Provided = sContentType.contains("application/x-gnutella2");

		if(!bG2Provided)
		{
			Send_ConnectError("503 Required network not provided");
			return;
		}

#ifndef _DISABLE_COMPRESSION
		QString sContentEnc = Parser::GetHeaderValue(sHs, "Content-Encoding");
		if(sContentEnc.contains("deflate"))
		{
			if(!EnableInputCompression())
			{
				systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Inflate init error!"));
				//qDebug() << "Inflate init error!";
				Close();
				return;
			}
		}

		if(m_bAcceptDeflate)
		{
			if(!EnableOutputCompression())
			{
				systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Deflate init error!"));
				//qDebug() << "Deflate init error!";
				Close();
				return;
			}
		}
#endif

		m_nState = nsConnected;
		emit NodeStateChanged();

		SendStartups();
		m_tLastPacketIn = m_tLastPacketOut = time(0);
		if(m_nType == G2_HUB)
		{
			m_pLocalTable = new CQueryHashTable();
		}

	}
	else
	{
		systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Connection to %1 rejected: %2").arg(this->m_oAddress.toString()).arg(sHs.left(sHs.indexOf("\r\n"))));
		//qDebug() << "Connection rejected: " << sHs.left(sHs.indexOf("\r\n"));
		m_nState = nsClosing;
		emit NodeStateChanged();
		Close();
	}
}

void CG2Node::ParseOutgoingHandshake()
{
	//QMutexLocker l(&Neighbours.m_pSection);
	QString sHs = Read(Peek(bytesAvailable()).indexOf("\r\n\r\n") + 4);

	//qDebug() << "Handshake receive:\n" << sHs;

	m_sHandshake += "Handshake in:\n" + sHs;

	QString sAccept = Parser::GetHeaderValue(sHs, "Accept");
	bool bAcceptG2 = sAccept.contains("application/x-gnutella2");

	if(!bAcceptG2)
	{
		Send_ConnectError("503 Required network not accepted");
		return;
	}

	QString sContentType = Parser::GetHeaderValue(sHs, "Content-Type");
	bool bG2Provided = sContentType.contains("application/x-gnutella2");

	if(!bG2Provided)
	{
		Send_ConnectError("503 Required network not provided");
		return;
	}

	m_sUserAgent = Parser::GetHeaderValue(sHs, "User-Agent");

	if(m_sUserAgent.isEmpty())
	{
		Send_ConnectError("503 Anonymous clients are not allowed here");
		return;
	}

	if( securityManager.isAgentBlocked(m_sUserAgent) )
	{
		Send_ConnectError("403 Access Denied, sorry");
		securityManager.ban( m_oAddress, Security::ban6Hours, true,
							 QString( "[AUTO] UA Blocked (%1)" ).arg( m_sUserAgent )
#ifdef _DEBUG
							 , QString( "g2node.cpp line 495" )
#endif
							 );
		return;
	}

	QString sTry = Parser::GetHeaderValue(sHs, "X-Try-Hubs");
	if(bAcceptG2 && bG2Provided && sTry.size())
	{
		hostCache.addXTry( sTry );
	}

	if(sHs.left(16) != "GNUTELLA/0.6 200")
	{
		systemLog.postLog(LogSeverity::Error, Components::G2, QString("Connection to %1 rejected: %2").arg(this->m_oAddress.toString()).arg(sHs.left(sHs.indexOf("\r\n"))));

		// Is it okay to count non-200 response as a failure? Needs some testing...
		hostCache.onFailure(m_oAddress);

		Close();
		return;
	}

	QString sRemoteIP = Parser::GetHeaderValue(sHs, "Remote-IP");
	if(!sRemoteIP.isEmpty())
	{
		Network.AcquireLocalAddress(sRemoteIP);
	}
	else
	{
		Send_ConnectError("503 Remote-IP header missing");
		return;
	}

	QString sUltra = Parser::GetHeaderValue(sHs, "X-Ultrapeer").toLower();

	if( sUltra.isEmpty() )
	{
		sUltra = Parser::GetHeaderValue(sHs, "X-Hub").toLower();
	}

	//QString sUltraNeeded = Parser::GetHeaderValue(sHs, "X-Ultrapeer-Needed").toLower();

	bool bUltra = (sUltra == "true");
	//bool bUltraNeeded = (sUltraNeeded == "true");

#ifndef _DISABLE_COMPRESSION
	QString sContentEnc = Parser::GetHeaderValue(sHs, "Content-Encoding");
	if(sContentEnc.contains("deflate"))
	{
		if(!EnableInputCompression())
		{
			systemLog.postLog(LogSeverity::Debug, Components::G2, "Inflate init error!");
			//qDebug() << "Inflate init error!";
			Close();
			return;
		}
	}
#endif

	bool bAcceptDeflate = false;
#ifndef _DISABLE_COMPRESSION
	QString sAcceptEnc = Parser::GetHeaderValue(sHs, "Accept-Encoding");
	if(sAcceptEnc.contains("deflate") && Neighbours.IsG2Hub())
	{
		bAcceptDeflate = true;
	}
#endif

	if(bUltra)
	{
		if(!Neighbours.NeedMoreG2(G2_HUB))
		{
			Send_ConnectError("503 Maximum hub connections reached");
			return;
		}
		m_nType = G2_HUB;
	}
	else
	{
		if( securityManager.isClientBad(m_sUserAgent) )
		{
			Send_ConnectError("403 Your client is too old or otherwise bad. Please upgrade.");
			return;
		}

		if(!Neighbours.NeedMoreG2(G2_LEAF))
		{
			Send_ConnectError("503 Maximum leaf connections reached");
			return;
		}
		m_nType = G2_LEAF;
	}

	Send_ConnectOK(true, bAcceptDeflate);

	hostCache.updateFailures( m_oAddress, 0 );

#ifndef _DISABLE_COMPRESSION
	if(bAcceptDeflate)
	{
		if(!EnableOutputCompression())
		{
			systemLog.postLog(LogSeverity::Debug, Components::G2, "Deflate init error!");
			//qDebug() << "Deflate init error!";
			Close();
			return;
		}
	}
#endif

	m_nState = nsConnected;
	emit NodeStateChanged();

	SendStartups();
	m_tLastPacketIn = m_tLastPacketOut = time(0);

	if(m_nType == G2_HUB)
	{
		m_pLocalTable = new CQueryHashTable();
	}

	systemLog.postLog(LogSeverity::Information, Components::G2, tr("Gnutella2 connection with %1 established.").arg(qPrintable(m_oAddress.toString())));
}
void CG2Node::Send_ConnectError(QString sReason)
{
	systemLog.postLog(LogSeverity::Information, Components::G2, tr("Rejecting connection with %1: %2 (%3)").arg(qPrintable(m_oAddress.toString())).arg(qPrintable(sReason)).arg(m_sUserAgent));

	QByteArray sHs;

	sHs += "GNUTELLA/0.6 " + sReason + "\r\n";
	sHs += "User-Agent: " + CQuazaaGlobals::USER_AGENT_STRING() + "\r\n";
	sHs += "Accept: application/x-gnutella2\r\n";
	sHs += "Content-Type: application/x-gnutella2\r\n";

	sHs += hostCache.getXTry();
	for(QList<CNeighbour*>::iterator it = Neighbours.begin(); it != Neighbours.end(); ++it)
	{
		CNeighbour* pNeighbour = *it;
		// Add neighbours with free slots, to promote faster connections.
		if( pNeighbour->m_nState == nsConnected
				&& pNeighbour->m_nProtocol == dpG2
				&& ((CG2Node*)pNeighbour)->m_nType == G2_HUB
				&& ((CG2Node*)pNeighbour) ->m_nLeafMax > 0
				&& 100 * ((CG2Node*)pNeighbour)->m_nLeafCount / ((CG2Node*)pNeighbour)->m_nLeafMax < 90 )
		{
			sHs += "," + pNeighbour->m_oAddress.toStringWithPort() + " " + common::getDateTimeUTC().toString("yyyy-MM-ddThh:mmZ");
		}
	}

	sHs += "\r\n\r\n";

	//qDebug() << "Handshake send:\n" << sHs;

	m_sHandshake += "Handshake out:\n" + sHs;

	Write(sHs);

	Close(true);
}
void CG2Node::Send_ConnectOK(bool bReply, bool bDeflated)
{
	QByteArray sHs;

	sHs += "GNUTELLA/0.6 200 OK\r\n";
	sHs += "User-Agent: " + CQuazaaGlobals::USER_AGENT_STRING() + "\r\n";
	if(Neighbours.IsG2Hub())
	{
		sHs += "X-Ultrapeer: True\r\n";
		sHs += "X-Hub: True\r\n";
	}
	else
	{
		sHs += "X-Ultrapeer: False\r\n";
		sHs += "X-Hub: False\r\n";
	}
	sHs += "Content-Type: application/x-gnutella2\r\n";

	if(!bReply)
	{
		// 2-handshake
#ifndef _DISABLE_COMPRESSION
		if(Neighbours.IsG2Hub() && m_nType == G2_HUB)
		{
			sHs += "Accept-Encoding: deflate\r\n";
		}

		if(bDeflated)
		{
			sHs += "Content-Encoding: deflate\r\n";
		}
#endif
		sHs += "Accept: application/x-gnutella2\r\n";
		sHs += "Remote-IP: ";
		sHs += m_oAddress.toString();
		sHs += "\r\n";
		sHs += "Listen-IP: " + Network.GetLocalAddress().toStringWithPort() + "\r\n";
	}
	else
	{
#ifndef _DISABLE_COMPRESSION
		if(bDeflated)
		{
			sHs += "Content-Encoding: deflate\r\n";
		}
#endif
	}

	sHs += "\r\n";

	//qDebug() << "Handshake send:\n" << sHs;

	m_sHandshake += "Handshake out:\n" + sHs;

	Write(sHs);

}

void CG2Node::SendStartups()
{
	if(Network.IsListening())
	{
		CEndPoint addr = Network.GetLocalAddress();
		G2Packet* pPacket = G2Packet::New("PI", true);
		pPacket->WritePacket("UDP", 6);
		pPacket->WriteHostAddress(addr);
		pPacket->WritePacket("TFW", 0);
		SendPacket(pPacket, false, true);
	}

	SendLNI();
}

void CG2Node::SendLNI()
{
	G2Packet* pLNI = G2Packet::New("LNI", true);
	if(Network.m_oAddress.protocol() == 0)
	{
		pLNI->WritePacket("NA", 6)->WriteHostAddress(Network.m_oAddress);
	}
	else
	{
		pLNI->WritePacket("NA", 18)->WriteHostAddress(Network.m_oAddress);
	}
	pLNI->WritePacket("GU", 16)->WriteGUID(quazaaSettings.Profile.GUID);
	pLNI->WritePacket("V", 4)->WriteString(CQuazaaGlobals::VENDOR_CODE(), false);

	if(Neighbours.IsG2Hub())
	{
		quint16 nLeavesMax = quazaaSettings.Gnutella2.NumLeafs;
		quint16 nLeaves = Neighbours.m_nLeavesConnectedG2;
		pLNI->WritePacket("HS", 4);
		pLNI->WriteIntLE<quint16>(nLeaves);
		pLNI->WriteIntLE<quint16>(nLeavesMax);
		pLNI->WritePacket("QK", 0);
	}

	pLNI->WritePacket("g2core", 0);

	SendPacket(pLNI, false, true);
}

void CG2Node::OnPacket(G2Packet* pPacket)
{
	//qDebug() << "Got packet " << pPacket->GetType() << pPacket->ToHex() << pPacket->ToASCII();

	//try
	//{
	if(!Network.RoutePacket(pPacket))
	{

		if(pPacket->IsType("PI"))
		{
			OnPing(pPacket);
		}
		else if(pPacket->IsType("PO"))
		{
			OnPong(pPacket);
		}
		else if(pPacket->IsType("LNI"))
		{
			OnLNI(pPacket);
		}
		else if(pPacket->IsType("KHL"))
		{
			OnKHL(pPacket);
		}
		else if(pPacket->IsType("QHT"))
		{
			OnQHT(pPacket);
		}
		else if(pPacket->IsType("Q2"))
		{
			OnQuery(pPacket);
		}
		else if(pPacket->IsType("QKR"))
		{
			OnQKR(pPacket);
		}
		else if(pPacket->IsType("QKA"))
		{
			OnQKA(pPacket);
		}
		else if(pPacket->IsType("QA"))
		{
			OnQA(pPacket);
		}
		else if(pPacket->IsType("QH2"))
		{
			OnQH2(pPacket);
		}
		else if(pPacket->IsType("HAW"))
		{
			OnHaw(pPacket);
		}
		else
		{
			systemLog.postLog(LogSeverity::Debug, Components::G2, QString("G2 TCP recieved unknown packet %1").arg(pPacket->GetType()));
			//qDebug() << "Unknown packet " << pPacket->GetType();
		}
	}
	/*}
	catch(...)
	{
		qDebug() << "Packet error in child packets";
	}*/
}

void CG2Node::OnPing(G2Packet* pPacket)
{
	ASSUME_LOCK(Neighbours.m_pSection);

	bool bUdp = false;
	bool bRelay = false;
	//	bool bTestFirewall = false;
	CEndPoint addr;

	if(pPacket->m_bCompound)
	{
		char szType[9];
		quint32 nLength = 0, nNext = 0;

		while(pPacket->ReadPacket(&szType[0], nLength))
		{
			nNext = pPacket->m_nPosition + nLength;

			if(strcmp("UDP", szType) == 0 && nLength >= 6)
			{
				pPacket->ReadHostAddress(&addr);
				if(addr.isValid())
				{
					bUdp = true;
				}
			}
			else if(strcmp("RELAY", szType) == 0)
			{
				bRelay = true;
			}
			else if(strcmp("TFW", szType) == 0)
			{
				//				bTestFirewall = true;
			}

			pPacket->m_nPosition = nNext;
		}

	}

	if(!bUdp && !bRelay)
	{
		// direct ping
		G2Packet* pPong = G2Packet::New("PO", false);
		SendPacket(pPong, false, true);
		return;
	}

	if(bUdp && !bRelay)
	{
		// /PI/UDP

		if(Neighbours.IsG2Hub()) // If we are a hub.
		{
			G2Packet* pRelay = G2Packet::New("RELAY");
			pPacket->PrependPacket(pRelay);

			int nRelayed = 0, nCount = Neighbours.GetCount();
			QList<int> lToRelayIndex;

			for(int i = 0; i < nCount && nRelayed < quazaaSettings.Gnutella2.PingRelayLimit; ++i)
			{
				int nIndex = qrand() % nCount;
				if(!lToRelayIndex.contains(nIndex))
				{
					CNeighbour* pNode = Neighbours.GetAt(nIndex);
					if(pNode != this
							&& pNode->m_nProtocol == dpG2
							&& pNode->m_nState == nsConnected
							&& static_cast<CG2Node*>(pNode)->m_nType == G2_LEAF )
					{
						pPacket->AddRef();
						((CG2Node*)pNode)->SendPacket(pPacket, true, true);
						nRelayed++;
					}
				}
			}
			return;
		}
	}

	if(bUdp && bRelay)
	{
		G2Packet* pPong = G2Packet::New("PO", true);
		pPong->WritePacket("RELAY", 0);
		Datagrams.SendPacket(addr, pPong, true);
		pPong->Release();
	}
}

void CG2Node::OnPong(G2Packet* pPacket)
{
	Q_UNUSED(pPacket);

	if(m_nPingsWaiting > 0)
	{
		m_nPingsWaiting--;

		if(m_nPingsWaiting == 0)
		{
			m_tRTT = m_tRTTTimer.elapsed();
		}
	}
}

void CG2Node::OnLNI(G2Packet* pPacket)
{
	if(!pPacket->m_bCompound)
	{
		return;
	}

	bool hasNA = false;
	QUuid pGUID;
	bool hasGUID = false;

	CEndPoint hostAddr;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while(pPacket->ReadPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("NA", szType) == 0 && nLength >= 6)
		{
			if(nLength >= 18)
			{
				pPacket->ReadHostAddress(&hostAddr, false);
			}
			else
			{
				pPacket->ReadHostAddress(&hostAddr);
			}

			if(hostAddr.isValid())
			{
				hasNA = true;

				if(!m_bInitiated)
				{
					m_oAddress = hostAddr;
				}
			}

		}
		else if(strcmp("GU", szType) == 0 && nLength >= 16)
		{
			hasGUID = true;
			pGUID = pPacket->ReadGUID();
		}
		else if(strcmp("HS", szType) == 0 && nLength >= 4)
		{
			if(m_nType == G2_HUB)
			{
				m_nLeafCount = pPacket->ReadIntLE<quint16>();
				m_nLeafMax = pPacket->ReadIntLE<quint16>();
			}
		}
		else if(strcmp("QK", szType) == 0)
		{
			m_bCachedKeys = true;
		}
		else if(strcmp("g2core", szType) == 0)
		{
			m_bG2Core = true;
		}

		pPacket->m_nPosition = nNext;
	}

	if(hasNA && hasGUID)
	{
		QMutexLocker l(&Network.m_pSection);
		Network.m_oRoutingTable.Add(pGUID, this, true);
	}

}
void CG2Node::OnKHL(G2Packet* pPacket)
{
	if(!pPacket->m_bCompound)
	{
		return;
	}

	const quint32 tNow = common::getTNowUTC();
	qint32 nDiff = 0;

	m_pHubGroup->Clear();

	char szType[9], szInner[9];
	quint32 nLength = 0, nInnerLength = 0;
	bool bCompound = false;
	quint32 nNext = 0, nInnerNext = 0;

	while ( pPacket->ReadPacket(&szType[0], nLength, &bCompound ) )
	{
		nNext = pPacket->m_nPosition + nLength;

		// NH - Neighbouring Hub
		if(strcmp("NH", szType) == 0)
		{
			// Child packet types:
			// /KHL/NH/GU - GUID
			// /KHL/NH/V  - Vendor Code
			// /KHL/NH/LS - Library Statistics
			// /KHL/NH/HS - Hub Status

			QUuid pGUID;

			if(bCompound)
			{
				while(pPacket->m_nPosition < nNext && pPacket->ReadPacket(&szInner[0], nInnerLength))
				{
					nInnerNext = pPacket->m_nPosition + nInnerLength;

					if(strcmp("GU", szInner) == 0 && nInnerLength >= 16)
					{
						pGUID = pPacket->ReadGUID();
					}

					pPacket->m_nPosition = nInnerNext;
				}

				nLength = nNext - pPacket->m_nPosition;
			}

			if(nLength >= 6)
			{
				CEndPoint pAddr;

				if(nLength >= 18)
				{
					pPacket->ReadHostAddress(&pAddr, false);
				}
				else
				{
					pPacket->ReadHostAddress(&pAddr);
				}

				if(m_nType == G2_HUB)
				{
					m_pHubGroup->Add(pAddr);
				}

				if(!pGUID.isNull())
				{
					QMutexLocker l(&Network.m_pSection);

					Network.m_oRoutingTable.Add(pGUID, this, &pAddr, false);
				}
			}
		}
		// CH - Cached Hub
		else if(strcmp("CH", szType) == 0)
		{
			// Child packet types:
			// /KHL/CH/GU - GUID
			// /KHL/CH/V  - Vendor Code
			// /KHL/CH/LS - Library Statistics
			// /KHL/CH/HS - Hub Status

			QString sVendor;

			if( bCompound )
			{
				while(pPacket->m_nPosition < nNext && pPacket->ReadPacket(&szInner[0], nInnerLength))
				{
					nInnerNext = pPacket->m_nPosition + nInnerLength;

					if(strcmp("V", szInner) == 0 && nInnerLength >= 4)
					{
						sVendor = pPacket->ReadString(4);
					}

					pPacket->m_nPosition = nInnerNext;
				}

				nLength = nNext - pPacket->m_nPosition;
			}

			if(nLength >= 10)
			{
				CEndPoint ep;
				// quint32 nTs = 0;

				if(nLength >= 22)
				{
					pPacket->ReadHostAddress(&ep, false);
				}
				else
				{
					pPacket->ReadHostAddress(&ep);
				}

				/*nTs = */pPacket->ReadIntLE<quint32>();

				if ( !sVendor.isEmpty() && securityManager.isVendorBlocked( sVendor ) )
				{
					// TODO: Investigate: This is the cause of many bans of already banned IPs...
					securityManager.ban( ep, Security::ban6Hours, true,
										 QString( "[AUTO] Vendor blocked (%1)" ).arg( sVendor )
#ifdef _DEBUG
										 , QString( "g2node.cpp line 1131" )
#endif
										 );
				}
				else
				{
					hostCache.add( ep, nDiff + tNow );
				}
			}
		}
		// TS - Timestamp
		else if ( strcmp( "TS", szType ) == 0 )
		{
			// Note: No child packets

			if(bCompound)
			{
				pPacket->SkipCompound(nLength);
			}

			if(nLength >= 4)
			{
				nDiff = tNow - pPacket->ReadIntLE<quint32>();
			}
		}
		pPacket->m_nPosition = nNext;
	}
}

void CG2Node::OnQHT(G2Packet* pPacket)
{
	if(m_pRemoteTable == 0)
	{
		if(!Neighbours.IsG2Hub())
		{
			systemLog.postLog(LogSeverity::Debug, Components::G2, QString("Recieved unexpected Query Routing Table, ignoring"));
			//qDebug() << "Received unexpected Query Routing Table, ignoring";
			return;
		}

		m_pRemoteTable = new CQueryHashTable();
	}

	bool bLive = m_pRemoteTable->m_bLive;

	if(!m_pRemoteTable->OnPacket(pPacket))
	{
		systemLog.postLog(LogSeverity::Error, Components::G2, tr("Neighbour %1 sent bad query hash table update. Closing connection.").arg(m_oAddress.toString().toLocal8Bit().constData()));
		Close();
		return;
	}

	if(m_pRemoteTable->m_bLive && !bLive)
	{
		systemLog.postLog(LogSeverity::Notice, Components::G2, tr("Neighbour %1 updated its query hash table. %2 bits %3% full.").arg(m_oAddress.toString().toUtf8().constData()).arg(m_pRemoteTable->m_nBits).arg(m_pRemoteTable->GetPercent()));
	}

	if(m_nType == G2_LEAF && m_pRemoteTable && m_pRemoteTable->GetPercent() > 90)
	{
		systemLog.postLog(LogSeverity::Error, Components::G2, tr("Dropping neighbour %1 - hash table fill percentage too high.").arg(m_oAddress.toString().toLocal8Bit().constData()));
		Close();
		return;
	}

	if(m_nType == G2_LEAF && m_pRemoteTable->m_pGroup == 0)
	{
		QueryHashMaster.Add(m_pRemoteTable);
	}
}

void CG2Node::OnQKR(G2Packet* pPacket)
{
	if(!pPacket->m_bCompound || m_nType != G2_LEAF)
	{
		return;
	}

	char szType[9];
	quint32 nLength = 0, nNext = 0;
	bool bCacheOK = true;
	CEndPoint addr;

	while(pPacket->ReadPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("QNA", szType) == 0 && nLength >= 6)
		{
			if(nLength >= 18)
			{
				pPacket->ReadHostAddress(&addr, false);
			}
			else
			{
				pPacket->ReadHostAddress(&addr);
			}
		}
		else if(strcmp("REF", szType) == 0)
		{
			bCacheOK = false;
		}

		pPacket->m_nPosition = nNext;
	}

	if ( addr.isFirewalled() )
	{
		return;
	}

	const quint32 tNow = common::getTNowUTC();

//#if ENABLE_HOST_CACHE_BENCHMARKING
//	QElapsedTimer tHostCache;
//	tHostCache.start();
//#endif

	QMutexLocker l( &hostCache.m_pSection );

//#if ENABLE_HOST_CACHE_BENCHMARKING
//	qint64 tHCLock = tHostCache.elapsed();
//	QElapsedTimer tHostCacheWork;
//	tHostCacheWork.start();
//#endif

	CG2HostCacheHost* pHost = bCacheOK ? hostCache.get( addr ) : NULL;

//#if ENABLE_HOST_CACHE_BENCHMARKING
//	qint64 tHCWork = tHostCacheWork.elapsed();
//#endif

	if ( pHost && pHost->queryKey() && pHost->keyHost() == Network.m_oAddress &&
		 tNow - pHost->keyTime() < quazaaSettings.Gnutella2.QueryKeyTime )
	{
		G2Packet* pQKA = G2Packet::New( "QKA", true );
		if ( addr.protocol() == 0 )
		{
			pQKA->WritePacket( "QNA", 6 )->WriteHostAddress( addr );
		}
		else
		{
			pQKA->WritePacket( "QNA", 18 )->WriteHostAddress( addr );
		}
		pQKA->WritePacket( "QK", 4 )->WriteIntLE<quint32>( pHost->queryKey() );
		pQKA->WritePacket( "CACHED", 0 );
		SendPacket( pQKA, true, true );
	}
	else
	{
		G2Packet* pQKR = G2Packet::New( "QKR", true );
		if ( addr.protocol() == 0 )
		{
			pQKR->WritePacket( "SNA", 6 )->WriteHostAddress( m_oAddress );
		}
		else
		{
			pQKR->WritePacket( "SNA", 18 )->WriteHostAddress( m_oAddress );
		}

		if ( Network.m_oAddress.protocol() == 0 )
		{
			pQKR->WritePacket( "RNA", 6 )->WriteHostAddress( Network.m_oAddress );
		}
		else
		{
			pQKR->WritePacket( "RNA", 18 )->WriteHostAddress( Network.m_oAddress );
		}

		Datagrams.SendPacket( addr, pQKR, false );
		pQKR->Release();
	}

//#if ENABLE_HOST_CACHE_BENCHMARKING
//	l.unlock();
//	hostCache.m_nLockWaitTime.fetchAndAddRelaxed( tHCLock );
//	hostCache.m_nWorkTime.fetchAndAddRelaxed( tHCWork );
//#endif
}

void CG2Node::OnQKA(G2Packet* pPacket)
{
	if ( !pPacket->m_bCompound )
	{
		return;
	}

	m_tKeyRequest = 0;

	quint32 nKey = 0;
	CEndPoint addr;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while ( pPacket->ReadPacket( &szType[0], nLength ) )
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("QK", szType) == 0 && nLength >= 4)
		{
			nKey = pPacket->ReadIntLE<quint32>();
		}
		else if(strcmp("QNA", szType) == 0)
		{
			if(nLength >= 6)
			{
				if(nLength >= 18)
				{
					// IPv6 with port
					pPacket->ReadHostAddress(&addr, false);
				}
				else if(nLength >= 16)
				{
					// IPv6 without port
					Q_IPV6ADDR ip6;
					pPacket->Read(&ip6, 16);
					addr.setAddress(ip6);
				}
				else
				{
					// IPv4 with port
					pPacket->ReadHostAddress(&addr);
				}
			}
			else if(nLength >= 4)
			{
				// IPv4 without port
				quint32 ip4 = pPacket->ReadIntBE<quint32>();
				addr.setAddress(ip4);
				addr.setPort(6346);
			}
		}

		pPacket->m_nPosition = nNext;
	}

	const quint32 tNow = common::getTNowUTC();
	hostCache.addKey( addr, tNow, &m_oAddress, nKey, tNow );

#if LOG_QUERY_HANDLING
		systemLog.postLog( LogSeverity::Debug,
						   QString( "Got a query key from %1 via %2 = 0x%3" ).arg(
							   addr.toString().toLocal8Bit().constData()).arg(m_oAddress.toString().toLocal8Bit().constData()).arg(QString().number(nKey, 16)));
		//qDebug("Got a query key from %s via %s = 0x%x", addr.toString().toLocal8Bit().constData(), m_oAddress.toString().toLocal8Bit().constData(), nKey);
#endif // LOG_QUERY_HANDLING
}
void CG2Node::OnQA(G2Packet* pPacket)
{
	QUuid oGUID;
	SearchManager.OnQueryAcknowledge(pPacket, m_oAddress, oGUID);

	// TCP /QA - no need for routing, it's either for us or to be dropped
}

void CG2Node::OnQH2(G2Packet* pPacket)
{
	if(!pPacket->m_bCompound)
	{
		return;
	}

	QueryHitInfo* pInfo = CQueryHit::ReadInfo(pPacket, &m_oAddress);

	if( securityManager.isVendorBlocked( pInfo->m_sVendor ) ) // Block foxy client search results. We can't download from them any way.
	{
		securityManager.ban( pInfo->m_oNodeAddress, Security::ban6Hours, true,
							 QString( "[AUTO] Vendor blocked (%1)" ).arg( pInfo->m_sVendor )
#ifdef _DEBUG
							 , QString( "g2node.cpp line 1371" )
#endif
							 );
	} else {
		if(SearchManager.OnQueryHit(pPacket, pInfo))
		{
			Network.m_pSection.lock();

			if(Neighbours.IsG2Hub() && pInfo->m_nHops < 7)
			{
				Network.m_oRoutingTable.Add(pInfo->m_oNodeGUID, this, false);
				pPacket->m_pBuffer[pPacket->m_nLength - 17]++;
				Network.RoutePacket(pInfo->m_oGUID, pPacket);
			}

			Network.m_pSection.unlock();

			delete pInfo;
		}
	}
}

void CG2Node::OnQuery(G2Packet* pPacket)
{
	if(!pPacket->m_bCompound)
	{
		return;
	}

	CQueryPtr pQuery = CQuery::FromPacket(pPacket);

	if ( pQuery.isNull() )
	{
#if LOG_MALFORMATTED_QUERIES
		systemLog.postLog( LogSeverity::Error,  Components::G2,
						   "Received malformatted query from neighbour %s, ignoring.",
						   qPrintable( m_oAddress.toString() ) );
#endif // LOG_QUERY_HANDLING
		return;
	}

	/*
	 * Anti-DDoS
	 * Code below is to minimize the impact caused by forwarded searches.
	 * We restrict number of queries with a given return address sent by this neighbouring hub.
	 * We allow unlimited queried for this connected hub (when its address is specified in query).
	 * Our hub can still be searched via other neighbouring hubs.
	 * This is to protect inter-hub query forwarding where nothing is checked.
	 * The main problem is still there, though.
	 */

	/*if( Neighbours.IsG2Hub() && m_nType == G2_HUB ) // must be both hubs
	{
		QHash<quint32,quint32>::const_iterator itEntry = m_lRABan.find(oReturnAddr.ip);
		if( itEntry != m_lRABan.end() && time(0) - itEntry.value() < 60 && itEntry.key() != oReturnAddr.ip )
		{
			qDebug() << "Dropping query for return address " << oReturnAddr.toString() << "on hub" << m_oAddress.toString();
			return; // drop excess packets
		}
		m_lRABan.insert(oReturnAddr.ip, time(0));
		qDebug() << "Banning return address" << oReturnAddr.toString() << "on hub " << m_oAddress.toString() << " num banned: " << m_lRABan.size();
	}*/

	if(Network.m_oRoutingTable.Add(pQuery->m_oGUID, this, false))
	{
		if( m_nType == G2_LEAF )
		{
			SendPacket(Neighbours.CreateQueryAck(pQuery->m_oGUID, true, this, true), false, true);
		}

		if( Neighbours.IsG2Hub() )
		{
			Neighbours.RouteQuery(pQuery, pPacket, this, (m_nType != G2_HUB));
		}
	}
}

qint64 CG2Node::writeToNetwork(qint64 nBytes)
{
	qint64 nTotalSent = 0;

	do
	{
		if(GetOutputBuffer()->isEmpty() && !m_lSendQueue.isEmpty())
		{
			G2Packet* pPacket = m_lSendQueue.dequeue();
			pPacket->ToBuffer(GetOutputBuffer());
			pPacket->Release();
		}

		qint64 nSent = CNeighbour::writeToNetwork(nBytes - nTotalSent);

		if(nSent <= 0)
		{
			return nTotalSent ? nTotalSent : nSent;
		}

		nTotalSent += nSent;
	}
	while(nTotalSent < nBytes);

	return nTotalSent;
}

void CG2Node::SendHAW()
{
	G2Packet* pPacket = G2Packet::New("HAW");

	pPacket->WritePacket("NA", Network.m_oAddress.protocol() == QAbstractSocket::IPv4Protocol ? 6 : 18);
	pPacket->WriteHostAddress(Network.m_oAddress);

	pPacket->WritePacket("V", 4);
	pPacket->WriteString(CQuazaaGlobals::VENDOR_CODE());

	pPacket->WritePacket("HS", 2);
	pPacket->WriteIntLE<quint16>(Neighbours.m_nLeavesConnectedG2);

	pPacket->WriteByte(0); // end of child packets
	pPacket->WriteByte(100);
	pPacket->WriteByte(0);
	QUuid oGUID = QUuid::createUuid();
	pPacket->WriteGUID(oGUID);

	SendPacket(pPacket, false, true);
}

void CG2Node::OnHaw(G2Packet *pPacket)
{
	if ( ! pPacket->m_bCompound ) 	return;

	QString strVendor;
	char szType[9];
	quint32 nLength = 0, nNext = 0;

	CEndPoint addr;

	while ( pPacket->ReadPacket( &szType[0], nLength ) )
	{
		nNext = pPacket->m_nPosition + nLength;

		if ( strcmp("V", szType) == 0 && nLength >= 4 )
		{
			strVendor = pPacket->ReadString( 4 );
		}
		else if ( strcmp("NA", szType) == 0 )
		{
			if(nLength >= 6)
			{
				if(nLength >= 18)
				{
					// IPv6 with port
					pPacket->ReadHostAddress(&addr, false);
				}
				else
				{
					// IPv4 with port
					pPacket->ReadHostAddress(&addr);
				}
			}
		}

		pPacket->m_nPosition = nNext;
	}

	if ( pPacket->GetRemaining() < 2 + 16 ) return;

	BYTE* pPtr	= pPacket->m_pBuffer + pPacket->m_nPosition;
	BYTE nTTL	= pPacket->ReadByte();
	BYTE nHops	= pPacket->ReadByte();

	/*	QUuid oGUID = */pPacket->ReadGUID();

	if( !strVendor.isEmpty() && securityManager.isVendorBlocked(strVendor) )
	{
		securityManager.ban( addr, Security::ban6Hours, true,
							 QString( "[AUTO] Vendor blocked (%1)" ).arg( strVendor )
#ifdef _DEBUG
							 , QString( "g2node.cpp line 1545" )
#endif
							 );
		return;	// We don't want to propagate these...
	}
	else
	{
		hostCache.add( addr, common::getTNowUTC() );
	}

	if ( nTTL > 0 && nHops < 255 )
	{
		//m_pGUIDCache->Add( oGUID, this );

		pPtr[0] = nTTL  - 1;
		pPtr[1] = nHops + 1;

		if ( CG2Node* pNeighbour = (CG2Node*)Neighbours.RandomNode( dpG2, G2_HUB,  this ) )
		{
			pNeighbour->SendPacket( pPacket, false, false );
		}
	}
}

