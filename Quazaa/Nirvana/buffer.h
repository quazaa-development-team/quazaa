/*
** buffer.h
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

#ifndef BUFFER_H
#define BUFFER_H

#include "idprovider.h"

#include "types.h"
class QByteArray;

// TODO: remove all asserts after careful testing

#define DEBUG_BUFFER 1

/**
 * @brief The CBuffer class is designed to store POD in a byte representation.
 */
class Buffer
{
protected:
	char*   m_pBuffer;        // allocated block
	quint32 m_nAllocatedSize; // length of the allocated block
	quint32 m_nMinimumAllocationSize;

	quint32 m_nPreceding;     // amount of free allocated space preceding the used space
	quint32	m_nCurrentSize;   // length of the current block of data contained in the buffer
	char*   m_pData;          // start of used buffer space

#if DEBUG_BUFFER
	quint32 m_nID;
	static IDProvider<quint32> m_oIDProvider;
#endif

public:
	Buffer( quint32 nMinimum = 1024u );
	~Buffer();

	inline char*        data();
	inline const char*  data() const;
	inline quint32      size() const;
	inline quint32      capacity() const;
	inline bool         isEmpty() const;
	inline void         setMinimumAllocationSize( quint32 nSize );
	inline Buffer&      clear();

	/**
	 * @brief append writes nLength bytes of data from source to the end of this buffer. A deep copy
	 * of the data in question is performed.
	 * @param pData : the source
	 * @param nLength : the amount of bytes to copy
	 * @return *this
	 */
	Buffer& append( const void* pData, const quint32 nLength );
	Buffer& append( const char* pCStr );
	Buffer& append( const QByteArray& baData );
	Buffer& append( const Buffer& pOther );
	Buffer& append( const Buffer* pOther );

	/**
	 * @brief prepend writes nLength bytes of data to the beginning of this buffer. A deep copy of
	 * the data in question is performed. Note that this operation requires moving all of the buffer
	 * content, so it might be considerably slower than appending data.
	 * @param pData : the source
	 * @param nLength : the number of bytes to prepend
	 * @return *this
	 */
	Buffer& prepend( const void* pData, const quint32 nLength );
	Buffer& prepend( const char* pCStr );

	/**
	 * @brief insert inserts nLength amount of data bytes at nOffset into the buffer.
	 * @param nOffset : the insert position
	 * @param pData : the data
	 * @param nLength : number of bytes to insert
	 * @return *this
	 */
	Buffer& insert( const quint32 nOffset, const void* pData, const quint32 nLength );
	Buffer& insert( const quint32 nOffset, const char* pCStr );

	/**
	 * @brief remove removes nLength bytes starting at nPos from the buffer.
	 * @param nPos : the position at which to start
	 * @param nLength : the number of bytes to remove
	 * @return the buffer after the requested operation
	 */
	Buffer& remove( const quint32 nLength, const quint32 nPos = 0 );

	/**
	 * @brief ensure makes sure the buffer is able to append at least nLength more bytes.
	 * @param nLength : the number of bytes
	 */
	void ensure( const quint32 nLength );

	/**
	 * @brief ensureFront makes sure the buffer is able to prepend at least nLength more bytes.
	 * @param nLength : the number of bytes
	 */
	void ensureFront( const quint32 nLength );

	/**
	 * @brief resize resizes the buffer to hold nLength bytes of data. Note that the actual capacity
	 * might be larger than nLength after this operation.
	 * @param nLength : the new size of the buffer
	 */
	void resize( const quint32 nLength );

	QString toHex() const;
	QString toAscii() const;
	QString dump() const;

private:
	void reallocate( quint32 nNewSize );

	/**
	 * @brief moveDataTo moves all data to position nOffset in the allocated space. Make sure to
	 * ensure() enough space before calling this method.
	 * @param nOffset : the offset position
	 */
	void moveDataTo( const quint32 nOffset );

#if DEBUG_BUFFER
	void writeMessage( QString& sMessage );
#endif
};

char* Buffer::data()
{
	return m_pData;
}

const char* Buffer::data() const
{
	return m_pData;
}

quint32 Buffer::size() const
{
	return m_nCurrentSize;
}

quint32 Buffer::capacity() const
{
	return m_nAllocatedSize;
}

bool Buffer::isEmpty() const
{
	return !m_nCurrentSize;
}

void Buffer::setMinimumAllocationSize( quint32 nSize )
{
	m_nMinimumAllocationSize = nSize;
}

Buffer& Buffer::clear()
{
	m_nCurrentSize = 0;
	m_nPreceding   = 0;

	m_pData = m_pBuffer;

	return *this;
}

#endif // BUFFER_H
