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

CBuffer::CBuffer(quint32 nMinimum) :
	m_pBuffer(0),
	m_nLength(0),
	m_nBuffer(0)
{
	m_nMinimum = nMinimum;
}

CBuffer::~CBuffer()
{
	if(m_pBuffer)
	{
		free(m_pBuffer);
	}
}

CBuffer& CBuffer::append(const void* pData, const quint32 nLength)
{
	if(pData == 0 || nLength == 0)
	{
		return *this;
	}

	ensure(nLength);

	memcpy(m_pBuffer + m_nLength, pData, nLength);

	m_nLength += nLength;

	return *this;
}

CBuffer& CBuffer::append(const char* pStr)
{
	return append(pStr, qstrlen(pStr));
}

CBuffer& CBuffer::append(QByteArray& baData)
{
	return append(baData.data(), baData.size());
}

CBuffer& CBuffer::append(CBuffer& pOther)
{
	return append(pOther.data(), pOther.size());
}

CBuffer &CBuffer::append(CBuffer *pOther)
{
	return append(pOther->data(), pOther->size());
}

CBuffer& CBuffer::prepend(const void* pData, const quint32 nLength)
{
	return insert(0, pData, nLength);
}

CBuffer& CBuffer::prepend(const char* pStr)
{
	return prepend(pStr, qstrlen(pStr));
}

CBuffer& CBuffer::insert(const quint32 i, const void* pData, const quint32 nLength)
{
	if(pData == 0)
	{
		return *this;
	}

	ensure(nLength);

	memmove(m_pBuffer + i + nLength, m_pBuffer + i, m_nLength - i);

	memcpy(m_pBuffer + i, pData, nLength);

	m_nLength += nLength;

	return *this;
}

CBuffer& CBuffer::insert(const quint32 i, const char* pStr)
{
	return insert(i, pStr, qstrlen(pStr));
}

CBuffer& CBuffer::remove(const quint32 nPos, const quint32 nLength)
{
	if(nPos == 0 && nLength >= m_nLength)
	{
		m_nLength = 0;
	}
	else if(nPos + nLength >= m_nLength)
	{
		m_nLength = nPos;
	}
	else
	{
		memmove(m_pBuffer + nPos, m_pBuffer + nPos + nLength, m_nLength - nPos - nLength);
		m_nLength -= nLength;
	}

	return *this;
}

CBuffer& CBuffer::remove(const quint32 nLength)
{
	return remove(0, nLength);
}

void CBuffer::ensure(const quint32 nLength)
{
	if(nLength > 0xffffffff - m_nBuffer)
	{
		qWarning() << "nLength > UINT_MAX in CBuffer::ensure()";
		throw std::bad_alloc();
	}

	if(m_nBuffer - m_nLength > nLength)
	{
		// We shrink the buffer if we allocated twice the minimum and we actually need less than minimum
		if(m_nBuffer > m_nMinimum * 2 && m_nLength + nLength < m_nMinimum)
		{
			const quint32 nBuffer = m_nMinimum;
			char* pBuffer = (char*)realloc(m_pBuffer, nBuffer);
			if(! pBuffer)
			{
				return;
			}
			m_nBuffer = nBuffer;
			m_pBuffer = pBuffer;
		}
		return;
	}

	quint32 nBuffer = m_nLength + nLength;

	// first alloc will be m_nMinimum bytes or 1024
	// if we need more, we allocate twice as needed
	if(nBuffer < qMax(1024u, m_nMinimum))
	{
		nBuffer = qMax(1024u, m_nMinimum);
	}
	else
	{
		nBuffer *= 2;
	}

	char* pBuffer = (char*)realloc(m_pBuffer, nBuffer);

	if(!pBuffer)
	{
		qWarning() << "Out of memory in CBuffer::ensure()";
		throw std::bad_alloc();
	}

	m_nBuffer = nBuffer;
	m_pBuffer = pBuffer;
}

void CBuffer::resize(const quint32 nLength)
{
	if(nLength <= m_nLength || nLength <= m_nBuffer)
	{
		m_nLength = nLength;
	}
	else if(nLength > m_nBuffer)
	{
		char* pBuffer = (char*)realloc(m_pBuffer, nLength * 2);
		if(!pBuffer)
		{
			qWarning() << "Out of memory in CBuffer::resize()";
			throw std::bad_alloc();
		}
		m_pBuffer = pBuffer;
		m_nBuffer = nLength * 2;
		m_nLength = nLength;
	}
}

QString CBuffer::toHex() const
{
	const char* pszHex = "0123456789ABCDEF";
	QByteArray strDump;

	strDump.resize(m_nLength * 3);
	char* pszDump = strDump.data();

	for(quint32 i = 0 ; i < m_nLength ; i++)
	{
		int nChar = *reinterpret_cast<uchar*>(&m_pBuffer[i]);
		if(i)
		{
			*pszDump++ = ' ';
		}
		*pszDump++ = pszHex[ nChar >> 4 ];
		*pszDump++ = pszHex[ nChar & 0x0F ];
	}

	*pszDump = 0;

	return strDump;
}

QString CBuffer::toAscii() const
{
	QByteArray strDump;

	strDump.resize(m_nLength + 1);
	char* pszDump = strDump.data();

	for(uint i = 0 ; i < m_nLength ; i++)
	{
		int nChar = *reinterpret_cast<uchar*>(&m_pBuffer[i]);
		*pszDump++ = (nChar >= 32 ? nChar : '.');
	}

	*pszDump = 0;

	return strDump;
}

QString CBuffer::dump() const
{
	QString sHex = toHex();
	QString sAscii = toAscii();
	QString sRet;

	int nOffset = 0;

	sRet = QString("Length: %1 \nOffset      Hex                             ASCII\n------------------------------------------------------\n").arg(m_nLength);

	for( ; nOffset < sAscii.length(); nOffset += 10 )
	{
		QString sLine("0x%1  %2  %3\n");

		sRet += sLine.arg(nOffset, 8, 16, QLatin1Char('0')).arg(sHex.mid(nOffset * 3, 10 * 3), -30, QLatin1Char(' ')).arg(sAscii.mid(nOffset, 10));
	}

	return sRet;
}
