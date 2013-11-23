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

#include "queryhashtable.h"
#include "queryhashmaster.h"
#include "queryhashgroup.h"
#include <QString>
#include "network.h"
#include "neighbour.h"
#include "g2node.h"
#include "g2packet.h"
#include "zlibutils.h"
#include <QByteArray>
#include <QDateTime>
#include "quazaasettings.h"
#include "buffer.h"
#include "query.h"
#include "Hashes/hash.h"

#include "debug_new.h"

// Parts of this code are borrowed from Shareaza

CQueryHashTable::CQueryHashTable() : QObject(0),
	m_bLive(false)
	,	m_nCookie(0ul)
	,	m_pHash(0)
	,	m_nHash(0ul)
	,	m_nBits(0ul)
	,	m_nInfinity(1ul)
	,	m_nCount(0ul)
	,	m_pBuffer(new CBuffer(131072))    // 128KB
	,	m_pGroup(0)
{
}

CQueryHashTable::~CQueryHashTable()
{
	if(m_pGroup)
	{
		QueryHashMaster.remove(this);
	}

	delete [] m_pHash;
	delete m_pBuffer;
}

void CQueryHashTable::create()
{
	const bool bGrouped = (m_pGroup != 0);
	if(bGrouped)
	{
		QueryHashMaster.remove(this);
	}

	delete [] m_pHash;

	m_bLive		= true;
	m_nCookie	= time(0) + 1;
	m_nBits		= quazaaSettings.Library.QueryRouteSize;
	m_nHash		= 1u << m_nBits;
	m_pHash		= new uchar[(m_nHash + 31) / 8 ];
	m_nCount	= 0;

	memset(m_pHash, 0xFF, (m_nHash + 31) / 8);

	if(bGrouped)
	{
		QueryHashMaster.add(this);
	}
}

void CQueryHashTable::clear()
{
	if(!m_pHash)
	{
		return;
	}

	const bool bGrouped = (m_pGroup != 0);
	if(bGrouped)
	{
		QueryHashMaster.remove(this);
	}

	m_nCookie	= time(0) + 1;
	m_nCount	= 0;

	memset(m_pHash, 0xFF, (m_nHash + 31) / 8);

	if(bGrouped)
	{
		QueryHashMaster.add(this);
	}
}

bool CQueryHashTable::merge(const CQueryHashTable* pSource)
{
	if(!m_pHash || !pSource->m_pHash)
	{
		return false;
	}

	if(m_nHash == pSource->m_nHash)
	{
		uchar* pSourcePtr	= pSource->m_pHash;
		uchar* pDestPtr		= m_pHash;

		for(quint32 nPosition = m_nHash >> 3 ; nPosition ; --nPosition)
		{
			register uchar nSourceByte = *pSourcePtr;
			register uchar nDestByte = *pDestPtr;

#define DO_MERGE(MASKVAL) \
	if ( ! ( nSourceByte & MASKVAL ) && ( nDestByte & MASKVAL ) ) \
	{ \
		*pDestPtr &= ~ MASKVAL; \
		++m_nCount; \
	}

			DO_MERGE(0x01);
			DO_MERGE(0x02);
			DO_MERGE(0x04);
			DO_MERGE(0x08);
			DO_MERGE(0x10);
			DO_MERGE(0x20);
			DO_MERGE(0x40);
			DO_MERGE(0x80);
#undef DO_MERGE

			++pSourcePtr;
			++pDestPtr;
		}
	}
	else
	{
		int nDestScale		= 1;
		int nSourceScale	= 1;

		if(m_nHash > pSource->m_nHash)
		{
			quint32 nIterate = pSource->m_nHash;
			for(; nIterate < m_nHash ; nIterate *= 2)
			{
				++nDestScale;
			}

			if(nIterate != m_nHash)
			{
				return false;
			}
		}
		else if(m_nHash < pSource->m_nHash)
		{
			quint32 nIterate = m_nHash;
			for(; nIterate < pSource->m_nHash ; nIterate *= 2)
			{
				++nSourceScale;
			}

			if(nIterate != pSource->m_nHash)
			{
				return false;
			}
		}

		uchar* pSourcePtr	= pSource->m_pHash;
		uchar* pDestPtr		= m_pHash;
		uchar nSourceMask	= 0x01;
		uchar nDestMask		= 0x01;

		for(quint32 nDest = 0, nSource = 0 ; nDest < m_nHash && nSource < pSource->m_nHash ;)
		{
			bool bValue = true;

			for(int nSample = 0 ; nSample < nSourceScale ; ++nSample, ++nSource)
			{
				if((*pSourcePtr & nSourceMask) == 0)
				{
					bValue = false;
				}

				if(nSourceMask == 0x80)
				{
					nSourceMask = 0x01;
					++pSourcePtr;
				}
				else
				{
					nSourceMask <<= 1;
				}
			}

			for(int nSample = 0 ; nSample < nDestScale ; ++nSample, ++nDest)
			{
				if(! bValue && (*pDestPtr & nDestMask))
				{
					*pDestPtr &= ~nDestMask;
					++m_nCount;
				}

				if(nDestMask == 0x80)
				{
					nDestMask = 0x01;
					++pDestPtr;
				}
				else
				{
					nDestMask <<= 1;
				}
			}
		}
	}

	m_nCookie = time(0) + 1;

	return true;
}

bool CQueryHashTable::merge(const CQueryHashGroup* pSource)
{
	if(!m_pHash || !pSource->m_pHash)
	{
		return false;
	}

	if(m_nHash == pSource->m_nHash)
	{
		uchar* pSourcePtr	= pSource->m_pHash;
		uchar* pDestPtr		= m_pHash;

		for(quint32 nPosition = m_nHash >> 3 ; nPosition ; --nPosition)
		{
			register uchar nDestByte = *pDestPtr;

#define DO_MERGE(MASKVAL) \
	if ( *pSourcePtr++ && ( nDestByte & MASKVAL ) ) \
	{ \
		*pDestPtr &= ~ MASKVAL; \
		m_nCount++; \
	}

			DO_MERGE(0x01);
			DO_MERGE(0x02);
			DO_MERGE(0x04);
			DO_MERGE(0x08);
			DO_MERGE(0x10);
			DO_MERGE(0x20);
			DO_MERGE(0x40);
			DO_MERGE(0x80);
#undef DO_MERGE

			++pDestPtr;
		}
	}
	else
	{
		int nDestScale		= 1;
		int nSourceScale	= 1;

		if(m_nHash > pSource->m_nHash)
		{
			quint32 nIterate = pSource->m_nHash;
			for(; nIterate < m_nHash ; nIterate *= 2)
			{
				++nDestScale;
			}

			if(nIterate != m_nHash)
			{
				return false;
			}
		}
		else if(m_nHash < pSource->m_nHash)
		{
			quint32 nIterate = m_nHash;
			for(; nIterate < pSource->m_nHash ; nIterate *= 2)
			{
				++nSourceScale;
			}

			if(nIterate != pSource->m_nHash)
			{
				return false;
			}
		}

		uchar* pSourcePtr	= pSource->m_pHash;
		uchar* pDestPtr		= m_pHash;
		uchar nDestMask		= 0x01;

		for(quint32 nDest = 0, nSource = 0 ; nDest < m_nHash && nSource < pSource->m_nHash ;)
		{
			bool bValue = true;

			for(int nSample = 0 ; nSample < nSourceScale ; nSample++, nSource++)
			{
				if(*pSourcePtr++)
				{
					bValue = false;
				}
			}

			for(int nSample = 0 ; nSample < nDestScale ; ++nSample, ++nDest)
			{
				if(! bValue && (*pDestPtr & nDestMask))
				{
					*pDestPtr &= ~nDestMask;
					++m_nCount;
				}

				if(nDestMask == 0x80)
				{
					nDestMask = 0x01;
					++pDestPtr;
				}
				else
				{
					nDestMask <<= 1;
				}
			}
		}
	}

	m_nCookie = time(0);

	return true;
}

bool CQueryHashTable::patchTo(const CQueryHashTable* pTarget,
							  CG2Node* pNeighbour)
{
	if(!pTarget->m_pHash)
	{
		return false;
	}

	if(m_nCookie == pTarget->m_nCookie)
	{
		return false;
	}

	m_nCookie	= pTarget->m_nCookie;
	m_nCount	= pTarget->m_nCount;

	bool bChanged = false;

	if(!m_pHash || m_nHash != pTarget->m_nHash)
	{
		delete [] m_pHash;
		m_pHash = 0;

		m_nBits		= pTarget->m_nBits;
		m_nHash		= pTarget->m_nHash;
		m_pHash		= new uchar[(m_nHash + 31) / 8 ];

		memset(m_pHash, 0xFF, (m_nHash + 31) / 8);

		G2Packet* pReset = G2Packet::newPacket("QHT");
		pReset->writeByte(0);
		pReset->writeIntLE(m_nHash);
		pReset->writeByte(1);

		pNeighbour->sendPacket(pReset, false, true);

		bChanged = true;
	}

	CBuffer baBuffer;
	baBuffer.resize((m_nHash + 31) / 8);
	uchar* pBuffer	= (uchar*)baBuffer.data();
	uchar* pHashT	= pTarget->m_pHash;
	uchar* pHashS	= m_pHash;

	const quint32 nEnd = (m_nHash + 31) / 32;
	quint32* const pDwordBuffer = reinterpret_cast< quint32* >(pBuffer);
	const quint32* const pDwordHashS = reinterpret_cast< quint32* >(pHashS);
	const quint32* const pDwordHashT = reinterpret_cast< quint32* >(pHashT);
	for(quint32 nPosition = 0; nPosition < nEnd; ++nPosition)
	{
		if((pDwordBuffer[ nPosition ] = pDwordHashS[ nPosition ] ^ pDwordHashT[ nPosition ]) != 0)
		{
			bChanged = true;
		}
	}
	if(bChanged)
	{
		memcpy(pHashS, pHashT, (m_nHash + 31) / 8);
	}

	if(!bChanged && m_bLive)
	{
		return false;
	}

	baBuffer.resize(m_nHash / 8);

	if(!ZLibUtils::compressBuffer(baBuffer))
	{
		systemLog.postLog(LogSeverity::Debug, "QHT compress error");
		//qDebug() << "QHT compress error";
		return false;
	}

	quint32 nFrags = 1;
	const quint32 nFragSize = 2048;
	quint32 nToWrite = baBuffer.size();

	while(nToWrite > nFrags * nFragSize)
	{
		nFrags++;
	}

	quint32 nOffset = 0;
	for(uchar nFrag = 1; nFrag <= nFrags; nFrag++)
	{
		G2_QHT_PATCH p;
		p.nCmd = 1;
		p.nBits = 1;
		p.nCompression = 1;
		p.nFragCount = nFrags;
		p.nFragNum = nFrag;

		G2Packet* pPatch = G2Packet::newPacket("QHT");
		pPatch->write((void*)&p, sizeof(p));

		quint32 nFs = qMin(nToWrite, nFragSize);

		pPatch->write((void*)(baBuffer.data() + nOffset), nFs);

		nOffset += nFs;
		nToWrite -= nFs;
		pNeighbour->sendPacket(pPatch, false, true);
	}

	m_bLive = true;

	return true;
}

bool CQueryHashTable::onPacket(G2Packet* pPacket)
{
	if(pPacket->m_nLength < 1)
	{
		return false;
	}

	quint32 nLength = pPacket->m_nLength;
	if(pPacket->m_bCompound)
	{
		pPacket->skipCompound(nLength);
	}

	uchar nVariant = pPacket->readByte();

	if(nVariant == 0)
	{
		return onReset(pPacket);
	}
	else if(nVariant == 1)
	{
		return onPatch(pPacket);
	}

	return false;
}

bool CQueryHashTable::onReset(G2Packet* pPacket)
{
	if(pPacket->m_nLength != 6)
	{
		return false;
	}

	quint32 nHashSize	= 0;

	const bool bGrouped = (m_pGroup != 0);
	if(bGrouped)
	{
		QueryHashMaster.remove(this);
	}

	nHashSize	= pPacket->readIntLE<quint32>();
	m_nInfinity	= pPacket->readByte();

	if(nHashSize < 64)
	{
		return false;
	}

	if(!m_pHash || nHashSize != m_nHash)
	{
		delete [] m_pHash;
		m_pHash = 0;

		for(m_nHash = 1, m_nBits = 0 ; m_nHash < nHashSize ; ++m_nBits)
		{
			m_nHash *= 2;
		}

		if(m_nHash != nHashSize)
		{
			return false;
		}

		if(m_nBits > 24)
		{
			return false;
		}

		m_pHash	= new uchar[(m_nHash + 31) / 8 ];
	}

	memset(m_pHash, 0xFF, (m_nHash + 31) / 8);

	if(bGrouped)
	{
		QueryHashMaster.add(this);
	}

	m_bLive		= false;
	m_nCookie	= time(0);
	m_nCount	= 0;

	m_pBuffer->clear();

	return true;
}

bool CQueryHashTable::onPatch(G2Packet* pPacket)
{
	if(pPacket->m_nLength < 5)
	{
		return false;
	}

	if(!m_pHash)
	{
		return false;
	}

	if(!m_pBuffer)
	{
		return false;
	}

	uchar nSequence		= pPacket->readByte();
	uchar nMaximum		= pPacket->readByte();
	uchar nCompression	= pPacket->readByte();
	uchar nBits			= pPacket->readByte();

	if(nBits != 1 && nBits != 4 && nBits != 8)
	{
		return false;
	}

	if(nSequence < 1 || nSequence > nMaximum)
	{
		return false;
	}

	if(nCompression > 1)
	{
		return false;
	}

	if(nSequence == 1)
	{
		m_pBuffer->clear();
	}

	m_pBuffer->append((char*)pPacket->m_pBuffer + pPacket->m_nPosition,
					  pPacket->m_nLength - pPacket->m_nPosition);

	if(nSequence < nMaximum)
	{
		return true;
	}

	if(nCompression == 1)
	{
		ZLibUtils::uncompressBuffer(*m_pBuffer);
	}

	if(m_pBuffer->size() != m_nHash / (8 / nBits))
	{
		m_pBuffer->clear();
		return false;
	}

	uchar* pData		= (uchar*)m_pBuffer->data();
	uchar* pHash		= m_pHash;

	const bool bGroup = (m_pGroup && m_pGroup->m_nHash == m_nHash);
	uchar* pGroup	= bGroup ? m_pGroup->m_pHash : 0;

	if(nBits == 1)
	{
		for(quint32 nPosition = (m_nHash >> 3) ; nPosition ; --nPosition, ++pHash, ++pData)
		{
			for(uchar nMask = 1 ; ; nMask <<= 1)
			{
				if(*pData & nMask)
				{
					if(*pHash & nMask)
					{
						++m_nCount;
						*pHash &= ~nMask;
						if(bGroup)
						{
#ifdef _DEBUG
							Q_ASSERT(*pGroup < 255);
							if(*pGroup == 0)
							{
								++m_pGroup->m_nCount;
							}
#endif
							++(*pGroup);
						}
					}
					else
					{
						--m_nCount;
						*pHash |= nMask;

						if(bGroup)
						{
#ifdef _DEBUG
							Q_ASSERT(*pGroup);
							if(*pGroup == 1)
							{
								--m_pGroup->m_nCount;
							}
#endif
							--(*pGroup);
						}
					}
				}

				++pGroup;

				if(nMask == 0x80)
				{
					break;
				}
			}
		}
	}
	else
	{
		m_pBuffer->clear();
		return false;
	}

	m_bLive		= true;
	m_nCookie	= time(0);

	if(bGroup)
	{
		QueryHashMaster.invalidate();
	}

	return true;
}

void CQueryHashTable::addExactString(const QString& strString)
{
	if(! m_pHash)
	{
		return;
	}

	QByteArray baUTF8;
	baUTF8 = strString.toUtf8();

	addExact(baUTF8.data(), baUTF8.size());
}

void CQueryHashTable::add(const char* pszString, quint32 nLength)
{
	if(nLength < 4)
	{
		return;
	}

	quint32 tNow = time(0);

	quint32 nHash	= hashWord(pszString, nLength, m_nBits);
	uchar* pHash	= m_pHash + (nHash >> 3);
	uchar nMask	= uchar(1 << (nHash & 7));
	if(*pHash & nMask)
	{
		m_nCookie = tNow;
		++m_nCount;
		*pHash &= ~nMask;
	}

	if(nLength >= 5)
	{
		nHash	= hashWord(pszString, nLength - 1, m_nBits);
		pHash	= m_pHash + (nHash >> 3);
		nMask	= uchar(1 << (nHash & 7));
		if(*pHash & nMask)
		{
			m_nCookie = tNow;
			++m_nCount;
			*pHash &= ~nMask;
		}

		nHash	= hashWord(pszString, nLength - 2, m_nBits);
		pHash	= m_pHash + (nHash >> 3);
		nMask	= uchar(1 << (nHash & 7));
		if(*pHash & nMask)
		{
			m_nCookie = tNow;
			++m_nCount;
			*pHash &= ~nMask;
		}
	}
}

void CQueryHashTable::addExact(const char* pszString, quint32 nLength)
{
	if(! nLength)
	{
		return;
	}

	quint32 nHash	= hashWord(pszString, nLength, m_nBits);
	uchar* pHash	= m_pHash + (nHash >> 3);
	uchar nMask		= uchar(1 << (nHash & 7));
	if(*pHash & nMask)
	{
		m_nCookie = time(0);
		++m_nCount;
		*pHash &= ~nMask;
	}
}

bool CQueryHashTable::checkString(const QString& strString) const
{
	if(!m_bLive || !m_pHash || strString.isEmpty())
	{
		return true;
	}

	QByteArray baUTF8;
	baUTF8 = strString.toUtf8();

	quint32 nHash	= hashWord(baUTF8.data(), baUTF8.size(), m_nBits);
	uchar* pHash	= m_pHash + (nHash >> 3);
	uchar nMask		= uchar(1 << (nHash & 7));

	return !(*pHash & nMask);
}

bool CQueryHashTable::checkHash(const quint32 nHash) const
{
	if(!m_bLive || !m_pHash)
	{
		return true;
	}

	quint32 lHash	= nHash >> (32 - m_nBits);
	uchar* pHash	= m_pHash + (lHash >> 3);
	uchar nMask		= uchar(1 << (lHash & 7));

	return !(*pHash & nMask);
}

int CQueryHashTable::getPercent() const
{
	if(!m_pHash || !m_nHash)
	{
		return 0;
	}

	return m_nCount * 100 / m_nHash;
}

quint32 CQueryHashTable::hashWord(const char* pSz, quint32 nLength, qint32 nBits)
{
	quint32 nNumber = 0;
	int nByte = 0;
	for(; nLength > 0 ; nLength--, pSz++)
	{
		int nValue = tolower(*pSz) & 0xFF;
		nValue = nValue << (nByte * 8);
		nByte = (nByte + 1) & 3;
		nNumber = nNumber ^ nValue;
	}
	return hashNumber(nNumber, nBits);
}

quint32 CQueryHashTable::hashNumber(quint32 nNumber, qint32 nBits)
{
	quint64 nProduct = (quint64)nNumber * (quint64)0x4F1BBCDC;
	quint64 nHash = (nProduct << 32) >> (32 + (32 - nBits));
	return (quint32)nHash;
}

void CQueryHashTable::addString(const QString& strString)
{
	if(!m_pHash)
	{
		return;
	}

	QStringList keywords;

	if(CQueryHashTable::makeKeywords(strString, keywords))
	{
		foreach(QString kw, keywords)
		{
			QByteArray baWord = kw.toUtf8();
			add(baWord.data(), baWord.size());
		}
	}
}

int CQueryHashTable::makeKeywords(QString sPhrase, QStringList& outList)
{
	systemLog.postLog(LogSeverity::Debug, QString("Making keywords from: %1").arg(sPhrase));
	//qDebug() << "Making keywords from:" << sPhrase;

	sPhrase = sPhrase.replace("_", " ").simplified().toLower();

	// split it into words
	QStringList lOut;
	lOut = sPhrase.split(QRegExp("\\W+"), QString::SkipEmptyParts);

	// now filter out too short words and only numeric
	QRegExp rx("^\\d+$");
	foreach(QString sWord, lOut)
	{
		// not specs compliant, Shareaza does this too
		if(sWord.length() < 4)
		{
			continue;
		}

		if(rx.indexIn(sWord) != -1)
		{
			continue;
		}

		outList.append(sWord);

		if(sWord.length() > 5)
		{
			outList.append(sWord.left(sWord.length() - 1));
			outList.append(sWord.left(sWord.length() - 2));
		}
	}

	foreach(QString sDebug, outList)
	{
		systemLog.postLog(LogSeverity::Debug, QString("Added keyword: %1").arg(sDebug));
		//qDebug() << "Added keyword:" << sDebug;
	}

	return outList.size();
}

bool CQueryHashTable::checkQuery(CQueryPtr pQuery)
{
	if( !m_bLive || !m_pHash )
		return true;

	for(int i = 0; i < pQuery->m_lHashes.size(); ++i)
	{
		if(checkString(pQuery->m_lHashes[i].toURN()))
			return true;
	}

	int nWords = 0, nWordHits = 0;

	if( pQuery->m_lHashedKeywords.size() )
	{
		foreach(quint32 nHash, pQuery->m_lHashedKeywords)
		{
			nWords++;
			if(checkHash(nHash))
				nWordHits++;
		}
	}

	return (nWords >= 3) ? (nWordHits * 3 / nWords >= 2) : (nWords == nWordHits && nWords > 0);
}

