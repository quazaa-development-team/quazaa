/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2012.
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


#include "hash.h"
#include "systemlog.h"
#include <QCryptographicHash>
#include "3rdparty/CyoEncode/CyoEncode.h"
#include "3rdparty/CyoEncode/CyoDecode.h"

#include "debug_new.h"

Hash::Hash( const Hash& rhs ) : // Right Hash Set
	m_pContext( NULL ),
	m_bFinalized( true ),
	m_baRawValue( rhs.m_baRawValue ),
	m_nHashAlgorithm( rhs.m_nHashAlgorithm ),
	m_eType( rhs.m_eType )
{
	if ( !rhs.m_bFinalized )
	{
		systemLog.postLog( LogSeverity::Debug,
						   QObject::tr( "WARNING: Copying non-finalized CHash" ) );
	}
}

Hash::Hash( Algorithm algo ) :
	m_bFinalized( false ),
	m_nHashAlgorithm( algo ),
	m_eType( Hash::algoToType( algo ) )
{
	switch ( algo )
	{
	case Hash::SHA1:
		m_pContext = new QCryptographicHash( QCryptographicHash::Sha1 );
		break;
	case Hash::MD4:
		m_pContext = new QCryptographicHash( QCryptographicHash::Md4 );
		break;
	case Hash::MD5:
		m_pContext = new QCryptographicHash( QCryptographicHash::Md5 );
		break;
	default:
		m_pContext = NULL; /* error? */
	}
}

Hash::Hash( QByteArray baRaw, Hash::Algorithm algo ) :
	m_pContext( NULL ),
	m_bFinalized( true ),
	m_baRawValue( baRaw ),
	m_nHashAlgorithm( algo ),
	m_eType( Hash::algoToType( algo ) )
{
	if ( baRaw.size() != Hash::byteCount( algo ) )
	{
		throw invalid_hash_exception();
	}
}

Hash::~Hash()
{
	if ( m_pContext )
	{
		switch ( m_nHashAlgorithm )
		{
		case Hash::SHA1:
		case Hash::MD5:
		case Hash::MD4:
			delete ( ( QCryptographicHash* )m_pContext );
			break;
		default:
			Q_ASSERT( false );
		}
	}
}

// Returns raw hash length by hash family
int Hash::byteCount( int algo )
{
	switch ( algo )
	{
	case Hash::SHA1:
		return 20;
	case Hash::MD4:
		return 16;
	case Hash::MD5:
		return 16;
	default:
		return 0;
	}
}

// Parses URN and returns CHash pointer if conversion succeed, 0 otherwise
Hash* Hash::fromURN( QString sURN )
{
	// try to get hash family from URN
	// urn:tree:tiger:/

	if ( sURN.size() < 16 )
	{
		throw invalid_hash_exception();
	}

	QByteArray baFamily;
	int nStart = ( strncmp( "urn:", sURN.toLocal8Bit().data(), 4 ) == 0 ? 4 : 0 );
	int nStartHash = sURN.indexOf( ":", nStart ) + 1;
	baFamily = sURN.mid( nStart, nStartHash - nStart - 1 ).toLower().toLocal8Bit();
	QByteArray baValue = sURN.mid( nStartHash ).toLocal8Bit();
	char pVal[ 128 ];

	if ( baFamily == "sha1" && baValue.length() == 32 )
	{
		// sha1 base32 encoded
		if ( cyoBase32Validate( baValue.data(), baValue.length() ) == 0 )
		{
			// valid sha1/base32
			cyoBase32Decode( ( char* )&pVal, baValue.data(), baValue.length() );
			Hash* pRet = new Hash( QByteArray( ( char* )&pVal ), Hash::SHA1 );
			return pRet;
		}
		else
		{
			qDebug() << "Failed to validate base32 encoding for sha1.";
		}
	}
	else if ( baFamily == "md5" && baValue.length() == 32 )
	{
		if ( cyoBase16Validate( baValue.data(), baValue.length() ) == 0 )
		{
			cyoBase16Decode( ( char* )&pVal, baValue.data(), baValue.length() );
			Hash* pRet = new Hash( QByteArray( ( char* )&pVal ), Hash::MD5 );
			return pRet;
		}
		else
		{
			qDebug() << "Failed to validate base16 encoding for md5.";
		}
	}

	return 0;
}

Hash* Hash::fromRaw( QByteArray& baRaw, Hash::Algorithm algo )
{
	try
	{
		Hash* pRet = new Hash( baRaw, algo );
		return pRet;
	}
	catch ( ... )
	{

	}
	return 0;
}

int Hash::lengthForUrn( const QString& urn )
{
	if ( urn == "urn:sha1:" )
	{
		return 32;
	}
	if ( urn == "urn:ed2k:" )
	{
		return 32;
	}
	if ( urn == "urn:ed2khash:" )
	{
		return 32;
	}
	if ( urn == "urn:tree:tiger:" )
	{
		return 39;
	}
	if ( urn == "urn:btih:" )
	{
		return 40;
	}
	if ( urn == "urn:bitprint:" )
	{
		return 72;
	}
	if ( urn == "urn:md5:" )
	{
		return 32;
	}
	return -1;
}

// Returns URN as string
QString Hash::toURN() const
{
	switch ( m_nHashAlgorithm )
	{
	case Hash::SHA1:
		return QString( "urn:sha1:" ) + toString();
	case Hash::MD5:
		return QString( "urn:md5:" ) + toString();
	case Hash::MD4:
		break;
	default:
		Q_ASSERT( false );
	}

	return QString();
}

// Returns hash value as a string in most natural encoding
QString Hash::toString() const
{
	char pBuff[128];
	memset( &pBuff, 0, sizeof( pBuff ) );

	switch ( m_nHashAlgorithm )
	{
	case Hash::SHA1:
		cyoBase32Encode( ( char* )&pBuff, rawValue().data(), 20 );
		break;
	case Hash::MD5:
		cyoBase16Encode( ( char* )&pBuff, rawValue().data(), 16 );
		break;
	case Hash::MD4:
		break;
	default:
		Q_ASSERT( false );
	}

	return QString( pBuff );
}

void Hash::finalize()
{
	if ( !m_bFinalized )
	{
		Q_ASSERT( m_pContext );

		switch ( m_nHashAlgorithm )
		{
		case Hash::SHA1:
		case Hash::MD5:
		case Hash::MD4:
			m_baRawValue = ( ( QCryptographicHash* )m_pContext )->result();
			delete( ( QCryptographicHash* )m_pContext );
			m_pContext = 0;
			m_bFinalized = true;
			break;
		default:
			Q_ASSERT( false );
		}
	}
}

void Hash::addData( const char* pData, quint32 nLength )
{
	Q_ASSERT( !m_bFinalized && m_pContext );

	switch ( m_nHashAlgorithm )
	{
	case Hash::SHA1:
	case Hash::MD5:
	case Hash::MD4:
		( ( QCryptographicHash* )m_pContext )->addData( pData, nLength );
		break;
	default:
		Q_ASSERT( false );
	}
}
void Hash::addData( QByteArray baData )
{
	addData( baData.data(), baData.length() );
}

QString Hash::getFamilyName() const
{
	switch ( m_nHashAlgorithm )
	{
	case Hash::SHA1:
		return QString( "sha1" );
	case Hash::MD5:
		return QString( "md5" );
	case Hash::MD4:
		return QString( "md4" );
	default:
		Q_ASSERT( false );
	}

	return "";
}

Hash::Type Hash::type() const
{
	return m_eType;
}

/**
 * @brief HashVector::algoToType converts a CHash::Algorithm to a HashVector::Type.
 * @param eAlgo The CHash::Algorithm.
 * @return an HashVector::Type.
 */
Hash::Type Hash::algoToType( Hash::Algorithm eAlgo )
{
	switch ( eAlgo )
	{
	case Hash::SHA1:
		return Hash::SHA1TYPE;

	case Hash::MD5:
		return Hash::MD5TYPE;

	case Hash::MD4:
		return Hash::MD4TYPE;

	default:
		Q_ASSERT( Hash::NO_OF_TYPES == Hash::NO_OF_HASH_ALGOS ); // plz implement me ;)
		Q_ASSERT( false ); // invalid types are not allowed

		return Hash::NO_OF_TYPES;
	}
}

QDataStream& operator<<( QDataStream& s, const Hash& rhs )
{
	s << rhs.toURN();
	return s;
}

QDataStream& operator>>( QDataStream& s, Hash& rhs )
{
	QString sTmp;
	s >> sTmp;
	Hash* pHash = Hash::fromURN( sTmp );
	rhs = *pHash;
	delete pHash;
	return s;
}

/**
 * @brief Constructor. Creates an empty HashVector.
 */
HashSet::HashSet() :
	m_pHashes( new Hash*[Hash::NO_OF_TYPES] )
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		m_pHashes[i] = NULL;
	}
}

HashSet::HashSet( const HashSet& other ) :
	m_pHashes( new Hash*[Hash::NO_OF_TYPES] )
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( other.m_pHashes[i] )
		{
			m_pHashes[i] = new Hash( *other.m_pHashes[i] );
		}
		else
		{
			m_pHashes[i] = NULL;
		}
	}
}

/**
 * @brief Destructor.
 */
HashSet::~HashSet()
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			delete m_pHashes[i];
		}
	}

	delete[] m_pHashes;
}

/**
 * @brief HashVector::insert Adds a new CHash to the vector. If an equal CHash or a conflicting
 * CHash is present within the vector, no changes are made to the vector. Note: This takes ownership
 * of the hash. In case it is not added, it is deleted.
 * @param pHash The pointer to the hash.
 * @return true if the Hash was added or an equal Hash was already present; false if a conflicting
 * Hash is present in the vector.
 */
bool HashSet::insert( Hash* pHash )
{
	Q_ASSERT( pHash );

	Hash::Type t = pHash->type();

	if ( !m_pHashes[t] )
	{
		m_pHashes[t] = pHash;
		return true;
	}
	else if ( *m_pHashes[t] == *pHash )
	{
		delete pHash;
		return true;
	}
	else
	{
		delete pHash;
		return false;
	}
}

/**
 * @brief HashVector::insert Adds a new CHash to the vector. If an equal CHash or a conflicting
 * CHash is present within the vector, no changes are made to the vector.
 * @param rHash Reference to the CHash to insert.
 * @return true if the Hash was added or an equal Hash was already present; false if a conflicting
 * Hash is present in the vector.
 */
bool HashSet::insert( const Hash& rHash )
{
	Hash::Type t = rHash.type();

	if ( !m_pHashes[t] )
	{
		m_pHashes[t] = new Hash( rHash );
		return true;
	}
	else if ( *m_pHashes[t] == rHash )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief HashVector::insert Allows to insert the hashes managed by an other HashVector into this
 * one. If there is a conflict, neither HashVector is modified.
 * @param other The other HashVector.
 * @return true if the insertion was successful (e.g. no conflicts); false otherwise.
 */
bool HashSet::insert( const HashSet& other )
{
	std::vector<Hash*> vNewHashes;
	vNewHashes.reserve( other.hashCount() );

	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		// if there is a hash at position i
		if ( other.m_pHashes[i] )
		{
			if ( !conflicts( *other.m_pHashes[i] ) )
			{
				// if it doesn't clash with an existing hash
				// and we don't have a hash for that type yet
				if ( !m_pHashes[i] )
				{
					// remember the hash for later insertion
					vNewHashes.push_back( new Hash( *other.m_pHashes[i] ) );
				}
			}
			else
			{
				// conflict detected; clear remembered hashes
				for ( size_t i = 0, nMax = vNewHashes.size(); i < nMax; ++i )
				{
					delete vNewHashes[i];
				}
				return false;
			}
		}
	}

	// no conflicts detected; insert the hashes
	for ( size_t i = 0, nMax = vNewHashes.size(); i < nMax; ++i )
	{
		const quint8 type = vNewHashes[i]->type();
		m_pHashes[type] = vNewHashes[i];
	}

	return true;
}

/**
 * @brief HashVector::remove Allows to remove the specified hash from the vector.
 * @param rHash A hash.
 * @return true if the hash was found and removed; false if it was not present in the vector.
 */
bool HashSet::remove( const Hash& rHash )
{
	const quint8 type = rHash.type();

	if ( m_pHashes[type] && *m_pHashes[type] == rHash )
	{
		delete m_pHashes[type];
		m_pHashes[type] = NULL;
		return true;
	}

	return false;
}

/**
 * @brief HashVector::remove Allows to remove any hash with a matching Algorithm (can be either
 * zero or one) from the vector.
 * @param eAlgo The Algorithm to remove from the vector.
 * @return true if a hash with the specified Algorithm was present and removed; false otherwise.
 */
bool HashSet::remove( Hash::Algorithm eAlgo )
{
	const quint8 type = Hash::algoToType( eAlgo );

	if ( m_pHashes[type] )
	{
		delete m_pHashes[type];
		m_pHashes[type] = NULL;
		return true;
	}

	return false;
}

/**
 * @brief HashVector::get Allows to retrieve the stored CHash pertaining to the specified Algorithm.
 * @param eAlgo The Algorithm.
 * @return A pointer to the stored CHash.
 */
const Hash* const HashSet::get( Hash::Algorithm eAlgo ) const
{
	return m_pHashes[ Hash::algoToType( eAlgo ) ];
}

/**
 * @brief HashVector::getFirst Allows to retrieve the most important hash handled by this vector.
 * @return A pointer to the stored CHash.
 */
const Hash* const HashSet::mostImportant() const
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			return m_pHashes[i];
		}
	}

	Q_ASSERT( false );
	return NULL;
}

/**
 * @brief contains Allows to check whether the vector contains the specified hash.
 * @param rHash Const reference to the hash.
 * @return true if the vector contains the specified hash; false otherwise.
 */
bool HashSet::contains( const Hash& rHash ) const
{
	const quint8 type = rHash.type();

	if ( m_pHashes[type] && *m_pHashes[type] == rHash )
	{
		return true;
	}

	return false;
}

/**
 * @brief HashVector::conflicts Allows to check whether the specified hash conflicts with a hash
 * already present in the vector.
 * @param rHash
 * @return
 */
bool HashSet::conflicts( const Hash& rHash ) const
{
	const quint8 type = rHash.type();

	if ( m_pHashes[type] && *m_pHashes[type] != rHash )
	{
		return true;
	}

	return false;
}

/**
 * @brief HashVector::size Allows to access the vector size.
 * @return Always NO_OF_TYPES.
 */
// TODO: check usage
quint8 HashSet::size() const
{
	return Hash::NO_OF_TYPES;
}

/**
 * @brief HashVector::hashCount Allows to find out the number of hashes stored within the vector.
 * @return The amount of non NULL hashes within the vector.
 */
quint8 HashSet::hashCount() const
{
	quint8 nCount = 0;
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			++nCount;
		}
	}

	return nCount;
}

/**
 * @brief HashVector::empty Allows to find out whether there haven't been any hashes set.
 * @return true if there are no hashes in the vector; false otherwise.
 */
bool HashSet::empty() const
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			return false;
		}
	}

	return true;
}

void HashSet::simplifyByHashPriority( quint8 nNumberOfHashes )
{
	while ( hashCount() > nNumberOfHashes )
	{
		quint8 i = Hash::NO_OF_TYPES;
		while ( i != 0 )
		{
			--i;

			if ( m_pHashes[i] )
			{
				delete m_pHashes[i];
				m_pHashes[i] = NULL;
				break;
			}
		}
	}
}

/**
 * @brief HashVector::operator [] Allows access based on position. Note that NULL may be returned.
 * @param nPos The position.
 * @return A pointer to the CHash at position nPos.
 */
const Hash* const & HashSet::operator[]( quint8 nPos ) const
{
#ifdef _DEBUG
	Q_ASSERT( nPos < Hash::NO_OF_TYPES );
#endif // _DEBUG

	return m_pHashes[nPos];
}

/**
 * @brief HashVector::operator == Allows to compare two HashVectors.
 * @param other The other HashVector.
 * @return true if all hashes present in both vectors are equal; false if a hash stored in one of
 * the vectors is missing or different in the other.
 */
bool HashSet::operator==(const HashSet& other) const
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] && other.m_pHashes[i] )
		{
			if ( *m_pHashes[i] != *other.m_pHashes[i] )
			{
				return false;
			}
		}
		else if ( m_pHashes[i] || other.m_pHashes[i] )
		{
			return false;
		}
	}

	return true;
}

bool HashSet::operator!=( const HashSet& other ) const
{
	return !( *this == other );
}

/**
 * @brief HashVector::matches Allows to compare two HashVectors.
 * @param other The other HashVector.
 * @return true if no conflicting hashes were found AND there was at least one pair of matched
 * hashes.
 */
bool HashSet::matches(const HashSet& other) const
{
	quint8 nCount = 0;
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] && other.m_pHashes[i] )
		{
			if ( *m_pHashes[i] != *other.m_pHashes[i] )
			{
				return false;
			}
			else
			{
				++nCount;
			}
		}
	}

	return nCount;
}

QList<Hash>& operator<<( QList<Hash>& list, const HashSet& vector )
{
	for ( quint8 i = 0, nSize = vector.size(); i < nSize; ++i )
	{
		list.append( *vector[i] );
	}

	return list;
}

QList<Hash>& operator>>( QList<Hash>& list, HashSet& vector )
{
	for ( int i = 0, nSize = list.size(); i < nSize; ++i )
	{
		vector.insert( list[i] );
	}

	return list;
}

HashSet& operator<<( HashSet& vector, const QList<Hash>& list )
{
	for ( int i = 0, nSize = list.size(); i < nSize; ++i )
	{
		vector.insert( list[i] );
	}

	return vector;
}

HashSet& operator>>( HashSet& vector, QList<Hash>& list )
{
	for ( quint8 i = 0, nSize = vector.size(); i < nSize; ++i )
	{
		list.append( *vector[i] );
	}

	return vector;
}
