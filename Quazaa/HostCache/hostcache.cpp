/*
** hostcache.cpp
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

#include "hostcache.h"
#include "hostcachehost.h"

using namespace HostManagement;

HostCache::HostCache() :
	m_pHostCacheDiscoveryThread( NULL ),
	m_oIDProvider( IDProvider<SourceID>() ),
	m_tLastSave( common::getTNowUTC() ),
	m_nMaxFailures( 0 ),
	m_nSizeAtomic( 0 ),
	m_nConnectablesAtomic( 0 )
{
}

HostCache::~HostCache()
{
}

void HostCache::lock()
{
	m_pSection.lock();
}

void HostCache::unlock()
{
	m_pSection.unlock();
}

/**
 * @brief start initializes the Host Cache.
 * Make sure this is not called before QApplication is instantiated.
 * Any custom initializations should be made within asyncStartUpHelper().
 * Locking: YES (asynchronous)
 */
void HostCache::start()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "start()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	QThread* pThread = new QThread();

	// set thread name
	pThread->setObjectName( "Host Cache and Discovery" );

	moveToThread( pThread );
	pThread->start( QThread::LowPriority );

	// Handle destruction gracefully.
	connect( pThread, &QThread::finished, pThread, &QObject::deleteLater );

	m_pHostCacheDiscoveryThread = pThread;
	QMetaObject::invokeMethod( this, "startUpInternal", Qt::BlockingQueuedConnection );
}

void HostCache::stop()
{
	// speed up HostCacheHost deletion process
	HostCacheHost::setShutDownFlag();

	m_pSection.lock();
	stopInternal();
	m_pSection.unlock();

	// Terminate thread and delete Host Cache as well as Discovery.
	m_pHostCacheDiscoveryThread->quit();
}

/**
 * @brief registerMetaTypes registers the necessary meta types for signal and slot connections.
 * Locking: /
 */
void HostCache::registerMetaTypes()
{
	static int foo = qRegisterMetaType< HostData* >( "HostData*" );
	static int bar = qRegisterMetaType< QSharedPointer<HostCacheHost> >( "QSharedPointer<HostCacheHost>" );

	Q_UNUSED( foo );
	Q_UNUSED( bar );

	registerMetaTypesInternal();
}

/**
 * @brief hasConnectable allows to find out whether the cache currently holds connectable hosts.
 * Locking: /
 * @return true if at least one connectable host is present; false otherwise.
 */
bool HostCache::hasConnectable()
{
	return m_nConnectablesAtomic.load();
}

/**
 * @brief size allows access to the number of Hosts in the Cache.
 * Locking: /
 * @return the number of hosts in the cache.
 */
quint32 HostCache::size() const
{
	return m_nSizeAtomic.loadAcquire();
}

/**
 * @brief CHostCache::isEmpty allows to check whether the Host Cache is empty.
 * Locking: /
 * @return true if the cache contains no hosts; false otherwise.
 */
bool HostCache::isEmpty() const
{
	return !m_nSizeAtomic.loadAcquire();
}
