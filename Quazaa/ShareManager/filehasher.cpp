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

#include "filehasher.h"
#include "Hashes/hash.h"
#include <QFile>
#include <QByteArray>
#include "sharemanager.h"
#include "quazaasettings.h"
#include <QElapsedTimer>

#include "debug_new.h"

QMutex FileHasher::m_pSection;
QQueue<SharedFilePtr> FileHasher::m_lQueue;
FileHasher** FileHasher::m_pHashers = 0;
quint32  FileHasher::m_nMaxHashers = 1;
quint32  FileHasher::m_nRunningHashers = 0;
QWaitCondition FileHasher::m_oWaitCond;

FileHasher::FileHasher( QObject* parent ) : QThread( parent )
{
	m_bActive = true;
	m_nId = -1;
}

FileHasher::~FileHasher()
{
	m_bActive = false;
	if ( isRunning() )
	{
		wait();
	}
}

FileHasher* FileHasher::hashFile( SharedFilePtr pFile )
{
	m_pSection.lock();

	if ( m_pHashers == 0 )
	{
		m_nMaxHashers = qMin<quint32>( 4, qMax<quint32>( 2, QThread::idealThreadCount() ) );
		m_pHashers = new FileHasher*[m_nMaxHashers];
		for ( uint i = 0; i < m_nMaxHashers; i++ )
		{
			m_pHashers[i] = 0;
		}
	}

	//qDebug() << "File" << pFile->m_sFilename << "queued for hashing";
	m_lQueue.enqueue( pFile );

	FileHasher* pHasher = 0;

	if ( m_nRunningHashers < ( uint )m_lQueue.size() && m_nRunningHashers < m_nMaxHashers )
	{
		for ( uint i = 0; i < m_nMaxHashers; i++ )
		{
			if ( !m_pHashers[i] )
			{
				systemLog.postLog( LogSeverity::Debug, QString( "Starting hasher: %1" ).arg( m_nRunningHashers ) );
				m_pHashers[i] = new FileHasher();
				pHasher = m_pHashers[i];
				pHasher->m_nId = i;
				connect( pHasher, SIGNAL( queueEmpty() ), &shareManager, SLOT( RunHashing() ), Qt::UniqueConnection );
				connect( pHasher, SIGNAL( fileHashed( SharedFilePtr ) ), &shareManager, SLOT( OnFileHashed( SharedFilePtr ) ),
						 Qt::UniqueConnection );
				connect( pHasher, SIGNAL( hasherStarted( int ) ), &shareManager, SIGNAL( hasherStarted( int ) ) );
				connect( pHasher, SIGNAL( hasherFinished( int ) ), &shareManager, SIGNAL( hasherFinished( int ) ) );
				connect( pHasher, SIGNAL( hashingProgress( int, QString, double, int ) ), &shareManager, SIGNAL( hashingProgress( int,
																												 QString, double, int ) ) );
				m_pHashers[i]->start( ( quazaaSettings.Library.HighPriorityHashing ? QThread::NormalPriority :
										QThread::LowestPriority ) );
				m_nRunningHashers++;
				break;
			}
		}
	}

	m_pSection.unlock();

	FileHasher::m_oWaitCond.wakeOne();

	return pHasher;
}

void FileHasher::run()
{
	QElapsedTimer tTimer;
	QByteArray baBuffer;

	static const int nBufferSize = 2 * 1024 * 1024;

	emit hasherStarted( m_nId );

	m_pSection.lock();

	while ( !m_lQueue.isEmpty() )
	{
		SharedFilePtr pFile = m_lQueue.dequeue();
		systemLog.postLog( LogSeverity::Debug, QString( "Hashing %1" ).arg( pFile->fileName() ) );

		m_pSection.unlock();

		emit hashingProgress( m_nId, pFile->fileName(), 0, 0 );

		bool bHashed = true;

		std::vector<Hash*> vHashes;

		if ( pFile->exists() && pFile->open( QFile::ReadOnly ) )
		{
			baBuffer.resize( nBufferSize );

			vHashes.push_back( new Hash( Hash::SHA1 ) );
			vHashes.push_back( new Hash( Hash::MD5  ) );

			tTimer.start();
//			double nLastPercent = 0;
			quint64 nFileSize = pFile->size();
			quint64 nTotalRead = 0, nLastTotalRead = 0;

			emit hashingProgress( m_nId, pFile->fileName(), 0, 0 );

			while ( !pFile->atEnd() )
			{
				if ( !m_bActive )
				{
					systemLog.postLog( LogSeverity::Debug, QString( "CFileHasher aborting..." ) );
					//qDebug() << "CFileHasher aborting...";
					bHashed = false;
					break;
				}
				qint64 nRead = pFile->read( baBuffer.data(), nBufferSize );

				if ( nRead < 0 )
				{
					bHashed = false;
					systemLog.postLog( LogSeverity::Debug, QString( "File read error: %1" ).arg( pFile->error() ) );
					//qDebug() << "File read error:" << f.error();
					break;
				}
				else if ( nRead < nBufferSize )
				{
					baBuffer.resize( nRead );
				}

				nTotalRead += nRead;

				for ( size_t i = 0, nSize = vHashes.size(); i < nSize; ++i )
				{
					vHashes[i]->addData( baBuffer );
				}

				if ( tTimer.elapsed() >= 1000 )
				{
					double nPercent = 100.0f * nTotalRead / float( nFileSize );
					int nRate = ( tTimer.elapsed() * ( nTotalRead - nLastTotalRead ) ) / 1000;
//					nLastPercent = nPercent;
					nLastTotalRead = nTotalRead;
					tTimer.start();
					emit hashingProgress( m_nId, pFile->fileName(), nPercent, nRate );
				}
			}

			emit hashingProgress( m_nId, pFile->fileName(), 100, ( tTimer.elapsed() * ( nTotalRead - nLastTotalRead ) ) / 1000 );
			pFile->close();
		}
		else
		{
			systemLog.postLog( LogSeverity::Debug, QString( "File open error: %1" ).arg( pFile->error() ) );
			//qDebug() << "File open error: " << f.error();
			bHashed = false;
		}

		if ( bHashed )
		{
			for ( size_t i = 0, nSize = vHashes.size(); i < nSize; ++i )
			{
				vHashes[i]->finalize();
				pFile->setHash( vHashes[i] );
				systemLog.postLog( LogSeverity::Debug, QString( "%1" ).arg( vHashes[i]->toURN() ) );
				//qDebug() << pFile->m_lHashes[i]->ToURN();
			}
			emit fileHashed( pFile );
		}

		msleep( 150 );

		m_pSection.lock();

		if ( !m_bActive )
		{
			break;
		}

		if ( bHashed && m_lQueue.isEmpty() )
		{
			emit queueEmpty();
			systemLog.postLog( LogSeverity::Debug, QString( "Hasher waiting..." ) );
			//qDebug() << "Hasher " << this << "waiting...";
			FileHasher::m_oWaitCond.wait( &m_pSection, 10000 );
		}
	}

	for ( uint i = 0; i < m_nMaxHashers; ++i )
	{
		if ( m_pHashers[i] == this )
		{
			m_pHashers[i] = 0;
			deleteLater();
			--m_nRunningHashers;

			if ( !m_nRunningHashers )
			{
				delete[] m_pHashers;
				m_pHashers = 0;
			}
			break;
		}
	}

	m_pSection.unlock();

	systemLog.postLog( LogSeverity::Debug, QString( "CFileHasher done. %1" ).arg( m_nRunningHashers ) );

	emit hasherFinished( m_nId );
}

