/*
** hash.h
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

#ifndef HASH_H
#define HASH_H

#include "types.h"

struct invalid_hash_exception {};

// TODO: using shared data for both CHash and HashVector might be advantageous
class CHash
{
public:
	// When adding new hashes, make sure the values are assigned by hash importance.
	// Exclude Algorithm::Null from NO_OF_HASH_TYPES
	enum Algorithm { Null = 0, MD4 = 1, MD5 = 2, SHA1 = 10, NO_OF_HASH_ALGOS = 3 };

private:
	// Sorted by descending order of importance. For usage within HashVector.
	enum Type { SHA1TYPE = 0, MD5TYPE = 1, MD4TYPE = 2, NO_OF_TYPES = 3 };

protected:
	void*               m_pContext;
	bool                m_bFinalized;
	QByteArray          m_baRawValue;

private:
	CHash::Algorithm    m_nHashAlgorithm;
	CHash::Type     m_eType;          // for usage with HashVector only

public:
	CHash( const CHash& rhs );
	CHash( Algorithm algo );
	CHash( QByteArray baRaw, CHash::Algorithm algo );
	~CHash();

	static int	byteCount( int algo );

	static CHash* fromURN( QString sURN );
	static CHash* fromRaw( QByteArray& baRaw, CHash::Algorithm algo );

	static int lengthForUrn( const QString& urn );

	QString toURN() const;
	QString toString() const;

	void addData( const char* pData, quint32 nLength );
	void addData( QByteArray baData );

	QString getFamilyName() const;

	void finalize();

	inline CHash::Algorithm algorithm() const;
	inline QByteArray rawValue() const;

	inline bool operator==( const CHash& oHash ) const;
	inline bool operator!=( const CHash& oHash ) const;
	inline bool operator>( const CHash& oHash ) const;
	inline bool operator<( const CHash& oHash ) const;

private: // for use with HashVector only
	CHash::Type type() const;
	static CHash::Type algoToType( CHash::Algorithm eAlgo );

	friend class HashSet;
};

// allows using CHash with std::unordered_map
namespace std
{
template <>
struct hash<CHash> : public unary_function<CHash, size_t>
{
	size_t operator()( const CHash& value ) const
	{
		return qHash( value.rawValue() );
	}
};
}

CHash::Algorithm CHash::algorithm() const
{
	return m_nHashAlgorithm;
}

QByteArray CHash::rawValue() const
{
	return m_baRawValue;
}

bool CHash::operator ==( const CHash& oHash ) const
{
	Q_ASSERT( oHash.m_bFinalized && m_bFinalized );
	return ( oHash.m_nHashAlgorithm == m_nHashAlgorithm && oHash.m_baRawValue == m_baRawValue );
}

bool CHash::operator !=( const CHash& oHash ) const
{
	return !( *this == oHash );
}

bool CHash::operator <( const CHash& oHash ) const
{
	return ( m_baRawValue < oHash.m_baRawValue );
}

bool CHash::operator >( const CHash& oHash ) const
{
	return ( m_baRawValue > oHash.m_baRawValue );
}

QDataStream& operator<<( QDataStream& s, const CHash& rhs );
QDataStream& operator>>( QDataStream& s, CHash& rhs );

class HashSet
{
private:
	CHash** m_pHashes;

public:
	HashSet();
	HashSet( const HashSet& other );
	~HashSet();

	bool insert( CHash* pHash );
	bool insert( const CHash& rHash );
	bool insert( const HashSet& other );

	bool remove( const CHash& rHash );
	bool remove( CHash::Algorithm eAlgo );

	const CHash* const mostImportant() const;
	const CHash* const get( CHash::Algorithm eAlgo ) const;

	bool contains( const CHash& rHash ) const;
	bool conflicts( const CHash& rHash ) const;

	quint8 size() const;
	quint8 hashCount() const;
	bool empty() const;

	void simplifyByHashPriority( quint8 nNumberOfHashes );

public:
	const CHash* const & operator[]( quint8 pos ) const;
	bool operator==( const HashSet& other ) const;
	bool operator!=( const HashSet& other ) const;
	bool matches( const HashSet& other ) const;

	friend class CHash;
	//friend class HashRule;
};

QList<CHash>& operator<<(       QList<CHash>& list, const HashSet& vector );
QList<CHash>& operator>>( const QList<CHash>& list,       HashSet& vector );
HashSet&   operator<<(       HashSet& vector, const QList<CHash>& list );
HashSet&   operator>>( const HashSet& vector,       QList<CHash>& list );

#endif // HASH_H
