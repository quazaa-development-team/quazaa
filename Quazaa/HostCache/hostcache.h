/*
** hostcache.h
**
** Copyright © Quazaa Development Team, 2013-2013.
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

#ifndef HOSTCACHE_H
#define HOSTCACHE_H

#include <QObject>

#include <QMutex>
#include <QThread>
#include <QAtomicInt>

#include "hostcachehost.h"

typedef QSharedPointer<QThread>         SharedThreadPtr;
typedef QSharedPointer<HostCacheHost>   SharedHostPtr;

class HostCache : public QObject
{
	Q_OBJECT

public:
	mutable QMutex          m_pSection;

	// Thread used by the Host Cache
	SharedThreadPtr         m_pHostCacheDiscoveryThread;

	mutable quint32         m_tLastSave;
	quint8                  m_nMaxFailures;
	QAtomicInt              m_nSizeAtomic;
	QAtomicInt              m_nConnectablesAtomic;

protected:

private:

public:
	HostCache();
	virtual ~HostCache();

	/**
	 * @brief start initializes the Host Cache.
	 * Make sure this is not called before QApplication is instantiated.
	 * Any custom initializations should be made within asyncStartUpHelper().
	 * Locking: YES (asynchronous)
	 */
	void            start();

	/**
	 * @brief registerMetaTypes registers the necessary meta types for signal and slot connections.
	 * Locking: /
	 */
	void            registerMetaTypes();

	/**
	 * @brief hasConnectable allows to find out whether the cache currently holds connectable hosts.
	 * @return true if at least one connectable host is present; false otherwise.
	 */
	bool            hasConnectable();

signals:
	/**
	 * @brief hostAdded informs about a new host having been added.
	 * @param pHost : the host
	 */
	void            hostAdded(SharedHostPtr pHost);

	/**
	 * @brief hostRemoved informs about a host having been removed.
	 * @param pHost : the host
	 */
	void            hostRemoved(SharedHostPtr pHost);

	/**
	 * @brief hostInfo info signal to get informed about all hosts within the cache.
	 * See requestHostList() for more information.
	 * @param pHost : the host
	 */
	void            hostInfo(SharedHostPtr pHost);

	/**
	 * @brief hostUpdated informs about a host having been updated.
	 * @param nID : the GUI ID of the updated host
	 */
	void            hostUpdated(quint32 nID);

public slots:

private slots:
	/**
	 * @brief asyncStartUpHelper is a private helper method for start().
	 * It contains among other things the signal slot connections specific to the respective host
	 * cache.
	 * Locking: YES
	 */

	virtual void    asyncStartUpHelper() = NULL;
};

#endif // HOSTCACHE_H
