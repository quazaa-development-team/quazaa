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

QDataStream& operator<<(QDataStream& s, const Download& rhs)
{
	// basic info
	s << quint32(1); // version
	s << "dn" << rhs.m_sDisplayName;
	s << "tn" << rhs.m_sTempName;
	s << "s" << rhs.m_nSize;
	s << "cs" << rhs.m_nCompletedSize;
	s << "state" << rhs.m_nState;
	s << "mf" << rhs.m_bMultifile;
	s << "pr" << rhs.m_nPriority;

	// files
	foreach ( const Download::FileListItem& i, rhs.m_lFiles )
	{
		s << "file" << quint32(1); // version
		s << "name" << i.sFileName;
		s << "path" << i.sPath;
		s << "tempname" << i.sTempName;
		s << "so" << i.nStartOffset;
		s << "eo" << i.nEndOffset;
		foreach ( const CHash& rHash, i.lHashes )
		{
			s << "hash" << rHash.toURN();
		}
		s << "file-end";
	}

	// sources
	foreach ( CDownloadSource* pSource, rhs.m_lSources )
	{
		s << *pSource;
	}

	s << "completed-frags";
	Fragments::SerializeOut(s, rhs.m_lCompleted);
	s << "verified-frags";
	Fragments::SerializeOut(s, rhs.m_lCompleted);

	s << "eof";
	return s;
}
QDataStream& operator>>(QDataStream& s, Download& rhs)
{
	quint32 nVer;

	s >> nVer;

	if( nVer == 1 )
	{
		QByteArray sTag;
		s >> sTag;
		sTag.chop(1);

		while( sTag != "eof" && s.status() == QDataStream::Ok )
		{
			if( sTag == "dn" )
			{
				s >> rhs.m_sDisplayName;
			}
			else if( sTag == "tn" )
			{
				s >> rhs.m_sTempName;
			}
			else if( sTag == "s" )
			{
				quint64 nSize;
				s >> nSize;
				rhs.m_nSize = nSize;

				Fragments::List oAct(nSize), oCp(nSize), oVer(nSize);
				rhs.m_lActive.swap(oAct);
				rhs.m_lCompleted.swap(oCp);
				rhs.m_lVerified.swap(oVer);
			}
			else if( sTag == "cs" )
			{
				s >> rhs.m_nCompletedSize;
			}
			else if( sTag == "state" )
			{
				int x;
				s >> x;
				rhs.m_nState = static_cast<Download::DownloadState>(x);
			}
			else if( sTag == "mf" )
			{
				s >> rhs.m_bMultifile;
			}
			else if( sTag == "pr" )
			{
				s >> rhs.m_nPriority;
			}
			else if( sTag == "file" )
			{
				quint32 nVerF;
				s >> nVerF;

				if( nVerF == 1 )
				{
					Download::FileListItem item;

					QByteArray sTag2;
					s >> sTag2;
					sTag2.chop(1);

					while( sTag2 != "file-end" && s.status() == QDataStream::Ok )
					{
						if( sTag2 == "name" )
						{
							s >> item.sFileName;
						}
						else if( sTag2 == "path" )
						{
							s >> item.sPath;
						}
						else if( sTag2 == "tempname" )
						{
							s >> item.sTempName;
						}
						else if( sTag2 == "so" )
						{
							s >> item.nStartOffset;
						}
						else if( sTag2 == "eo" )
						{
							s >> item.nEndOffset;
						}
						else if( sTag2 == "hash" )
						{
							QString sHash;
							s >> sHash;
							CHash* pHash = CHash::fromURN(sHash);
							item.lHashes.append(*pHash);
							delete pHash;
						}

						s >> sTag2;
						sTag2.chop(1);
					}
				}
			}
			else if( sTag == "download-source" )
			{
				CDownloadSource* pSource = new CDownloadSource(&rhs);
				s >> *pSource;
				rhs.addSource(pSource);
			}
			else if( sTag == "completed-frags" )
			{
				Fragments::SerializeIn(s, rhs.m_lCompleted);
			}
			else if( sTag == "verified-frags" )
			{
				Fragments::SerializeIn(s, rhs.m_lVerified);
			}

			s >> sTag;
			sTag.chop(1);
		}
	}

	Q_ASSERT(rhs.m_lActive.size() == rhs.m_lCompleted.size() && rhs.m_lCompleted.size() == rhs.m_lVerified.size());
	return s;
}

Download::Download(QueryHit* pHit, QObject *parent) :
	QObject(parent),
	m_lCompleted(pHit->m_nObjectSize),
	m_lVerified(pHit->m_nObjectSize),
	m_lActive(pHit->m_nObjectSize),
	m_bSignalSources(false),
	m_nPriority(125),
	m_bModified(true),
	m_nTransfers(0)
{
	Q_ASSERT(pHit != NULL);

	int nSources = 0;
	m_sDisplayName = pHit->m_sDescriptiveName;
	m_nSize = pHit->m_nObjectSize;
	m_nCompletedSize = 0;
	m_sTempName = getTempFileName(m_sDisplayName);

	m_lHashes.reserve( CHash::NO_OF_HASH_TYPES );

	FileListItem oFile;
	oFile.sFileName = fixFileName(m_sDisplayName);
	oFile.sTempName = m_sTempName;
	oFile.nStartOffset = 0;
	oFile.nEndOffset = m_nSize - 1;
	m_lFiles.append(oFile);
	m_bMultifile = false;
	// hashes are not needed here, for single file download

	nSources = addSource(pHit);

	setState( dsQueued );

	systemLog.postLog( LogSeverity::Notice, Component::Downloads,
					   qPrintable( tr( "Created download for %s with %d sources." ) ),
					   qPrintable( m_sDisplayName ), nSources );
}

Download::~Download()
{
	ASSUME_LOCK(downloads.m_pSection);

	qDeleteAll(m_lSources);
}

void Download::start()
{
	m_tStarted = common::getDateTimeUTC();
	setState(dsPending);
}

bool Download::addSource(CDownloadSource *pSource)
{
	ASSUME_LOCK(downloads.m_pSection);

	Q_ASSERT(pSource->m_pDownload == this);

	foreach ( CDownloadSource* pThis, m_lSources )
	{
		if( (!pThis->m_oGUID.isNull() && pThis->m_oGUID == pSource->m_oGUID)
				|| (pThis->m_oAddress == pSource->m_oAddress)
				|| (!pThis->m_sURL.isEmpty() && pThis->m_sURL == pSource->m_sURL))
		{
			return false;
		}
	}

	m_lSources.append(pSource);

	if( m_bSignalSources )
		emit sourceAdded(pSource);

	return true;
}
int Download::addSource(QueryHit *pHit)
{
	QueryHit* pCurrentHit = pHit;
	int nSources = 0;

	while( pCurrentHit )
	{
		// add hashes
		CHash* pNewHashes                      = &pCurrentHit->m_lHashes[0];
		const HashVector::size_type nNewHashes =  pCurrentHit->m_lHashes.size();

		for ( HashVector::size_type n = 0; n < nNewHashes; ++n )
		{
			bool bNew = true;

			CHash* pExistingHashes                      = &m_lHashes[0];
			const HashVector::size_type nExistingHashes =  m_lHashes.size();

			for ( HashVector::size_type i = 0; i < nExistingHashes; ++i )
			{
				if ( pNewHashes[n] == pExistingHashes[i] )
				{
					bNew = false;
					break;
				}
			}

			if ( bNew )
			{
				m_lHashes.push_back( pNewHashes[n] );
			}
		}

		CDownloadSource* pSource = new CDownloadSource(this, pCurrentHit);
		if( addSource(pSource) )
		{
			nSources++;
		}
		else
		{
			delete pSource;
		}
		pCurrentHit = pCurrentHit->m_pNext;
	}

	return nSources;
}

void Download::removeSource(CDownloadSource *pSource)
{
	ASSUME_LOCK(downloads.m_pSection);

	for( int i = 0; i < m_lSources.size(); ++i )
	{
		if( m_lSources.at(i) == pSource )
		{
			m_lSources.removeAt(i);
		}
	}
}

int Download::startTransfers(int /*nMaxTransfers*/)
{
	ASSUME_LOCK(downloads.m_pSection);

	return 0; // must return the number of just started transfers
}

void Download::stopTransfers()
{
	ASSUME_LOCK(downloads.m_pSection);

}

bool Download::sourceExists(CDownloadSource *pSource)
{
	return (m_lSources.indexOf(pSource) != -1);
}

QList<CTransfer *> Download::getTransfers()
{
	ASSUME_LOCK(transfers.m_pSection);

	return transfers.getByOwner(this);
}

Fragments::List Download::getWantedFragments()
{
	// for now...
	Fragments::List oList = inverse(m_lCompleted);

	return oList;
}

Fragments::List Download::getPossibleFragments(const Fragments::List &oAvailable, Fragments::Fragment &oLargest)
{
	Fragments::List oPossible(oAvailable);

	if( oAvailable.empty() )
	{
		oPossible = getWantedFragments();
	}
	else
	{
		Fragments::List oTmp = inverse(getWantedFragments());
		oPossible.erase(oTmp.begin(), oTmp.end());
	}

	if( oPossible.empty() )
		return oPossible;

	oLargest = *oPossible.largest_range();

	QList<CTransfer*> lTransfers = getTransfers();
	foreach ( CTransfer* pTransfer, lTransfers )
	{
		CDownloadTransfer* pTr = qobject_cast<CDownloadTransfer*>(pTransfer);

		if( pTr )
		{
			pTr->subtractRequested(oPossible);

			if( oPossible.empty() )
				break;
		}
	}

	return oPossible;
}

void Download::saveState()
{
	QString sFileName = quazaaSettings.Downloads.IncompletePath + "/" + m_sTempName;
	QString sFileNameT = sFileName;

	sFileName.append(".!qd");
	sFileNameT.append(".bak");

	QFile f(sFileNameT);

	if( f.open(QFile::WriteOnly) )
	{
		QDataStream s(&f);

		s << *this;

		f.close();

		QFile::remove(sFileName);
		QFile::rename(sFileNameT, sFileName);

		m_bModified = false;
	}
}

void Download::setState(Download::DownloadState state)
{
	m_nState = state;
	emit stateChanged(state);
}

// Invoked by download model
// let the download go to the model first, giving a chance to connect signals to corresponding objects
// then request sources for this download, so model can stay in sync
void Download::emitSources()
{
	if( !m_bSignalSources )
	{
		m_bSignalSources = true;
		foreach ( CDownloadSource* pSource, m_lSources )
		{
			emit sourceAdded(pSource);
		}
	}
}


