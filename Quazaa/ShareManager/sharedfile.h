/*
** SharedFile.h
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

#ifndef SHAREDFILE_H
#define SHAREDFILE_H

#include <QSharedPointer>

#include "file.h"

class QSqlDatabase;

class CSharedFile : public CFile
{

public:
	bool	m_bShared;

public:
	explicit CSharedFile( QObject* parent = NULL );
	explicit CSharedFile( const QString& file, QObject* parent = NULL );
	explicit CSharedFile( const QFile& file, QObject* parent = NULL );
	explicit CSharedFile( const QDir& dir, const QString& file, QObject* parent = NULL );
	explicit CSharedFile( const QFileInfo& fileinfo, QObject* parent = NULL );

	~CSharedFile() {}

	void serialize( QSqlDatabase* pDatabase );

private:
	void setup();
};

typedef QSharedPointer<CSharedFile> CSharedFilePtr;

#endif // SHAREDFILE_H
