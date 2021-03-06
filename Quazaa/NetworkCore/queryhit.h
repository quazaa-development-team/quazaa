/*
** queryhit.h
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

#ifndef QUERYHIT_H
#define QUERYHIT_H

#include "types.h"

class G2Packet;
class CQuery;
class CHash;

struct QueryHitInfo
{
	CEndPoint	    m_oNodeAddress;
	CEndPoint		m_oSenderAddress;
	QUuid           m_oGUID;
	QUuid           m_oNodeGUID;
	QList<CEndPoint>    m_lNeighbouringHubs;
	quint8          m_nHops;
	QString			m_sVendor;

	QueryHitInfo()
	{
		m_sVendor = "";
	}
};

class CQueryHit
{
public:
	CQueryHit*      m_pNext;

	QSharedPointer<QueryHitInfo>   m_pHitInfo;

	QList<CHash>	m_lHashes;
	QString         m_sDescriptiveName;         // File name
	QString         m_sURL;                     // http://{IP}:{port}/uri-res/N2R?{URN}
	QString         m_sMetadata;
	quint64         m_nObjectSize;
	quint32         m_nCachedSources;
	bool            m_bIsPartial;
	quint64         m_nPartialBytesAvailable;
	QString         m_sPreviewURL;

	bool            m_bIsP2PIMCapable;

public:
	CQueryHit();
	CQueryHit(CQueryHit* pHit);
	~CQueryHit();

	static QueryHitInfo* readInfo(G2Packet* pPacket, CEndPoint* pSender = 0);
	static CQueryHit*    readPacket(G2Packet* pPacket, QueryHitInfo* pHitInfo);

	void resolveURLs();
	bool isValid(CQuery* pQuery = 0);
};

typedef QSharedPointer<CQueryHit> QueryHitSharedPtr;

#endif // QUERYHIT_H
