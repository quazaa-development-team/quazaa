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

class Handshake;
class NetworkConnection;
class RateController;
class QTimer;

class Handshakes : public QTcpServer
{
	Q_OBJECT

protected:
	QSet<Handshake*>    m_lHandshakes;
	quint32             m_nAccepted;
	RateController* 	m_pController;
	QTimer*				m_pTimer;
	bool				m_bActive;

public:
	QMutex	m_pSection;

public:
	Handshakes( QObject* parent = 0 );
	virtual ~Handshakes();

	bool isFirewalled()
	{
		return !m_nAccepted;
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
	void incomingConnection( qintptr handle );

	void removeHandshake( Handshake* pHs );

	void processNeighbour( Handshake* pHs );

	friend class Handshake;
};

extern Handshakes handshakes;
extern CThread handshakesThread;

#endif // HANDSHAKES_H
