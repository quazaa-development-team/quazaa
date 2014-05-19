/*
** hash.h
**
** Copyright © Quazaa Development Team, 2009-2014.
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

#include <QHash>
#include <QByteArray>

struct invalid_hash_exception {};

// TODO: using shared data for both CHash and/or HashVector might be advantageous
// TODO: AICH support:http://www.emule-project.net/home/perl/help.cgi?l=1&topic_id=589&rm=show_topic
class Hash
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
	Hash::Algorithm    m_nHashAlgorithm;
	Hash::Type     m_eType;          // for usage with HashVector only

public:
	Hash( const Hash& rhs );
	Hash( Algorithm algo );
	Hash( QByteArray baRaw, Hash::Algorithm algo );
	~Hash();

	static int	byteCount( int algo );

	static Hash* fromURN( QString sURN );
	static Hash* fromRaw( QByteArray& baRaw, Hash::Algorithm algo );

	static int lengthForUrn( const QString& urn );

	QString toURN() const;
	QString toString() const;

	void addData( const char* pData, quint32 nLength );
	void addData( QByteArray baData );

	QString getFamilyName() const;

	void finalize();

	inline Hash::Algorithm algorithm() const;
	inline QByteArray rawValue() const;

	inline bool operator==( const Hash& oHash ) const;
	inline bool operator!=( const Hash& oHash ) const;
	inline bool operator>( const Hash& oHash ) const;
	inline bool operator<( const Hash& oHash ) const;

private: // for use with HashVector only
	Hash::Type type() const;
	static Hash::Type algoToType( Hash::Algorithm eAlgo );

	friend class HashSet;
};

// allows using Hash with std::unordered_map
namespace std
{
template <>
struct hash<Hash> : public unary_function<Hash, size_t>
{
	size_t operator()( const Hash& value ) const
	{
		return qHash( value.rawValue() );
	}
};
}

Hash::Algorithm Hash::algorithm() const
{
	return m_nHashAlgorithm;
}

QByteArray Hash::rawValue() const
{
	return m_baRawValue;
}

bool Hash::operator ==( const Hash& oHash ) const
{
	Q_ASSERT( oHash.m_bFinalized && m_bFinalized );
	return ( oHash.m_nHashAlgorithm == m_nHashAlgorithm && oHash.m_baRawValue == m_baRawValue );
}

bool Hash::operator !=( const Hash& oHash ) const
{
	return !( *this == oHash );
}

bool Hash::operator <( const Hash& oHash ) const
{
	return ( m_baRawValue < oHash.m_baRawValue );
}

bool Hash::operator >( const Hash& oHash ) const
{
	return ( m_baRawValue > oHash.m_baRawValue );
}

QDataStream& operator<<( QDataStream& s, const Hash& rhs );
QDataStream& operator>>( QDataStream& s, Hash& rhs );

#endif // HASH_H
