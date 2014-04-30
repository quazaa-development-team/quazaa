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


#ifndef NEIGHBOURSCONNECTIONS_H
#define NEIGHBOURSCONNECTIONS_H

#include "neighboursrouting.h"

class NetworkConnection;
class RateController;

class NeighboursConnections : public NeighboursRouting
{
	Q_OBJECT
protected:
	RateController* m_pController;
public:
	quint32 m_nHubsConnectedG2;
	quint32 m_nLeavesConnectedG2;
	quint32 m_nUnknownInitiated;
	quint32 m_nUnknownIncoming;
public:
	NeighboursConnections( QObject* parent = 0 );
	virtual ~NeighboursConnections();

	virtual void connectNode();
	virtual void disconnectNode();

	void addNode( Neighbour* pNode );
	void removeNode( Neighbour* pNode );

	Neighbour* randomNode( DiscoveryProtocol::Protocol nProtocol,
						   int nType, Neighbour* pNodeExcept );

	void disconnectYoungest( DiscoveryProtocol::Protocol nProtocol,
							 int nType = 0, bool bCore = false );

	virtual quint32 downloadSpeed();
	virtual quint32 uploadSpeed();

signals:

public slots:
	Neighbour* connectTo( EndPoint oAddress, DiscoveryProtocol::Protocol nProtocol,
						  bool bAutomatic = true );
	Neighbour* onAccept( NetworkConnection* pConn );

	virtual void maintain();
};

#endif // NEIGHBOURSCONNECTIONS_H
