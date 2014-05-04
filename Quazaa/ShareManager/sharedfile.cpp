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

#include "sharedfile.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QVariant>
#include <QMap>
#include <QMetaType>
#include <QFileInfo>

#include "queryhashtable.h"
#include "debug_new.h"

SharedFile::SharedFile( QObject* parent ) :
	File( parent )
{
	setup();
}

SharedFile::SharedFile( const QString& file, QObject* parent ) :
	File( file, parent )
{
	setup();
}

SharedFile::SharedFile( const QFile& file, QObject* parent ) :
	File( file, parent )
{
	setup();
}

SharedFile::SharedFile( const QDir& dir, const QString& file, QObject* parent ) :
	File( dir, file, parent )
{
	setup();
}

SharedFile::SharedFile( const QFileInfo& fileinfo, QObject* parent ) :
	File( fileinfo, parent )
{
	setup();
}

void SharedFile::serialize( QSqlDatabase* pDatabase )
{
	// get directory id if needed
	if ( m_nDirectoryID == 0 )
	{
		QSqlQuery query( *pDatabase );

		query.prepare( "SELECT id FROM dirs WHERE path LIKE '?'" );
		query.bindValue( 0, absolutePath() );
		if ( !query.exec() )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "SQL Query failed: %1" ).arg( query.lastError().text() ) );
			return;
		}

		if ( query.size() != 1 )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "Bad record number: %1" ).arg( query.size() ) );
			return;
		}

		query.next();

		m_nDirectoryID = query.value( 0 ).toLongLong();
	}

	// now insert or update file record
	if ( m_nFileID == 0 )
	{
		// insert

		pDatabase->transaction();

		QSqlQuery query( *pDatabase );
		query.prepare( "INSERT INTO files (dir_id, name, size, last_modified, shared) VALUES (?,?,?,?,?)" );
		query.bindValue( 0, m_nDirectoryID );
		query.bindValue( 1, fileName() );
		query.bindValue( 2, size() );
		query.bindValue( 3, lastModified().toTime_t() );
		query.bindValue( 4, m_bShared );
		if ( !query.exec() )
		{
			systemLog.postLog( LogSeverity::Debug, QString( "Cannot insert new record: %1" ).arg( query.lastError().text() ) );
			return;
		}

		qint64 nFileID = query.lastInsertId().toLongLong();
		qDebug() << "New file ID: " << nFileID;

		QSqlQuery q2( *pDatabase );
		q2.exec( QString( "DELETE FROM hashes WHERE file_id = %1" ).arg( nFileID ) );

		QMap<QString, QVariant> mapValues;
		mapValues.insert( "file_id", QVariant( nFileID ) );

		for ( quint8 i = 0, nSize = m_vHashes.size(); i < nSize; ++i )
		{
			const Hash* const pHash =  m_vHashes[i];
			if ( pHash )
			{
				if ( pDatabase->record( "hashes" ).contains( pHash->getFamilyName() ) )
				{
					mapValues.insert( pHash->getFamilyName(), pHash->rawValue() );
				}
			}
		}

		if ( mapValues.count() > 1 )
		{
			QSqlQuery qh( *pDatabase );

			QString sFields, sValues;

			for ( QMap<QString, QVariant>::iterator itValues = mapValues.begin(); itValues != mapValues.end(); itValues++ )
			{
				sFields.append( itValues.key() ).append( "," );
			}
			sFields.truncate( sFields.size() - 1 );

			for ( int i = 0; i < mapValues.count(); i++ )
			{
				sValues += "?,";
			}
			sValues.truncate( sValues.size() - 1 );

			qh.prepare( QString( "INSERT INTO hashes (" + sFields + ") VALUES (" + sValues + ")" ) );

			int nCurrPos = 0;
			for ( QMap<QString, QVariant>::iterator itValues = mapValues.begin(); itValues != mapValues.end(); itValues++ )
			{
				qh.bindValue( nCurrPos, QVariant( itValues.value() ) );
				nCurrPos++;
			}

			if ( !qh.exec() )
			{
				systemLog.postLog( LogSeverity::Debug,
								   QString( "Cannot insert hashes: %1 %2" ).arg( qh.lastError().text() ).arg( qh.executedQuery() ) );
			}
		}

		QStringList lKeywords;
		QueryHashTable::makeKeywords( fileName(), lKeywords );

		QSqlQuery qkw( *pDatabase );
		qkw.prepare( "INSERT OR IGNORE INTO keywords (keyword) VALUES (?)" );
		foreach ( const QString & sKey, lKeywords )
		{
			qkw.bindValue( 0, QVariant( sKey ) );
			qkw.exec();
		}

		pDatabase->commit();
	}
}

void SharedFile::setup()
{
	m_bShared = false;

	static int dummy = qRegisterMetaType<SharedFilePtr>( "SharedFilePtr" );
	Q_UNUSED( dummy );
}

