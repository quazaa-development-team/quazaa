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

#include "download.h"
#include "queryhit.h"
#include "downloadsource.h"
#include "downloads.h"
#include "transfers.h"
#include "downloadtransfer.h"

#include "commonfunctions.h"
#include "quazaasettings.h"

#include <QFile>

#include "debug_new.h"

using namespace common;

QDataStream& operator<<( QDataStream& oStream, const Download& rhs )
{
	// basic info
	oStream << quint32( 1 ); // version
	oStream << "dn" << rhs.m_sDisplayName;
	oStream << "tn" << rhs.m_sTempName;
	oStream << "s" << rhs.m_nSize;
	oStream << "cs" << rhs.m_nCompletedSize;
	oStream << "state" << rhs.m_nState;
	oStream << "mf" << rhs.m_bMultifile;
	oStream << "pr" << rhs.m_nPriority;

	// files
	foreach ( const Download::FileListItem & item, rhs.m_lFiles )
	{
		oStream << "file" << quint32( DOWNLOAD_CODE_FILE_VERSION ); // version
		oStream << "name" << item.sFileName;
		oStream << "path" << item.sPath;
		oStream << "tempname" << item.sTempName;
		oStream << "so" << item.nStartOffset;
		oStream << "eo" << item.nEndOffset;

		for ( quint8 i = 0, nSize = item.vHashes.size(); i < nSize; ++i )
		{
			if ( item.vHashes[i] )
			{
				oStream << "hash" << item.vHashes[i]->toURN();
			}
		}

		oStream << "file-end";
	}

	// sources
	foreach ( DownloadSource * pSource, rhs.m_lSources )
	{
		oStream << *pSource;
	}

	oStream << "completed-frags";
	Fragments::SerializeOut( oStream, rhs.m_lCompleted );
	oStream << "verified-frags";
	Fragments::SerializeOut( oStream, rhs.m_lCompleted );

	oStream << "eof";
	return oStream;
}
QDataStream& operator>>( QDataStream& s, Download& rhs )
{
	quint32 nVersion;

	s >> nVersion;

	if ( nVersion == 1 ) // DOWNLOAD_CODE_FILE_VERSION
	{
		QByteArray sTag;
		s >> sTag;
		sTag.chop( 1 );

		while ( sTag != "eof" && s.status() == QDataStream::Ok )
		{
			if ( sTag == "dn" )
			{
				s >> rhs.m_sDisplayName;
			}
			else if ( sTag == "tn" )
			{
				s >> rhs.m_sTempName;
			}
			else if ( sTag == "s" )
			{
				quint64 nSize;
				s >> nSize;
				rhs.m_nSize = nSize;

				Fragments::List oAct( nSize ), oCp( nSize ), oVer( nSize );
				rhs.m_lActive.swap( oAct );
				rhs.m_lCompleted.swap( oCp );
				rhs.m_lVerified.swap( oVer );
			}
			else if ( sTag == "cs" )
			{
				s >> rhs.m_nCompletedSize;
			}
			else if ( sTag == "state" )
			{
				int x;
				s >> x;
				rhs.m_nState = static_cast<Download::DownloadState>( x );
			}
			else if ( sTag == "mf" )
			{
				s >> rhs.m_bMultifile;
			}
			else if ( sTag == "pr" )
			{
				s >> rhs.m_nPriority;
			}
			else if ( sTag == "file" )
			{
				quint32 nVerF;
				s >> nVerF;

				if ( nVerF == 1 )
				{
					Download::FileListItem item;

					QByteArray sTag2;
					s >> sTag2;
					sTag2.chop( 1 );

					while ( sTag2 != "file-end" && s.status() == QDataStream::Ok )
					{
						if ( sTag2 == "name" )
						{
							s >> item.sFileName;
						}
						else if ( sTag2 == "path" )
						{
							s >> item.sPath;
						}
						else if ( sTag2 == "tempname" )
						{
							s >> item.sTempName;
						}
						else if ( sTag2 == "so" )
						{
							s >> item.nStartOffset;
						}
						else if ( sTag2 == "eo" )
						{
							s >> item.nEndOffset;
						}
						else if ( sTag2 == "hash" )
						{
							QString sHash;
							s >> sHash;
							item.vHashes.insert( Hash::fromURN( sHash ) );
						}

						s >> sTag2;
						sTag2.chop( 1 );
					}
				}
			}
			else if ( sTag == "download-source" )
			{
				DownloadSource* pSource = new DownloadSource( &rhs );
				s >> *pSource;
				rhs.addSource( pSource );
			}
			else if ( sTag == "completed-frags" )
			{
				Fragments::SerializeIn( s, rhs.m_lCompleted );
			}
			else if ( sTag == "verified-frags" )
			{
				Fragments::SerializeIn( s, rhs.m_lVerified );
			}

			s >> sTag;
			sTag.chop( 1 );
		}
	}

	Q_ASSERT( rhs.m_lActive.size() == rhs.m_lCompleted.size() && rhs.m_lCompleted.size() == rhs.m_lVerified.size() );
	return s;
}

Download::Download( QueryHit* pHit, QObject* parent ) :
	QObject( parent ),
	m_lCompleted( pHit->m_nObjectSize ),
	m_lVerified( pHit->m_nObjectSize ),
	m_lActive( pHit->m_nObjectSize ),
	m_bSignalSources( false ),
	m_nPriority( 125 ),
	m_bModified( true ),
	m_nTransfers( 0 )
{
	Q_ASSERT( pHit != NULL );

	int nSources = 0;
	m_sDisplayName = pHit->m_sDescriptiveName;
	m_nSize = pHit->m_nObjectSize;
	m_nCompletedSize = 0;

	FileListItem oFile;
	oFile.sFileName = fixFileName( m_sDisplayName );
	oFile.sTempName = m_sTempName;
	oFile.nStartOffset = 0;
	oFile.nEndOffset = m_nSize - 1;
	m_lFiles.append( oFile );
	m_bMultifile = false;

	nSources = addSource( pHit );
	m_sTempName = getIncompleteFileName( m_vHashes );
	if ( m_sTempName.isEmpty() )
	{
		m_sTempName = m_sDisplayName;
	}

	setState( dsQueued );

	systemLog.postLog( LogSeverity::Notice, Component::Downloads,
					   qPrintable( tr( "Created download for %s with %d sources." ) ),
					   qPrintable( m_sDisplayName ), nSources );
}

Download::~Download()
{
	ASSUME_LOCK( downloads.m_pSection );

	qDeleteAll( m_lSources );
}

void Download::start()
{
	m_tStarted = common::getDateTimeUTC();
	setState( dsPending );
}

bool Download::addSource( DownloadSource* pSource )
{
	ASSUME_LOCK( downloads.m_pSection );

	Q_ASSERT( pSource->m_pDownload == this );

	foreach ( DownloadSource * pThis, m_lSources )
	{
		if ( ( !pThis->m_oGUID.isNull() && pThis->m_oGUID == pSource->m_oGUID )
			 || ( pThis->m_oAddress == pSource->m_oAddress )
			 || ( !pThis->m_sURL.isEmpty() && pThis->m_sURL == pSource->m_sURL ) )
		{
			return false;
		}
	}

	m_lSources.append( pSource );

	if ( m_bSignalSources )
	{
		emit sourceAdded( pSource );
	}

	return true;
}

// TODO: check whether this is actually ever called for hits with m_pNext != NULL
int Download::addSource( QueryHit* pHit )
{
	QueryHit* pCurrentHit = pHit;
	int nSources = 0;

	while ( pCurrentHit )
	{
		if ( !m_vHashes.insert( pCurrentHit->m_vHashes ) )
		{
			// TODO: handle hash conflicts
		}

		DownloadSource* pSource = new DownloadSource( this, pCurrentHit );
		if ( addSource( pSource ) )
		{
			++nSources;
		}
		else
		{
			delete pSource;
		}
		pCurrentHit = pCurrentHit->m_pNext;
	}

	return nSources;
}

void Download::removeSource( DownloadSource* pSource )
{
	ASSUME_LOCK( downloads.m_pSection );

	for ( int i = 0; i < m_lSources.size(); ++i )
	{
		if ( m_lSources.at( i ) == pSource )
		{
			m_lSources.removeAt( i );
		}
	}
}

int Download::startTransfers( int /*nMaxTransfers*/ )
{
	ASSUME_LOCK( downloads.m_pSection );

	return 0; // must return the number of just started transfers
}

void Download::stopTransfers()
{
	ASSUME_LOCK( downloads.m_pSection );

}

bool Download::sourceExists( DownloadSource* pSource )
{
	return ( m_lSources.indexOf( pSource ) != -1 );
}

QList<Transfer*> Download::getTransfers()
{
	ASSUME_LOCK( transfers.m_pSection );

	return transfers.getByOwner( this );
}

Fragments::List Download::getWantedFragments()
{
	// for now...
	Fragments::List oList = inverse( m_lCompleted );

	return oList;
}

Fragments::List Download::getPossibleFragments( const Fragments::List& oAvailable, Fragments::Fragment& oLargest )
{
	Fragments::List oPossible( oAvailable );

	if ( oAvailable.empty() )
	{
		oPossible = getWantedFragments();
	}
	else
	{
		Fragments::List oTmp = inverse( getWantedFragments() );
		oPossible.erase( oTmp.begin(), oTmp.end() );
	}

	if ( oPossible.empty() )
	{
		return oPossible;
	}

	oLargest = *oPossible.largest_range();

	QList<Transfer*> lTransfers = getTransfers();
	foreach ( Transfer * pTransfer, lTransfers )
	{
		DownloadTransfer* pTr = qobject_cast<DownloadTransfer*>( pTransfer );

		if ( pTr )
		{
			pTr->subtractRequested( oPossible );

			if ( oPossible.empty() )
			{
				break;
			}
		}
	}

	return oPossible;
}

void Download::saveState()
{
	QString sFileName = quazaaSettings.Downloads.IncompletePath + "/" + m_sTempName;
	QString sFileNameT = sFileName;

	sFileName.append( ".!qd" );
	sFileNameT.append( ".bak" );

	QFile f( sFileNameT );

	if ( f.open( QFile::WriteOnly ) )
	{
		QDataStream s( &f );

		s << *this;

		f.close();

		QFile::remove( sFileName );
		QFile::rename( sFileNameT, sFileName );

		m_bModified = false;
	}
}

void Download::setState( Download::DownloadState state )
{
	m_nState = state;
	emit stateChanged( state );
}

// Invoked by download model
// let the download go to the model first, giving a chance to connect signals to corresponding objects
// then request sources for this download, so model can stay in sync
void Download::emitSources()
{
	if ( !m_bSignalSources )
	{
		m_bSignalSources = true;
		foreach ( DownloadSource * pSource, m_lSources )
		{
			emit sourceAdded( pSource );
		}
	}
}


