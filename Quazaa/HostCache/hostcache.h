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
#include "idprovider.h"

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
	QThread*            m_pHostCacheDiscoveryThread;
	IDProvider<quint32> m_oIDProvider;

protected:
	mutable QMutex      m_pSection;

	mutable quint32     m_tLastSave;
	quint8              m_nMaxFailures;
	QAtomicInt          m_nSizeAtomic;
	QAtomicInt          m_nConnectablesAtomic;

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
	 * <br><b>Locking: YES</b> (asynchronous)
	 *
	 * Make sure this is not called before QApplication is instantiated.
	 * Any custom initializations should be made within asyncStartUpHelper().
	 */
	void            start();

	/**
	 * @brief stop prepares the Host Cache for deletion on application shutdown.
	 * <br><b>Locking: YES</b>
	 */
	void            stop();

	/**
	 * @brief registerMetaTypes registers the necessary meta types for signal and slot connections.
	 * <br><b>Locking: /</b>
	 */
	void            registerMetaTypes();

	/**
	 * @brief hasConnectable allows to find out whether the cache currently holds connectable
	 * HostCacheHosts.
	 * <br><b>Locking: /</b>
	 *
	 * @return <code>true</code> if at least one connectable host is present;
	 * <br><code>false<code> otherwise.
	 */
	bool            hasConnectable();

	/**
	 * @brief size allows access to the number of [HostCacheHosts](@ref HostCacheHost) in the Cache.
	 * <br><b>Locking: /</b>
	 *
	 * @return the number of [HostCacheHosts](@ref HostCacheHost) in the cache.
	 */
	quint32         size() const;

	/**
	 * @brief isEmpty allows to check whether the Host Cache is empty.
	 * <br><b>Locking: /</b>
	 *
	 * @return <code>true</code> if the cache contains no hosts;
	 * <br><code>false<code> otherwise.
	 */
	bool            isEmpty() const;

	/**
	 * @brief onConnectSuccess informs the cache about a successful connection to a host that has
	 * (possibly) been retrieved from the cache.
	 * <br><b>Locking: YES</b> (asynchronous)
	 *
	 * @param rHost  The host that has been successfully connected to.
	 */
	virtual void    onConnectSuccess( const EndPoint& rHost ) = 0;

signals:
	/**
	 * @brief hostAdded informs about a new Host having been added.
	 * @param pHost  GUI data representing a HostCacheHost
	 */
	void            hostAdded( HostData* pHostData );

	/**
	 * @brief hostRemoved informs about a host having been removed.
	 * @param pHost  QSharedPointer to a HostCacheHost
	 */
	void            hostRemoved( QSharedPointer<HostCacheHost> pHost );

	/**
	 * @brief hostInfo info signal to get informed about all hosts within the cache.
	 * @see requestHostList()
	 * @param pHost  GUI data representing a HostCacheHost
	 */
	void            hostInfo( HostData* pHost );

	/**
	 * @brief hostUpdated informs about a host having been updated.
	 * @param nID  The GUI ID of the updated HostCacheHost
	 */
	void            hostUpdated( quint32 nID ); // may not be used by all subclass implementations

	/**
	 * @brief loadingFinished is emitted after new Hosts have been loaded.
	 */
	void            loadingFinished();

	/**
	 * @brief clearTriggered is emitted if the entire cache is cleared instead of emitting
	 * hostRemoved() for each host.
	 */
	void            clearTriggered();

private:
	/**
	 * @brief stopInternal prepares the Host Cache (sub classes) for deletion.
	 * <br><b>Locking: REQUIRED</b>
	 */
	virtual void    stopInternal() = 0;

	/**
	 * @brief registerMetaTypesInternal handles registering the necessary meta types of child
	 * classes.
	 * <br><b>Locking: /</b>
	 */
	virtual void    registerMetaTypesInternal() = 0;

private slots:
	/**
	 * @brief asyncStartUpHelper is a private helper method for start().
	 * <br><b>Locking: YES</b>
	 *
	 * It contains among other things the signal/slot connections specific to the respective host
	 * cache.
	 */
	virtual void    startUpInternal() = 0;
};

} // namespace HostManagement

#endif // HOSTCACHE_H
