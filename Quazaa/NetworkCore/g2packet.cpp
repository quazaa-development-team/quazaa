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

#include <stdexcept>

#include "g2packet.h"
#include "systemlog.h"
#include "buffer.h"

#include "debug_new.h"

G2PacketPool G2Packets;

G2Packet::G2Packet() :
	m_pNext( NULL ),
	m_nReference( 0 ),
	m_pBuffer( NULL ),
	m_nBuffer( 0 ),
	m_nLength( 0 ),
	m_nPosition( 0 ),
	m_bCompound( false ),
	m_bBigEndian( false )
{
	memset( &m_sType[0], 0, sizeof( m_sType ) );
}

G2Packet::~G2Packet()
{
	if ( m_nReference )
	{
		systemLog.postLog( LogSeverity::Debug, Component::G2,
						   QString( "%1 not released" ).arg( ( char* )&m_sType[0] ) );
	}
	Q_ASSERT( !m_nReference );

	if ( m_pBuffer )
	{
		free( m_pBuffer );
	}
}

void G2Packet::reset()
{
	Q_ASSERT( !m_nReference );

	m_pNext			= 0;
	m_nLength		= 0;
	m_nPosition		= 0;

	memset( &m_sType[0], 0, sizeof( m_sType ) );
	m_bCompound = false;
}

void G2Packet::seek( quint32 nPosition, int nRelative )
{
	if ( nRelative == seekStart )
	{
		m_nPosition = qMax( 0u, qMin<quint32>( m_nLength, nPosition ) );
	}
	else
	{
		m_nPosition = qMax( 0u, qMin<quint32>( m_nLength, m_nLength - nPosition ) );
	}
}

uchar* G2Packet::writeGetPointer( quint32 nLength, quint32 nOffset )
{
	if ( nOffset == 0xFFFFFFFF )
	{
		nOffset = m_nLength;
	}

	if ( !ensure( nLength ) )
	{
		return 0;
	}

	if ( nOffset != m_nLength )
	{
		memmove( m_pBuffer + nOffset + nLength, m_pBuffer + nOffset, m_nLength - nOffset );
	}

	m_nLength += nLength;

	return m_pBuffer + nOffset;
}

char* G2Packet::getType() const
{
	return ( char* )&m_sType;
}

void G2Packet::deletePacket()
{
	G2Packets.deletePacket( this );
}

G2Packet* G2Packet::newPacket( const char* pszType, bool bCompound )
{
	G2Packet* pPacket = G2Packets.newPacket();

	if ( pszType )
	{
		size_t nLength = strlen( pszType );
		strncpy_s( pPacket->m_sType, pszType, nLength );
		pPacket->m_sType[nLength] = 0;
	}

	pPacket->m_bCompound = bCompound;

	return pPacket;
}
G2Packet* G2Packet::newPacket( char* pSource )
{
	G2Packet* pPacket   = newPacket();

	uchar nControlByte  = *( uchar* )pSource++;

	uchar nLenLen       = ( nControlByte & 0xC0 ) >> 6;
	uchar nTypeLen      = ( nControlByte & 0x38 ) >> 3;

	pPacket->m_bCompound    = nControlByte & G2_FLAG_COMPOUND;
	pPacket->m_bBigEndian   = nControlByte & G2_FLAG_BIG_ENDIAN;

	quint32 nLength = 0;

	if ( pPacket->m_bBigEndian )
	{
		throw std::logic_error( "New G2 packet is big endian." );
	}
	else
	{
		char* pLenOut = ( char* )&nLength;
		while ( nLenLen-- )
		{
			*pLenOut++ = *pSource++;
		}
	}

	++nTypeLen;
	char* pszType = pPacket->m_sType;
	for ( ; nTypeLen-- ; )
	{
		*pszType++ = *pSource++;
	}
	*pszType++ = 0;

	pPacket->write( pSource, nLength );

	return pPacket;
}

G2PacketPool::G2PacketPool()
{
	m_pFree = 0;
	m_nFree = 0;
}

G2PacketPool::~G2PacketPool()
{
	clear();
}

G2Packet* G2Packet::writePacket( G2Packet* pPacket )
{
	if ( pPacket == 0 )
	{
		return 0;
	}
	writePacket( pPacket->m_sType, pPacket->m_nLength, pPacket->m_bCompound );
	write( pPacket->m_pBuffer, pPacket->m_nLength );
	return this;
}

G2Packet* G2Packet::writePacket( const char* pszType, quint32 nLength, bool bCompound )
{
	Q_ASSERT( strlen( pszType ) > 0 );
	Q_ASSERT( nLength <= 0xFFFFFF );

	char nTypeLen	= ( char )( strlen( pszType ) - 1 ) & 0x07;
	char nLenLen	= 0;

	if ( nLength )
	{
		nLenLen++;
		if ( nLength > 0xFF )
		{
			nLenLen++;
			if ( nLength > 0xFFFF )
			{
				nLenLen++;
			}
		}
	}

	char nFlags = ( nLenLen << 6 ) + ( nTypeLen << 3 );

	if ( bCompound )
	{
		nFlags |= G2_FLAG_COMPOUND;
	}

	write( &nFlags, 1 );

	write( &nLength, nLenLen );

	write( const_cast<char*>( pszType ), nTypeLen + 1 );

	m_bCompound = true;

	return this;
}

bool G2Packet::readPacket( char* pszType, quint32& nLength, bool* pbCompound )
{
	if ( getRemaining() == 0 )
	{
		return false;
	}

	char nInput = readByte();
	if ( nInput == 0 )
	{
		return false;
	}

	char nLenLen	= ( nInput & 0xC0 ) >> 6;
	char nTypeLen	= ( nInput & 0x38 ) >> 3;
	char nFlags		= ( nInput & 0x07 );

	if ( getRemaining() < nTypeLen + nLenLen + 1 )
	{
		throw std::underflow_error( "Packet read will not reach end." );
	}

	nLength = 0;
	read( &nLength, nLenLen );

	if ( getRemaining() < ( int )( nLength + nTypeLen + 1 ) )
	{
		throw std::underflow_error( "Packet read will not reach end." );
	}

	read( pszType, nTypeLen + 1 );
	pszType[ nTypeLen + 1 ] = 0;

	if ( pbCompound )
	{
		*pbCompound = ( nFlags & G2_FLAG_COMPOUND ) == G2_FLAG_COMPOUND;
	}
	else
	{
		if ( nFlags & G2_FLAG_COMPOUND )
		{
			skipCompound( nLength );
		}
	}

	return true;
}

bool G2Packet::skipCompound()
{
	if ( m_bCompound )
	{
		quint32 nLength = m_nLength;
		return skipCompound( nLength );
	}

	return true;
}

bool G2Packet::skipCompound( quint32& nLength, quint32 nRemaining )
{
	quint32 nStart	= m_nPosition;
	quint32 nEnd	= m_nPosition + nLength;

	while ( m_nPosition < nEnd )
	{
		char nInput = readByte();
		if ( nInput == 0 )
		{
			break;
		}

		char nLenLen	= ( nInput & 0xC0 ) >> 6;
		char nTypeLen	= ( nInput & 0x38 ) >> 3;
		char nFlags	= ( nInput & 0x07 );
		Q_UNUSED( nFlags );

		if ( m_nPosition + nTypeLen + nLenLen + 1 > nEnd )
		{
			throw std::overflow_error( "Packet will read past end." );
		}

		quint32 nPacket = 0;

		read( &nPacket, nLenLen );

		if ( m_nPosition + nTypeLen + 1 + nPacket > nEnd )
		{
			throw std::overflow_error( "Packet will read past end." );
		}

		m_nPosition += nPacket + nTypeLen + 1;
	}

	nEnd = m_nPosition - nStart;
	if ( nEnd > nLength )
	{
		throw std::overflow_error( "Packet will read past end." );
	}
	nLength -= nEnd;

	return nRemaining ? nLength >= nRemaining : true;
}

void G2Packet::toBuffer( Buffer* pBuffer ) const
{
	Q_ASSERT( strlen( m_sType ) > 0 );

	char nLenLen	= 0;
	if ( m_nLength )
	{
		nLenLen++;
		if ( m_nLength > 0xFF )
		{
			nLenLen++;
			if ( m_nLength > 0xFFFF )
			{
				nLenLen++;
			}
		}
	}

	char nTypeLen	= ( char )( strlen( m_sType ) - 1 ) & 0x07;

	char nFlags = ( nLenLen << 6 ) + ( nTypeLen << 3 );

	if ( m_bCompound )
	{
		nFlags |= G2_FLAG_COMPOUND;
	}

	pBuffer->append( ( char* )&nFlags, 1 );

	quint32 nLength = m_nLength;
	pBuffer->append( ( char* )&nLength, nLenLen );

	pBuffer->append( ( char* )&m_sType[0], nTypeLen + 1 );

	pBuffer->append( ( char* )m_pBuffer, m_nLength );
}

quint32 G2Packet::peakUIntBytes( const uchar* pSource, const quint8 nCount, bool bBigEndian )
{
	Q_ASSERT( pSource );
	Q_ASSERT( nCount <= 4 );

	quint32 nPeak = 0;
	uchar* pDestination	= ( uchar* )&nPeak;

	// the source is big endian
	if ( bBigEndian )
	{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		for ( quint8 i = 0; i < nCount; ++i )
		{
			*pDestination++ = *pSource++;
		}
#elif Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		pDestination += nCount;
		for ( quint8 i = 0; i < nCount; ++i )
		{
			*pDestination-- = *pSource++;
		}
#endif
	}
	else // the source is little endian
	{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
		pDestination += 3;
		for ( quint8 i = 0; i < nCount; ++i )
		{
			*pDestination-- = pSource++;
		}
#elif Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		for ( quint8 i = 0; i < nCount; ++i )
		{
			*pDestination++ = *pSource++;
		}
#endif
	}

	return nPeak;
}

//////////////////////////////////////////////////////////////////////
// G2Packet buffer stream read

G2Packet* G2Packet::readBuffer( Buffer* pBuffer )
{
	if ( !pBuffer )
	{
		return NULL;
	}

	if ( pBuffer->size() < 2 )
	{
		return NULL;
	}

	// The Control Byte
	// +----+----+----+----+----+----+----+----+
	// | 7    6  | 5    4    3  | 2  | 1  | 0  | Bit
	// +----+----+----+----+----+----+----+----+
	// | Len_Len | Name_Len - 1 | CF | BE | // |
	// +----+----+----+----+----+----+----+----+
	char nControlByte = *pBuffer->data();

	// "A zero control byte identifies the end of a stream of packets[...]"
	if ( !nControlByte )
	{
		pBuffer->remove( 1 );
		return NULL;
	}

	//bool bCompound  =  nControlByte & G2_FLAG_COMPOUND;
	bool bBigEndian =  nControlByte & G2_FLAG_BIG_ENDIAN;

	if ( bBigEndian )
	{
		pBuffer->clear();
		return NULL;
	}

	// nLenLen is the number of bytes in the length field of the packet, which immediately follows
	// the control byte. There are two bits here which means the length field can be up to 3 bytes
	// long. nLenLen can be zero [...].
	// nNameLen is the number of bytes in the packet name field MINUS ONE. There are three bits here
	// which means that packet names can be 1 to 8 bytes long inclusive. [...]
	uchar nLenLen  = ( nControlByte & G2_PACKET_LENLEN_BITS ) >> 6;
	uchar nNameLen = ( nControlByte & G2_PACKET_NAMELEN_BITS ) >> 3;

	quint32 nTotalSize = ( quint32 )nLenLen + nNameLen + 2u;
	if ( ( quint32 )pBuffer->size() < nTotalSize )
	{
		return NULL;
	}

	// read the packet length from buffer
	const quint32 nLength = peakUIntBytes( ( uchar* )pBuffer->data() + 1, nLenLen, bBigEndian );
	nTotalSize += nLength;

#ifdef _DEBUG
	if ( !bBigEndian )
	{
		quint32 nLength2 = 0;

		char* pLenIn	= pBuffer->data() + 1;
		char* pLenOut	= ( char* )&nLength2;
		for ( char nLenCnt = nLenLen ; nLenCnt-- ; )
		{
			*pLenOut++ = *pLenIn++;
		}

		Q_ASSERT( nLength == nLength2 );
	}
#endif

	if ( ( quint32 )pBuffer->size() < nTotalSize )
	{
		return NULL;
	}

	G2Packet* pPacket = G2Packet::newPacket( pBuffer->data() );
	pBuffer->remove( nTotalSize );

	return pPacket;
}

QString G2Packet::readString( quint32 nMaximum )
{
	nMaximum = qMin<quint32>( nMaximum, m_nLength - m_nPosition );
	if ( !nMaximum )
	{
		return QString();
	}

	const uchar* pInput = m_pBuffer + m_nPosition;
	uchar* pScan = const_cast<uchar*>( pInput );

	quint32 nLength = 0;

	for ( ; nLength < nMaximum; nLength++ )
	{
		m_nPosition++;
		if ( ! *pScan )
		{
			break;
		}
		pScan++;
	}

	return QString::fromUtf8( ( char* )pInput, nLength );
}
void G2Packet::writeString( QString sToWrite, bool bTerminate )
{
	QByteArray baUTF8 = sToWrite.toUtf8();

	ensure( baUTF8.size() + 1 );

	memcpy( m_pBuffer + m_nLength, baUTF8.constData(), baUTF8.size() );

	m_nLength += baUTF8.size();

	if ( bTerminate )
	{
		m_pBuffer[m_nLength] = 0;
		++m_nLength;
	}
}

QString G2Packet::toHex() const
{
	const char* pszHex = "0123456789ABCDEF";
	QByteArray strDump;

	strDump.resize( m_nLength * 3 );
	char* pszDump = strDump.data();

	for ( quint32 i = 0 ; i < m_nLength ; i++ )
	{
		int nChar = m_pBuffer[i];
		if ( i )
		{
			*pszDump++ = ' ';
		}
		*pszDump++ = pszHex[ nChar >> 4 ];
		*pszDump++ = pszHex[ nChar & 0x0F ];
	}

	*pszDump = 0;

	return strDump;
}

QString G2Packet::toASCII() const
{
	QByteArray strDump;

	strDump.resize( m_nLength + 1 );
	char* pszDump = strDump.data();

	for ( uint i = 0 ; i < m_nLength ; i++ )
	{
		int nChar = m_pBuffer[i];
		*pszDump++ = ( nChar >= 32 ? nChar : '.' );
	}

	*pszDump = 0;

	return strDump;
}

bool G2Packet::getTo( QUuid& pGUID )
{
	if ( m_bCompound == false )
	{
		return false;
	}
	if ( getRemaining() < 4 + 16 )
	{
		return false;
	}

	uchar* pTest = m_pBuffer + m_nPosition;

	if ( pTest[0] != 0x48 )
	{
		return false;
	}
	if ( pTest[1] != 0x10 )
	{
		return false;
	}
	if ( pTest[2] != 'T' )
	{
		return false;
	}
	if ( pTest[3] != 'O' )
	{
		return false;
	}

	m_nPosition = 4;
	pGUID = readGUID();
	m_nPosition = 0;

	return true;
}

void G2PacketPool::clear()
{
	for ( int nIndex = m_pPools.size() - 1 ; nIndex >= 0 ; nIndex-- )
	{
		G2Packet* pPool = ( G2Packet* )m_pPools[ nIndex ];
		delete [] pPool;
	}

	m_pPools.clear();;
	m_pFree = 0;
	m_nFree = 0;
}

//////////////////////////////////////////////////////////////////////
// G2PacketPool new pool setup

void G2PacketPool::newPool()
{
	G2Packet* pPool = 0;
	int nPitch = 0, nSize = 256;

	nPitch	= sizeof( G2Packet );
	pPool	= new G2Packet[ nSize ];

	m_pPools.append( pPool );

	char* pchars = ( char* )pPool;

	while ( nSize-- > 0 )
	{
		pPool = ( G2Packet* )pchars;
		pchars += nPitch;

		pPool->m_pNext = m_pFree;
		m_pFree = pPool;
		m_nFree++;
	}
}

G2Packet* G2Packet::addOrReplaceChild( const char* pFind, G2Packet* pReplacement, bool bRelease,
									   bool bPreserveExtensions )
{
	m_nPosition = 0; // start at zero

	if ( !m_bCompound )
	{
		// not a compound packet, just prepend this child
		return prependPacket( pReplacement, bRelease );
	}

	G2Packet* pNew = G2Packet::newPacket( m_sType, m_bCompound );

	char szType[9];
	quint32 nLength = 0, nNext = 0;
	bool bCompound = false, bFound = false;

	while ( readPacket( &szType[0], nLength, &bCompound ) )
	{
		nNext = m_nPosition + nLength;

		if ( strcmp( pFind, szType ) == 0 )
		{
			bFound = true;
			if ( !bPreserveExtensions || !bCompound )
			{
				pNew->writePacket( pReplacement );
			}
			else
			{
				// if this is a compound packet, we need to copy child packets

				quint32 nCompoundLength = nLength;
				uchar* pStart = m_pBuffer + m_nPosition;
				skipCompound( nLength );
				nCompoundLength -= nLength;

				G2Packet* pReplace = G2Packet::newPacket( pReplacement->m_sType, true );
				pReplace->write( pStart, nCompoundLength );
				if ( !pReplacement->m_bCompound && pReplacement->m_nLength > 0 )
				{
					pReplace->writeByte( 0 );
				}
				pReplace->write( pReplacement->m_pBuffer, pReplacement->m_nLength );
				pNew->writePacket( pReplace );
				pReplace->release();
			}
		}
		else
		{
			pNew->writePacket( szType, nLength, bCompound );
			pNew->write( m_pBuffer + m_nPosition, nLength );
		}

		m_nPosition = nNext;
	}

	if ( !bFound )
	{
		pNew->release();
		return prependPacket( pReplacement, bRelease );
	}

	if ( getRemaining() )
	{
		pNew->writeByte( 0 );
		pNew->write( m_pBuffer + m_nPosition, m_nLength - m_nPosition );
	}

	uchar*  pBuff = m_pBuffer;
	quint32 nBuff = m_nBuffer;
	m_pBuffer = pNew->m_pBuffer;
	m_nBuffer = pNew->m_nBuffer;
	pNew->m_pBuffer = pBuff;
	pNew->m_nBuffer = nBuff;
	m_nLength = pNew->m_nLength;
	m_nPosition = 0;
	pNew->release();
	if ( bRelease )
	{
		pReplacement->release();
	}

	return this;
}

G2Packet* G2Packet::prependPacket( G2Packet* pPacket, bool bRelease )
{
	if ( pPacket == 0 )
	{
		return 0;
	}

	quint32 nLength = pPacket->m_nLength;

	Q_ASSERT( strlen( ( char* )&pPacket->m_sType ) > 0 );
	Q_ASSERT( nLength <= 0xFFFFFF );

	char nTypeLen	= ( char )( strlen( ( char* )&pPacket->m_sType ) - 1 ) & 0x07;
	char nLenLen	= 0;

	if ( nLength )
	{
		nLenLen++;
		if ( nLength > 0xFF )
		{
			nLenLen++;
			if ( nLength > 0xFFFF )
			{
				nLenLen++;
			}
		}
	}

	char nFlags = ( nLenLen << 6 ) + ( nTypeLen << 3 );

	if ( pPacket->m_bCompound )
	{
		nFlags |= G2_FLAG_COMPOUND;
	}

	quint32 nExpand = 2 + nLenLen + nTypeLen + pPacket->m_nLength + ( m_bCompound ? 0 : 1 );
	uchar* pWrite = writeGetPointer( nExpand, 0 );
	*pWrite = nFlags;
	pWrite++;
	memcpy( pWrite, &nLength, nLenLen );
	pWrite += nLenLen;
	memcpy( pWrite, &pPacket->m_sType, nTypeLen + 1 );
	pWrite += nTypeLen + 1;
	memcpy( pWrite, pPacket->m_pBuffer, pPacket->m_nLength );
	pWrite += pPacket->m_nLength;
	if ( !m_bCompound )
	{
		*pWrite = 0;
	}

	m_bCompound = true;

	if ( bRelease )
	{
		pPacket->release();
	}

	return this;
}

QString G2Packet::dump() const
{
	QString sHex = toHex();
	QString sAscii = toASCII();
	QString sRet;

	int nOffset = 0;

	sRet = QString( "Type: %1, length: %2, compound: %3\nOffset      Hex                             ASCII\n------------------------------------------------------\n" ).arg(
			   m_sType ).arg( m_nLength ).arg( m_bCompound );

	for ( ; nOffset < sAscii.length(); nOffset += 10 )
	{
		QString sLine( "0x%1  %2  %3\n" );

		sRet += sLine.arg( nOffset, 8, 16, QLatin1Char( '0' ) ).arg( sHex.mid( nOffset * 3, 10 * 3 ), -30,
																	 QLatin1Char( ' ' ) ).arg( sAscii.mid( nOffset, 10 ) );
	}

	return sRet;
}

