/*
** $Id: timeoutreadlocker.h $
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

#ifndef TIMEOUTREADLOCKER_H
#define TIMEOUTREADLOCKER_H

#include <QReadWriteLock>
#include <QAtomicInt>

/**
 * Convenience class to make sure the lock state is always well defined while allowing to use
 * timeouts. This Class can also be used recursively. Plz make sure you don't modify the
 * QReadWriteLock externally while using it in this class, as this will result in undefined
 * behavior.
 */
class CTimeoutReadLocker
{
private:
	QReadWriteLock* m_pRWLock;
	QAtomicInt      m_nLockCount;

public:
	inline CTimeoutReadLocker( QReadWriteLock* lock, bool& success, int timeout = -1 );
	inline ~CTimeoutReadLocker();

	inline QReadWriteLock* readWriteLock() const;

	inline bool relock( int timeout = -1 );
	inline void unlock();
};

/**
 * @brief CTimeoutReadLocker::CTimeoutReadLocker
 * @param lock
 * @param success
 * @param timeout in ms
 */
CTimeoutReadLocker::CTimeoutReadLocker( QReadWriteLock* lock, bool& success, int timeout ) :
	m_pRWLock( lock ),
	m_nLockCount( 0 )
{
	success = lock->tryLockForRead( timeout );
	m_nLockCount.fetchAndAddOrdered( ( quint8 )success );
}

CTimeoutReadLocker::~CTimeoutReadLocker()
{
	while ( m_nLockCount.loadAcquire() )
	{
		unlock();
	}
}

QReadWriteLock* CTimeoutReadLocker::readWriteLock() const
{
	return m_pRWLock;
}

bool CTimeoutReadLocker::relock( int timeout )
{
	bool result = m_pRWLock->tryLockForRead( timeout );
	m_nLockCount.fetchAndAddOrdered( ( quint8 )result );
	return result;
}

void CTimeoutReadLocker::unlock()
{
	m_pRWLock->unlock();
	m_nLockCount.fetchAndAddOrdered( -1 );
}

#endif // TIMEOUTREADLOCKER_H
