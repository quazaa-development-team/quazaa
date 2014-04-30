/*
** g2packet.h
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

#ifndef G2PACKET_H
#define G2PACKET_H

#include "types.h"
#include <QtGlobal>
#include <QMutex>
#include <QList>
#include <stdexcept>

// The Control Byte
// +----+----+----+----+----+----+----+----+
// | 7    6  | 5    4    3  | 2  | 1  | 0  | Bit
// +----+----+----+----+----+----+----+----+
// | Len_Len | Name_Len - 1 | CF | BE | // |
// +----+----+----+----+----+----+----+----+

#define G2_FLAG_BIG_ENDIAN      0x02
#define G2_FLAG_COMPOUND        0x04
#define G2_PACKET_NAMELEN_BITS  0x38
#define G2_PACKET_LENLEN_BITS   0xC0


class Buffer;

class G2Packet
{
	// Construction
protected:
	G2Packet();
	~G2Packet();

public:
	static G2Packet* newPacket( const char* pszType = 0, bool bCompound = false );
	static G2Packet* newPacket( char* pSource );

	// Attributes
public:
	G2Packet* 	m_pNext;
	quint32		m_nReference;

public:
	uchar*		m_pBuffer;
	quint32		m_nBuffer;
	quint32		m_nLength;
	quint32		m_nPosition;
	char		m_sType[9];
	bool		m_bCompound;  // true: packet contains child packet(s); false: otherwise.   (CF bit)
	bool		m_bBigEndian; // true: numeric values in big endian format; false otherwise (BE bit)

	enum { seekStart, seekEnd };

	// Operations
public:
	void	reset();
	void	seek( quint32 nPosition, int nRelative = seekStart );
	uchar* 	writeGetPointer( quint32 nLength, quint32 nOffset = 0xFFFFFFFF );
public:
	char* 	getType() const;

public:
	G2Packet* 	writePacket( G2Packet* pPacket );
	G2Packet* 	writePacket( const char* pszType, quint32 nLength, bool bCompound = false );
	G2Packet*	prependPacket( G2Packet* pPacket, bool bRelease = true );
	bool	readPacket( char* pszType, quint32& nLength, bool* pbCompound = 0 );
	bool	skipCompound();
	bool	skipCompound( quint32& nLength, quint32 nRemaining = 0 );
	bool	getTo( QUuid& pGUID );

public:
	/**
	 * @brief readBuffer reads data from a given buffer and returns the corresponding G2 packet.
	 * @param pBuffer : the buffer
	 * @return ptr to new G2Packet; NULL if no G2 packet cound be read.
	 */
	static	G2Packet* readBuffer( Buffer* pBuffer );
	void	toBuffer( Buffer* pBuffer ) const;

	// Inline Packet Operations
	inline bool isType( const char* sType );
	inline int getRemaining();
	inline bool ensure( quint32 nBytes );

	inline void read( void* pData, int nLength );
	inline void write( void* pData, int nLength );

	template <typename T> inline T readIntBE();
	template <typename T> inline T readIntLE();

	template <typename T> inline void writeIntBE( T nValue );
	template <typename T> inline void writeIntLE( T nValue );

	/**
	 * @brief peakUIntBytes reads nCount bytes starting from source and treats them as a quint32
	 * @param pSource : the byte source
	 * @param nCount : number of bytes - max 4
	 * @param bBigEndian : whether the source has big or little endian encoding
	 * @return an unsigned 32bit integer represantation of the peaked bytes
	 */
	static quint32 peakUIntBytes( const uchar* pSource, const quint8 nCount, bool bBigEndian );

	inline uchar readByte();
	inline void writeByte( uchar nByte );
	inline void readHostAddress( EndPoint* pDest, bool bIP4 = true );
	inline QUuid readGUID();
	inline void writeHostAddress( const EndPoint& src );
	inline void writeGUID( QUuid& guid );

public:
	QString readString( quint32 nMaximum = 0xFFFFFFFF );
	void writeString( QString sToWrite, bool bTerminate = false );
	G2Packet* addOrReplaceChild( const char* pFind, G2Packet* pReplacement, bool bRelease = true,
								 bool bPreserveExtensions = true );
	// Inline Allocation

public:
	inline void addRef();
	inline void release();
	inline void releaseChain();
	void deletePacket();

	QString toHex() const;
	QString toASCII() const;
	QString dump() const;

protected:
	friend class G2PacketPool;
};

class G2PacketPool
{
	// Construction
public:
	G2PacketPool();
	~G2PacketPool();

	// Attributes
protected:
	G2Packet* 	m_pFree;
	quint32		m_nFree;
protected:
	QMutex				m_pSection;
	QList<G2Packet*>	m_pPools;

	// Operations
protected:
	void	clear();
	void	newPool();

	// Inlines
public:
	inline G2Packet* newPacket();
	inline void deletePacket( G2Packet* pPacket );

};

// Inlines impl

// G2Packet
bool G2Packet::isType( const char* sType )
{
	return strcmp( sType, m_sType ) == 0;
}

int G2Packet::getRemaining()
{
	return m_nLength - m_nPosition;
}

bool G2Packet::ensure( quint32 nBytes )
{
	if ( m_nLength + nBytes > m_nBuffer )
	{
		m_nBuffer += qMax( nBytes, 128u );
		m_pBuffer = ( uchar* )realloc( m_pBuffer, m_nBuffer );

		if ( !m_pBuffer )
		{
			return false;
		}
	}

	return true;
}

void G2Packet::read( void* pData, int nLength )
{
	if ( m_nPosition + nLength > m_nLength )
	{
		throw std::overflow_error( "Packet will read past end." );
	}
	memcpy( pData, m_pBuffer + m_nPosition, nLength );
	m_nPosition += nLength;
}

void G2Packet::write( void* pData, int nLength )
{
	ensure( nLength );

	memcpy( m_pBuffer + m_nLength, pData, nLength );
	m_nLength += nLength;
}

template <typename T>
T G2Packet::readIntBE()
{
	if ( m_nLength - m_nPosition < sizeof( T ) )
	{
		throw std::overflow_error( "Packet will read past end." );
	}

	T nRet = qFromBigEndian( *( T* )( m_pBuffer + m_nPosition ) );
	m_nPosition += sizeof( T );
	return nRet;
}

template <typename T>
T G2Packet::readIntLE()
{
	if ( m_nLength - m_nPosition < sizeof( T ) )
	{
		throw std::overflow_error( "Packet will read past end." );
	}

	T nRet = qFromLittleEndian( *( T* )( m_pBuffer + m_nPosition ) );
	m_nPosition += sizeof( T );
	return nRet;
}

template <typename T>
void G2Packet::writeIntBE( T nValue )
{
	nValue = qToBigEndian( nValue );
	write( reinterpret_cast<void*>( &nValue ), sizeof( T ) );
}

template <typename T>
void G2Packet::writeIntLE( T nValue )
{
	nValue = qToLittleEndian( nValue );
	write( reinterpret_cast<void*>( &nValue ), sizeof( T ) );
}

uchar G2Packet::readByte()
{
	uchar nRet;
	read( &nRet, 1 );
	return nRet;
}

void G2Packet::writeByte( uchar nByte )
{
	write( &nByte, 1 );
}

void G2Packet::readHostAddress( EndPoint* pDest, bool bIP4 )
{
	if ( bIP4 )
	{
		quint32 nIP;
		quint16 nPort;
		nIP = readIntBE<quint32>();
		nPort = readIntLE<quint16>();
		pDest->setAddress( nIP );
		pDest->setPort( nPort );
	}
	else
	{
		Q_IPV6ADDR ip6;
		quint16 nPort;
		read( &ip6, 16 );
		nPort = readIntLE<quint16>();
		pDest->setAddress( ip6 );
		pDest->setPort( nPort );
	}
}

QUuid G2Packet::readGUID()
{
	QUuid ret;
	ret.data1 = readIntLE<uint>();
	ret.data2 = readIntLE<ushort>();
	ret.data3 = readIntLE<ushort>();
	read( &ret.data4[0], 8 );

	return ret;
}

void G2Packet::writeHostAddress( const EndPoint& src )
{
	if ( src.protocol() == QAbstractSocket::IPv4Protocol )   // IPv4
	{
		ensure( 6 );
		writeIntBE( src.toIPv4Address() );
		writeIntLE( src.port() );
	}
	else // IPv6
	{
		ensure( 18 );
		Q_IPV6ADDR ip6 = src.toIPv6Address();
		write( &ip6, 16 );
		writeIntLE( src.port() );
	}
}

void G2Packet::writeGUID( QUuid& guid )
{
	ensure( 16 );
	writeIntLE( guid.data1 );
	writeIntLE( guid.data2 );
	writeIntLE( guid.data3 );
	write( &guid.data4[0], 8 );
}

void G2Packet::addRef()
{
	m_nReference++;
}

void G2Packet::release()
{
	if ( this != NULL && ! --m_nReference )
	{
		deletePacket();
	}
}

void G2Packet::releaseChain()
{
	if ( this == NULL )
	{
		return;
	}

	for ( G2Packet* pPacket = this ; pPacket ; )
	{
		G2Packet* pNext = pPacket->m_pNext;
		pPacket->release();
		pPacket = pNext;
	}
}

// G2PacketPool
G2Packet* G2PacketPool::newPacket()
{
	m_pSection.lock();

	if ( !m_nFree )
	{
		newPool();
	}
	Q_ASSERT( m_nFree > 0 );

	G2Packet* pPacket = m_pFree;
	m_pFree = m_pFree->m_pNext;
	--m_nFree;

	m_pSection.unlock();

	pPacket->reset();
	pPacket->addRef();

	return pPacket;
}

void G2PacketPool::deletePacket( G2Packet* pPacket )
{
	Q_ASSERT( pPacket != NULL );
	Q_ASSERT( pPacket->m_nReference == 0 );

	m_pSection.lock();

	pPacket->m_pNext = m_pFree;
	m_pFree = pPacket;
	m_nFree ++;

	m_pSection.unlock();
}

extern G2PacketPool G2Packets;

#endif // G2PACKET_H
