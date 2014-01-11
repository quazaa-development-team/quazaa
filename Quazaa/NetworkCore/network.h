/*
** network.h
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

#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QMutex>
#include "types.h"
#include "routetable.h"

// TODO: rename external variables

// Note: quazaaglobals.h contains preprocessor defines for debug output.
#include "quazaaglobals.h"

class QTimer;
class CThread;
class G2Packet;

class CManagedSearch;

class CNetwork : public QObject
{
	Q_OBJECT

public:
	QMutex  m_pSection;

public:
	bool             m_bActive;
	QTimer*          m_pSecondTimer;
	CEndPoint	     m_oAddress;

	CRouteTable      m_oRoutingTable;
	quint32          m_tCleanRoutesNext;

	bool             m_bSharesReady;

public:
	CNetwork(QObject* parent = 0);
	~CNetwork();

	void start();
	void stop();

	/**
	 * @brief acquireLocalAddress tries to set our own IP to the IP in sHeader.
	 * @param sHeader : String representation of the IP without port
	 * @return true if IP could be parsed; false otherwise
	 */
	bool acquireLocalAddress(const QString& sHeader);

	bool isListening();
	bool isFirewalled();

	bool routePacket(QUuid& pTargetGUID, G2Packet* pPacket, bool bLockNeighbours = false, bool bBuffered = true);
	bool routePacket(G2Packet* pPacket, CG2Node* pNbr = 0);

	inline CEndPoint getLocalAddress()
	{
		return m_oAddress;
	}


	bool isConnectedTo(CEndPoint addr);

public slots:
	void onSecondTimer();

	void setupThread();
	void cleanupThread();

	void connectToNode(CEndPoint& addr);

	void onSharesReady();

signals:
	void localAddressChanged();
};

extern CNetwork Network;
extern CThread NetworkThread;
#endif // NETWORK_H
