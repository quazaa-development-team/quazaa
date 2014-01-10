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

#include "types.h"
class QByteArray;

class CBuffer
{
protected:
	char* 	m_pBuffer;	// allocated block
	quint32	m_nLength;	// length of data it stores
	quint32	m_nBuffer;	// length of this block
	quint32 m_nMinimum;

	// inlines
public:
	inline char* data()
	{
		return m_pBuffer;
	}

	inline quint32 size() const
	{
		return m_nLength;
	}

	inline quint32 capacity() const
	{
		return m_nBuffer;
	}

	inline bool isEmpty() const
	{
		return (m_nLength == 0);
	}

	inline void setMinimumSize(quint32 nSize)
	{
		m_nMinimum = nSize;
	}

	inline CBuffer& clear()
	{
		m_nLength = 0;
		return *this;
	}

public:
	CBuffer(quint32 nMinimum = 1024u);
	~CBuffer();

	CBuffer& append(const void* pData, const quint32 nLength);
	CBuffer& append(const char* pStr);
	CBuffer& append(QByteArray& baData);
	CBuffer& append(CBuffer& pOther);
	CBuffer& append(CBuffer* pOther);

	CBuffer& prepend(const void* pData, const quint32 nLength);
	CBuffer& prepend(const char* pStr);

	CBuffer& insert(const quint32 i, const void* pData, const quint32 nLength);
	CBuffer& insert(const quint32 i, const char* pStr);

	/**
	 * @brief remove removes nLength bytes starting at nPos from the buffer.
	 * @param nPos : the position at which to start
	 * @param nLength : the number of bytes to remove
	 * @return the buffer after the requested operation
	 */
	CBuffer& remove(const quint32 nLength, const quint32 nPos = 0);

	void	 ensure(const quint32 nLength);

	void	 resize(const quint32 nLength);

	QString toHex() const;
	QString toAscii() const;
	QString dump() const;
};

#endif // BUFFER_H
