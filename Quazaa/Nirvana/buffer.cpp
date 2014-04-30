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

#include "buffer.h"

#include <QByteArray>
#include <exception>

#include "debug_new.h"

IDProvider<quint32> Buffer::m_oIDProvider;

Buffer::Buffer( quint32 nMinimum ) :
	m_pBuffer( NULL ),
	m_nAllocatedSize( 0 ),
	m_nMinimumAllocationSize( nMinimum ),
	m_nPreceding( 0 ),
	m_nCurrentSize( 0 ),
	m_pData( NULL )
#if DEBUG_BUFFER
	, m_nID( m_oIDProvider.aquire() )
#endif
{
}

Buffer::~Buffer()
{
#if DEBUG_BUFFER
	m_oIDProvider.release( m_nID );
#endif

	if ( m_pBuffer )
	{
		free( m_pBuffer );
	}
}

Buffer& Buffer::append( const void* pData, const quint32 nLength )
{
	if ( pData && nLength )
	{
		ensure( nLength );

		memcpy( m_pData + m_nCurrentSize, pData, nLength );

		m_nCurrentSize += nLength;
	}

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Appended %0 bytes of data." ).arg( QString::number( nLength ) ) );
#endif // DEBUG_BUFFER

	return *this;
}

Buffer& Buffer::append( const char* pCStr )
{
	return append( pCStr, qstrlen( pCStr ) );
}

Buffer& Buffer::append( const QByteArray& baData )
{
	return append( baData.data(), baData.size() );
}

Buffer& Buffer::append( const Buffer& pOther )
{
	return append( pOther.data(), pOther.size() );
}

Buffer& Buffer::append( const Buffer* pOther )
{
	return append( pOther->data(), pOther->size() );
}

Buffer& Buffer::prepend( const void* pData, const quint32 nLength )
{
	ensureFront( nLength );

	memcpy( m_pData - nLength, pData, nLength );

	m_pData      -= nLength;
	m_nPreceding -= nLength;

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Prepended %0 bytes of data." ).arg( QString::number( nLength ) ) );
#endif // DEBUG_BUFFER

	return *this;
}

Buffer& Buffer::prepend( const char* pCStr )
{
	return prepend( pCStr, qstrlen( pCStr ) );
}

Buffer& Buffer::insert( const quint32 nOffset, const void* pData, const quint32 nLength )
{
	if ( pData )
	{
		// if there is enough space preceeding and it makes sense, use it
		if ( m_nPreceding >= nLength && nOffset < m_nCurrentSize / 2 )
		{
			memmove( m_pData - nLength, m_pData, nOffset );
			memcpy( m_pData, pData, nLength );

			m_pData      -= nLength;
			m_nPreceding -= nLength;
		}
		else
		{
			ensure( nLength );

			memmove( m_pData + nOffset + nLength, m_pData + nOffset, m_nCurrentSize - nOffset );
			memcpy( m_pData + nOffset, pData, nLength );
		}

		m_nCurrentSize += nLength;
	}

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Inserted %0 bytes of data at %1."
						 ).arg( QString::number( nLength ), QString::number( nOffset ) ) );
#endif // DEBUG_BUFFER

	return *this;
}

Buffer& Buffer::insert( const quint32 nOffset, const char* pCStr )
{
	return insert( nOffset, pCStr, qstrlen( pCStr ) );
}

Buffer& Buffer::remove( const quint32 nLength, const quint32 nPos )
{
	if ( !nPos )
	{
		// no data remains of previously used block
		if ( nLength >= m_nCurrentSize )
		{
			m_nCurrentSize = 0;
		}
		else // data remains at the end of previously used block
		{
			m_pData += nLength;
			m_nPreceding += nLength;
			m_nCurrentSize -= nLength;
		}
	}
	else
	{
		// data remains at the beginning of previously used block
		if ( nPos + nLength >= m_nCurrentSize )
		{
			m_nCurrentSize = nPos;
		}
		else // data remains at the beginning and the end of previously used block
		{
			memmove( m_pData + nPos, m_pData + nPos + nLength, m_nCurrentSize - nPos - nLength );
			m_nCurrentSize -= nLength;
		}
	}

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Removed %0 bytes of data at %1."
						 ).arg( QString::number( nLength ), QString::number( nPos ) ) );
#endif // DEBUG_BUFFER

	return *this;
}

void Buffer::ensure( const quint32 nLength )
{
	if ( nLength > 0xffffffff - m_nAllocatedSize )
	{
		qWarning() << "nLength > UINT_MAX in CBuffer::ensure()";
		throw std::bad_alloc();
	}

	quint32 nFree = m_nAllocatedSize - m_nCurrentSize;
	quint32 nTrailing = nFree - m_nPreceding;
	if ( nTrailing >= nLength )
	{
		// We shrink the buffer if we allocated twice the minimum and we actually need less than
		// the minimum.
		if ( m_nAllocatedSize >= m_nMinimumAllocationSize * 2 &&
			 m_nCurrentSize + m_nPreceding + nLength < m_nMinimumAllocationSize )
		{
			reallocate( m_nCurrentSize + m_nPreceding + nLength );
		}
	}
	else if ( nFree >= nLength )
	{
		// move everything to start of allocated block
		moveDataTo( 0 );
	}
	else // we need to allocate more memory
	{
		reallocate( m_nCurrentSize + m_nPreceding + nLength );
	}

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Ensured space for %0 more bytes of data."
						 ).arg( QString::number( nLength ) ) );
#endif // DEBUG_BUFFER
}

void Buffer::ensureFront( const quint32 nLength )
{
	if ( m_nPreceding < nLength )
	{
		quint32 nRequired = nLength - m_nPreceding;
		// if there is not enough trailing space to fit what doesn't fit in front
		if ( m_nAllocatedSize - m_nCurrentSize - m_nPreceding < nRequired )
		{
			reallocate( nLength + m_nCurrentSize );
		}

		moveDataTo( nLength );
	}

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Ensured space %0 bytes of data (front)."
						 ).arg( QString::number( nLength ) ) );
#endif // DEBUG_BUFFER
}

void Buffer::resize( const quint32 nLength )
{
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );

	if ( m_nPreceding )
	{
		moveDataTo( 0 );
	}

	if ( nLength > m_nAllocatedSize )
	{
		reallocate( nLength );
	}

	m_nCurrentSize = nLength;

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Resized buffer to %0 bytes length."
						 ).arg( QString::number( nLength ) ) );
#endif // DEBUG_BUFFER
}

QString Buffer::toHex() const
{
	const char* pszHex = "0123456789ABCDEF";
	QByteArray strDump;

	strDump.resize( m_nCurrentSize * 3 );
	char* pszDump = strDump.data();

	for ( quint32 i = 0 ; i < m_nCurrentSize ; ++i )
	{
		int nChar = *reinterpret_cast<uchar*>( &m_pData[i] );
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

QString Buffer::toAscii() const
{
	QByteArray strDump;

	strDump.resize( m_nCurrentSize + 1 );
	char* pszDump = strDump.data();

	for ( uint i = 0 ; i < m_nCurrentSize ; ++i )
	{
		int nChar = *reinterpret_cast<uchar*>( &m_pData[i] );
		*pszDump++ = ( nChar >= 32 ? nChar : '.' );
	}

	*pszDump = 0;

	return strDump;
}

QString Buffer::dump() const
{
	QString sHex = toHex();
	QString sAscii = toAscii();
	QString sRet;

	int nOffset = 0;

	sRet = QString( "Length: %1 \nOffset      Hex                     " ).arg( m_nCurrentSize ) +
		   QString( "        ASCII\n------------------------------------------------------\n" );

	for ( ; nOffset < sAscii.length(); nOffset += 10 )
	{
		QString sLine( "0x%1  %2  %3\n" );

		sRet += sLine.arg( nOffset, 8, 16, QLatin1Char( '0' )
						 ).arg( sHex.mid( nOffset * 3, 10 * 3 ), -30, QLatin1Char( ' ' )
							  ).arg( sAscii.mid( nOffset, 10 ) );
	}

	return sRet;
}

void Buffer::reallocate( quint32 nNewSize )
{
	// first alloc will be m_nMinimum bytes or 1024
	// if we need more, we allocate twice as needed
	quint32 nMinSize = qMax( 1024u, m_nMinimumAllocationSize );
	if ( nNewSize < nMinSize )
	{
		nNewSize = nMinSize;
	}
	else
	{
		nNewSize *= 2;
	}

	char* pBuffer = ( char* )realloc( m_pBuffer, nNewSize );
	if ( !pBuffer )
	{
		qWarning() << "Out of memory in Buffer::reallocate()";
		throw std::bad_alloc();
	}

	m_nAllocatedSize = nNewSize;
	m_pBuffer = pBuffer;
	m_pData = m_pBuffer + m_nPreceding;

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Reallocated buffer with %0 bytes of length."
						 ).arg( QString::number( nNewSize ) ) );
#endif // DEBUG_BUFFER
}

void Buffer::moveDataTo( const quint32 nOffset )
{
	memmove( m_pBuffer + nOffset, m_pData, m_nCurrentSize );
	m_pData = m_pBuffer + nOffset;
	m_nPreceding = nOffset;

#if DEBUG_BUFFER
	Q_ASSERT( m_pData == m_pBuffer + m_nPreceding );
	Q_ASSERT( m_nPreceding + m_nCurrentSize <= m_nAllocatedSize );
	writeMessage( QString( "Meved all data to buffer offset %1."
						 ).arg( QString::number( nOffset ) ) );
#endif // DEBUG_BUFFER

}

#if DEBUG_BUFFER
void Buffer::writeMessage( QString& sMessage )
{
	QString s;
	s = QString( "Buffer %0 (Allocated: %1, Preceding: %2, Used Size: %3) "
			   ).arg( QString::number( m_nID ), QString::number( m_nAllocatedSize ),
					  QString::number( m_nPreceding ), QString::number( m_nCurrentSize ) );
	s += sMessage;

	qDebug() << s.toLocal8Bit().data();
}

#endif // DEBUG_BUFFER
