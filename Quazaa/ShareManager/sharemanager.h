/*
** sharemanager.h
**
** Copyright © Quazaaa Development Team, 2009-2013.
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

#ifndef SHAREMANAGER_H
#define SHAREMANAGER_H

#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include "thread.h"
#include "sharedfile.h"

class QueryHashTable;

class ShareManager : public QObject
{
	Q_OBJECT

public:
	QMutex			m_oSection;
protected:
	QWaitCondition	m_oQueryCond;
	QSqlDatabase	m_oDatabase;
	bool			m_bActive;
	bool			m_bReady;

	QList<QSqlRecord> m_lQueryResults;

	QueryHashTable* 	m_pTable;
	bool				m_bTableReady;

	qint32				m_nRemainingFiles;
public:
	explicit ShareManager( QObject* parent = 0 );

	void start();
	void stop();

	void addDir( QString sPath );

	void removeDir( QString sPath );
	void removeDir( quint64 nId );
	void removeFile( QString sPath );
	void removeFile( quint64 nFileId );

	void scanFolder( QString sPath, qint64 nParentID = 0 );

	QueryHashTable* getHashTable();

	bool sharesAreReady()
	{
		return m_bReady;
	}

	QList<QSqlRecord> query( const QString sQuery );

protected:
	void buildHashTable();
signals:
	void sharesReady();
	void executeQuery( const QString& sQuery );

signals:
	void hasherStarted( int ); // int - hasher id
	void hasherFinished( int ); // int - hasher id
	void hashingProgress( int, QString, double, int ); // hasher id, filename, percent, rate
	void remainingFilesChanged( qint32 );

public slots:
	void setupThread();
	void cleanupThread();

	void runHashing();
	void onFileHashed( SharedFilePtr pFile );

protected slots:
	void syncShares();
	void execQuery( const QString& sQuery );
};


extern CThread shareManagerThread;
extern ShareManager shareManager;
#endif // SHAREMANAGER_H
