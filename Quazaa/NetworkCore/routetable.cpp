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

#include "routetable.h"

#include "debug_new.h"

CRouteTable::CRouteTable()
{
}
CRouteTable::~CRouteTable()
{
	qDeleteAll(m_lRoutes);
	m_lRoutes.clear();
}

bool CRouteTable::add(QUuid& pGUID, CG2Node* pNeighbour, CEndPoint* pEndpoint, bool bNoExpire)
{

	//qDebug() << "CRouteTable::Add " << pGUID << pNeighbour << pEndpoint << bNoExpire;

	if(!pNeighbour && !pEndpoint)
	{
		return false;
	}

	if((quint32)m_lRoutes.size() >= MaxRoutes)
	{
		expireOldRoutes(true);
	}

	G2RouteItem* pRoute = 0;

	if( m_lRoutes.contains(pGUID) )
	{
		pRoute = m_lRoutes.value(pGUID);
	}
	else
	{
		pRoute = new G2RouteItem();
		m_lRoutes.insert(pGUID, pRoute);
	}

	if(bNoExpire && pNeighbour)
	{
		pRoute->nExpireTime = 0;
	}
	else
	{
		pRoute->nExpireTime = time(0) + RouteExpire;
	}

	pRoute->pGUID = pGUID;

	if(pNeighbour)
	{
		pRoute->pNeighbour = pNeighbour;
	}
	if(pEndpoint)
	{
		pRoute->pEndpoint = *pEndpoint;
	}

	Q_ASSERT_X(m_lRoutes[pGUID]->pNeighbour != 0 || !m_lRoutes[pGUID]->pEndpoint.isNull(), Q_FUNC_INFO, "Whooops! No neighbour and no endpoint!");

	return true;

}
bool CRouteTable::add(QUuid& pGUID, CG2Node* pNeighbour, bool bNoExpire)
{
	return add(pGUID, pNeighbour, 0, bNoExpire);
}
bool CRouteTable::add(QUuid& pGUID, CEndPoint& pEndpoint, bool bNoExpire)
{
	return add(pGUID, 0, &pEndpoint, bNoExpire);
}

void CRouteTable::remove(QUuid& pGUID)
{
	G2RouteItem* pRoute = m_lRoutes.value(pGUID, 0);
	if( pRoute )
	{
		m_lRoutes.remove(pGUID);
		delete pRoute;
	}
}
void CRouteTable::remove(CG2Node* pNeighbour)
{
	for(QHash<QUuid, G2RouteItem*>::iterator itRoute = m_lRoutes.begin(); itRoute != m_lRoutes.end();)
	{
		if(itRoute.value()->pNeighbour == pNeighbour)
		{
			delete *itRoute;
			itRoute = m_lRoutes.erase(itRoute);
		}
		else
		{
			itRoute++;
		}
	}
}

bool CRouteTable::find(QUuid& pGUID, CG2Node** ppNeighbour, CEndPoint* pEndpoint)
{
	Q_ASSERT_X(ppNeighbour || pEndpoint, Q_FUNC_INFO, "Invalid arguments");

	if(m_lRoutes.contains(pGUID))
	{
		if(ppNeighbour)
		{
			*ppNeighbour = m_lRoutes[pGUID]->pNeighbour;
		}
		if(pEndpoint)
		{
			*pEndpoint = m_lRoutes[pGUID]->pEndpoint;
		}

		Q_ASSERT_X(*ppNeighbour != 0 || !pEndpoint->isNull(), Q_FUNC_INFO, "Found GUID but no destination");

		m_lRoutes[pGUID]->nExpireTime = time(0) + RouteExpire;

		return true;
	}

	return false;
}

void CRouteTable::expireOldRoutes(bool bForce)
{
	quint32 tNow = time(0);

	// First expired.
	for(QHash<QUuid, G2RouteItem*>::iterator itRoute = m_lRoutes.begin(); itRoute != m_lRoutes.end();)
	{
		if(itRoute.value()->nExpireTime < tNow)
		{
			delete *itRoute;
			itRoute = m_lRoutes.erase(itRoute);
		}
		else
		{
			itRoute++;
		}
	}

	// Now, we are forced to clean something
	// only if the list is full at 75%

	if(bForce && m_lRoutes.size() >= MaxRoutes * 0.75)
	{
		qint32 tExpire = RouteExpire;

		while(m_lRoutes.size() > MaxRoutes * 0.75)
		{
			if( tExpire > 0 )
			{
				tExpire /= 2;
			}
			else if( tExpire < 0 )
			{
				tExpire *= 2;
			}
			else
			{
				tExpire = -10;
			}

			// We reduce the hash to three quarters of its value.
			for( QHash<QUuid, G2RouteItem*>::iterator itRoute = m_lRoutes.begin(); itRoute != m_lRoutes.end() && m_lRoutes.size() > MaxRoutes * 0.75; )
			{
				if( itRoute.value()->nExpireTime < tNow + tExpire )
				{
					delete *itRoute;
					itRoute = m_lRoutes.erase(itRoute);
				}
				else
				{
					itRoute++;
				}
			}
		}
	}
}

void CRouteTable::clear()
{
	qDeleteAll(m_lRoutes);
	m_lRoutes.clear();
}

void CRouteTable::dump()
{

	quint32 tNow = time(0);

	systemLog.postLog(LogSeverity::Debug, "----------------------------------");
	systemLog.postLog(LogSeverity::Debug, "Dumping routing table:");
	systemLog.postLog(LogSeverity::Debug, QString("Table size: ").arg(m_lRoutes.size()));

	for(QHash<QUuid, G2RouteItem*>::iterator itRoute = m_lRoutes.begin(); itRoute != m_lRoutes.end(); itRoute++)
	{
		qint64 nExpire = itRoute.value()->nExpireTime - tNow;
		if(itRoute.value()->nExpireTime == 0)
		{
			nExpire = 0;
		}
		systemLog.postLog( LogSeverity::Debug, Components::G2, "%s %i %s TTL %i",
						   qPrintable( itRoute.key().toString() ), itRoute.value()->pNeighbour,
						   qPrintable( itRoute.value()->pEndpoint.toString() ), nExpire );
	}

	systemLog.postLog(LogSeverity::Debug, "End of data");
	systemLog.postLog(LogSeverity::Debug, "----------------------------------");
}

