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

#include "network.h"
#include "hostcache.h"
#include "geoiplist.h"
#include "quazaasettings.h"
#include "Misc/timedsignalqueue.h"
#include "Security/securitymanager.h"

#include "debug_new.h"

CHostCache hostCache;

/**
 * @brief CHostCache::CHostCache Constructor.
 */
CHostCache::CHostCache():
	m_oLokalAddress( CEndPoint() ),
#if ENABLE_HOST_CACHE_BENCHMARKING
	m_nLockWaitTime( 0 ),
	m_nWorkTime( 0 ),
#endif
	m_tLastSave( common::getTNowUTC() ),
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
CHostCache::~CHostCache()
{
	clear();
}

/**
 * @brief start initializes the Host Cache. Make sure this is called after QApplication is
 * instantiated.
 * Locking: YES (asynchronous)
 */
void CHostCache::start()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "start()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pHostCacheDiscoveryThread = TSharedThreadPtr( new QThread() );

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
void CHostCache::add(const CEndPoint host, const quint32 tTimeStamp)
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
void CHostCache::addKey(const CEndPoint host, const quint32 tTimeStamp, CEndPoint* pKeyHost, const quint32 nKey, const quint32 tNow)
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
void CHostCache::addAck(const CEndPoint host, const quint32 tTimeStamp, const quint32 tAck, const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addAck()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "addSyncAck", Qt::QueuedConnection, Q_ARG(CEndPoint, host),
							   Q_ARG(quint32, tTimeStamp), Q_ARG(quint32, tAck),
							   Q_ARG(quint32, tNow), Q_ARG(bool, true) );
}

/**
 * @brief CHostCache::updateFailures updates the number of failures of a given host.
 * Locking: YES (asynchronous)
 * @param oAddress: the host
 * @param nFailures: its new failures
 */
void CHostCache::updateFailures(const CEndPoint& oAddress, const quint32 nFailures)
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
 * @brief CHostCache::update updates the timestamp of a host reprosented by its THostCacheIterator.
 * Note that the caller needs to make sure the host is actually part of the cache.
 * Locking: REQUIRED
 * @param itHost: the iterator - must be valid and not the end of the list.
 * @param tTimeStamp: its timestamp
 * @return the CHostCacheHost pointer pertaining to the updated host.
 */
CHostCacheHost* CHostCache::update(THostCacheIterator& itHost, const quint32 tTimeStamp,
								   quint32 nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "update()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	CHostCacheHost* pHost = *itHost;

	Q_ASSERT( pHost->failures() < m_vlHosts.size() );

	THostCacheList& lHosts = m_vlHosts[pHost->failures()];
	Q_ASSERT( itHost !=  lHosts.end() );

	Q_ASSERT( nFailures <= m_nMaxFailures );
	if ( nFailures > m_nMaxFailures )
		nFailures = m_nMaxFailures;

	// create new host with correct data
	CHostCacheHost* pNew = new CHostCacheHost( *pHost, tTimeStamp, nFailures );

	m_nSizeAtomic.fetchAndAddRelaxed( -1 );
	lHosts.erase( itHost );
	delete pHost;
	pHost = NULL;

#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache,
					   QString( "pNew->failures(): "  ) + QString::number( pNew->failures() ) +
					   QString( " m_vlHosts.size(): " ) + QString::number( m_vlHosts.size()  ) +
					   QString( " m_nMaxFailures: "   ) + QString::number( m_nMaxFailures    ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING
	Q_ASSERT( pNew->failures() < m_vlHosts.size() );
	insert( pNew, m_vlHosts[nFailures] );

	return pNew;
}

/**
 * @brief CHostCache::remove removes a CEndPoint from the cache.
 * Locking: YES (asynchronous)
 * @param oHost: the CEndPoint
 */
void CHostCache::remove(const CEndPoint& oHost)
{
	QMetaObject::invokeMethod( this, "removeSync", Qt::QueuedConnection, Q_ARG(CEndPoint, oHost) );
}

/**
 * @brief CHostCache::remove removes a CHostCacheHost from the cache.
 * Note: It should never be necessary to remove a host manually from the cache, manual user
 * interaction excepted. Any banned hosts are removed from the cache automatically.
 * Locking: REQUIRED
 * @param pHost: the CHostCacheHost - is set to NULL on completion.
 */
void CHostCache::remove(CHostCacheHost*& pHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "remove(CHCHost)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	quint8 nFailures = pHost->failures();
	Q_ASSERT( nFailures < m_vlHosts.size() );

	THostCacheIterator it = find( pHost );

	if ( it != m_vlHosts[nFailures].end() )
	{
		remove( it, nFailures );
	}

	delete pHost;
	pHost = NULL;
}

/**
 * @brief CHostCache::addXTry adds XTry hosts to the cache.
 * Locking: YES (asynchronous)
 * @param sHeader: a string representation of the XTry header
 */
void CHostCache::addXTry(QString sHeader)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addXTry()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "asyncAddXTry", Qt::QueuedConnection,
							   Q_ARG(QString, sHeader) );
}

/**
 * @brief CHostCache::getXTry allows to generate an XTry header.
 * Locking: YES
 * @return the XTry header
 */
QString CHostCache::getXTry() const
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "getXTry()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	Q_ASSERT ( m_vlHosts.size() > 0 );
	const THostCacheList& lHosts = m_vlHosts[0];

	if ( !lHosts.size() )
	{
		m_pSection.unlock();
		return QString();    // sorry, no hosts in cache
	}

	const quint32 nMax      = 10;

	QString sReturn;
	quint32 nCount = 0;

	foreach ( CHostCacheHost* pHost, lHosts )
	{
		QDateTime tTimeStamp;
		tTimeStamp.setTimeSpec( Qt::UTC );
		tTimeStamp.setTime_t( pHost->timestamp() );

		sReturn.append( pHost->address().toStringWithPort() + " " );
		sReturn.append( tTimeStamp.toString( "yyyy-MM-ddThh:mmZ" ) );

		++nCount;

		if ( nCount == nMax )
			break;

		sReturn.append( "," );
	}

	if ( sReturn.isEmpty() )
	{
		m_pSection.unlock();
		return QString();
	}

	m_pSection.unlock();
	return QString( "X-Try-Hubs: " ) + sReturn;
}

/**
 * @brief CHostCache::onFailure increases the failure counter of a given CEndPoint
 * Locking: YES (asynchronous)
 * @param addr: the CEndPoint
 */
void CHostCache::onFailure(const CEndPoint& addr)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "onFailure()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QMetaObject::invokeMethod( this, "asyncOnFailure", Qt::QueuedConnection,
							   Q_ARG(CEndPoint, addr) );
}

/**
 * @brief CHostCache::getConnectable allows to obtain a CHostCacheHost to connect to from the Host
 * Cache.
 * Locking: REQUIRED
 * @param oExcept: a set of Hosts to exempt.
 * @param sCountry: the preferred country. If there are no hosts from the specified country or the
 * country ZZ is specified, country information is ignored.
 * @return a connectable CHostCacheHost.
 */
CHostCacheHost* CHostCache::getConnectable(const QSet<CHostCacheHost*>& oExcept, QString sCountry)
{
/*#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "getConnectable()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING*/

	ASSUME_LOCK( m_pSection );

	bool bCountry = ( sCountry != "ZZ" );
	static bool bSecondAttempt = false;

	if ( !m_nSizeAtomic.load() )
	{
		return NULL;
	}

	// First try untested or working hosts, then fall back to failed hosts to increase chances for
	// successful connection
	for ( quint8 nFailures = 0; nFailures < m_nMaxFailures &&
		  nFailures < m_vlHosts.size(); ++nFailures )
	{
		foreach ( CHostCacheHost * pHost, m_vlHosts[nFailures] )
		{
			if ( bCountry && pHost->address().country() != sCountry )
			{
				continue;
			}

			if ( pHost->connectable() )
			{
				if ( !oExcept.contains( pHost ) )
					return pHost;
			}
		}
	}

	CHostCacheHost* pReturn = NULL;

	if ( bSecondAttempt )
	{
		// Don't try a third time :D
		bSecondAttempt = false;
	}
	else
	{
		maintain();
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
bool CHostCache::hasConnectable()
{
/*#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "hasConnectable()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING*/

	m_pSection.lock();
	bool bReturn = getConnectable();
	m_pSection.unlock();

	return bReturn;
}

/**
 * @brief CHostCache::clear removes all hosts and frees all memory.
 * Locking: YES
 */
void CHostCache::clear()
{
	m_pSection.lock();

	THCLVector::iterator itFailures = m_vlHosts.begin();

	while ( itFailures != m_vlHosts.end() )
	{
		THostCacheList& lHosts = (*itFailures);
		while( lHosts.size() )
		{
			delete lHosts.back();
			lHosts.pop_back();
		}

		++itFailures;
	}

	m_vlHosts.clear();
	m_nSizeAtomic.store( 0 );

	m_pSection.unlock();
}

/**
 * @brief CHostCache::save saves the hosts in the cache.
 * Locking: REQUIRED
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void CHostCache::save(const quint32 tNow) const
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "save()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	quint32 nCount = common::securredSaveFile( common::globalDataFiles, "hostcache.dat",
											   Components::HostCache, this,
											   &CHostCache::writeToFile );
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
void CHostCache::pruneOldHosts(const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "pruneOldHosts()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	CHostCacheHost* pHost;

	const quint32 tExpire = tNow - quazaaSettings.Gnutella2.HostExpire;
	for ( quint8 nFailures = 0; nFailures < m_vlHosts.size(); ++nFailures )
	{
		THostCacheList& lHosts = m_vlHosts[nFailures];

		if ( lHosts.size() )
			pHost = lHosts.back(); // undefined behavior if list is empty :)

		while ( lHosts.size() && pHost->timestamp() < tExpire )
		{
			lHosts.pop_back();
			delete pHost;
			pHost = lHosts.back();

			m_nSizeAtomic.fetchAndAddRelaxed( -1 );
		}
	}
}

/**
 * @brief CHostCache::pruneByQueryAck removes all hosts with a tAck older than
 * tNow - quazaaSettings.Gnutella2.QueryHostDeadline.
 * Locking: YES
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void CHostCache::pruneByQueryAck(const quint32 tNow)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "pruneByQA()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	const quint32 tAckExpire = tNow - quazaaSettings.Gnutella2.QueryHostDeadline;
	for ( quint8 nFailures = 0; nFailures < m_vlHosts.size(); ++nFailures )
	{
		THostCacheList& lHosts = m_vlHosts[nFailures];

		for ( THostCacheIterator itHost = lHosts.begin(); itHost != lHosts.end(); ++itHost )
		{
			if ( (*itHost)->ack() && (*itHost)->ack() < tAckExpire )
			{
				delete *itHost;
				itHost = lHosts.erase( itHost );
				m_nSizeAtomic.fetchAndAddRelaxed( -1 );
			}
			else
			{
				++itHost;
			}
		}
	}
	m_pSection.unlock();
}

/**
  * Helper method for save()
  * Locking: REQUIRED
  * @return the number of hosts written to file
  */
quint32 CHostCache::writeToFile(const void * const pManager, QFile& oFile)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "writeToFile()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	QDataStream oStream( &oFile );
	CHostCache* pHostCache = (CHostCache*)pManager;

	ASSUME_LOCK( pHostCache->m_pSection );

	const quint16 nVersion = HOST_CACHE_CODE_VERSION;
	const quint32 nCount   = (quint32)pHostCache->m_nSizeAtomic.load();

	oStream << nVersion;
	oStream << nCount;

	if ( nCount )
	{
		THCLVector::iterator itFailures = pHostCache->m_vlHosts.begin();

		while ( itFailures != pHostCache->m_vlHosts.end() )
		{
			foreach ( CHostCacheHost* pHost, *itFailures )
			{
				oStream << pHost->address();
				oStream << pHost->failures();

				oStream << pHost->timestamp();
				oStream << pHost->lastConnect();
			}

			++itFailures;
		}
	}

	return nCount;
}

/**
 * @brief CHostCache::localAddressChanged needs to be triggered on lokal IP changes.
 */
void CHostCache::localAddressChanged()
{
	Network.m_pSection.lock();
	m_oLokalAddress = Network.GetLocalAddress();
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
CHostCacheHost* CHostCache::addSync(CEndPoint host, quint32 tTimeStamp, bool bLock)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, "addSync()" );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	const quint32 tNow = common::getTNowUTC();

	if ( bLock )
		m_pSection.lock();

	ASSUME_LOCK( m_pSection );

	CHostCacheHost* pReturn = addSyncHelper( host, tTimeStamp, tNow );

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
CHostCacheHost* CHostCache::addSyncKey(CEndPoint host, quint32 tTimeStamp, CEndPoint* pKeyHost,
									   const quint32 nKey, const quint32 tNow, bool bLock)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addSyncKey()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	if ( bLock )
		m_pSection.lock();

	CHostCacheHost* pReturn = addSyncHelper( host, tTimeStamp, tNow );

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
CHostCacheHost* CHostCache::addSyncAck(CEndPoint host, quint32 tTimeStamp,
									   const quint32 tAck, const quint32 tNow, bool bLock)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addSyncAck()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	if ( bLock )
		m_pSection.lock();

	CHostCacheHost* pReturn = addSyncHelper( host, tTimeStamp, tNow );

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
void CHostCache::removeSync(CEndPoint oHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "remove(CEndPoint)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	quint8 nFailures;
	THostCacheIterator it = find( oHost, nFailures );

	Q_ASSERT( nFailures < m_vlHosts.size() );

	if ( it != m_vlHosts[nFailures].end() )
	{
		delete *it;
		remove( it, nFailures );
	}

	m_pSection.unlock();
}

/**
 * @brief CHostCache::sanityCheck performs the sanity check after a new security rule has been
 * added.
 * Locking: YES
 */
void CHostCache::sanityCheck()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "sanityCheck()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	THCLVector::iterator itFailures = m_vlHosts.begin();

	while ( itFailures != m_vlHosts.end() )
	{
		THostCacheList& lHosts = *itFailures;
		THostCacheIterator itHost = lHosts.begin();
		CHostCacheHost* pHost = NULL;

		while ( itHost != lHosts.end() )
		{
			pHost = *itHost;

			if ( securityManager.isNewlyDenied( pHost->address() ) )
				itHost = remove( itHost, pHost->failures() );
			else
				++itHost;
		}

		++itFailures;
	}

	m_pSection.unlock();

	QMetaObject::invokeMethod( &securityManager, "sanityCheckPerformed", Qt::QueuedConnection );
}

/**
 * @brief CHostCache::maintain keeps everything neat and tidy.
 * Locking: YES
 */
void CHostCache::maintain()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "maintain()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	const quint32 tNow = common::getTNowUTC();

	quint8 nFailure = (quint8)(m_vlHosts.size() - 1);
	m_nMaxFailures  = quazaaSettings.Connection.FailureLimit;
	static quint8 nLastFailureLimit = m_nMaxFailures;

	if ( nLastFailureLimit != m_nMaxFailures )
	{
		// increase the vector size
		while ( nFailure < m_nMaxFailures )
		{
			m_vlHosts.push_back( THostCacheList() );
			++nFailure;
		}

		// Remove all hosts in lists with failure limit higher than settings
		while ( nFailure > m_nMaxFailures )
		{
			while ( m_vlHosts[nFailure].size() )
			{
				removeWorst( nFailure );
			}

			m_vlHosts.pop_back();
		}

		nLastFailureLimit = m_nMaxFailures;
	}

	// nMaxSize == 0 means that he size limit has been disabled.
	const quint32 nMaxSize = quazaaSettings.Gnutella.HostCacheSize;
	if ( nMaxSize && (quint32)m_nSizeAtomic.load() > nMaxSize )
	{
		const quint32 nMax = nMaxSize - nMaxSize / 4;

		// TODO: remove these 2 after testing
		Q_ASSERT( nMax > 0 );
		Q_ASSERT( nFailure == m_nMaxFailures );

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

	// Update m_bConnectable for all hosts that are currently marked as unconnectable
	quint32 tThrottle = quazaaSettings.Gnutella.ConnectThrottle;
	THCLVector::iterator itFailures = m_vlHosts.begin();
	while ( itFailures != m_vlHosts.end() )
	{
		Q_ASSERT( tNow > tThrottle ); // if not, the following if statement is wrong

		foreach ( CHostCacheHost* pHost, *itFailures )
		{
			// Note: if ( !pHost->m_tLastConnect ), the following statement also evaluates to true.
			if ( !pHost->connectable() )
				pHost->setConnectable( tNow > pHost->lastConnect() + tThrottle );
		}
		++itFailures;

		// tThrottle = Gnutella.ConnectThrottle + pHost->failures() * Connection.FailurePenalty
		tThrottle += quazaaSettings.Connection.FailurePenalty;
	}

	m_pSection.unlock();
}

/**
 * @brief addSyncHelper adds synchronously.
 * Locking: REQUIRED
 * @param host: the CEndPoint to add
 * @param tTimeStamp: its timestamp
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the CEndPoint
 */
CHostCacheHost* CHostCache::addSyncHelper(const CEndPoint& host, quint32 tTimeStamp,
										  const quint32 tNow, quint32 nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "addSyncHelper()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	if ( !host.isValid() )
	{
		return NULL;
	}

	if ( host.isFirewalled() )
	{
		return NULL;
	}

	// At this point the security check should already have been performed.
	// Q_ASSERT( !securityManager.isDenied(host) );
	if ( securityManager.isDenied(host) )
	{
		return NULL;
	}

	// Don't add own IP to the cache.
	if ( host == m_oLokalAddress )
	{
		return NULL;
	}

	if ( tTimeStamp > tNow )
	{
		tTimeStamp = tNow - 60 ;
	}

	{   // update existing if such can be found
		quint8 nOldFailures;
		THostCacheIterator itPrevious = find( host, nOldFailures );

		Q_ASSERT( nOldFailures < m_vlHosts.size() );
		THostCacheList& lHosts = m_vlHosts[nOldFailures];

		if ( itPrevious != lHosts.end() )
		{
			CHostCacheHost* pUpdate = update( itPrevious, tTimeStamp, nFailures );
			return pUpdate;
		}
	}

	Q_ASSERT( nFailures <= m_nMaxFailures );
	if ( nFailures > m_nMaxFailures )
		nFailures = m_nMaxFailures;

	// create host, find place in sorted list, insert it there
	CHostCacheHost* pNew = new CHostCacheHost( host, tTimeStamp, nFailures );
	insert( pNew, m_vlHosts[nFailures] );

	return pNew;
}

/**
 * @brief CHostCache::insert inserts a new CHostCacheHost on the correct place into lHosts.
 * Locking: REQUIRED
 * @param pNew: the CHostCacheHost
 * @param tTimeStamp: its timestamp
 * @param lHosts: the list of hosts
 */
void CHostCache::insert(CHostCacheHost* pNew, THostCacheList& lHosts)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "insert()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	THostCacheIterator it = lHosts.begin();

	// advance until 1 element past the last element with higher timestamp
	while ( it != lHosts.end() && (*it)->timestamp() > pNew->timestamp() )
		++it;

	if ( it == lHosts.begin() ) // it hasn't been advanced
	{
		lHosts.push_front( pNew );
	}
	else
	{
		// go 1 element back (to last element with higher timestamp) and insert after this element
		lHosts.insert( --it, pNew );
	}

	// remember own position in list
	pNew->setIterator( it );

	m_nSizeAtomic.fetchAndAddRelaxed( 1 );
}

/**
 * @brief CHostCache::remove removes a host by its iterator. Caller must make sure to free the
 * memory (if requried).
 * Locking: REQUIRED
 * @param itHost: the iterator - must be valid and not the end of the list.
 * @param nFailures: the number of failures
 * @return the new iterator
 */
THostCacheIterator CHostCache::remove(THostCacheIterator& itHost, const quint8 nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "remove(iterator)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	Q_ASSERT( nFailures < m_vlHosts.size() );

	m_nSizeAtomic.fetchAndAddRelaxed( -1 );
	return m_vlHosts[nFailures].erase( itHost );
}

/**
 * @brief CHostCache::removeWorst removes the oldest host with a given failure limit - or a smaller
 * one, if there are no hosts with the requested limit.
 * Locking: REQUIRED
 * @param nFailures: the number of failures; this is set to the number of failures of the removed
 * host
 */
void CHostCache::removeWorst(quint8& nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "removeWorst()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	if ( nFailures >= m_vlHosts.size() )
		nFailures = (quint8)(m_vlHosts.size() - 1); // decrease to highest possible value

	while ( !m_vlHosts[nFailures].size() )
	{
		--nFailures;

		if ( nFailures == 255 ) // overflow
			return;
	}

	delete m_vlHosts[nFailures].back();
	m_vlHosts[nFailures].pop_back();
	m_nSizeAtomic.fetchAndAddRelaxed( -1 );
}

/**
 * @brief CHostCache::find allows to obtain the list iterator of a given CEndPoint.
 * Locking: REQUIRED
 * @param oHost: the given CEndPoint
 * @param nFailures: allows the caller to obtain the amount of failures of the host.
 * @return the iterator respectively m_vlHosts[0].end() if not found.
 */
THostCacheIterator CHostCache::find(const CEndPoint& oHost, quint8& nFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "find(CEndPoint)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	THCLVector::iterator itFailures = m_vlHosts.begin();
	quint8 nFailIter = 0;
	while ( itFailures != m_vlHosts.end() )
	{
		THostCacheList &lHosts = *itFailures;
		for ( THostCacheIterator it = lHosts.begin(); it != lHosts.end(); ++it )
		{
			if ( (*it)->address() == oHost )
			{
				nFailures = (*it)->failures();
				Q_ASSERT( nFailIter == nFailures );
				return it;
			}
		}
		++itFailures;
		++nFailIter;
	}

	nFailures = 0;
	return (m_vlHosts[0]).end();
}

/**
 * @brief CHostCache::find allows to obtain the list iterator of a given CHostCacheHost.
 * Locking: REQUIRED
 * @param pHost: the given CHostCacheHost
 * @return the iterator respectively m_vlHosts[pHost->failures()].end() if not found.
 */
THostCacheIterator CHostCache::find(const CHostCacheHost* const pHost)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "find(CHCHost)" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pHost->failures() < m_vlHosts.size() );

	return pHost->iteratorValid() ? pHost->iterator() : m_vlHosts[pHost->failures()].end();
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
void CHostCache::load()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "load()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	QFile file( common::getLocation( common::globalDataFiles ) + "hostcache.dat" );

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

		CHostCacheHost* pHost = NULL;

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

			pHost = NULL;
			oAddress.clear();
			nFailures    = 0;
			tTimeStamp   = 0;
			tLastConnect = 0;
		}
	}

	file.close();

	pruneOldHosts( tNow );
	m_pSection.unlock();

	systemLog.postLog( LogSeverity::Debug,
					   m_sMessage + QObject::tr( "Loaded %1 hosts." ).arg( m_nSizeAtomic.load() ) );
}

/**
 * @brief CHostCache::asyncStartUpHelper helper method for start().
 * Locking: YES
 */
void CHostCache::asyncStartUpHelper()
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncStartUpH()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	connect( &securityManager, SIGNAL( performSanityCheck() ), this, SLOT( sanityCheck() ) );
	connect( &Network, SIGNAL( LocalAddressChanged() ), this, SLOT( localAddressChanged() ) );

	m_nMaxFailures = quazaaSettings.Connection.FailureLimit;
	for ( quint8 i = 0; i <= m_nMaxFailures; ++i )
		m_vlHosts.push_back( THostCacheList() );

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
void CHostCache::asyncUpdateFailures(CEndPoint oAddress, quint32 nNewFailures)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncUpdtFail()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	quint8 nOldFailures;
	THostCacheIterator itHost = find( oAddress, nOldFailures );

	Q_ASSERT( nOldFailures < m_vlHosts.size() );

	if ( nOldFailures != nNewFailures && itHost != m_vlHosts[nOldFailures].end() )
	{
		CHostCacheHost* pHost = *itHost;
		remove( itHost, nOldFailures );      // remove host with old failure count


		if ( nNewFailures <= m_nMaxFailures )
		{
			// insert new host with correct failure count into correct list
			insert( new CHostCacheHost( *pHost, pHost->timestamp(), nNewFailures ),
					m_vlHosts[nNewFailures] );
		}

		// free memory of old host
		delete pHost;
	}

	m_pSection.unlock();
}

/**
 * @brief CHostCache::asyncAddXTry helper method for addXTry().
 * Locking: YES
 * @param sHeader
 */
void CHostCache::asyncAddXTry(QString sHeader)
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
void CHostCache::asyncOnFailure(CEndPoint addr)
{
#if ENABLE_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "asyncOnFailure()" ) );
#endif //ENABLE_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	quint8 nFailures;
	THostCacheIterator itHost = find( addr, nFailures );

	Q_ASSERT( nFailures < (quint8)(m_vlHosts.size()) );

	THostCacheList& lHosts = m_vlHosts[nFailures];
	if ( itHost != lHosts.end() )
	{
		CHostCacheHost* pHost = *itHost;
		remove( itHost, nFailures );      // remove host with old failure count

		if ( nFailures < m_nMaxFailures ) // if failure count may be increased
		{
			// insert new host with correct failure count into correct list
			insert( new CHostCacheHost( *pHost, pHost->timestamp(), ++nFailures ),
					m_vlHosts[nFailures] );
		}

		// free memory of old host
		delete pHost;
	}

	m_pSection.unlock();
}
