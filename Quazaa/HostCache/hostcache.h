/*
** hostcache.h
**
** Copyright © Quazaa Development Team, 2013-2014.
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

#include <QMutex>
#include <QThread>
#include <QAtomicInt>

#include "endpoint.h"

// TODO: add mechanism to remove hosts and discovery services by their origin

class HostData;

namespace HostManagement
{

class HostCacheHost;

class HostCache : public QObject
{
	Q_OBJECT

public:
	// Thread used by the Host Cache
	QThread*        m_pHostCacheDiscoveryThread;

protected:
	mutable QMutex  m_pSection;

	mutable quint32 m_tLastSave;
	quint8          m_nMaxFailures;
	QAtomicInt      m_nSizeAtomic;
	QAtomicInt      m_nConnectablesAtomic;

public:
	HostCache();
	virtual ~HostCache();

	/**
	 * @brief lock locks the HostCache for synchronous access.
	 */
	void            lock();

	/**
	 * @brief unlock unlocks the HostCache.
	 */
	void            unlock();

	/**
	 * @brief start initializes the Host Cache.
	 * Make sure this is not called before QApplication is instantiated.
	 * Any custom initializations should be made within asyncStartUpHelper().
	 * Locking: YES (asynchronous)
	 */
	void            start();

	/**
	 * @brief stop prepares the Host Cache for deletion on application shutdown.
	 */
	void            stop();

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

	/**
	 * @brief size allows access to the number of Hosts in the Cache.
	 * Locking: /
	 * @return the number of hosts in the cache.
	 */
	quint32         size() const;

	/**
	 * @brief CHostCache::isEmpty allows to check whether the Host Cache is empty.
	 * Locking: /
	 * @return true if the cache contains no hosts; false otherwise.
	 */
	bool            isEmpty() const;

signals:
	/**
	 * @brief hostAdded informs about a new host having been added.
	 * @param pHost : the host
	 */
	void            hostAdded( HostData* pHostData );

	/**
	 * @brief hostRemoved informs about a host having been removed.
	 * @param pHost : the host
	 */
	void            hostRemoved( QSharedPointer<HostCacheHost> pHost );

	/**
	 * @brief hostInfo info signal to get informed about all hosts within the cache.
	 * See requestHostList() for more information.
	 * @param pHost : the host
	 */
	void            hostInfo( HostData* pHost );

	/**
	 * @brief hostUpdated informs about a host having been updated.
	 * @param nID : the GUI ID of the updated host
	 */
	//void            hostUpdated(quint32 nID);

	/**
	 * @brief loadingFinished is emitted after new Hosts have been loaded.
	 */
	void            loadingFinished();

	/**
	 * @brief clearTriggered
	 */
	void            clearTriggered();

private:
	/**
	 * @brief stopInternal prepares the Host Cache (sub classes) for deletion.
	 * Locking: REQUIRED
	 */
	virtual void    stopInternal() = 0;

	/**
	 * @brief registerMetaTypesInternal handles registering the necessary meta types of child
	 * classes.
	 */
	virtual void    registerMetaTypesInternal() = 0;

private slots:
	/**
	 * @brief asyncStartUpHelper is a private helper method for start().
	 * It contains among other things the signal slot connections specific to the respective host
	 * cache.
	 * Locking: YES
	 */
	virtual void    startUpInternal() = 0;
};

} // namespace HostManagement

#endif // HOSTCACHE_H
