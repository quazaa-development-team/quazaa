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

#include "debug_new.h"

Buffer::Buffer( quint32 nMinimum ) :
	m_pBuffer( NULL ),
	m_nAllocatedSize( 0 ),
	m_nMinimumAllocationSize( nMinimum ),
	m_nCurrentSize( 0 )
{
}

Buffer::~Buffer()
{
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

		memcpy( m_pBuffer + m_nCurrentSize, pData, nLength );

		m_nCurrentSize += nLength;
	}

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
	return insert( 0, pData, nLength );
}

Buffer& Buffer::prepend( const char* pCStr )
{
	return insert( 0, pCStr, qstrlen( pCStr ) );
}

Buffer& Buffer::insert( const quint32 nOffset, const void* pData, const quint32 nLength )
{
	if ( pData )
	{
		ensure( nLength );

		memmove( m_pBuffer + nOffset + nLength, m_pBuffer + nOffset, m_nCurrentSize - nOffset );
		memcpy( m_pBuffer + nOffset, pData, nLength );

		m_nCurrentSize += nLength;
	}

	return *this;
}

Buffer& Buffer::insert( const quint32 nOffset, const char* pCStr )
{
	return insert( nOffset, pCStr, qstrlen( pCStr ) );
}

Buffer& Buffer::remove( const quint32 nLength, const quint32 nPos )
{
	if ( !nPos && nLength >= m_nCurrentSize )
	{
		m_nCurrentSize = 0;
	}
	else if ( nPos + nLength >= m_nCurrentSize )
	{
		m_nCurrentSize = nPos;
	}
	else
	{
		memmove( m_pBuffer + nPos, m_pBuffer + nPos + nLength, m_nCurrentSize - nPos - nLength );
		m_nCurrentSize -= nLength;
	}

	return *this;
}

void Buffer::ensure( const quint32 nLength )
{
	if ( nLength > 0xffffffff - m_nAllocatedSize )
	{
		qWarning() << "nLength > UINT_MAX in CBuffer::ensure()";
		throw std::bad_alloc();
	}

	if ( m_nAllocatedSize > nLength + m_nCurrentSize )
	{
		// We shrink the buffer if we allocated twice the minimum and we actually need less than
		// the minimum.
		if ( m_nAllocatedSize > m_nMinimumAllocationSize * 2 &&
			 m_nCurrentSize + nLength < m_nMinimumAllocationSize )
		{
			try
			{
				reallocate( m_nMinimumAllocationSize );
			}
			catch ( std::bad_alloc )
			{
				// failing to shrink is OK, we just ignore that
			}
		}
	}
	else // we need to allocate more memory
	{
		quint32 nNewBufferSize = m_nCurrentSize + nLength;

		// first alloc will be m_nMinimum bytes or 1024
		// if we need more, we allocate twice as needed
		quint32 nMin = qMax( 1024u, m_nMinimumAllocationSize );
		if ( nNewBufferSize < nMin )
		{
			nNewBufferSize = nMin;
		}
		else
		{
			nNewBufferSize *= 2;
		}

		reallocate( nNewBufferSize );
	}
}

void Buffer::resize( const quint32 nLength )
{
	if ( nLength > m_nAllocatedSize )
	{
		reallocate( qMax( nLength * 2, m_nMinimumAllocationSize ) );
	}

	m_nCurrentSize = nLength;
}

QString Buffer::toHex() const
{
	const char* pszHex = "0123456789ABCDEF";
	QByteArray strDump;

	strDump.resize( m_nCurrentSize * 3 );
	char* pszDump = strDump.data();

	for ( quint32 i = 0 ; i < m_nCurrentSize ; ++i )
	{
		int nChar = *reinterpret_cast<uchar*>( &m_pBuffer[i] );
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
		int nChar = *reinterpret_cast<uchar*>( &m_pBuffer[i] );
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

void Buffer::reallocate( quint32 nNewSize ) throw( std::bad_alloc )
{
	char* pBuffer = ( char* )realloc( m_pBuffer, nNewSize );
	if ( !pBuffer )
	{
		qWarning() << "Out of memory in Buffer::reallocate()";
		throw std::bad_alloc();
	}
	m_pBuffer = pBuffer;
	m_nAllocatedSize = nNewSize;
}
