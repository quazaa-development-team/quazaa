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

CHash::CHash(const CHash &rhs) //Right Hash Set
{
	if ( !rhs.m_bFinalized )
	{
		systemLog.postLog(LogSeverity::Debug, QObject::tr( "WARNING: Copying non-finalized CHash" ) );
	}

	m_baRawValue = rhs.m_baRawValue;
	m_nHashAlgorithm = rhs.m_nHashAlgorithm;
	m_bFinalized = true;
	m_pContext = 0;
}

CHash::CHash(Algorithm algo)
{
	m_bFinalized = false;
	m_nHashAlgorithm = algo;

	switch( algo )
	{
	case CHash::SHA1:
		m_pContext = new QCryptographicHash( QCryptographicHash::Sha1 );
		break;
	case CHash::MD4:
		m_pContext = new QCryptographicHash( QCryptographicHash::Md4 );
		break;
	case CHash::MD5:
		m_pContext = new QCryptographicHash( QCryptographicHash::Md5 );
		break;
	default:
		m_pContext = NULL; /* error? */
	}
}

CHash::CHash(QByteArray baRaw, CHash::Algorithm algo)
{
	if ( baRaw.size() != CHash::byteCount( algo ) )
	{
		throw invalid_hash_exception();
	}
	m_baRawValue = baRaw;
	m_nHashAlgorithm = algo;
	m_pContext = 0;
	m_bFinalized = true;
}

CHash::~CHash()
{
	if ( m_pContext )
	{
		switch( m_nHashAlgorithm )
		{
		case CHash::SHA1:
		case CHash::MD5:
		case CHash::MD4:
			delete ( (QCryptographicHash*)m_pContext );
			break;
		default:
			Q_ASSERT( false );
		}
	}
}

// Returns raw hash length by hash family
int CHash::byteCount(int algo)
{
	switch( algo )
	{
	case CHash::SHA1:
		return 20;
	case CHash::MD4:
		return 16;
	case CHash::MD5:
		return 16;
	default:
		return 0;
	}
}

// Parses URN and returns CHash pointer if conversion succeed, 0 otherwise
CHash* CHash::fromURN(QString sURN)
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
		if ( cyoBase32Validate(baValue.data(), baValue.length()) == 0 )
		{
			// valid sha1/base32
			cyoBase32Decode( (char*)&pVal, baValue.data(), baValue.length() );
			CHash* pRet = new CHash(QByteArray( (char*)&pVal ), CHash::SHA1);
			return pRet;
		}
		else
		{
			qDebug() << "Failed to validate base32 encoding for sha1.";
		}
	}
	else if(baFamily == "md5" && baValue.length() == 32)
	{
		if(cyoBase16Validate(baValue.data(), baValue.length()) == 0)
		{
			cyoBase16Decode((char*)&pVal, baValue.data(), baValue.length());
			CHash* pRet = new CHash(QByteArray((char*)&pVal), CHash::MD5);
			return pRet;
		}
		else
		{
			qDebug() << "Failed to validate base16 encoding for md5.";
		}
	}

	return 0;
}

CHash* CHash::fromRaw(QByteArray &baRaw, CHash::Algorithm algo)
{
	try
	{
		CHash* pRet = new CHash( baRaw, algo );
		return pRet;
	}
	catch( ... )
	{

	}
	return 0;
}

int CHash::lengthForUrn(const QString &urn)
{
	if ( urn == "urn:sha1:" )
		return 32;
	if ( urn == "urn:ed2k:" )
		return 32;
	if ( urn == "urn:ed2khash:" )
		return 32;
	if ( urn == "urn:tree:tiger:" )
		return 39;
	if ( urn == "urn:btih:" )
		return 40;
	if ( urn == "urn:bitprint:" )
		return 72;
	if ( urn == "urn:md5:" )
		return 32;
	return -1;
}

// Returns URN as string
QString CHash::toURN() const
{
	switch ( m_nHashAlgorithm )
	{
	case CHash::SHA1:
		return QString( "urn:sha1:" ) + toString();
	case CHash::MD5:
		return QString("urn:md5:") + toString();
	case CHash::MD4:
		break;
	default:
		Q_ASSERT( false );
	}

	return QString();
}

// Returns hash value as a string in most natural encoding
QString CHash::toString() const
{
	char pBuff[128];
	memset( &pBuff, 0, sizeof( pBuff ) );

	switch( m_nHashAlgorithm )
	{
	case CHash::SHA1:
		cyoBase32Encode( (char*)&pBuff, rawValue().data(), 20 );
		break;
	case CHash::MD5:
		cyoBase16Encode((char*)&pBuff, rawValue().data(), 16);
		break;
	case CHash::MD4:
		break;
	default:
		Q_ASSERT( false );
	}

	return QString( pBuff );
}

void CHash::finalize()
{
	if(!m_bFinalized)
	{
		Q_ASSERT(m_pContext);

		switch(m_nHashAlgorithm)
		{
		case CHash::SHA1:
		case CHash::MD5:
		case CHash::MD4:
			m_baRawValue = ((QCryptographicHash*)m_pContext)->result();
			delete((QCryptographicHash*)m_pContext);
			m_pContext = 0;
			m_bFinalized = true;
			break;
		default:
			Q_ASSERT( false );
		}
	}
}

void CHash::addData(const char *pData, quint32 nLength)
{
	Q_ASSERT( !m_bFinalized && m_pContext );

	switch( m_nHashAlgorithm )
	{
	case CHash::SHA1:
	case CHash::MD5:
	case CHash::MD4:
		( (QCryptographicHash*)m_pContext )->addData( pData, nLength );
		break;
	default:
		Q_ASSERT( false );
	}
}
void CHash::addData(QByteArray baData)
{
	addData( baData.data(), baData.length() );
}

QString CHash::getFamilyName() const
{
	switch ( m_nHashAlgorithm )
	{
	case CHash::SHA1:
		return QString( "sha1" );
	case CHash::MD5:
		return QString( "md5" );
	case CHash::MD4:
		return QString( "md4" );
	default:
		Q_ASSERT( false );
	}

	return "";
}

QDataStream& operator<<(QDataStream& s, const CHash& rhs)
{
	s << rhs.toURN();
	return s;
}

QDataStream& operator>>(QDataStream& s, CHash& rhs)
{
	QString sTmp;
	s >> sTmp;
	CHash* pHash = CHash::fromURN(sTmp);
	rhs = *pHash;
	delete pHash;
	return s;
}

QList<CHash>& operator<<(QList<CHash>& list, const HashVector& vector)
{
	for ( size_t i = 0, nSize = vector.size(); i < nSize; ++i )
	{
		list.append( vector[i] );
	}

	return list;
}

QList<CHash>& operator>>(QList<CHash>& list, HashVector& vector)
{
	vector.reserve( list.size() + vector.size() );
	for ( int i = 0, nSize = list.size(); i < nSize; ++i )
	{
		vector.push_back( list[i] );
	}

	return list;
}

HashVector& operator<<(HashVector& vector, const QList<CHash>& list)
{
	vector.reserve( list.size() + vector.size() );
	for ( int i = 0, nSize = list.size(); i < nSize; ++i )
	{
		vector.push_back( list[i] );
	}

	return vector;
}

HashVector& operator>>(HashVector& vector, QList<CHash>& list)
{
	for ( size_t i = 0, nSize = vector.size(); i < nSize; ++i )
	{
		list.append( vector[i] );
	}

	return vector;
}

