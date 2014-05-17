/*
** hash.cpp
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

#include <QDebug>
#include <QtGlobal>
#include <QCryptographicHash>

#include "hash.h"
#include "systemlog.h"
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

/**
 * @brief Hash::toString Allows to obtain a string representation of the hash value.
 * @return The hash value as a string in its most natural encoding.
 */
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

/**
 * @brief Hash::getFamilyName Allows access to the lower cased hash family name.
 * @return The lower cased hash family name. Examples: sha1, md5, md4, ...
 */
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

