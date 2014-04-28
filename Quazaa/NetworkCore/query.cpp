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

#include "query.h"
#include "g2packet.h"
#include "queryhashtable.h"
#include "network.h"
#include "Hashes/hash.h"

#include "debug_new.h"

Query::Query()
{
	m_nMinimumSize = 0;
	m_nMaximumSize = Q_UINT64_C(0xffffffffffffffff);
}

void Query::setGUID(QUuid& guid)
{
	m_oGUID = guid;
}

void Query::setDescriptiveName(QString sDN)
{
	m_sDescriptiveName = sDN;
}
void Query::setMetadata(QString sMeta)
{
	m_sMetadata = sMeta;
}
void Query::setSizeRestriction(quint64 nMin, quint64 nMax)
{
	m_nMinimumSize = nMin;
	m_nMaximumSize = nMax;
}
void Query::addURN(const CHash& pHash)
{
	m_lHashes.append(pHash);
}

G2Packet* Query::toG2Packet(CEndPoint* pAddr, quint32 nKey)
{
	G2Packet* pPacket = G2Packet::newPacket("Q2", true);

	//bool bWantURL = true;
	bool bWantDN = (!m_sDescriptiveName.isEmpty());
	bool bWantMD = !m_sMetadata.isEmpty();
	//bool bWantPFS = true;

	if(pAddr)
	{
		G2Packet* pUDP = pPacket->writePacket("UDP", 10);
		pUDP->writeHostAddress(*pAddr);
		pUDP->writeIntLE(nKey);
	}

	if(bWantDN)
	{
		pPacket->writePacket("DN", m_sDescriptiveName.toUtf8().size())->writeString(m_sDescriptiveName, false);
	}
	if(bWantMD)
	{
		pPacket->writePacket("MD", m_sMetadata.toUtf8().size())->writeString(m_sMetadata, false);
	}

	foreach ( const CHash& rHash, m_lHashes )
	{
		pPacket->writePacket("URN", rHash.getFamilyName().size() + CHash::byteCount(rHash.getAlgorithm()) + 1);
		pPacket->writeString(rHash.getFamilyName() + "\0" + rHash.rawValue(), false);
	}

	/*if( m_nMinimumSize > 0 && m_nMaximumSize < 0xFFFFFFFFFFFFFFFF )
	{
		G2Packet* pSZR = pPacket->WriteChild("SZR");
		pSZR->writeIntLE(m_nMinimumSize);
		pSZR->writeIntLE(m_nMaximumSize);
	}
	else if( m_nMinimumSize > 0 )
	{
		G2Packet* pSZR = pPacket->WriteChild("SZR");
		pSZR->writeIntLE(m_nMinimumSize);
		pSZR->writeIntLE(0xFFFFFFFFFFFFFFFF);
	}
	else if( m_nMaximumSize < 0xFFFFFFFFFFFFFFFF )
	{
		G2Packet* pSZR = pPacket->WriteChild("SZR");
		pSZR->writeIntLE(0);
		pSZR->writeIntLE(m_nMaximumSize);
	}

	if( bWantURL || bWantDN || bWantMD || bWantPFS )
	{
		G2Packet* pInt = pPacket->WriteChild("I");
		if( bWantURL )
			pInt->writeString("URL", true);
		if( bWantDN )
			pInt->writeString("DN", true);
		if( bWantMD )
			pInt->writeString("MD", true);
		if( bWantPFS )
			pInt->writeString("PFS", true);
	}*/

	pPacket->writeByte(0);
	pPacket->writeGUID(m_oGUID);

	return pPacket;
}

void Query::buildG2Keywords(QString strPhrase)
{
	QStringList lPositive, lNegative;

	strPhrase = strPhrase.trimmed().replace("_", " ").normalized(QString::NormalizationForm_KC).toLower().append(" ");
	QRegExp re("(-?\\\".*\\\"|-?\\w+)\\W+", Qt::CaseSensitive, QRegExp::RegExp2);
	re.setMinimal(true);

	QStringList list;
	int pos = 0, oldPos = 0;
	bool hasDash = false;

	while ((pos = re.indexIn(strPhrase, pos)) != -1)
	{
		QString sWord = re.cap(1);

		if( hasDash && pos - re.matchedLength() - oldPos == 0 && list.last().size() < 4 && sWord.size() < 4)
		{
			list.last().append("-").append(sWord);
		}
		else
		{
			list << sWord;
		}

		oldPos = pos;
		pos += re.matchedLength();

		if( strPhrase.mid(pos - 1, 1) == "-" )
		{
			hasDash = true;
		}
		else
		{
			hasDash = false;
		}
	}

	list.removeDuplicates();

	for(QStringList::iterator itWord = list.begin(); itWord != list.end();)
	{
		if((*itWord).size() < 4)
		{
			itWord = list.erase(itWord);
		}
		else
		{
			itWord++;
		}
	}

	QRegExp rx("\\w+", Qt::CaseSensitive, QRegExp::RegExp2);

	foreach ( const QString& sWord, list )
	{
		if( sWord.at(0) == '-' && sWord.at(1) != '"' )
		{
			// plain negative word
			m_sG2NegativeWords.append(sWord.mid(1)).append(",");
			lNegative.append(sWord.mid(1));
		}
		else if( sWord.at(0) == '"' )
		{
			// positive quoted phrase
			m_sG2PositiveWords.append(sWord).append(",");

			// extract words
			int p = 0;
			while( (p = rx.indexIn(sWord, p)) != -1 )
			{
				lPositive.append(rx.cap(0));
				p += rx.matchedLength();
			}
		}
		else if( sWord.at(0) == '-' && sWord.at(1) == '"' )
		{
			// negative quoted phrase
			m_sG2NegativeWords.append(sWord).append(",");

			// extract words
			int p = 0;
			while( (p = rx.indexIn(sWord, p)) != -1 )
			{
				lNegative.append(rx.cap(0));
				p += rx.matchedLength();
			}
		}
		else
		{
			// plain positive word
			m_sG2PositiveWords.append(sWord).append(",");
			lPositive.append(sWord);
		}
	}

	m_sG2PositiveWords.chop(1);
	m_sG2NegativeWords.chop(1);

	foreach( const QString& sWord, lNegative )
	{
		lPositive.removeAll(sWord);
	}

	foreach ( const QString& sWord, lPositive )
	{
		quint32 nHash = QueryHashTable::hashWord(sWord.toUtf8().constData(), sWord.toUtf8().size(), 32);
		m_lHashedKeywords.append(nHash);
	}
}

QuerySharedPtr Query::fromPacket(G2Packet *pPacket, CEndPoint *pEndpoint)
{
	QuerySharedPtr pQuery(new Query());

	try
	{
		if( pQuery->fromG2Packet(pPacket, pEndpoint) )
			return pQuery;
	}
	catch(...)
	{

	}

	return QuerySharedPtr();
}

bool Query::fromG2Packet(G2Packet *pPacket, CEndPoint *pEndpoint)
{
	if( !pPacket->m_bCompound )
		return false;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while(pPacket->readPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if( strcmp("UDP", szType) == 0 && nLength >= 6 )
		{
			if( nLength > 18 )
			{
				// IPv6
				pPacket->readHostAddress(&m_oEndpoint, false);
			}
			else
			{
				// IPv4
				pPacket->readHostAddress(&m_oEndpoint);
			}

			if( m_oEndpoint.isNull() && pEndpoint )
				m_oEndpoint = *pEndpoint;

			if( nLength >= 10 || nLength >= 22 )
			{
				m_nQueryKey = pPacket->readIntLE<quint32>();

				quint32* pKey = (quint32*)(pPacket->m_pBuffer + pPacket->m_nPosition - 4);
				*pKey = 0;
			}
		}
		else if( strcmp("DN", szType) == 0 )
		{
			m_sDescriptiveName = pPacket->readString(nLength);
		}
		else if( strcmp("URN", szType) == 0 )
		{
			QString sURN;
			QByteArray hashBuff;
			sURN = pPacket->readString();

			if(nLength >= 44u && sURN.compare("bp") == 0)
			{
				hashBuff.resize(CHash::byteCount(CHash::SHA1));
				pPacket->read(hashBuff.data(), CHash::byteCount(CHash::SHA1));
				CHash* pHash = CHash::fromRaw(hashBuff, CHash::SHA1);
				if(pHash)
				{
					m_lHashes.append(*pHash);
					delete pHash;
				}
				// TODO: Tiger
			}
			else if(nLength >= CHash::byteCount(CHash::SHA1) + 5u && sURN.compare("sha1") == 0)
			{
				hashBuff.resize(CHash::byteCount(CHash::SHA1));
				pPacket->read(hashBuff.data(), CHash::byteCount(CHash::SHA1));
				CHash* pHash = CHash::fromRaw(hashBuff, CHash::SHA1);
				if(pHash)
				{
					m_lHashes.append(*pHash);
					delete pHash;
				}
			}
		}
		else if( strcmp("SZR", szType) == 0 && nLength >= 8 )
		{
			if( nLength >= 16 )
			{
				m_nMinimumSize = pPacket->readIntLE<quint64>();
				m_nMaximumSize = pPacket->readIntLE<quint64>();
			}
			else
			{
				m_nMinimumSize = pPacket->readIntLE<quint32>();
				m_nMaximumSize = pPacket->readIntLE<quint32>();
			}

		}
		else if( strcmp("I", szType) == 0 )
		{

		}

		// TODO: /Q2/MD

		pPacket->m_nPosition = nNext;
	}

	if( pPacket->getRemaining() < 16 )
		return false;

	m_oGUID = pPacket->readGUID();

	return checkValid();
}

bool Query::checkValid()
{
	buildG2Keywords(m_sDescriptiveName);

	if( m_lHashes.isEmpty() && m_lHashedKeywords.isEmpty() )
		return false;

	return true;
}

