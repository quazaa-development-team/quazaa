/*
** filehasher.h
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

#ifndef FILEHASHER_H
#define FILEHASHER_H

#include "sharedfile.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include "ShareManager/sharedfile.h"

class FileHasher: public QThread
{
	Q_OBJECT
public:
	static QMutex   m_pSection;
	static QQueue<SharedFilePtr> m_lQueue;
	static FileHasher** m_pHashers;
	static quint32  m_nMaxHashers;
	static quint32  m_nRunningHashers;

	static QWaitCondition m_oWaitCond;

	bool m_bActive;
	int	 m_nId;
public:
	FileHasher( QObject* parent = 0 );
	~FileHasher();
	static FileHasher* hashFile( SharedFilePtr pFile );
	void run();

signals:
	void fileHashed( SharedFilePtr );
	void queueEmpty();
	void hasherStarted( int ); // int - hasher id
	void hasherFinished( int ); // int - hasher id
	void hashingProgress( int, QString, double, int ); // hasher id, filename, percent, rate
};

#endif // FILEHASHER_H
