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

#include "chatsessiong2.h"

#include "quazaaglobals.h"
#include "quazaasettings.h"
#include "parser.h"
#include "g2packet.h"
#include "network.h"
#include <QXmlStreamReader>
#include "chatconverter.h"
#include <QTextDocument>

#ifdef _DEBUG
#include "debug_new.h"
#endif

/*
 *	Gnutella2 after handshake sequence
 *	L - local client (initiating chat connection)
 *	R - remote client
 *
 *	1. L => R /UPROC (mandatory)
 *	1. L <= R /UPROC (mandatory)
 *	2. L <= R /UPROD/XML with user profile (gprofile, xml, mandatory)
 *	2. L => R /UPROD/XML with gprofile (mandatory)
 *	3. L => R /CHATREQ (mandatory)
 *			  /CHATREQ/USERGUID (with R's raw GUID, read from R's /UPROD packet, mandatory)
 *	4. L <= R /CHATANS (mandatory)
 *			  /CHATANS/USERGUID (with R's raw GUID, mandatory)
 *			  /CHATANS/AWAY (optional, sent when R is away, payload text representation of idle time)
 *			  /CHATANS/ACCEPT (optional, sent if /?/USERGUID match and R wants to accept chat)
 *			  /CHATANS/DENY (optional, sent if /?/USERGUID do not match or R does not want to chat, in this case connection should be closed)
 *
 *	Exchanging messages
 *	/CMSG
 *	/CMSG/BODY (required, the message)
 *	/CMSG/ACT (optional, sent on /me events)
 *	/CMSG packet should be sent only after successfully completed above sequence, if sent before, it should be ignored
 *
 */

CChatSessionG2::CChatSessionG2(CEndPoint oRemoteHost, QObject *parent) :
	CChatSession(parent)
{
	m_oRemoteHost = m_oAddress = oRemoteHost;
}

void CChatSessionG2::connectNode()
{
	CNetworkConnection::connectTo(m_oRemoteHost);
	CChatSession::connectNode();
}

void CChatSessionG2::onConnectNode()
{
	qDebug() << "OnConnect";

	emit systemMessage("Connected, handshaking...");

	qDebug() << "sending handshake";
	m_nState = csHandshaking;

	QByteArray baHs;

	baHs = "CHAT CONNECT/0.2\r\n";
	baHs+= "Accept: application/x-gnutella2\r\n";
		baHs+= "User-Agent: " + CQuazaaGlobals::USER_AGENT_STRING() + "\r\n";
	baHs+= "Listen-IP: " + Network.getLocalAddress().toStringWithPort() + "\r\n\r\n";

	write(baHs);

	qDebug() << "Chat send:\n" << baHs;
}
void CChatSessionG2::onDisconnectNode()
{
	CChatSession::onDisconnectNode();
}

void CChatSessionG2::onRead()
{
	if( m_nState == csHandshaking )
	{
		if(peek(bytesAvailable()).indexOf("\r\n\r\n") != -1)
		{
			if(m_bInitiated)
			{
				parseOutgoingHandshake();
			}
			else
			{
				//ParseIncomingHandshake();
			}
		}
	}
	else if( m_nState == csConnected || m_nState == csActive )
	{
		G2Packet* pPacket = 0;
		try
		{
			while((pPacket = G2Packet::readBuffer(getInputBuffer())))
			{
				onPacket(pPacket);

				pPacket->release();
			}
		}
		catch(...)
		{
			if(pPacket)
			{
				pPacket->release();
			}

			emit systemMessage("Received corrupted G2 packet, connection lost.");

			CNetworkConnection::close();
		}
	}
}

void CChatSessionG2::parseOutgoingHandshake()
{
	QString sHs = read(peek(bytesAvailable()).indexOf("\r\n\r\n") + 4);

	qDebug() << "Chat received:\n" << sHs;

	if( sHs.left(12) == "CHAT/0.2 200" )
	{
		QString sAccept = Parser::getHeaderValue(sHs, "Accept");
		if( !sAccept.contains("application/x-gnutella2") )
		{
			send_ChatError("503 Required protocol not accepted");
			CNetworkConnection::close(true);
			emit systemMessage("Remote host does not support Gnutella2");
			return;
		}

		QString sContentType = Parser::getHeaderValue(sHs, "Content-Type");
		if( !sContentType.contains("application/x-gnutella2") )
		{
			send_ChatError("503 Required protocol not provided");
			CNetworkConnection::close(true);
			emit systemMessage("Remote host does not support Gnutella2");
			return;
		}

		QString sUA = Parser::getHeaderValue(sHs, "User-Agent");
		if( sUA.indexOf("shareaza", 0, Qt::CaseInsensitive) != -1 )
			m_bShareaza = true;

		emit systemMessage("Exchanging profile information...");
		m_nState = csConnected;
		send_ChatOK(true);
		sendStartups();
		return;
	}

	qDebug() << m_oAddress.toStringWithPort() << "does not accept chat";

	close();
}

void CChatSessionG2::send_ChatOK(bool bReply)
{
	QByteArray sHs;

	sHs = "CHAT/0.2 200 OK\r\n";
	if( !bReply )
	{
		// 2nd header
		sHs+= "Accept: application/x-gnutella2\r\n";
				sHs+= "User-Agent: " + CQuazaaGlobals::USER_AGENT_STRING() + "\r\n";
		sHs+= "Listen-IP: " + Network.getLocalAddress().toStringWithPort() + "\r\n";
	}
	sHs+= "Content-Type: application/x-gnutella2\r\n\r\n";

	write(sHs);

	qDebug() << "Chat send:\n" << sHs;

}

void CChatSessionG2::send_ChatError(QString sReason)
{
	QByteArray sHs;

	sHs = "CHAT/0.2 " + sReason.toLocal8Bit() + "\r\n";
		sHs+= "User-Agent: " + CQuazaaGlobals::USER_AGENT_STRING() + "\r\n\r\n";

	write(sHs);
}

void CChatSessionG2::sendStartups()
{
	qDebug() << "sending startups";
	G2Packet* pPacket = G2Packet::newPacket("UPROC", false);

	// A dirty workaround for Shareaza bug...
	if( m_bShareaza )
		pPacket->writeString("dupa");

	sendPacket(pPacket);
}

void CChatSessionG2::sendPacket(G2Packet *pPacket, bool bRelease)
{
	qDebug() << "Sending packet" << pPacket->getType();
	pPacket->toBuffer(getOutputBuffer());
	if(bRelease)
		pPacket->release();
	emit readyToTransfer();
}

void CChatSessionG2::onPacket(G2Packet *pPacket)
{
	qDebug() << "Received chat packet: " << pPacket->getType();

	if( pPacket->isType("UPROC") )
	{
		onUPROC(pPacket);
	}
	else if( pPacket->isType("UPROD") )
	{
		onUPROD(pPacket);
	}
	else if( pPacket->isType("CHATANS") )
	{
		onCHATANS(pPacket);
	}
	else if( pPacket->isType("CMSG") )
	{
		onCMSG(pPacket);
	}
	else
	{
		qDebug() << "Received unknown chat packet: " << pPacket->getType();
	}

}

void CChatSessionG2::onUPROC(G2Packet *pPacket)
{
	Q_UNUSED(pPacket);

	// Temporary code, needs to be moved to GProfile class

	QString sXML = "<?xml version=\"1.0\"?><gprofile xmlns=\"http://www.shareaza.com/schemas/GProfile.xsd\"><gnutella guid=\"%1\"/><identity><handle primary=\"%2\"/></identity></gprofile>";
	sXML = sXML.arg(quazaaSettings.Profile.GUID.toString().toUpper().replace("{", "").replace("}", "")).arg(quazaaSettings.Profile.GnutellaScreenName);

	G2Packet* pD = G2Packet::newPacket("UPROD", true);
	pD->writePacket("XML", sXML.toUtf8().size());
	pD->writeString(sXML);

	sendPacket(pD);
}

void CChatSessionG2::onUPROD(G2Packet *pPacket)
{
	qDebug() << "OnUPROD";

	if( !pPacket->m_bCompound )
		return;

	QUuid oGUID;
	bool hasXML = false;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while(pPacket->readPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("XML", szType) == 0)
		{
			// Temporary code, needs to be moved to GProfile class
			QXmlStreamReader oXML;
			bool hasGprofile = false;
			bool hasGUID = false;
			bool hasIdentity = false;
			bool hasNick = false;
			oXML.addData(pPacket->readString(nLength));

			while(!oXML.atEnd() && !oXML.hasError())
			{
				QXmlStreamReader::TokenType token = oXML.readNext();

				if( token == QXmlStreamReader::StartDocument)
				{
					continue;
				}
				else if( token == QXmlStreamReader::StartElement )
				{
					if( oXML.name() == "gprofile" )
					{
						/*
						if( oXML.attributes().value("xmlns") != "http://www.shareaza.com/schemas/GProfile.xsd" )
							return;*/
						hasGprofile = true;
						continue;
					}
					else if( hasGprofile && oXML.name() == "gnutella" )
					{
						if( oXML.attributes().hasAttribute("guid") )
						{
							QString sTemp = oXML.attributes().value("guid").toString();
							QUuid oTemp(sTemp);
							oGUID = oTemp;
							hasGUID = true;
							continue;
						}
					}
					else if( hasGprofile && oXML.name() == "identity" )
					{
						hasIdentity = true;
						continue;
					}
					else if( hasGprofile && hasIdentity && oXML.name() == "handle" )
					{
						if( oXML.attributes().hasAttribute("primary") )
						{
							m_sNick = oXML.attributes().value("primary").toString();
							hasNick = true;
							continue;
						}
					}
				}
			}

			if( !hasNick || !hasGUID )
				return;

			hasXML = true;
		}

		pPacket->m_nPosition = nNext;
	}

	if( !hasXML )
		return;

	m_oGUID = oGUID;
	emit guidChanged(oGUID);
	emit nickChanged(m_sNick);

	if( m_bInitiated ) // TODO PUSH handling
	{
		G2Packet* pReq = G2Packet::newPacket("CHATREQ", true);
		pReq->writePacket("USERGUID", 16);
		pReq->writeGUID(m_oGUID);

		sendPacket(pReq);
	}
}

void CChatSessionG2::onCHATANS(G2Packet *pPacket)
{
	char szType[9];
	quint32 nLength = 0, nNext = 0;

	bool bAccepted = false;

	while(pPacket->readPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("USERGUID", szType) == 0 && nLength >= 16)
		{
			QUuid oGUID;
			oGUID = pPacket->readGUID();
			if( !oGUID.isNull() )
			{
				m_oGUID = oGUID;
				emit guidChanged(oGUID);
			}
		}
		else if (strcmp("ACCEPT", szType) == 0 )
		{
			emit systemMessage("Private conversation accepted, you're now chatting with " + m_sNick);
			qDebug() << "chat accepted";
			bAccepted = true;
		}
		else if( strcmp("DENY", szType) == 0 )
		{
			emit systemMessage("Private conversation denied by remote host, sorry.");
			qDebug() << "chat denied";
			CNetworkConnection::close();
			return;
		}
		else if( strcmp("AWAY", szType) == 0 )
		{
			emit systemMessage(m_sNick + " is away.");
			qDebug() << "he is away";
			bAccepted = true;
		}

		pPacket->m_nPosition = nNext;
	}

	if( bAccepted )
		m_nState = csActive;
}

void CChatSessionG2::onCMSG(G2Packet *pPacket)
{
	if( !pPacket->m_bCompound || m_nState < csConnected )
		return;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	QString sMessage;
	bool bAction = false;

	while(pPacket->readPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if(strcmp("BODY", szType) == 0)
		{
			sMessage = pPacket->readString(nLength);
		}
		else if (strcmp("ACT", szType) == 0 )
		{
			bAction = true;
		}

		pPacket->m_nPosition = nNext;
	}

	if( !sMessage.isEmpty() )
	{
		emit incomingMessage(sMessage, bAction);
	}
}

void CChatSessionG2::sendMessage(QString sMessage, bool bAction)
{
	qDebug() << "Send message:" << sMessage << bAction;

	G2Packet* pPacket = G2Packet::newPacket("CMSG", true);
	if( bAction )
		pPacket->writePacket("ACT", 0);

	G2Packet* pBody = G2Packet::newPacket("BODY");
	pBody->writeString(sMessage);
	pPacket->writePacket(pBody);
	pBody->release();

	sendPacket(pPacket);
}
void CChatSessionG2::sendMessage(QTextDocument *pMessage, bool bAction)
{
	CChatConverter oConv(pMessage);

	sendMessage(oConv.toBBCode(), bAction);

	delete pMessage;
}

