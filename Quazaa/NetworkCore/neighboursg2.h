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


#ifndef NEIGHBOURSG2_H
#define NEIGHBOURSG2_H

#include "neighboursconnections.h"

class G2Packet;

class CNeighboursG2 : public CNeighboursConnections
{
	Q_OBJECT
public:
	CNeighboursG2(QObject* parent = 0);
	virtual ~CNeighboursG2();

	void dispatchKHL();
	bool switchG2ClientMode(G2NodeType nRequestedMode);
	bool needMoreG2(G2NodeType nType);

	virtual void connectNode();

	G2Packet* createQueryAck(QUuid oGUID, bool bWithHubs = true, CNeighbour* pExcept = 0, bool bDone = true);

	void hubBalancing();

protected:
	quint32 m_nNextKHL;
	quint32 m_nLNIWait;
	bool	m_bNeedLNI;
	G2NodeType m_nClientMode;
	quint32 m_nUpdateWait;

	quint32 m_nSecsTrying;		// How long we are without hub connection?
	quint32 m_tLastModeChange;	// When we changed client mode last time?
	quint32 m_nHubBalanceWait;
	quint32 m_nPeriodsLow;
	quint32 m_nPeriodsHigh;


signals:

public slots:
	virtual void maintain();

public:
	inline bool isG2Hub()
	{
		return (m_nClientMode == G2_HUB);
	}
};

#endif // NEIGHBOURSG2_H
