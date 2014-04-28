/*
** chatcore.h
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

#ifndef CHATCORE_H
#define CHATCORE_H

#include "types.h"
#include <QObject>
#include "thread.h"

class CChatSession;
class RateController;
class NetworkConnection;

class CChatCore : public QObject
{
	Q_OBJECT
public:
	QMutex	m_pSection;
protected:
	QList<CChatSession*> m_lSessions;
	RateController* m_pController;
	bool m_bActive;
public:
	CChatCore(QObject *parent = 0);
	virtual ~CChatCore();

	void OnAccept(NetworkConnection* pConn, DiscoveryProtocol::Protocol nProto);

protected:
	void Add(CChatSession* pSession);
	void Remove(CChatSession* pSession);

	void Start();
	void Stop();

protected slots:
	void OnTimer();

public:

signals:
	void openChatWindow(CChatSession*);

public slots:


	friend class CChatSession;
};

extern CChatCore ChatCore;
extern CThread ChatThread;
#endif // CHATCORE_H
