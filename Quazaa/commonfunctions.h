/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2012.
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
#include <QDateTime>
#include <QReadWriteLock>

#include "systemlog.h"

//#define NO_OF_REGISTRATIONS 8

namespace common
{
void folderOpen( QString file );
QString vendorCodeToName( QString vendorCode );
QString fixFileName( QString sName );
QString getTempFileName( QString sName );

QString formatBytes( quint64 nBytesPerSec );
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

//    struct registeredSet
//    {
//        quint64 num64;
//        quint8 num8;
//    };
//    struct registerEntry
//    {
//        quint64 num64[NO_OF_REGISTRATIONS];
//    };
//    static quint64 registeredNumbers[NO_OF_REGISTRATIONS] = {};

//    registeredSet registerNumber();
//    bool unregisterNumber(registeredSet registered);

//    /**
//      * Returns an empty but initilized registerEntry.
//      */
//    inline registerEntry getRegisterEntry()
//    {
//        registerEntry res = { {} };
//        return res;
//    }

//    /**
//      * Returns true if registering set to entry was successful; false if already registered.
//      */
//    inline bool applyRegisteredSet( registeredSet &set, registerEntry &entry )
//    {
//        if ( entry.num64[set.num8] & set.num64 )
//        {
//            return false;
//        }
//        else
//        {
//            entry.num64[set.num8] &= set.num64;
//            return true;
//        }
//    }

//    /**
//      * Returns true if set was already registered to entry; false if already registered.
//      */
//    inline bool isRegisteredToEntry( registeredSet &set, registerEntry &entry )
//    {
//        if ( entry.num64[set.num8] & set.num64 )
//        {
//            return true;
//        }
//        else
//        {
//            return false;
//        }
//    }

template <typename T>
inline T getRandomNum( T min, T max )
{
	return min + T( ( ( max - min ) + 1 ) * ( double )( qrand() ) / ( RAND_MAX + 1.0 ) );
}

// TODO: Make this work.
// This generates a read/write iterator from a read-only iterator.
/*template<class T> inline typename T::iterator getRWIterator(T container, typename T::const_iterator const_it)
{
	typename T::iterator i = container.begin();
	typename T::const_iterator container_begin_const = container.begin();
	int nDistance = std::distance< typename T::const_iterator >( container_begin_const, const_it );
	std::advance( i, nDistance );
	return i;
}*/
}

#endif // COMMONFUNCTIONS_H

