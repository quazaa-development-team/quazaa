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


#ifndef NEIGHBOURSROUTING_H
#define NEIGHBOURSROUTING_H

#include "neighboursbase.h"

class G2Packet;
class Query;

typedef QSharedPointer<Query> QuerySharedPtr;

class NeighboursRouting : public NeighboursBase
{
	Q_OBJECT
public:
	NeighboursRouting( QObject* parent = 0 );
	virtual ~NeighboursRouting();

	void routeQuery( QuerySharedPtr pQuery, G2Packet* pPacket, Neighbour* pFrom = 0, bool bToHubs = true );

signals:

public slots:

};

#endif // NEIGHBOURSROUTING_H
