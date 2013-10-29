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

CQuery::CQuery()
{
	m_nMinimumSize = 0;
	m_nMaximumSize = Q_UINT64_C(0xffffffffffffffff);
}

void CQuery::SetGUID(QUuid& guid)
{
	m_oGUID = guid;
}

void CQuery::SetDescriptiveName(QString sDN)
{
	m_sDescriptiveName = sDN;
}
void CQuery::SetMetadata(QString sMeta)
{
	m_sMetadata = sMeta;
}
void CQuery::SetSizeRestriction(quint64 nMin, quint64 nMax)
{
	m_nMinimumSize = nMin;
	m_nMaximumSize = nMax;
}
void CQuery::AddURN(const CHash& pHash)
{
	m_lHashes.append(pHash);
}

G2Packet* CQuery::ToG2Packet(CEndPoint* pAddr, quint32 nKey)
{
	G2Packet* pPacket = G2Packet::New("Q2", true);

	//bool bWantURL = true;
	bool bWantDN = (!m_sDescriptiveName.isEmpty());
	bool bWantMD = !m_sMetadata.isEmpty();
	//bool bWantPFS = true;

	if(pAddr)
	{
		G2Packet* pUDP = pPacket->WritePacket("UDP", 10);
		pUDP->WriteHostAddress(*pAddr);
		pUDP->WriteIntLE(nKey);
	}

	if(bWantDN)
	{
		pPacket->WritePacket("DN", m_sDescriptiveName.toUtf8().size())->WriteString(m_sDescriptiveName, false);
	}
	if(bWantMD)
	{
		pPacket->WritePacket("MD", m_sMetadata.toUtf8().size())->WriteString(m_sMetadata, false);
	}

	foreach(CHash pHash, m_lHashes)
	{
		pPacket->WritePacket("URN", pHash.GetFamilyName().size() + CHash::ByteCount(pHash.getAlgorithm()) + 1);
		pPacket->WriteString(pHash.GetFamilyName() + "\0" + pHash.RawValue(), false);
	}

	/*if( m_nMinimumSize > 0 && m_nMaximumSize < 0xFFFFFFFFFFFFFFFF )
	{
		G2Packet* pSZR = pPacket->WriteChild("SZR");
		pSZR->WriteIntLE(m_nMinimumSize);
		pSZR->WriteIntLE(m_nMaximumSize);
	}
	else if( m_nMinimumSize > 0 )
	{
		G2Packet* pSZR = pPacket->WriteChild("SZR");
		pSZR->WriteIntLE(m_nMinimumSize);
		pSZR->WriteIntLE(0xFFFFFFFFFFFFFFFF);
	}
	else if( m_nMaximumSize < 0xFFFFFFFFFFFFFFFF )
	{
		G2Packet* pSZR = pPacket->WriteChild("SZR");
		pSZR->WriteIntLE(0);
		pSZR->WriteIntLE(m_nMaximumSize);
	}

	if( bWantURL || bWantDN || bWantMD || bWantPFS )
	{
		G2Packet* pInt = pPacket->WriteChild("I");
		if( bWantURL )
			pInt->WriteString("URL", true);
		if( bWantDN )
			pInt->WriteString("DN", true);
		if( bWantMD )
			pInt->WriteString("MD", true);
		if( bWantPFS )
			pInt->WriteString("PFS", true);
	}*/

	pPacket->WriteByte(0);
	pPacket->WriteGUID(m_oGUID);

	return pPacket;
}

void CQuery::BuildG2Keywords(QString strPhrase)
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

	foreach(QString sWord, list)
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

	foreach( QString sWord, lNegative )
	{
		lPositive.removeAll(sWord);
	}

	foreach(QString sWord, lPositive)
	{
		quint32 nHash = CQueryHashTable::HashWord(sWord.toUtf8().constData(), sWord.toUtf8().size(), 32);
		m_lHashedKeywords.append(nHash);
	}
}

CQueryPtr CQuery::FromPacket(G2Packet *pPacket, CEndPoint *pEndpoint)
{
	CQueryPtr pQuery(new CQuery());

	try
	{
		if( pQuery->FromG2Packet(pPacket, pEndpoint) )
			return pQuery;
	}
	catch(...)
	{

	}

	return CQueryPtr();
}

bool CQuery::FromG2Packet(G2Packet *pPacket, CEndPoint *pEndpoint)
{
	if( !pPacket->m_bCompound )
		return false;

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while(pPacket->ReadPacket(&szType[0], nLength))
	{
		nNext = pPacket->m_nPosition + nLength;

		if( strcmp("UDP", szType) == 0 && nLength >= 6 )
		{
			if( nLength > 18 )
			{
				// IPv6
				pPacket->ReadHostAddress(&m_oEndpoint, false);
			}
			else
			{
				// IPv4
				pPacket->ReadHostAddress(&m_oEndpoint);
			}

			if( m_oEndpoint.isNull() && pEndpoint )
				m_oEndpoint = *pEndpoint;

			if( nLength >= 10 || nLength >= 22 )
			{
				m_nQueryKey = pPacket->ReadIntLE<quint32>();

				quint32* pKey = (quint32*)(pPacket->m_pBuffer + pPacket->m_nPosition - 4);
				*pKey = 0;
			}
		}
		else if( strcmp("DN", szType) == 0 )
		{
			m_sDescriptiveName = pPacket->ReadString(nLength);
		}
		else if( strcmp("URN", szType) == 0 )
		{
			QString sURN;
			QByteArray hashBuff;
			sURN = pPacket->ReadString();

			if(nLength >= 44u && sURN.compare("bp") == 0)
			{
				hashBuff.resize(CHash::ByteCount(CHash::SHA1));
				pPacket->Read(hashBuff.data(), CHash::ByteCount(CHash::SHA1));
				CHash* pHash = CHash::FromRaw(hashBuff, CHash::SHA1);
				if(pHash)
				{
					m_lHashes.append(*pHash);
					delete pHash;
				}
				// TODO: Tiger
			}
			else if(nLength >= CHash::ByteCount(CHash::SHA1) + 5u && sURN.compare("sha1") == 0)
			{
				hashBuff.resize(CHash::ByteCount(CHash::SHA1));
				pPacket->Read(hashBuff.data(), CHash::ByteCount(CHash::SHA1));
				CHash* pHash = CHash::FromRaw(hashBuff, CHash::SHA1);
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
				m_nMinimumSize = pPacket->ReadIntLE<quint64>();
				m_nMaximumSize = pPacket->ReadIntLE<quint64>();
			}
			else
			{
				m_nMinimumSize = pPacket->ReadIntLE<quint32>();
				m_nMaximumSize = pPacket->ReadIntLE<quint32>();
			}

		}
		else if( strcmp("I", szType) == 0 )
		{

		}

		// TODO: /Q2/MD

		pPacket->m_nPosition = nNext;
	}

	if( pPacket->GetRemaining() < 16 )
		return false;

	m_oGUID = pPacket->ReadGUID();

	return CheckValid();
}

bool CQuery::CheckValid()
{
	BuildG2Keywords(m_sDescriptiveName);

	if( m_lHashes.isEmpty() && m_lHashedKeywords.isEmpty() )
		return false;

	return true;
}

