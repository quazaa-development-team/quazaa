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
#include "NetworkCore/Hashes/hashset.h"

class G2Packet;
class Query;

struct QueryHitInfo
{
	EndPoint            m_oNodeAddress;
	EndPoint            m_oSenderAddress;
	QUuid               m_oGUID;
	QUuid               m_oNodeGUID;
	QList<EndPoint>     m_lNeighbouringHubs;
	quint8              m_nHops;
	QString             m_sVendor;

	QueryHitInfo() {}
};

class QueryHit
{
public:
	QueryHit*       m_pNext;

	QSharedPointer<QueryHitInfo>   m_pHitInfo;

	HashSet         m_vHashes;
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
	QueryHit();
	QueryHit( QueryHit* pHit ); // deep copies hit without m_pNext
	~QueryHit();

	static QueryHitInfo* readInfo( G2Packet* pPacket, const EndPoint* const pSender = NULL );
	static QueryHit*     readPacket( G2Packet* pPacket, QueryHitInfo* pHitInfo );

	void resolveURLs();
	bool isValid( Query* pQuery = NULL ) const;
};

Q_DECLARE_METATYPE ( QueryHit )

typedef QSharedPointer<QueryHit> QueryHitSharedPtr;

#endif // QUERYHIT_H
