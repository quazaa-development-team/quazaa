/*
** handshakes.h
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

#ifndef HANDSHAKES_H
#define HANDSHAKES_H

#include <QTcpServer>
#include <QSet>
#include "types.h"
#include "thread.h"

class CHandshake;
class CNetworkConnection;
class CRateController;
class QTimer;

class CHandshakes : public QTcpServer
{
	Q_OBJECT

protected:
	QSet<CHandshake*>   m_lHandshakes;
	quint32             m_nAccepted;
	CRateController* 	m_pController;
	QTimer*				m_pTimer;
	bool				m_bActive;

public:
	QMutex	m_pSection;

public:
	CHandshakes(QObject* parent = 0);
	virtual ~CHandshakes();

	bool isFirewalled()
	{
		return (m_nAccepted == 0);
	}

public slots:
	void listen();
	void stop();
	void onTimer();

protected slots:
	void setupThread();
	void cleanupThread();

signals:

protected:
	void incomingConnection(qintptr handle);

	void removeHandshake(CHandshake* pHs);

	void processNeighbour(CHandshake* pHs);

	friend class CHandshake;
};

extern CHandshakes Handshakes;
extern CThread HandshakesThread;

#endif // HANDSHAKES_H
