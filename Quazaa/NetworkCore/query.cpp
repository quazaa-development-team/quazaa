/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2014.
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

#include <QRegularExpression>

#include "query.h"
#include "g2packet.h"
#include "queryhashtable.h"
#include "network.h"

#include "debug_new.h"

Query::Query()
{
	m_nMinimumSize = 0;
	m_nMaximumSize = Q_UINT64_C( 0xffffffffffffffff );
}

void Query::setGUID( QUuid& guid )
{
	m_oGUID = guid;
}

void Query::setDescriptiveName( QString sDN )
{
	m_sDescriptiveName = sDN;
}
void Query::setMetadata( QString sMeta )
{
	m_sMetadata = sMeta;
}
void Query::setSizeRestriction( quint64 nMin, quint64 nMax )
{
	m_nMinimumSize = nMin;
	m_nMaximumSize = nMax;
}
bool Query::addURN( const Hash& rHash )
{
	// returns false on hash conflict
	return m_vHashes.insert( rHash );
}

G2Packet* Query::toG2Packet( EndPoint* pAddr, quint32 nKey )
{
	G2Packet* pPacket = G2Packet::newPacket( "Q2", true );

	//bool bWantURL = true;
	bool bWantDN = ( !m_sDescriptiveName.isEmpty() );
	bool bWantMD = !m_sMetadata.isEmpty();
	//bool bWantPFS = true;

	if ( pAddr )
	{
		G2Packet* pUDP = pPacket->writePacket( "UDP", 10 );
		pUDP->writeHostAddress( *pAddr );
		pUDP->writeIntLE( nKey );
	}

	if ( bWantDN )
	{
		pPacket->writePacket( "DN", m_sDescriptiveName.toUtf8().size() )->writeString( m_sDescriptiveName, false );
	}
	if ( bWantMD )
	{
		pPacket->writePacket( "MD", m_sMetadata.toUtf8().size() )->writeString( m_sMetadata, false );
	}

	for ( quint8 i = 0, nSize = m_vHashes.size(); i < nSize; ++i )
	{
		if ( m_vHashes[i] )
		{
			pPacket->writePacket( "URN", m_vHashes[i]->getFamilyName().size() + Hash::byteCount( m_vHashes[i]->algorithm() ) + 1 );
			pPacket->writeString( m_vHashes[i]->getFamilyName() + "\0" + m_vHashes[i]->rawValue(), false );
		}
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

	pPacket->writeByte( 0 );
	pPacket->writeGUID( m_oGUID );

	return pPacket;
}

void Query::buildG2Keywords( QString strPhrase )
{
	QStringList lPositive, lNegative;

	strPhrase = strPhrase.trimmed().replace( "_", " "
											 ).normalized( QString::NormalizationForm_KC
														   ).toLower().append( " " );
	QStringList lWords;
	{
		QRegularExpression re( "(-?\\\".*\\\"|-?\\w+)\\W+",
							   QRegularExpression::InvertedGreedinessOption );
		int nPos = 0, nOldPos = 0;
		bool bHasDash = false;
		QRegularExpressionMatch oMatch;

		while ( ( oMatch = re.match( strPhrase, nPos ) ).hasMatch() )
		{
			nPos = re.match( strPhrase, nPos ).capturedStart();
			QString sWord = oMatch.captured( 1 );

			if ( bHasDash &&
				 nPos - oMatch.capturedLength() - nOldPos == 0 &&
				 lWords.last().size() < 4 &&
				 sWord.size() < 4 )
			{
				lWords.last().append( "-" ).append( sWord );
			}
			else
			{
				lWords << sWord;
			}

			nOldPos = nPos;
			nPos += oMatch.capturedLength();

			if ( strPhrase.mid( nPos - 1, 1 ) == "-" )
			{
				bHasDash = true;
			}
			else
			{
				bHasDash = false;
			}
		}
	}

	lWords.removeDuplicates();

	for ( QStringList::iterator itWord = lWords.begin(); itWord != lWords.end(); )
	{
		if ( ( *itWord ).size() < 4 )
		{
			itWord = lWords.erase( itWord );
		}
		else
		{
			itWord++;
		}
	}

	QRegularExpression rx( "\\w+" );

	foreach ( const QString& sWord, lWords )
	{
		if ( sWord.at( 0 ) == '-' && sWord.at( 1 ) != '"' )
		{
			// plain negative word
			m_sG2NegativeWords.append( sWord.mid( 1 ) ).append( "," );
			lNegative.append( sWord.mid( 1 ) );
		}
		else if ( sWord.at( 0 ) == '"' )
		{
			// positive quoted phrase
			m_sG2PositiveWords.append( sWord ).append( "," );

			// extract words
			int p = 0;
			QRegularExpressionMatch oMatch;
			while ( ( oMatch = rx.match( sWord, p ) ).hasMatch() )
			{
				p = oMatch.capturedStart() + oMatch.capturedLength();
				lPositive.append( oMatch.captured() );
			}
		}
		else if ( sWord.at( 0 ) == '-' && sWord.at( 1 ) == '"' )
		{
			// negative quoted phrase
			m_sG2NegativeWords.append( sWord ).append( "," );

			// extract words
			int p = 0;
			QRegularExpressionMatch oMatch;
			while ( ( oMatch = rx.match( sWord, p ) ).hasMatch() )
			{
				p = oMatch.capturedStart() + oMatch.capturedLength();
				lNegative.append( oMatch.captured() );
			}
		}
		else
		{
			// plain positive word
			m_sG2PositiveWords.append( sWord ).append( "," );
			lPositive.append( sWord );
		}
	}

	m_sG2PositiveWords.chop( 1 );
	m_sG2NegativeWords.chop( 1 );

	foreach( const QString& sWord, lNegative )
	{
		lPositive.removeAll( sWord );
	}

	foreach ( const QString& sWord, lPositive )
	{
		quint32 nHash = QueryHashTable::hashWord( sWord.toUtf8().constData(), sWord.toUtf8().size(), 32 );
		m_lHashedKeywords.append( nHash );
	}
}

QuerySharedPtr Query::fromPacket( G2Packet* pPacket, const EndPoint* const pEndpoint )
{
	QuerySharedPtr pQuery( new Query() );

	try
	{
		if ( pQuery->fromG2Packet( pPacket, pEndpoint ) )
		{
			return pQuery;
		}
	}
	catch ( ... )
	{
		systemLog.postLog( LogSeverity::Debug, Component::G2,
						   "Failed to parse Query from packet." );
	}

	return QuerySharedPtr();
}

bool Query::fromG2Packet( G2Packet* pPacket, const EndPoint* const pEndpoint )
{
	if ( !pPacket->m_bCompound )
	{
		return false;
	}

	char szType[9];
	quint32 nLength = 0, nNext = 0;

	while ( pPacket->readPacket( &szType[0], nLength ) )
	{
		nNext = pPacket->m_nPosition + nLength;

		if ( strcmp( "UDP", szType ) == 0 && nLength >= 6 )
		{
			if ( nLength > 18 )
			{
				// IPv6
				pPacket->readHostAddress( &m_oEndpoint, false );
			}
			else
			{
				// IPv4
				pPacket->readHostAddress( &m_oEndpoint );
			}

			if ( m_oEndpoint.isNull() && pEndpoint )
			{
				m_oEndpoint = *pEndpoint;
			}

			if ( nLength >= 10 || nLength >= 22 )
			{
				m_nQueryKey = pPacket->readIntLE<quint32>();

				quint32* pKey = ( quint32* )( pPacket->m_pBuffer + pPacket->m_nPosition - 4 );
				*pKey = 0;
			}
		}
		else if ( strcmp( "DN", szType ) == 0 )
		{
			m_sDescriptiveName = pPacket->readString( nLength );
		}
		else if ( strcmp( "URN", szType ) == 0 )
		{
			QString sURN;
			QByteArray hashBuff;
			sURN = pPacket->readString();

			if ( nLength >= 44u && sURN.compare( "bp" ) == 0 )
			{
				hashBuff.resize( Hash::byteCount( Hash::SHA1 ) );
				pPacket->read( hashBuff.data(), Hash::byteCount( Hash::SHA1 ) );
				Hash* pHash = Hash::fromRaw( hashBuff, Hash::SHA1 );
				if ( pHash )
				{
					m_vHashes.insert( pHash );
				}
				// TODO: Tiger
			}
			else if ( nLength >= Hash::byteCount( Hash::SHA1 ) + 5u && sURN.compare( "sha1" ) == 0 )
			{
				hashBuff.resize( Hash::byteCount( Hash::SHA1 ) );
				pPacket->read( hashBuff.data(), Hash::byteCount( Hash::SHA1 ) );
				Hash* pHash = Hash::fromRaw( hashBuff, Hash::SHA1 );
				if ( pHash )
				{
					m_vHashes.insert( pHash );
				}
			}
		}
		else if ( strcmp( "SZR", szType ) == 0 && nLength >= 8 )
		{
			if ( nLength >= 16 )
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
		else if ( strcmp( "I", szType ) == 0 )
		{

		}

		// TODO: /Q2/MD

		pPacket->m_nPosition = nNext;
	}

	if ( pPacket->getRemaining() < 16 )
	{
		return false;
	}

	m_oGUID = pPacket->readGUID();

	return checkValid();
}

bool Query::checkValid()
{
	buildG2Keywords( m_sDescriptiveName );

	if ( m_vHashes.empty() && m_lHashedKeywords.isEmpty() )
	{
		return false;
	}

	return true;
}

