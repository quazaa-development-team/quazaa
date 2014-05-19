﻿/*
** commonfunctions.h
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

#ifndef COMMONFUNCTIONS_H
#define COMMONFUNCTIONS_H

#include <iterator>
#include <stdlib.h>

#include <QList>
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QReadWriteLock>

#include "systemlog.h"
#include "NetworkCore/Hashes/hashset.h"

namespace common
{
void folderOpen( QString file );
QString vendorCodeToName( QString vendorCode );
QString fixFileName( QString sName );

/**
 * @brief getTempFileName Allows to obtain a name for an incomplete download file based on the most
 * important hash of that file.
 * @param vHashes The file HashSet.
 * @return The incomplete file name.
 */
QString getIncompleteFileName( const HashSet& vHashes );

QString formatBytes( quint64 nBytesPerSec );
QString writeSizeInWholeBytes( quint64 nBytes );
quint64 readSizeInBytes( QString sInput, bool& bOK );

/**
 * Used to indicate the 3 locations where settings and data files are stored on the system.
 */
typedef enum { programLocation, globalDataFiles, userDataFiles } Location;

/**
 * @brief securedSaveFile is designed to handle the saving of data files (such as the discovery
 * services list, the security rule list or the host cache file) to disk. It allows for
 * components to be designed in a way that allows to mostly ignore potential failures of writing
 * to disk process. In case of a failure, any previous file is left untouched, in case of a
 * success, any previous file with the specified file name is replaced.
 * Locking needs to be handled by the caller.
 * @param location: the location of the file to be written.
 * @param sFileName: path and name of the file relative to the specified path.
 * @param oComponent: component saving the file (for system log).
 * @param pManager: first argument of the writeData() function.
 * @param writeData(): Function pointer to the static function doing the actual writing to file.
 * @return true if successful, false otherwise.
 */
quint32 securedSaveFile( const QString& sPath, const QString& sFileName,
						 Component oComponent, const void* const pManager,
						 quint32 ( *writeData )( const void* const, QFile& ) );

/**
 * @brief getRandomUnusedPort
 * @param bClear - set this to true to clear the internal data structures. Frees ca. 2k RAM.
 * @return bClear ? 0 : a random port not known to be used by other applications
 */
quint16 getRandomUnusedPort( bool bClear = false );

inline quint32 getTNowUTC()
{
	return QDateTime::currentDateTimeUtc().toTime_t();
}

inline QDateTime getDateTimeUTC()
{
	QDateTime tNow = QDateTime::currentDateTimeUtc();
	Q_ASSERT( tNow.timeSpec() == Qt::UTC );
	return tNow;
}

template <typename T>
inline T getRandomNum( T min, T max )
{
	return min + T( ( ( max - min ) + 1 ) * ( double )( qrand() ) / ( RAND_MAX + 1.0 ) );
}

/**
 * This generates a (read/write) iterator from a (read-only) const_iterator.
 */
template<class T>
typename T::iterator getRWIterator( T& container, const typename T::const_iterator& const_it )
{
	typename T::iterator it = container.begin();
	typename T::const_iterator container_begin_const = container.begin();
	int nDistance = std::distance< typename T::const_iterator >( container_begin_const, const_it );
	std::advance( it, nDistance );
	return it;
}
}

#endif // COMMONFUNCTIONS_H

