/*
** routetable.h
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
#ifndef ROUTETABLE_H
#define ROUTETABLE_H

#include "types.h"
#include <QHash>

class CG2Node;

struct G2RouteItem
{
	QUuid           pGUID;
	CG2Node*        pNeighbour;
	CEndPoint	    pEndpoint;
	quint32         nExpireTime;

	G2RouteItem()
	{
		pNeighbour = 0;
		nExpireTime = 0;
	}

};

class CRouteTable
{
protected:
	QHash<QUuid, G2RouteItem*>  m_lRoutes;
public:
	CRouteTable();
	~CRouteTable();

	bool add(QUuid& pGUID, CG2Node* pNeighbour = 0, CEndPoint* pEndpoint = 0, bool bNoExpire = false);
	bool add(QUuid& pGUID, CG2Node* pNeighbour, bool bNoExpire = false);
	bool add(QUuid& pGUID, CEndPoint& pEndpoint, bool bNoExpire = false);

	void remove(QUuid& pGUID);
	void remove(CG2Node* pNeighbour);

	bool find(QUuid& pGUID, CG2Node** ppNeighbour = 0, CEndPoint* pEndpoint = 0);

	void expireOldRoutes(bool bForce = false);
	void clear();

	void dump();
};

const quint32 MaxRoutes = 50000;
const quint32 RouteExpire = 600;
#endif // ROUTETABLE_H
