/*
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
/**
 * @brief folderOpen Opens a folder in the local default viewer. If it does not exist, a new folder
 * with the specified path is created.
 * @param sPath  The folder path.
 */
void folderOpen(const QString& sPath );

/**
 * @brief vendorCodeToName Maps known four letter vendor codes to application names.
 * @param vendorCode  The vendor code to map.
 * @return The application name.
 */
QString vendorCodeToName( const QString& vendorCode );

/**
 * @brief fixFileName Replaces unallowed characters in a file name or path with underscores and
 * limits the length to 255 characters.
 * @param sName  The file name.
 * @return The fixed file name.
 */
QString fixFileName( const QString& sName );

/**
 * @brief getTempFileName Allows to obtain a name for an incomplete download file based on the most
 * important hash of that file.
 * @param vHashes  The file HashSet.
 * @return The incomplete file name.
 */
QString getIncompleteFileName( const HashSet& vHashes );

/**
 * @brief formatBytes Converts a size in bytes to a human readable expression.
 * @param nBytesPerSec  The number of bytes.
 * @return A string representation of a number followed by a corresponding suffix.
 * Output examples: 3 MiB, 999.92 TiB, 60.92 KiB
 */
QString formatBytes( quint64 nBytesPerSec );

/**
 * @brief writeSizeInWholeBytes Converts a size in bytes to a human readable expression.
 * @param nBytes  The number of bytes.
 * @return A string representation of the size in bytes while avoiding any data loss.
 * Output examples: 999999999 B, 2 TiB, 5456 MB
 */
QString writeSizeInWholeBytes( quint64 nBytes );

/**
 * @brief readSizeInBytes Converts a given string into a number of bytes.
 * @param sInput  The input string. Examples: "30 MB", "6KiB", "1345289375629 B", "10000 TB"
 * @param bOK     The conversion verificator. Is set to true if successful; false otherwise.
 * @return 0 if the conversion failed; the number of bytes read from the string representation
 * otherwise.
 */
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
 * @param sPath       Location where the file should be written to.
 * @param sFileName   Path and name of the file relative to the specified path.
 * @param oComponent  Component saving the file (for system log).
 * @param pManager    First argument of the writeData() function.
 * @param writeData() Function pointer to the static function doing the actual writing to file.
 * @return true if successful; false otherwise.
 */
quint32 securedSaveFile( const QString& sPath, const QString& sFileName,
						 Component oComponent, const void* const pManager,
						 quint32 ( *writeData )( const void* const, QFile& ) );

/**
 * @brief getRandomUnusedPort Allows to obtain a random unused port number.
 * @param bClear   Set this to true to clear the internal data structures. Frees ca. 2k RAM.
 * @return bClear  0 if bClear is set to true; otherwise a random port number not known to be used
 * by an other application.
 */
quint16 getRandomUnusedPort( bool bClear = false );

/**
 * @brief getTNowUTC Allows to access the current time (UTC).
 * @return The number of seconds sinec 0:00:00 on January 1st, 1970 (UTC)
 */
inline quint32 getTNowUTC()
{
	return QDateTime::currentDateTimeUtc().toTime_t();
}

/**
 * @brief getDateTimeUTC Allows to access the current time (UTC).
 * @return The current UTC time as a QDateTime object.
 */
inline QDateTime getDateTimeUTC()
{
	return QDateTime::currentDateTimeUtc();
}

/**
 * @brief intToUint allows casting from signed to unsigned int in a well-defined way.
 *
 * @param n  The int to cast.
 * @return an unsigned integer
 */
quint32 intToUint( qint32 n );

/**
 * @brief uintToInt allows casting from unsigned int to int in a well-defined way.
 *
 * @param n  The uint to cast.
 * @return an integer
 */
qint32 uintToInt( quint32 n );

template <typename T>
/**
 * @brief getRandomNum Allows to obtain a random number within the interval [min;max]
 * @param min  The minimum value.
 * @param max  The maximum value.
 * @return A random value between min and max.
 */
inline T getRandomNum( T min, T max )
{
	return min + T( ( ( max - min ) + 1 ) * ( double )( qrand() ) / ( RAND_MAX + 1.0 ) );
}

template<class T>
/**
 * @brief getRWIterator Allows to obtain a (read/write) iterator from a (read-only) const_iterator.
 * @param container  The container the iterator is a part of.
 * @param const_it   The const_iterator.
 * @return A RW iterator pointing to the same item as const_it.
 */
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

