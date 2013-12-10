/*
** hostcache.cpp
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

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

#include <QDir>
//#include <QMetaMethod>

#include "network.h"
#include "hostcache.h"
#include "geoiplist.h"
#include "quazaasettings.h"
#include "Misc/timedsignalqueue.h"
#include "Security/securitymanager.h"

#include "debug_new.h"

G2HostCache hostCache;

/**
 * @brief CHostCache::CHostCache Constructor.
 */
G2HostCache::G2HostCache() :
	m_pFailures( NULL ),
	m_oLokalAddress( CEndPoint() ),
#if ENABLE_HOST_CACHE_BENCHMARKING
	m_nLockWaitTime( 0 ),
	m_nWorkTime( 0 ),
#endif
	m_tLastSave( 0 ),
	m_nMaxFailures( 0 ),
	m_nSizeAtomic( 0 )
{
	static int foo = qRegisterMetaType<CEndPoint>( "CEndPoint" );
	static int bar = qRegisterMetaType<CEndPoint*>( "CEndPoint*" );

	Q_UNUSED( foo );
	Q_UNUSED( bar );
}

/**
 * @brief CHostCache::~CHostCache Destructor.
 */
G2HostCache::~G2HostCache()
{
	clear();
}

/**
 * @brief start initializes the Host Cache. Make sure this is called after QApplication is
 * instantiated.
 * Locking: YES (asynchronous)
 */
void G2HostCache::start()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "start()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pHostCacheDiscoveryThread = SharedThreadPtr( new QThread() );

	moveToThread( m_pHostCacheDiscoveryThread.data() );
	m_pHostCacheDiscoveryThread.data()->start( QThread::LowPriority );

	QMetaObject::invokeMethod( this, "asyncStartUpHelper", Qt::QueuedConnection );
}

/**
 * @brief CHostCache::add adds a CEndPoint asynchronously to the Host Cache.
 * Locking: YES (asynchronous)
 * @param host: the CEndPoint
 * @param tTimeStamp: its timestamp
 */
void G2HostCache::add(const CEndPoint host, const quint32 tTimeStamp)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "add()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "addSync", Qt::QueuedConnection, Q_ARG(CEndPoint, host),
							   Q_ARG(quint32, tTimeStamp), Q_ARG(bool, true) );
}

/**
 * @brief CHostCache::addKey adds a CEndPoint asynchronously to the Host Cache.
 * Locking: YES (asynchronous)
 * @param host
 * @param tTimeStamp
 * @param pKeyHost
 * @param nKey
 * @param tNow
 */
void G2HostCache::addKey(const CEndPoint host, const quint32 tTimeStamp, CEndPoint* pKeyHost, const quint32 nKey, const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addKey()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "addSyncKey", Qt::QueuedConnection, Q_ARG(CEndPoint, host),
							   Q_ARG(quint32, tTimeStamp), Q_ARG(CEndPoint*, pKeyHost),
							   Q_ARG(quint32, nKey),       Q_ARG(quint32, tNow),
							   Q_ARG(bool, true) );
}

/**
 * @brief CHostCache::addAck adds a CEndPoint asynchronously to the Host Cache.
 * Locking: YES (asynchronous)
 * @param host
 * @param tTimeStamp
 * @param tAck
 * @param tNow
 */
void G2HostCache::addAck(const CEndPoint host, const quint32 tTimeStamp, const quint32 tAck, const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addAck()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "addSyncAck", Qt::QueuedConnection, Q_ARG(CEndPoint, host),
							   Q_ARG(quint32, tTimeStamp), Q_ARG(quint32, tAck),
							   Q_ARG(quint32, tNow), Q_ARG(bool, true) );
}

/**
 * @brief CHostCache::get allows you to access the CHostCacheHost object pertaining to a given CEndPoint.
 * Locking: REQUIRED
 * @param oHost: The CEndPoint.
 * @return the CHostCacheHost; NULL if the CEndPoint has not been found in the cache.
 */
SharedG2HostPtr G2HostCache::get(const CEndPoint& oHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "get(const CEndPoint&)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	G2HostCacheIterator it = find( oHost );
	return ( ( it == m_lHosts.end() ) ? SharedG2HostPtr() : *it );
}

/**
 * @brief CHostCache::check allows to verify if a given CHostCacheHost is part of the cache. The
 * information is guaranteed to stay valid as long as the mutex is held.
 * Locking: REQUIRED
 * @param pHost: the CHostCacheHost to check.
 * @return true if the host could be found in the cache, false otherwise.
 */
bool G2HostCache::check(const SharedG2HostPtr pHost) const
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "check(const SharedG2HostPtr)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pHost->failures() <= m_nMaxFailures );

	G2HostCache* pThis = const_cast<G2HostCache*>( this );

	G2HostCacheIterator it = pThis->find( pHost );
	return it != pThis->m_lHosts.end();
}

/**
 * @brief CHostCache::updateFailures updates the number of failures of a given host.
 * Locking: YES (asynchronous)
 * @param oAddress: the host
 * @param nFailures: its new failures
 */
void G2HostCache::updateFailures(const CEndPoint& oAddress, const quint32 nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "updateFailures()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "asyncUpdateFailures", Qt::QueuedConnection,
							   Q_ARG(CEndPoint, oAddress), Q_ARG(quint32, nFailures) );
}

/**
 * @brief CHostCache::update updates the timestamp of a CEndPoint contained in the Host Cache.
 * Note that the caller needs to make sure the host is actually part of the cache.
 * Locking: REQUIRED
 * @param oHost: the CEndPoint
 * @param tTimeStamp: its new timestamp
 * @return the CHostCacheHost pointer pertaining to the updated host.
 */
/*CHostCacheHost* CHostCache::update(const CEndPoint& oHost, const quint32 tTimeStamp)
{
	ASSUME_LOCK( m_pSection );

	quint8 nFailures;
	THostCacheIterator it = find( oHost, nFailures );

	Q_ASSERT( nFailures < m_vlHosts.size() );

	if ( it == m_vlHosts[nFailures].end() )
		return NULL;

	return update( it, tTimeStamp );
}*/

/**
 * @brief CHostCache::update updates the timestamp of a host reprosented by its HostCacheIterator.
 * Note that the caller needs to make sure the host is actually part of the cache.
 * Locking: REQUIRED
 * @param itHost: must be valid and not the end of the list; invalidated by function call
 * @param tTimeStamp: its timestamp
 * @param nFailures: the new amount of failures. If nFailures > m_nMaxFailures, host will be removed
 * @return the CHostCacheHost pointer pertaining to the updated host.
 */
SharedG2HostPtr G2HostCache::update(const G2HostCacheIterator& itHost, const quint32 tTimeStamp,
									   const quint32 nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "update(G2HostCacheIterator&, const quint32, const quint32)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	Q_ASSERT( itHost !=  m_lHosts.end() );

	SharedG2HostPtr pHost = *itHost;
	SharedG2HostPtr pNew;

	// TODO: remove for Quazaa beta 1
	Q_ASSERT( pHost->failures() <= m_nMaxFailures );

	if ( nFailures <= m_nMaxFailures )
	{
		// create new host with correct data
		pNew = SharedG2HostPtr( new G2HostCacheHost( *pHost, tTimeStamp, nFailures ) );
	}

	m_nSizeAtomic.fetchAndAddRelaxed( -1 );
	m_lHosts.erase( itHost );

#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "pNew->failures(): " ) + QString::number( pNew->failures() ) +
					   QString( " m_lHosts.size(): " ) + QString::number( m_lHosts.size()  ) +
					   QString( " m_nMaxFailures: "  ) + QString::number( m_nMaxFailures    ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	if ( pNew )
	{
		insert( pNew );
	}

	return pNew;
}

/**
 * @brief CHostCache::remove removes a CEndPoint from the cache.
 * Locking: YES (asynchronous)
 * @param oHost: the CEndPoint
 */
void G2HostCache::remove(const CEndPoint& oHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "remove(CEndPoint&)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "removeSync", Qt::QueuedConnection, Q_ARG(CEndPoint, oHost) );
}

/**
 * @brief CHostCache::remove removes a CHostCacheHost from the cache.
 * Note: It should never be necessary to remove a host manually from the cache, manual user
 * interaction excepted. Any banned hosts are removed from the cache automatically.
 * Locking: REQUIRED
 * @param pHost: the CHostCacheHost - is set to NULL on completion.
 */
void G2HostCache::remove(SharedG2HostPtr pHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "remove(SharedG2HostPtr)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	G2HostCacheIterator it = find( pHost );

	if ( it != m_lHosts.end() )
	{
		remove( it );
	}
}

/**
 * @brief CHostCache::addXTry adds XTry hosts to the cache.
 * Locking: YES (asynchronous)
 * @param sHeader: a string representation of the XTry header
 */
void G2HostCache::addXTry(QString sHeader)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addXTry(QString)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "asyncAddXTry", Qt::QueuedConnection,
							   Q_ARG(QString, sHeader) );
}

/**
 * @brief CHostCache::getXTry allows to generate an XTry header.
 * Locking: YES
 * @return the XTry header
 */
QString G2HostCache::getXTry() const
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "getXTry()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	if ( !m_nSizeAtomic.load() )
	{
		return QString();    // sorry, no hosts in cache
	}

	m_pSection.lock();

	// TODO: remove in beta1
	Q_ASSERT( m_lHosts.size() > m_nMaxFailures + 1 ); // at least m_nMaxFailures + 2

	const quint32 nMax = 10;
	quint32 nCount     =  0;
	char nFailures     = -1;
	QString sReturn;

	foreach ( SharedG2HostPtr pHost, m_lHosts )
	{
		if ( pHost )
		{
			QDateTime tTimeStamp;
			tTimeStamp.setTimeSpec( Qt::UTC );
			tTimeStamp.setTime_t( pHost->timestamp() );

			sReturn.append( pHost->address().toStringWithPort() + " " );
			sReturn.append( tTimeStamp.toString( "yyyy-MM-ddThh:mmZ" ) );
			sReturn.append( "," );

			++nCount;

			if ( nCount == nMax )
				break;
		}
		else
		{
			if ( !nFailures )
				break;

			++nFailures;
		}
	}
	m_pSection.unlock();

	if ( sReturn.isEmpty() )
	{
		return QString();
	}
	else
	{
		// TODO: remove later
		Q_ASSERT( sReturn.at( sReturn.size() - 1 ) == QChar( ',' ) );

		return QString( "X-Try-Hubs: " ) + sReturn.remove( sReturn.size() - 1, 1 );
	}
}

/**
 * @brief CHostCache::onFailure increases the failure counter of a given CEndPoint
 * Locking: YES (asynchronous)
 * @param addr: the CEndPoint
 */
void G2HostCache::onFailure(const CEndPoint& addr)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "onFailure(CEndPoint&)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "asyncOnFailure", Qt::QueuedConnection,
							   Q_ARG(CEndPoint, addr) );
}

/**
 * @brief CHostCache::getConnectable allows to obtain a CHostCacheHost to connect to from the Host
 * Cache.
 * Locking: REQUIRED
 * @param oExcept: a set of Hosts to exempt.
 * @param sCountry: the preferred country. If there are no connectable hosts from the specified
 * country or the country ZZ is specified, country information is ignored.
 * @return a connectable CHostCacheHost.
 */
SharedG2HostPtr G2HostCache::getConnectable(const QSet<SharedG2HostPtr>& oExcept,
											 QString sCountry)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "getConnectable(const QSet<SharedG2HostPtr>&, QString)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	bool bCountry = ( sCountry != "ZZ" );
	static bool bSecondAttempt = false;

	if ( !m_nSizeAtomic.load() )
	{
		return SharedG2HostPtr();
	}

	// First try untested or working hosts, then fall back to failed hosts to increase chances for
	// successful connection
	foreach ( SharedG2HostPtr pHost, m_lHosts )
	{
		if ( pHost )
		{
			if ( bCountry && pHost->address().country() != sCountry )
			{
				continue;
			}

			if ( pHost->connectable() && !oExcept.contains( pHost ) )
			{
				return pHost;
			}
		}
	}

	SharedG2HostPtr pReturn;

	if ( bSecondAttempt )
	{
		// Don't try a third time :D
		bSecondAttempt = false;
	}
	else
	{
		maintainInternal(); // same as maintain() but without locking
		bSecondAttempt = true;
		pReturn = getConnectable( oExcept ); // ignore country on second attempt
	}

	return pReturn;
}

/**
 * @brief CHostCache::hasConnectable allows to access wthether there are currently connectable hosts
 * in the cache.
 * Locking: YES
 * @return true if there are; false otherwise.
 */
bool G2HostCache::hasConnectable()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "hasConnectable()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();
	bool bReturn = getConnectable();
	m_pSection.unlock();

	return bReturn;
}

/**
 * @brief CHostCache::clear removes all hosts and frees all memory.
 * Locking: YES
 */
void G2HostCache::clear()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "clear()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	// Make sure m_pFailures (contains iterators to NULL nodes) stays valid during removal.
	G2HostCacheIterator it = m_lHosts.begin();
	while ( it != m_lHosts.end() )
	{
		if ( (*it).isNull() )
		{
			++it;
		}
		else
		{
			it = m_lHosts.erase( it );
		}
	}

	m_pSection.unlock();
	m_nSizeAtomic.store( 0 );
}

/**
 * @brief CHostCache::save saves the hosts in the cache.
 * Locking: REQUIRED
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void G2HostCache::save(const quint32 tNow) const
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "save(const quint32)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	quint32 nCount = common::securedSaveFile( CQuazaaGlobals::DATA_PATH(), "hostcache.dat",
											  Components::HostCache, this,
											  &G2HostCache::writeToFile );
	if ( nCount )
	{
		m_tLastSave = tNow;
		systemLog.postLog( LogSeverity::Debug, Components::HostCache,
						   QObject::tr( "Saved %1 hosts." ).arg( nCount ) );
	}
}

/**
 * @brief CHostCache::pruneOldHosts removes all hosts older than
 * tNow - quazaaSettings.Gnutella2.HostExpire.
 * Locking: REQUIRED
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void G2HostCache::pruneOldHosts(const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "pruneOldHosts(const quint32)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	const quint32 tExpire = tNow - quazaaSettings.Gnutella2.HostExpire;

	SharedG2HostPtr pHost;
	G2HostCacheIterator it = --m_lHosts.end();

	// TODO: improve this

	// at least m_nMaxFailures + 1
	while ( it != m_lHosts.begin() )
	{
		pHost = *it;

		// if an access point or not old enough to remove
		if ( !pHost || pHost->timestamp() > tExpire )
		{
			--it;
			continue;
		}

		it = --m_lHosts.erase( it );

		m_nSizeAtomic.fetchAndAddRelaxed( -1 );
	}
}

/**
 * @brief CHostCache::pruneByQueryAck removes all hosts with a tAck older than
 * tNow - quazaaSettings.Gnutella2.QueryHostDeadline.
 * Locking: YES
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void G2HostCache::pruneByQueryAck(const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "pruneByQA()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	const quint32 tAckExpire = tNow - quazaaSettings.Gnutella2.QueryHostDeadline;
	for ( G2HostCacheIterator itHost = m_lHosts.begin(); itHost != m_lHosts.end(); )
	{
		if ( *itHost && (*itHost)->ack() && (*itHost)->ack() < tAckExpire )
		{
			itHost = m_lHosts.erase( itHost );
			m_nSizeAtomic.fetchAndAddRelaxed( -1 );
		}
		else
		{
			++itHost;
		}
	}
	m_pSection.unlock();
}

/**
  * Helper method for save()
  * Locking: REQUIRED
  * @return the number of hosts written to file
  */
quint32 G2HostCache::writeToFile(const void * const pManager, QFile& oFile)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "writeToFile()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QDataStream oStream( &oFile );
	G2HostCache* pHostCache = (G2HostCache*)pManager;

	ASSUME_LOCK( pHostCache->m_pSection );

	const quint16 nVersion = HOST_CACHE_CODE_VERSION;
	const quint32 nCount   = (quint32)pHostCache->m_nSizeAtomic.load();

	oStream << nVersion;
	oStream << nCount;

	if ( nCount )
	{
		foreach ( SharedG2HostPtr pHost, pHostCache->m_lHosts )
		{
			if ( pHost )
			{
				oStream << pHost->address();
				oStream << pHost->failures();

				oStream << pHost->timestamp();
				oStream << pHost->lastConnect();
			}
		}
	}

	return nCount;
}

/**
 * @brief CHostCache::count allows access to the size of the Host Cache.
 * Locking: /
 * @return the number of hosts in the cache.
 */
quint32 G2HostCache::count() const
{
	return m_nSizeAtomic.loadAcquire();
}

/**
 * @brief CHostCache::isEmpty allows to check whether the Host Cache is empty.
 * Locking: /
 * @return true if the cache contains no hosts; false otherwise.
 */
bool G2HostCache::isEmpty() const
{
	return !m_nSizeAtomic.loadAcquire();
}

/**
 * @brief requestHostInfo allows to request hostInfo() signals for all hosts.
 * Qt slot.
 * Locking: YES
 * @return the number of host info signals to expect
 */
quint32 G2HostCache::requestHostInfo()
{
	m_pSection.lock();

	quint32 nHosts = 0;

	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		if ( !(*it).isNull() )
		{
			emit hostInfo( *it );
			++nHosts;
		}
	}

	// TODO: remove later
	Q_ASSERT( nHosts == m_nSizeAtomic.load() );

	m_pSection.unlock();

	return nHosts;
}

/**
 * @brief Manager::registerMetaTypes registers the necessary meta types for signals and slots.
 * Locking: /
 */
void G2HostCache::registerMetaTypes()
{
	static int foo = qRegisterMetaType< SharedG2HostPtr >( "SharedG2HostPtr" );

	Q_UNUSED( foo );
}

/**
 * @brief CHostCache::localAddressChanged needs to be triggered on lokal IP changes.
 */
void G2HostCache::localAddressChanged()
{
	Network.m_pSection.lock();
	m_oLokalAddress = Network.getLocalAddress();
	Network.m_pSection.unlock();
}

/**
 * @brief CHostCache::addSync adds a given CEndPoint synchronously to the cache.
 * Locking: see bLock
 * @param host: the CEndPoint
 * @param tTimeStamp: its timestamp
 * @param bLock: does the method need to lock the mutex?
 * @return the CHostCacheHost pointer pertaining to the CEndPoint
 */
SharedG2HostPtr G2HostCache::addSync(CEndPoint host, quint32 tTimeStamp, bool bLock)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, "addSync()" );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	const quint32 tNow = common::getTNowUTC();

	if ( bLock )
		m_pSection.lock();

	ASSUME_LOCK( m_pSection );

	SharedG2HostPtr pReturn = addSyncHelper( host, tTimeStamp, tNow );

	if ( bLock )
		m_pSection.unlock();

	return pReturn;
}

/**
 * @brief CHostCache::addSync adds a given CEndPoint synchronously to the cache.
 * Locking: YES
 * @param host: the CEndPoint
 * @param tTimeStamp: its timestamp
 * @param pKeyHost: the query key host
 * @param nKey: the query key
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the CEndPoint
 */
SharedG2HostPtr G2HostCache::addSyncKey(CEndPoint host, quint32 tTimeStamp, CEndPoint* pKeyHost,
									   const quint32 nKey, const quint32 tNow, bool bLock)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addSyncKey()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	if ( bLock )
		m_pSection.lock();

	SharedG2HostPtr pReturn = addSyncHelper( host, tTimeStamp, tNow );

	if ( pReturn )
		pReturn->setKey( nKey, tNow, pKeyHost );

	if ( bLock )
		m_pSection.unlock();

	return pReturn;
}

/**
 * @brief CHostCache::addSync adds a given CEndPoint synchronously to the cache.
 * Locking: YES
 * @param host: the CEndPoint
 * @param tTimeStamp: its timestamp
 * @param tAck: the ack time
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the CEndPoint
 */
SharedG2HostPtr G2HostCache::addSyncAck(CEndPoint host, quint32 tTimeStamp,
									   const quint32 tAck, const quint32 tNow, bool bLock)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addSyncAck()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	if ( bLock )
		m_pSection.lock();

	SharedG2HostPtr pReturn = addSyncHelper( host, tTimeStamp, tNow );

	if ( pReturn )
		pReturn->setAck( tAck );

	if ( bLock )
		m_pSection.unlock();

	return pReturn;
}

/**
 * @brief CHostCache::removeSync removes a CEndPoint from the cache.
 * Note: It should never be necessary to remove a host manually from the cache, manual user
 * interaction excepted. Any banned hosts are removed from the cache automatically.
 * Locking: YES
 * @param oHost: the CEndPoint
 */
void G2HostCache::removeSync(CEndPoint oHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "remove(CEndPoint)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	G2HostCacheIterator it = find( oHost );

	if ( it != m_lHosts.end() )
	{
		remove( it );
	}

	m_pSection.unlock();
}

/**
 * @brief CHostCache::sanityCheck performs the sanity check after a new security rule has been
 * added.
 * Locking: YES
 */
void G2HostCache::sanityCheck()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "sanityCheck()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	//qDebug() << "[HostCache] Started sanity checking.";

	securityManager.m_oSanity.lockForRead();
	m_pSection.lock(); // obtain HostCache lock second in order to minimize HostCache lockdown time

	G2HostCacheIterator itHost = m_lHosts.begin();
	SharedG2HostPtr pHost;

	int nCount = 0;

	while ( itHost != m_lHosts.end() )
	{
		pHost = *itHost;

		if ( pHost && securityManager.m_oSanity.isNewlyDenied( pHost->address() ) )
		{
			itHost = remove( itHost );
			++nCount;
		}
		else
		{
			++itHost;
		}
	}

	m_pSection.unlock();
	securityManager.m_oSanity.unlock();

	QMetaObject::invokeMethod( &securityManager.m_oSanity, "sanityCheckPerformed",
							   Qt::QueuedConnection );

	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "Finished sanity checking. %1 hosts removed." ).arg( nCount ) );
}

/**
 * @brief CHostCache::maintain keeps everything neat and tidy.
 * Locking: YES
 */
void G2HostCache::maintain()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "maintain()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();
	qDebug() << "[HostCache] maintain() got lock!";
	maintainInternal();
	m_pSection.unlock();
}

void G2HostCache::maintainInternal()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "maintainInternal()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	const quint32 tNow = common::getTNowUTC();

	const quint8 nNewMaxFailures = quazaaSettings.Connection.FailureLimit;
	if ( m_nMaxFailures != nNewMaxFailures )
	{
		if ( m_nMaxFailures > nNewMaxFailures )
		{
			quint8 nFailure = m_nMaxFailures;

			// clear hosts with too much failures
			while ( nFailure > nNewMaxFailures )
			{
				removeWorst( nFailure );
			}

			// replace the old access point array by the new one
			G2HostCacheIterator* pNewFailures = new G2HostCacheIterator[nNewMaxFailures + 2];
			for ( quint8 i = 0; i < nNewMaxFailures + 2; ++i )
			{
				pNewFailures[i] = m_pFailures[i];
			}
			delete[] m_pFailures;
			m_pFailures = pNewFailures;

			// remove the now unnecessary access points
			for ( quint8 i = nNewMaxFailures; i < m_nMaxFailures; ++i )
			{
				Q_ASSERT( !m_lHosts.back() );
				m_lHosts.pop_back();
			}
		}
		//if ( m_nMaxFailures < nNewMaxFailures )
		else
		{
			G2HostCacheIterator* pNewFailures = new G2HostCacheIterator[nNewMaxFailures + 2];

			quint8 i = 0;
			// copy the access points that will lateron still be used
			while ( i < m_nMaxFailures + 2 )
			{
				pNewFailures[i] = m_pFailures[i];
				++i; // gcc doesn't like it if this is included in the previous statement.
			}

			G2HostCacheIterator it = m_lHosts.end();
			--it;

			// create the additional new access points
			while ( i < nNewMaxFailures + 2 )
			{
				m_lHosts.push_back( SharedG2HostPtr() );
				pNewFailures[i++] = ++it;
			}

			delete[] m_pFailures;
			m_pFailures = pNewFailures;
		}

		m_nMaxFailures = nNewMaxFailures;
	}

	// nMaxSize == 0 means that the size limit has been disabled.
	const quint32 nMaxSize = quazaaSettings.Gnutella.HostCacheSize;
	if ( nMaxSize && (quint32)m_nSizeAtomic.load() > nMaxSize )
	{
		const quint32 nMax = nMaxSize - nMaxSize / 4;
		quint8 nFailure    = m_nMaxFailures;

		// TODO: remove after testing
		Q_ASSERT( nMax > 0 );

		// remove 1/4 of all hosts if the cache gets too full - failed and oldest first
		while ( (quint32)m_nSizeAtomic.load() > nMax )
		{
			removeWorst( nFailure );
		}

		save( tNow );
	}
	else if ( tNow > m_tLastSave + 600 )
	{
		save( tNow );
	}

	// TODO: update m_bcon on attribute updates
	// TODO: check if statement

	// Update m_bConnectable for all hosts that are currently marked as unconnectable
	const quint16 tFailurePenalty = quazaaSettings.Connection.FailurePenalty;
	qint32 tThrottle = ((qint32)quazaaSettings.Gnutella.ConnectThrottle) - tFailurePenalty;

	Q_ASSERT( (qint64)tNow > tThrottle ); // if not, the following bool statement is wrong

	foreach ( SharedG2HostPtr pHost, m_lHosts )
	{
		if ( pHost )
		{
			// Note: if ( !pHost->m_tLastConnect ), the following statement also evaluates to true.
			if ( !pHost->connectable() )
				pHost->setConnectable( tNow > pHost->lastConnect() + tThrottle );
		}
		else // This will be triggered immediately at least once as the first node is always NULL.
		{
			// tThrottle = Gnutella.ConnectThrottle + pHost->failures() * Connection.FailurePenalty
			tThrottle += tFailurePenalty;
		}
	}
}

/**
 * @brief addSyncHelper adds synchronously.
 * Locking: REQUIRED
 * @param host: the CEndPoint to add
 * @param tTimeStamp: its timestamp
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the CEndPoint
 */
SharedG2HostPtr G2HostCache::addSyncHelper(const CEndPoint& oHostIP, quint32 tTimeStamp,
											  const quint32 tNow, quint32 nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addSyncHelper()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	if ( !oHostIP.isValid() )
	{
		return SharedG2HostPtr();
	}

	if ( oHostIP.isFirewalled() )
	{
		return SharedG2HostPtr();
	}

	Q_ASSERT( nFailures <= m_nMaxFailures ); //TODO: if this gets triggered, just comment it out. :)
	if ( nFailures > m_nMaxFailures )
	{
		return SharedG2HostPtr();
	}

	// At this point the security check should already have been performed.
	Q_ASSERT( !securityManager.isDenied( oHostIP ) );
	if ( securityManager.isDenied( oHostIP ) )
	{
		return SharedG2HostPtr();
	}

	// Don't add own IP to the cache.
	if ( oHostIP == m_oLokalAddress )
	{
		return SharedG2HostPtr();
	}

	if ( tTimeStamp > tNow )
	{
		tTimeStamp = tNow - 60 ;
	}

	{   // update existing if such can be found
		G2HostCacheIterator itPrevious = find( oHostIP );

		if ( itPrevious != m_lHosts.end() )
		{
			SharedG2HostPtr pUpdate = update( itPrevious, tTimeStamp, nFailures );
			return pUpdate;
		}
	}

	// create host, find place in sorted list, insert it there
	SharedG2HostPtr pNew = SharedG2HostPtr( new G2HostCacheHost( oHostIP, tTimeStamp, nFailures ) );
	insert( pNew );

	return pNew;
}

/**
 * @brief CHostCache::insert inserts a new CHostCacheHost on the correct place into lHosts.
 * Locking: REQUIRED
 * @param pNew: the CHostCacheHost
 * @param tTimeStamp: its timestamp
 * @param lHosts: the list of hosts
 */
void G2HostCache::insert(SharedG2HostPtr pNew)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "insert(SharedG2HostPtr, G2HostCacheIterator&)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pNew );

	G2HostCacheIterator it = m_pFailures[ pNew->failures() ];
	Q_ASSERT( !(*it) );

	// We got here via a call to m_pFailures, which means the first pointer is a NULL one.
	++it;

	// advance until 1 element past the last element with higher timestamp
	while ( it != m_lHosts.end() && *it && (*it)->timestamp() > pNew->timestamp() )
		++it;

	Q_ASSERT( it != m_lHosts.begin() && it != m_lHosts.end() );

	// The iterator points now to the element after the last one with
	// (*it)->timestamp() > pNew->timestamp().
	// Insert between the current position and the last one with higher timestamp.
	it = m_lHosts.insert( it, pNew );

	// remember own position in list
	pNew->setIterator( it );

	m_nSizeAtomic.fetchAndAddRelaxed( 1 );

	// TODO: remove in beta1
#ifdef _DEBUG
	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		SharedG2HostPtr pHost = *it;
		if ( pHost )
		{
			Q_ASSERT( pHost->iterator() == it );
		}
	}
#endif
}

/**
 * @brief CHostCache::remove removes a host by its iterator. Caller must make sure to free the
 * memory (if requried).
 * Locking: REQUIRED
 * @param itHost: the iterator - must be valid and not the end of the list.
 * @return the new iterator
 */
G2HostCacheIterator G2HostCache::remove(G2HostCacheIterator& itHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "remove(iterator)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( *itHost ); // only allow removal of hosts; don't touch m_pFailures[]

	m_nSizeAtomic.fetchAndAddRelaxed( -1 );

	G2HostCacheIterator itReturn = m_lHosts.erase( itHost );

	// TODO: remove in beta1
#ifdef _DEBUG
	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		SharedG2HostPtr pHost = *it;
		if ( pHost )
		{
			Q_ASSERT( pHost->iterator() == it );
		}
	}
#endif

	return itReturn;
}

/**
 * @brief CHostCache::removeWorst removes the oldest host with a given failure limit - or a smaller
 * one, if there are no hosts with the requested limit.
 * Locking: REQUIRED
 * @param nFailures: the number of failures; this is set to the number of failures of the removed
 * host
 */
void G2HostCache::removeWorst(quint8& nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "removeWorst()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	// TODO: remove in beta1
	Q_ASSERT( !(*m_lHosts.begin()) );

	if ( nFailures > m_nMaxFailures )
		nFailures = m_nMaxFailures; // decrease to highest possible value

	G2HostCacheIterator it = m_pFailures[nFailures + 1];
	--it;

	// skip over any failure access points in the way and make sure nFailures is properly decreased
	while ( !(*it) && it != m_lHosts.begin() )
	{
		--it;
		--nFailures;
	}

	if ( *it ) // if we got a valid host (e.g. if it != m_lHosts.begin() ), remove it
	{
		m_lHosts.erase( it );
		m_nSizeAtomic.fetchAndAddRelaxed( -1 );
	}
}

/**
 * @brief CHostCache::find allows to obtain the list iterator of a given CEndPoint.
 * Locking: REQUIRED
 * @param oHost: the given CEndPoint
 * @param nFailures: allows the caller to obtain the amount of failures of the host.
 * @return the iterator respectively m_vlHosts[0].end() if not found.
 */
G2HostCacheIterator G2HostCache::find(const CEndPoint& oHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "find(CEndPoint)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	qint16 nFailTest = -1;
	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		if ( *it )
		{
			if ( nFailTest != (*it)->failures() )
			{
				Q_ASSERT( nFailTest == (*it)->failures() );
			}

			if ( (*it)->address() == oHost )
			{
				return it;
			}
		}
		else
		{
			++nFailTest;
		}
	}

	return m_lHosts.end();
}

/**
 * @brief CHostCache::find allows to obtain the list iterator of a given CHostCacheHost.
 * Locking: REQUIRED
 * @param pHost: the given CHostCacheHost
 * @return the iterator respectively m_vlHosts[pHost->failures()].end() if not found.
 */
G2HostCacheIterator G2HostCache::find(const SharedG2HostPtr pHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "find(CHCHost)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pHost->failures() <= m_nMaxFailures );

	return pHost->iteratorValid() ? pHost->iterator() : m_lHosts.end();
}

/**
 * @brief CHostCache::find allows to obtain the list iterator of a given CHostCacheHost.
 * Locking: REQUIRED
 * @param pHost: the given CHostCacheHost
 * @return the iterator respectively m_vlHosts[pHost->failures()].end() if not found.
 */
/*THostCacheConstIterator CHostCache::find(const CHostCacheHost* const pHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, QString( "find(CHCHost)const" ), Components::HostCache );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pHost->failures() < m_vlHosts.size() );

	return pHost->iteratorValid() ? pHost->iterator() : m_vlHosts[pHost->failures()].end();
}*/

/**
 * @brief CHostCache::load reads the previously saved hosts from file and adds them to the cache.
 * Locking: YES
 */
void G2HostCache::load()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "load()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	QFile file( CQuazaaGlobals::DATA_PATH() + "hostcache.dat" );

	if ( !file.exists() || !file.open( QIODevice::ReadOnly ) )
		return;

	QDataStream oStream( &file );

	quint16 nVersion;
	quint32 nCount;

	oStream >> nVersion;
	oStream >> nCount;

	const quint32 tNow   = common::getTNowUTC();

	if ( nVersion == HOST_CACHE_CODE_VERSION ) // else get new hosts from Discovery Manager
	{
		CEndPoint oAddress;
		quint8  nFailures    = 0;
		quint32 tTimeStamp   = 0;
		quint32 tLastConnect = 0;

		SharedG2HostPtr pHost;

		while ( nCount )
		{
			oStream >> oAddress;
			oStream >> nFailures;

			oStream >> tTimeStamp;
			oStream >> tLastConnect;

			if ( tTimeStamp - tNow > 0 )
				tTimeStamp = tNow - 60;

			if ( securityManager.isDenied( oAddress ) )
			{
				continue;
			}

			pHost = addSyncHelper( oAddress, tTimeStamp, tNow, nFailures );;
			if ( pHost )
			{
				if ( tLastConnect > tNow )
					tLastConnect = tNow - 60;

				pHost->setLastConnect( tLastConnect );
			}

			--nCount; // 1 less to do

			oAddress.clear();
			nFailures    = 0;
			tTimeStamp   = 0;
			tLastConnect = 0;
		}
	}

	file.close();

	pruneOldHosts( tNow );
	m_pSection.unlock();

	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QObject::tr( "Loaded %1 hosts." ).arg( m_nSizeAtomic.load() ) );
}

/**
 * @brief CHostCache::asyncStartUpHelper helper method for start().
 * Locking: YES
 */
void G2HostCache::asyncStartUpHelper()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncStartUpH()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	connect( &securityManager.m_oSanity, SIGNAL( beginSanityCheck() ), this, SLOT( sanityCheck() ) );
	connect( &Network, SIGNAL( localAddressChanged() ), this, SLOT( localAddressChanged() ) );

	m_nMaxFailures = quazaaSettings.Connection.FailureLimit;
	m_pFailures    = new G2HostCacheIterator[m_nMaxFailures + 2];

	m_lHosts.push_back( SharedG2HostPtr() );
	G2HostCacheIterator it = m_lHosts.begin();
	m_pFailures[0] = it;

	// add m_nMaxFailures + 2 items to the list
	for ( quint8 i = 1; i < m_nMaxFailures + 2; ++i )
	{
		m_lHosts.push_back( SharedG2HostPtr() );
		m_pFailures[i] = ++it;
	}

	// Includes its own locking.
	load();
	maintain();

	signalQueue.push( this, "maintain", 10000, true );
}

/**
 * @brief CHostCache::asyncUpdateFailures helper method for updateFailures().
 * Locking: YES
 * @param oAddress: a CEndPoint
 * @param nFailures: its new value of failures
 */
void G2HostCache::asyncUpdateFailures(CEndPoint oAddress, quint32 nNewFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncUpdtFail()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	G2HostCacheIterator itHost = find( oAddress );

	if ( itHost != m_lHosts.end() )
	{
		SharedG2HostPtr pHost = *itHost;
		remove( itHost );      // remove host with old failure count

		if ( nNewFailures <= m_nMaxFailures )
		{
			G2HostCacheHost* pNew = new G2HostCacheHost( *pHost, pHost->timestamp(), nNewFailures );
			// insert new host with correct failure count into correct list
			insert( SharedG2HostPtr( pNew ) );
		}
	}

	m_pSection.unlock();
}

/**
 * @brief CHostCache::asyncAddXTry helper method for addXTry().
 * Locking: YES
 * @param sHeader
 */
void G2HostCache::asyncAddXTry(QString sHeader)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncAddXTry()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMutexLocker l( &m_pSection );

	// X-Try-Hubs: 86.141.203.14:6346 2010-02-23T16:17Z,91.78.12.117:1164 2010-02-23T16:17Z,89.74.83
	// .103:7972 2010-02-23T16:17Z,93.89.196.113:5649 2010-02-23T16:17Z,24.193.237.252:6346 2010-02-
	// 23T16:17Z,24.226.149.80:6346 2010-02-23T16:17Z,89.142.217.180:9633 2010-02-23T16:17Z,83.219.1
	// 12.111:6346 2010-02-23T16:17Z,201.17.187.205:6346 2010-02-23T16:17Z,213.29.19.41:6346 2010-02
	// -23T16:17Z,78.231.224.180:6346 2010-02-23T16:17Z,213.143.88.92:6346 2010-02-23T16:17Z,77.209.
	// 25.104:1515 2010-02-23T16:17Z,86.220.168.24:59153 2010-02-23T16:17Z,88.183.80.110:6346 2010-0
	// 2-23T16:17Z
	QStringList lEntries = sHeader.split( "," );

	if ( lEntries.isEmpty() )
	{
		return;
	}

	const quint32 tNow = common::getTNowUTC();
	for ( qint32 i = 0; i < lEntries.size(); ++i )
	{
		QStringList entry = lEntries.at( i ).split( " " );
		if ( entry.size() != 2 )
		{
			continue;
		}

		CEndPoint oAddress( entry.at( 0 ) );
		if ( !oAddress.isValid() || securityManager.isDenied( oAddress ) )
		{
			continue;
		}

		quint32 tTimeStamp;
		QDateTime oTimeStamp = QDateTime::fromString( entry.at( 1 ), "yyyy-MM-ddThh:mmZ" );
		oTimeStamp.setTimeSpec( Qt::UTC );
		if ( oTimeStamp.isValid() )
		{
			tTimeStamp = oTimeStamp.toTime_t();
		}
		else
		{
			tTimeStamp = tNow;
		}
		addSyncHelper( oAddress, tTimeStamp, tNow );
	}
}

/**
 * @brief CHostCache::asyncOnFailure helper method for onFailure().
 * Locking: YES
 * @param addr: the CEndPoint with the connection failure
 */
void G2HostCache::asyncOnFailure(CEndPoint addr)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncOnFailure()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	G2HostCacheIterator itHost = find( addr );

	if ( itHost != m_lHosts.end() )
	{
		SharedG2HostPtr pHost = *itHost;
		remove( itHost );      // remove host with old failure count

		quint8 nFailures = pHost->failures();
		if ( nFailures < m_nMaxFailures ) // if failure count may be increased
		{
			++nFailures;
			SharedG2HostPtr pNewHost = SharedG2HostPtr( new G2HostCacheHost( *pHost,
																			 pHost->timestamp(),
																			 nFailures ) );
			// insert new host with correct failure count into correct list
			insert( pNewHost );
		}
	}

	m_pSection.unlock();
}
