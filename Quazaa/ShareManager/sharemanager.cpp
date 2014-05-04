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

#include "sharemanager.h"

#include <QTimer>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QVariant>
#include <QList>

#include "quazaaglobals.h"
#include "quazaasettings.h"
#include "queryhashmaster.h"
#include "sharedfile.h"
#include "filehasher.h"
#include "types.h"

#include "debug_new.h"

CThread shareManagerThread;
ShareManager shareManager;

ShareManager::ShareManager( QObject* parent ) :
	QObject( parent )
{
	m_bActive = false;
	m_bReady = false;
	m_bTableReady = false;
	m_pTable = 0;
	m_nRemainingFiles = 0;
}

void ShareManager::start()
{
	QMutexLocker l( &m_oSection );
	systemLog.postLog( LogSeverity::Debug, QString( "Starting share manager..." ) );
	connect( this, SIGNAL( sharesReady() ), &queryHashMaster, SLOT( build() ) );
	shareManagerThread.start( "ShareManager", &m_oSection, this );
}

void ShareManager::setupThread()
{
	systemLog.postLog( LogSeverity::Debug, QString( "Setting up ShareManager thread" ) );
	m_oDatabase = QSqlDatabase::addDatabase( "QSQLITE", "Shares" );
	QDir path = QDir( QuazaaGlobals::SETTINGS_PATH() );
	if ( !path.exists() )
	{
		path.mkpath( QuazaaGlobals::SETTINGS_PATH() );
	}

	m_oDatabase.setDatabaseName( QString( "%1shares.sdb" ).arg( QuazaaGlobals::SETTINGS_PATH() ) );

	if ( !m_oDatabase.open() )
	{
		qWarning() << "Failed to open SQLITE database. Giving up. " << m_oDatabase.lastError().text();
		QTimer::singleShot( 1000, &shareManagerThread, SLOT( quit() ) );
		return;
	}

	systemLog.postLog( LogSeverity::Debug, QString( "Database opened successfully" ) );

	QSqlQuery query( m_oDatabase );

	systemLog.postLog( LogSeverity::Debug, QString( "Checking tables..." ) );

	// TODO: Better checks and error handling

	if ( !m_oDatabase.tables().contains( "dirs" ) || !m_oDatabase.tables().contains( "files" )
		 || !m_oDatabase.tables().contains( "hashes" ) || !m_oDatabase.tables().contains( "hash_queue" ) )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "Database is corrupted. Recreating..." ) );

		query.exec( "PRAGMA legacy_file_format = 0" );

		foreach ( const QString & sTable, m_oDatabase.tables() )
		{
			query.exec( QString( "DROP TABLE `%1`" ).arg( sTable ) );
		}

		// tables
		query.exec( "CREATE TABLE 'dirs' ('id' INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , 'path' TEXT NOT NULL, 'parent' INTEGER NOT NULL );" );
		query.exec( "CREATE TABLE 'files' ('file_id' INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , 'dir_id' INTEGER NOT NULL , 'name' VARCHAR(255) NOT NULL , 'size' INTEGER NOT NULL , 'last_modified' INTEGER NOT NULL , 'shared' BOOL NOT NULL  DEFAULT 1);" );
		query.exec( "CREATE TABLE 'hashes' ('file_id' INTEGER PRIMARY KEY NOT NULL  UNIQUE , 'sha1' BLOB(20) NOT NULL, 'md5' BLOB(16) NOT NULL);" );
		query.exec( "CREATE TABLE 'hash_queue' ('dir_id' INTEGER NOT NULL, 'filename' VARCHAR(255) NOT NULL);" );
		query.exec( "CREATE TABLE 'keywords' ('id' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, 'keyword' TEXT NOT NULL);" );

		// indexes
		query.exec( "CREATE INDEX 'dir_id' ON 'files' ('dir_id' ASC);" );
		query.exec( "CREATE UNIQUE INDEX 'file_id' ON 'hashes' ('file_id' ASC);" );
		query.exec( "CREATE INDEX 'name' ON 'files' ('name' ASC);" );
		query.exec( "CREATE INDEX 'parent' ON 'dirs' ('parent' ASC)" );
		query.exec( "CREATE UNIQUE INDEX 'keyword' ON 'keywords' ('keyword' ASC);" );
		query.exec( "CREATE INDEX 'sha1' ON 'hashes' ('sha1' ASC);" );

		systemLog.postLog( LogSeverity::Debug, QString( "Database recreated." ) );
	}
	else
	{
		systemLog.postLog( LogSeverity::Debug, QString( "Tables OK" ) );
	}

	systemLog.postLog( LogSeverity::Debug, QString( "Destroying hash queue." ) );
	query.exec( "DELETE FROM `hash_queue`;" );

	m_bActive = true;

	QTimer::singleShot( 30000, this, SLOT( syncShares() ) );

	connect( this, SIGNAL( executeQuery( const QString& ) ), this, SLOT( execQuery( const QString& ) ),
			 Qt::QueuedConnection );
}

void ShareManager::stop()
{
	m_oSection.lock();

	m_bActive     = false;
	m_bReady      = false;
	m_bTableReady = false;

	if ( m_pTable )
	{
		delete m_pTable;
		m_pTable = NULL;
	}
	disconnect( SIGNAL( executeQuery( const QString& ) ), this, SLOT( execQuery( const QString& ) ) );

	shareManagerThread.exit( 0 );
	m_oSection.unlock();
}

void ShareManager::cleanupThread()
{
	systemLog.postLog( LogSeverity::Debug, QString( "ShareManager: cleaning up." ) );

	if ( m_oDatabase.isOpen() )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "Closing Database connection." ) );
		QSqlDatabase::database( "Shares" ).close();
		QSqlDatabase::removeDatabase( "Shares" );
	}
}

void ShareManager::addDir( QString sPath )
{
	Q_UNUSED( sPath );

}

void ShareManager::removeDir( QString sPath )
{
	Q_UNUSED( sPath );

	QSqlQuery query( m_oDatabase );

	query.prepare( "SELECT id FROM dirs WHERE path = '?'" );
	query.addBindValue( QVariant( sPath ) );
	query.exec();

	while ( query.next() )
	{
		removeDir( query.record().value( 0 ).toUInt() );
	}
}

void ShareManager::removeDir( quint64 nId )
{
	QSqlQuery delq( m_oDatabase );

	delq.exec( QString( "SELECT id FROM dirs WHERE parent = %1" ).arg( nId ) );
	while ( delq.next() )
	{
		removeDir( delq.record().value( 0 ).toUInt() );
	}

	delq.exec( QString( "DELETE FROM hashes WHERE file_id IN (SELECT dir_id FROM files WHERE dir_id = %1)" ).arg( nId ) );
	delq.exec( QString( "DELETE FROM files WHERE dir_id = %1" ).arg( nId ) );
	delq.exec( QString( "DELETE FROM dirs WHERE id = %1" ).arg( nId ) );
}

void ShareManager::removeFile( QString sPath )
{
	Q_UNUSED( sPath );

}

void ShareManager::removeFile( quint64 nFileId )
{
	QSqlQuery delq( m_oDatabase );
	delq.exec( QString( "DELETE FROM hashes WHERE file_id = %1" ).arg( nFileId ) );
	delq.exec( QString( "DELETE FROM files WHERE file_id = %1" ).arg( nFileId ) );
}

void ShareManager::syncShares()
{
	QMutexLocker l( &m_oSection );

	systemLog.postLog( LogSeverity::Debug, QString( "Syncing Shares..." ) );

	QSqlQuery query( m_oDatabase );

	query.exec( "PRAGMA synchronous = 0" );
	query.exec( "PRAGMA temp_store = 2" );

	query.setForwardOnly( true );

	int nMissingDirs = 0, nMissingFiles = 0, nModifiedFiles = 0;

	// 1. Check for missing dirs
	if ( !query.exec( "SELECT id, path FROM dirs" ) )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( query.lastError().text() ) );
		return;
	}

	while ( query.next() )
	{
		QDir d( query.record().value( 1 ).toString() );
		if ( !d.exists() )
		{
			// delete all file entries that refer to the missing dir
			systemLog.postLog( LogSeverity::Debug, QString( "Directory %1 does not exist" ).arg( d.path() ) );
			removeDir( query.record().value( 0 ).toInt() );
			nMissingDirs++;
		}
	}

	// 2. Check for missing or modified files
	query.setForwardOnly( true );
	if ( !query.exec( "SELECT id, path FROM dirs" ) )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: " ).arg( query.lastError().text() ) );
		return;
	}

	while ( query.next() )
	{
		QSqlQuery fquery( m_oDatabase );

		if ( fquery.exec( QString( "SELECT file_id,name,size,last_modified FROM files WHERE dir_id = %1" ).arg(
							  query.record().value( 0 ).toInt() ) ) )
		{
			while ( fquery.next() )
			{
				QString sPath;
				sPath = query.record().value( 1 ).toString().append( "/" );
				sPath.append( fquery.record().value( 1 ).toString() );

				QFileInfo fi( sPath );
				if ( !fi.exists() )
				{
					systemLog.postLog( LogSeverity::Debug, QString( "File: %1 is missing" ).arg( sPath ) );
					removeFile( fquery.record().value( 0 ).toInt() );
					nMissingFiles++;
				}
				else
				{
					// check if modified

					bool bModified = false;

					if ( fi.size() != fquery.record().value( 2 ).toLongLong() )
					{
						systemLog.postLog( LogSeverity::Debug, QString( "Size mismatch" ) );
						bModified = true;
					}
					else if ( fi.lastModified().toTime_t() != fquery.record().value( 3 ).toUInt() )
					{
						systemLog.postLog( LogSeverity::Debug, QString( "Modified recently" ) );
						bModified = true;
					}

					if ( bModified )
					{
						systemLog.postLog( LogSeverity::Debug, QString( "File: %1 is midified, rehashing" ).arg( sPath ) );
						removeFile( fquery.record().value( 0 ).toInt() );
						nModifiedFiles++;
					}
				}
			}
		}
	}

	// fix slashes
	for ( int i = 0; i < quazaaSettings.Library.Shares.size(); i++ )
	{
		quazaaSettings.Library.Shares[i] = quazaaSettings.Library.Shares[i].replace( "\\", "/" );
	}

	// 3. Remove non-shared dirs (just in case)

	QList<qint64> lSharedDirs;

	// for each dir from settings
	foreach ( QString sPath, quazaaSettings.Library.Shares )
	{
		// find all subdirs and store their IDs in the list
		query.prepare( "SELECT id FROM dirs WHERE path LIKE ?" );
		query.bindValue( 0, QVariant( sPath.append( "%" ) ) );

		if ( !query.exec() )
		{
			systemLog.postLog( LogSeverity::Debug,
							   QString( "SQL Query failed: %1 %2 %3" ).arg( query.lastError().text() ).arg( query.lastError().number() ).arg(
								   query.executedQuery() ) );
		}

		while ( query.next() )
		{
			lSharedDirs.append( query.record().value( 0 ).toLongLong() );
		}
	}

	// now find and remove orphan dirs
	QString sIds;
	foreach ( const qint64 & nId, lSharedDirs )
	{
		sIds.append( QVariant( nId ).toString() ).append( "," );
	}
	if ( sIds.size() )
	{
		sIds.truncate( sIds.size() - 1 );

		systemLog.postLog( LogSeverity::Debug, QString( "Dir IDs: %1" ).arg( sIds ) );

		if ( !query.exec( QString( "SELECT id FROM dirs WHERE id NOT IN(%1)" ).arg( sIds ) ) )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( query.lastError().text() ) );
		}
		else
		{
			int nOrphaned = 0;

			while ( query.next() )
			{
				nOrphaned++;
				removeDir( query.record().value( 0 ).toLongLong() );
			}

			systemLog.postLog( LogSeverity::Debug, QString( "Found %1 orphaned dirs" ).arg( nOrphaned ) );
		}
	}

	// 4. Make sure that shared dirs are in db (quick and dirty, possibly bad :P)
	foreach ( const QString & sPath, quazaaSettings.Library.Shares )
	{
		query.prepare( "SELECT id FROM dirs WHERE path LIKE ?" );
		query.bindValue( 0, QVariant( sPath ) );
		if ( !query.exec() )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( query.lastError().text() ) );
			continue;
		}

		bool bFound = false;
		while ( query.next() )
		{
			bFound = true;
		}

		if ( !bFound )
		{
			QDir d( sPath );

			if ( !d.exists() )
			{
				// Try to create the dir
				if ( !d.mkpath( sPath ) )
				{
					systemLog.postLog( LogSeverity::Debug, QString( "Cannot create directory %1" ).arg( sPath ) );
					continue;
				}
			}

			QSqlQuery insq( m_oDatabase );
			insq.prepare( "INSERT INTO dirs (path, parent) VALUES(?,0)" );
			insq.bindValue( 0, QVariant( sPath ) );
			if ( !insq.exec() )
			{
				systemLog.postLog( LogSeverity::Debug,
								   QString( "Cannot insert new directory entry: %1" ).arg( insq.lastError().text() ) );
			}
		}
	}

	systemLog.postLog( LogSeverity::Debug,
					   QString( "Missing dirs: %1 missing files: %2 modified files %3" ).arg( nMissingDirs ).arg( nMissingFiles ).arg(
						   nModifiedFiles ) );

	// 5. Now we can start scanning shared dirs

	foreach ( const QString & sPath, quazaaSettings.Library.Shares )
	{
		if ( !m_bActive )
		{
			break;
		}

		l.unlock();
		scanFolder( sPath );
		l.relock();
	}

	query.exec( "PRAGMA synchronous = 1" );
	m_bReady = true;
	if ( m_bActive )
	{
		emit sharesReady();
		l.unlock();
		runHashing();
	}
}

// Recursively scan sPath for new files (modified files are already handled)
void ShareManager::scanFolder( QString sPath, qint64 nParentID )
{
	QMutexLocker l( &m_oSection );

	if ( !m_bActive )
	{
		return;
	}

	l.unlock();

	systemLog.postLog( LogSeverity::Debug, QString( "Scanning %1 %2 " ).arg( sPath ).arg( nParentID ) );

	QDir d( sPath );

	if ( !d.exists( sPath ) )
	{
		if ( !d.mkpath( sPath ) )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "Cannot create %1 skipping..." ).arg( sPath ) );
			return;
		}
	}

	if ( !d.isReadable() )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "Directory %1 is not readable,skipping..." ).arg( sPath ) );
		return;
	}

	// check if directory entry already exists
	l.relock();
	if ( !m_bActive )
	{
		return;
	}

	qint64 nDirID = 0;

	QSqlQuery query( m_oDatabase );
	query.prepare( "SELECT id FROM dirs WHERE parent = ? AND path LIKE ?" );
	query.bindValue( 0, QVariant( nParentID ) );
	query.bindValue( 1, QVariant( sPath ) );
	if ( !query.exec() )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed (dir id): %1" ).arg( query.lastError().text() ) );
		return;
	}

	if ( query.next() )
	{
		nDirID = query.record().value( 0 ).toLongLong();
		query.finish();
	}
	else
	{
		query.prepare( "INSERT INTO dirs (path, parent) VALUES(?,?)" );
		query.bindValue( 0, QVariant( sPath ) );
		query.bindValue( 1, QVariant( nParentID ) );
		if ( !query.exec() )
		{
			systemLog.postLog( LogSeverity::Debug,
							   QString( "SQL Query failed (missing dir): %1" ).arg( query.lastError().text() ) );
			return;
		}
		else
		{
			nDirID = query.lastInsertId().toLongLong();
		}
	}

	// now check if something is added to the dir
	query.prepare( "SELECT name FROM files WHERE dir_id = ?" );
	query.bindValue( 0, QVariant( nDirID ) );
	if ( !query.exec() )
	{
		systemLog.postLog( LogSeverity::Debug,
						   QString( "SQL Query failed (fetch files): %1" ).arg( query.lastError().text() ) );
		return;
	}

	QList<QString> lFilesInDB;
	QList<QString> lFilesInFS;

	while ( query.next() )
	{
		lFilesInDB.append( query.record().value( 0 ).toString() );
	}

	lFilesInFS = d.entryList( QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );

	foreach ( const QString & sFile, lFilesInDB )
	{
		lFilesInFS.removeOne( sFile );
	}

	m_oDatabase.transaction();
	foreach ( const QString & sFile, lFilesInFS )
	{
		QSqlQuery insq( m_oDatabase );
		insq.prepare( "INSERT INTO hash_queue (dir_id, filename) VALUES(?,?)" );
		insq.bindValue( 0, QVariant( nDirID ) );
		insq.bindValue( 1, QVariant( sFile ) );
		if ( !insq.exec() )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "Cannot queue file for hashing: %1" ).arg( insq.lastError().text() ) );
		}
		else
		{
			m_nRemainingFiles++;
			emit remainingFilesChanged( m_nRemainingFiles );
		}
	}
	m_oDatabase.commit();

	lFilesInDB.clear();
	lFilesInFS.clear();

	// now find and scan subdirs
	QList<QString> lSubdirs;
	lSubdirs = d.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks );

	/*m_oDatabase.transaction();
	foreach ( const QString& sDir, lSubdirs )
	{
		QSqlQuery insq(m_oDatabase);
		insq.prepare("INSERT OR IGNORE INTO dirs (path, parent) VALUES(?,?)");
		insq.bindValue(0, QVariant(d.absolutePath() + "/" + sDir));
		insq.bindValue(1, QVariant(nDirID));
		if( !insq.exec() )
		{
			qDebug() << "SQL Query failed (insert dirs):" << insq.lastError().text();
		}
	}
	m_oDatabase.commit();*/

	if ( !m_bActive )
	{
		return;
	}

	l.unlock();

	foreach ( const QString & sDir, lSubdirs )
	{
		scanFolder( d.absolutePath() + "/" + sDir, nDirID );
	}
}

// meant to be called from other threads
// Don't call it from ShareManager thread or it will deadlock
QList<QSqlRecord> ShareManager::query( const QString sQuery )
{
	QMutexLocker l( &m_oSection );

	m_lQueryResults.clear();
	emit executeQuery( sQuery );
	m_oQueryCond.wait( &m_oSection );

	QList<QSqlRecord> lRecs = m_lQueryResults;
	m_lQueryResults.clear();

	return lRecs;
}

void ShareManager::execQuery( const QString& sQuery )
{
	m_oSection.lock();

	QSqlQuery query( m_oDatabase );
	if ( !query.exec( sQuery ) )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( query.lastError().text() ) );
	}
	else
	{
		while ( query.next() )
		{
			m_lQueryResults.append( query.record() );
		}
	}

	m_oQueryCond.wakeAll();
	m_oSection.unlock();
}

void ShareManager::runHashing()
{
	QMutexLocker l( &m_oSection );

	systemLog.postLog( LogSeverity::Debug, QString( "CShareManager::RunHashing()" ) );
	//qDebug() << "CShareManager::RunHashing()";

	QSqlQuery query( m_oDatabase );
	query.prepare( "SELECT hq.rowid, hq.filename, d.id, d.path FROM hash_queue hq LEFT JOIN dirs d ON(hq.dir_id = d.id) LIMIT 100" );
	if ( !query.exec() )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( query.lastError().text() ) );
		//qDebug() << "SQL query failed: " << query.lastError().text();
		return;
	}

	QString sRowIDs;

	bool bFinished = true;

	while ( query.next() )
	{
		sRowIDs.append( query.record().value( 0 ).toString() ).append( "," );

		SharedFilePtr pFile( new SharedFile( query.record().value( 3 ).toString() + '/' + query.record().value(
												   1 ).toString() ) );
		pFile->setDirectoryID( query.record().value( 2 ).toLongLong() );

		FileHasher::hashFile( pFile );

		bFinished = false;
	}

	sRowIDs.truncate( sRowIDs.size() - 1 );

	query.exec( QString( "DELETE FROM hash_queue WHERE rowid IN(%1)" ).arg( sRowIDs ) );

	if ( bFinished )
	{
		buildHashTable();
		emit sharesReady();
	}
}

void ShareManager::onFileHashed( SharedFilePtr pFile )
{
	QMutexLocker l( &m_oSection );

	systemLog.postLog( LogSeverity::Debug, QString( "OnFileHashed" ) );

	pFile->refresh();
	pFile->m_bShared = true;
	pFile->serialize( &m_oDatabase );

	--m_nRemainingFiles;
	emit remainingFilesChanged( m_nRemainingFiles );
}

QueryHashTable* ShareManager::getHashTable()
{
	ASSUME_LOCK( m_oSection );
	if ( !m_bReady || !m_bTableReady )
	{
		return 0;
	}

	return m_pTable;
}

void ShareManager::buildHashTable()
{
	ASSUME_LOCK( m_oSection );
	if ( m_pTable == 0 )
	{
		m_pTable = new QueryHashTable();
		if ( !m_pTable )
		{
			return;
		}
		m_pTable->create();
	}

	m_pTable->clear();

	// TODO: Optimize it

	QSqlQuery q( m_oDatabase );
	q.prepare( "SELECT keyword FROM keywords" );
	if ( !q.exec() )
	{
		systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( q.lastError().text() ) );
		//qDebug() << "SQL query failed:" << q.lastError().text();
	}
	else
	{
		while ( q.next() )
		{
			m_pTable->addExactString( q.record().value( 0 ).toString() );
		}

		q.prepare( "SELECT sha1 FROM hashes" );
		if ( !q.exec() )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( q.lastError().text() ) );
			//qDebug() << "SQL query failed:" << q.lastError().text();
		}
		else
		{
			while ( q.next() )
			{
				QByteArray m_oHash = q.record().value( 0 ).toByteArray();
				Hash* pHash = Hash::fromRaw( m_oHash, Hash::SHA1 );
				if ( pHash )
				{
					m_pTable->addExactString( pHash->toURN() );
					delete pHash;
				}
			}
		}
		m_bTableReady = true;
	}
}

