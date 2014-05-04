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

// Note: quazaaglobals.h contains preprocessor defines for debug output.
#include "quazaaglobals.h"

class QTimer;
class CThread;
class G2Packet;
class ManagedSearch;

/**
 * @brief The NetworkG2 class represents the Gnutella2 network within Quazaa. It allows to connect
 * to and disconnect from the network and manages the G2 network thread.
 */
class NetworkG2 : public QObject
{
	Q_OBJECT

public:
	QMutex  m_pSection;

public:
	bool             m_bActive;
	QTimer*          m_pSecondTimer;
	EndPoint	     m_oAddress;

	RouteTable       m_oRoutingTable;
	quint32          m_tCleanRoutesNext;

	bool             m_bSharesReady;

public:
	NetworkG2( QObject* parent = 0 );
	~NetworkG2();

	/**
	 * @brief start Starts the G2 network thread and connects to the network if the network has not
	 * already been started.
	 */
	void start();

	/**
	 * @brief NetworkG2::stop Disconnects from the G2 network and stops the network thread if it is
	 * currently active.
	 */
	void stop();

	/**
	 * @brief acquireLocalAddress tries to set our own IP to the IP in sHeader.
	 * @param sHeader : String representation of the IP without port
	 * @return true if IP could be parsed; false otherwise
	 */
	bool acquireLocalAddress( const QString& sHeader );

	bool isListening() const;
	bool isFirewalled() const;

	bool routePacket( const QUuid& pTargetGUID, G2Packet* pPacket, bool bLockNeighbours = false,
					  bool bBuffered = true );
	bool routePacket( G2Packet* pPacket, G2Node* pNbr = NULL );

	inline const EndPoint& localAddress() const
	{
		return m_oAddress;
	}

	bool isConnectedTo( const EndPoint& addr ) const;

public slots:
	void onSecondTimer();

	void setupThread();
	void cleanupThread();

	void connectToNode( const EndPoint& addr );

	void onSharesReady();

signals:
	void localAddressChanged();
};

extern NetworkG2 networkG2;
extern CThread networkThread;
#endif // NETWORK_H
