/*
** chatsessiong2.h
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

#ifndef CHATSESSIONG2_H
#define CHATSESSIONG2_H

#include "chatsession.h"

class G2Packet;

class CChatSessionG2 : public CChatSession
{
	Q_OBJECT
protected:
	CEndPoint m_oRemoteHost;
public:
	CChatSessionG2(CEndPoint oRemoteHost, QObject *parent = 0);

	void connectNode();

protected:
	void parseOutgoingHandshake();

	void send_ChatOK(bool bReply);
	void send_ChatError(QString sReason);
	void sendStartups();
	void sendPacket(G2Packet* pPacket, bool bRelease = true);
	void onPacket(G2Packet* pPacket);
	void onUPROC(G2Packet* pPacket);
	void onUPROD(G2Packet* pPacket);
	void onCHATANS(G2Packet* pPacket);
	void onCMSG(G2Packet* pPacket);

signals:

public slots:
	void onConnectNode();
	void onDisconnectNode();
	void onRead();

	void sendMessage(QString sMessage, bool bAction = false);
	void sendMessage(QTextDocument* pMessage, bool bAction = false);

};

#endif // CHATSESSIONG2_H
