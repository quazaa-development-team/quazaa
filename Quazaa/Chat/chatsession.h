/*
** chatsession.h
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

#ifndef CHATSESSION_H
#define CHATSESSION_H

#include "networkconnection.h"
#include <QUuid>

class CWidgetPrivateMessage;
class G2Packet;
class QTextDocument;

enum ChatState
{
	csNull,
	csConnecting,
	csHandshaking,
	csConnected,
	csActive,
	csClosing,
	csClosed
};

class CChatSession : public CNetworkConnection
{
	Q_OBJECT
protected:
	CWidgetPrivateMessage* m_pWidget;
	bool				  m_bShareaza;
public:
	QUuid			  m_oGUID;
	DiscoveryProtocol m_nProtocol;
	ChatState		  m_nState;
	QString			  m_sNick;
public:
	CChatSession(QObject *parent = 0);
	virtual ~CChatSession();

	virtual void connectNode();

	virtual void onTimer(quint32 tNow);

	void setupWidget(CWidgetPrivateMessage* pWg);

signals:
	void incomingMessage(QString, bool = false);
	void systemMessage(QString);
	void nickChanged(QString);
	void guidChanged(QUuid);

public slots:
	void onConnectNode();
	void onDisconnectNode();
	void onRead();
	void onError(QAbstractSocket::SocketError e);
	void onStateChange(QAbstractSocket::SocketState s);

	virtual void sendMessage(QString sMessage, bool bAction = false) = 0;
	virtual void sendMessage(QTextDocument* pMessage, bool bAction = false) = 0;

};

#endif // CHATSESSION_H
