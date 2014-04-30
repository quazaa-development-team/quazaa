/*
** queryhashtable.h
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

#ifndef QUERYHASHTABLE_H
#define QUERYHASHTABLE_H

#include "types.h"
#include <QObject>
class QString;
class QByteArray;
class G2Node;
class G2Packet;
class QByteArray;
class Buffer;
class QueryHashGroup;
class Query;

typedef QSharedPointer<Query> QuerySharedPtr;

class QueryHashTable : public QObject
{
	Q_OBJECT
public:
	QueryHashTable();
	virtual ~QueryHashTable();

public:
	bool            m_bLive;
	quint32         m_nCookie;
	uchar*          m_pHash;
	quint32         m_nHash;
	quint32         m_nBits;
	quint32         m_nInfinity;
	quint32         m_nCount;
	Buffer*         m_pBuffer;
	QueryHashGroup* m_pGroup;

public:
	static quint32 hashWord( const char* pSz, const quint32 nLength, qint32 nBits );
	static quint32 hashNumber( quint32 nNumber, qint32 nBits );

public:
	static int makeKeywords( QString sPhrase, QStringList& outList );

public:
	void	create();
	void	clear();
	bool	merge( const QueryHashTable* pSource );
	bool	merge( const QueryHashGroup* pSource );
	bool	patchTo( const QueryHashTable* pTarget, G2Node* pNeighbour );
	bool	onPacket( G2Packet* pPacket );
	void	addString( const QString& strString );
	void	addExactString( const QString& strString );
	void	addWord( const QByteArray& sWord );
	bool	checkString( const QString& strString ) const;
	bool	checkHash( const quint32 nHash ) const;
	bool	checkQuery( QuerySharedPtr pQuery );
	int		getPercent() const;
protected:
	bool	onReset( G2Packet* pPacket );
	bool	onPatch( G2Packet* pPacket );
	void	add( const char* pszString, quint32 nLength );
	void	addExact( const char* pszString, quint32 nLength );
};

#pragma pack(push,1)

struct G2_QHT_RESET
{
	char    nCmd;       // 0 - reset
	quint32 nTableSize; // 2 ^ size_in_bits
	char    nInfinity;  // 1
};
struct G2_QHT_PATCH
{
	char    nCmd;           // 1 - patch
	char    nFragNum;       //
	char    nFragCount;
	char    nCompression;   // 0 - bez, 1 defl
	char    nBits;          // 1
	//(data)
};

#pragma pack(pop)

#endif // QUERYHASHTABLE_H
