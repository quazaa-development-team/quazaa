/*
** g2hostcache.cpp
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

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

#include <QDir>

#ifndef QUAZAA_SETUP_UNIT_TESTS
#include "neighbours.h"
#include "g2node.h"
#endif // QUAZAA_SETUP_UNIT_TESTS

#include "g2hostcache.h"

#include "g2hostcachehost.h"

#include "geoiplist.h"
#include "quazaasettings.h"
#include "Misc/timedsignalqueue.h"
#include "Security/securitymanager.h"

#include "debug_new.h"

using namespace HostManagement;

G2HostCache hostCache;

/**
 * @brief G2HostCache::G2HostCache Constructor.
 */
G2HostCache::G2HostCache() :
	m_pFailures( NULL ),
#if ENABLE_G2_HOST_CACHE_BENCHMARKING
	m_nLockWaitTime( 0 ),
	m_nWorkTime( 0 ),
#endif
	m_oLokalAddress( EndPoint() ),
	m_bLoading( false )
{
	static int foo = qRegisterMetaType<EndPoint>( "EndPoint" );
	Q_UNUSED( foo );
}

/**
 * @brief G2HostCache::~G2HostCache Destructor.
 */
G2HostCache::~G2HostCache()
{
	clear();
}

/**
 * @brief G2HostCache::add adds a EndPoint asynchronously to the Host Cache.
 * Locking: YES (asynchronous)
 * @param oHost: the EndPoint - is expected to have been checked against the security manager
 * @param tTimeStamp: its timestamp
 */
void G2HostCache::add( const EndPoint& oHost, quint32 tTimeStamp , SourceID nSource )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "add()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pfAddSync.invoke( this, Qt::QueuedConnection, Q_ARG( EndPoint, oHost ),
						Q_ARG( quint32, tTimeStamp ), Q_ARG( SourceID, nSource ) );
}

void G2HostCache::add( const EndPoint& oHost, quint32 tTimeStamp, const EndPoint& oSource )
{
	m_pfAddSyncSource.invoke( this, Qt::QueuedConnection, Q_ARG( EndPoint, oHost ),
							  Q_ARG( quint32, tTimeStamp ), Q_ARG( EndPoint, oSource ) );
}

void G2HostCache::add( const InsertList& lHosts, const EndPoint& oSource )
{
	m_pfAddSyncList.invoke( this, Qt::QueuedConnection, Q_ARG( InsertList, lHosts ),
							Q_ARG( EndPoint, oSource ) );
}

/**
 * @brief G2HostCache::addKey adds a EndPoint asynchronously to the Host Cache.
 * Locking: YES (asynchronous)
 * @param oHost: the EndPoint - is expected to have been checked against the security manager
 * @param tTimeStamp
 * @param oKeyHost
 * @param nKey
 * @param tNow
 */
void G2HostCache::addKey( const EndPoint& oHost, quint32 tTimeStamp,
						  const EndPoint& oKeyHost, quint32 nKey, quint32 tNow )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "addKey()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pfAddSyncKey.invoke( this, Qt::QueuedConnection,   Q_ARG( EndPoint, oHost ),
						   Q_ARG( quint32, tTimeStamp ), Q_ARG( EndPoint, oKeyHost ),
						   Q_ARG( quint32, nKey ),       Q_ARG( quint32, tNow ) );
}

/**
 * @brief G2HostCache::addAck adds a EndPoint asynchronously to the Host Cache.
 * Locking: YES (asynchronous)
 * @param oHost: the EndPoint - is expected to have been checked against the security manager
 * @param tTimeStamp
 * @param tAck
 * @param tNow
 */
void G2HostCache::addAck( const EndPoint& oHost, quint32 tTimeStamp, quint32 tAck, quint32 tNow )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "addAck()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pfAddSyncAck.invoke( this, Qt::QueuedConnection, Q_ARG( EndPoint, oHost ),
						   Q_ARG( quint32, tTimeStamp ), Q_ARG( quint32, tAck ),
						   Q_ARG( quint32, tNow ) );
}

/**
 * @brief G2HostCache::remove removes a EndPoint from the cache.
 * Locking: YES (asynchronous)
 * @param oHost: the EndPoint
 */
void G2HostCache::remove( const EndPoint& oHost )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "remove(EndPoint&)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pfRemoveSync.invoke( this, Qt::QueuedConnection, Q_ARG( EndPoint, oHost ) );
}

/**
 * @brief G2HostCache::remove removes a HostCacheHost synchronously from the cache.
 * Note: It should never be necessary to remove a host manually from the cache, manual user
 * interaction excepted. Any banned hosts are removed from the cache automatically.
 * Locking: REQUIRED
 * @param pHost: the CHostCacheHost - is set to NULL on completion.
 */
void G2HostCache::remove( SharedG2HostPtr pHost )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "remove(SharedG2HostPtr)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	G2HostCacheIterator it = find( pHost.data() );

	if ( it != m_lHosts.end() )
	{
		erase( it );
	}
}

/**
 * @brief G2HostCache::asyncPruneByQueryAck removes all hosts with outdated acks (older than
 * tNow - quazaaSettings.Gnutella2.QueryHostDeadline).
 * Locking: YES (asynchronous)
 * @param tNow The current time in sec since 1970-01-01 UTC.
 */
void G2HostCache::pruneByQueryAck( const quint32 tNow )
{
	m_pfAsyncPruneByQueryAck.invoke( this, Qt::QueuedConnection, Q_ARG( quint32, tNow ) );
}

void G2HostCache::pruneBySourceID( SourceID nSource )
{
	m_pfAsyncPruneBySourceID.invoke( this, Qt::QueuedConnection, Q_ARG( SourceID, nSource ) );
}

/**
 * @brief G2HostCache::addXTry adds XTry hosts to the cache.
 * Locking: YES (asynchronous)
 * @param sHeader: a string representation of value part of the X-Try header.
 */
void G2HostCache::addXTry(const QString& sHeader )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "addXTry(QString)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pfAsyncAddXTry.invoke( this, Qt::QueuedConnection, Q_ARG( QString, sHeader ) );
}

/**
 * @brief G2HostCache::getXTry allows to generate an XTry header.
 * Locking: YES
 * @return the XTry header
 */
QString G2HostCache::getXTry() const
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "getXTry()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	QString sReturn;
	quint32 nCount = 0;

#ifndef QUAZAA_SETUP_UNIT_TESTS
	ASSUME_LOCK( neighbours.m_pSection );

	const QString sTNow = common::getDateTimeUTC().toString( "yyyy-MM-ddThh:mmZ" );

	// First add neighbours with free slots, to promote faster connections.
	for ( QList<Neighbour*>::iterator it = neighbours.begin(); it != neighbours.end(); ++it )
	{
		Neighbour* pNeighbour = *it;
		if ( pNeighbour->m_nState    == nsConnected &&
			 pNeighbour->m_nProtocol == DiscoveryProtocol::G2 )
		{
			G2Node* pNode = ( G2Node* )pNeighbour;
			if ( pNode->m_nType == G2_HUB &&
				 pNode->m_nLeafMax > 0 &&
				 100 * pNode->m_nLeafCount / pNode->m_nLeafMax < 90 )
			{
				sReturn += pNeighbour->address().toStringWithPort() + " " + sTNow + ",";
				++nCount;
			}
		}
	}
#endif // QUAZAA_SETUP_UNIT_TESTS

	const quint32 nMax = 10;

	// Only take hosts from the Host Cache if we didn't get enough from Neighbours, Note that nMax/2
	// hosts are sufficient when obtained from Neighbours as those are almost 100% connectable,
	// which we cannot assure for hosts within this cache.
	if ( nCount < nMax / 2 && m_nSizeAtomic.load() )
	{
		m_pSection.lock();

		// REMOVE for Quazaa 1.0
		Q_ASSERT( m_lHosts.size() > m_nMaxFailures + 1 ); // at least m_nMaxFailures + 2

		char nFailures = -1;
		for ( G2HostCacheConstIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
		{
			const SharedG2HostPtr& pHost = *it;
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
				{
					break;
				}
			}
			else
			{
				if ( !nFailures )
				{
					break;
				}

				++nFailures;
			}
		}
		m_pSection.unlock();
	}

	if ( sReturn.isEmpty() )
	{
		return QString();
	}
	else
	{
		// remove trailing ',' before returning
		return QString( "X-Try-Hubs: " ) + sReturn.remove( sReturn.size() - 1, 1 );
	}
}

/**
 * @brief G2HostCache::onFailure increases the failure counter of a given EndPoint
 * Locking: YES (asynchronous)
 * @param addr: the EndPoint
 */
void G2HostCache::onFailure( const EndPoint& addr )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "onFailure(EndPoint&)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pfAsyncOnFailure.invoke( this, Qt::QueuedConnection, Q_ARG( EndPoint, addr ) );
}

void G2HostCache::onConnectSuccess( const EndPoint& rHost )
{
	m_pfAsyncUpdateFailures.invoke( this, Qt::QueuedConnection,
									Q_ARG( EndPoint, rHost ), Q_ARG( quint32, 0 ) );
}

///**
// * @brief G2HostCache::updateFailures updates the number of failures of a given host.
// * Locking: YES (asynchronous)
// * @param oAddress: the host
// * @param nFailures: its new failures
// */
//void G2HostCache::updateFailures( const EndPoint& oAddress, const quint32 nFailures )
//{
//	m_pfAsyncUpdateFailures.invoke( this, Qt::QueuedConnection,
//									Q_ARG( EndPoint, oAddress ), Q_ARG( quint32, nFailures ) );
//}

/**
 * @brief G2HostCache::get allows you to access the CHostCacheHost object pertaining to a given
 * EndPoint.
 * Locking: REQUIRED
 * @param oHost: The EndPoint.
 * @return the CHostCacheHost; NULL if the EndPoint has not been found in the cache.
 */
SharedG2HostPtr G2HostCache::get( const EndPoint& oHost ) const
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "get(const EndPoint&)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	G2HostCacheConstIterator it = find( oHost );
	return ( ( it == end() ) ? SharedG2HostPtr() : *it );
}

/**
 * @brief G2HostCache::getConnectable allows to obtain a CHostCacheHost to connect to from the Host
 * Cache.
 * Locking: REQUIRED
 * @param oExcept: a set of Hosts to exempt.
 * @param sCountry: the preferred country. If there are no connectable hosts from the specified
 * country or the country ZZ is specified, country information is ignored.
 * @return a connectable CHostCacheHost.
 */
SharedG2HostPtr G2HostCache::getConnectable( const QSet<SharedG2HostPtr>& oExcept,
											 QString sCountry )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "getConnectable(const QSet<SharedG2HostPtr>&, QString)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	bool bCountry = ( sCountry != "ZZ" );
	static bool bSecondAttempt = false;

	if ( !m_nSizeAtomic.load() )
	{
		return SharedG2HostPtr();
	}

	if ( m_nConnectablesAtomic.load() )
	{
		// First try untested or working hosts, then fall back to failed hosts to increase chances
		// for successful connection
		for ( G2HostCacheConstIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
		{
			const SharedG2HostPtr& pHost = *it;
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
 * @brief G2HostCache::requestHostInfo allows to request hostInfo() signals for all hosts.
 * Qt slot.
 * Locking: YES
 * @return the number of host info signals to expect
 */
quint32 G2HostCache::requestHostInfo()
{
	//qDebug() << "[G2HostCache] requestHostInfo() - Size: " << m_nSizeAtomic.load();

	m_pSection.lock();

	quint32 nHosts = 0;

	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		SharedG2HostPtr pG2Host = *it;

		// if the shared pointer represented by the iterator has been initialized
		if ( pG2Host )
		{
			Q_ASSERT( it == pG2Host->iterator() );
			Q_ASSERT( pG2Host->address().isValid() );

			emit hostInfo( new HostData( qSharedPointerCast<HostCacheHost>( pG2Host ) ) );
			++nHosts;
		}
	}

	// REMOVE for alpha 1
	Q_ASSERT( nHosts == m_nSizeAtomic.load() );

	m_pSection.unlock();

	return nHosts;
}

/**
 * @brief G2HostCache::check allows to verify if a given HostCacheHost is part of the cache. The
 * information is guaranteed to stay valid as long as the mutex is held.
 * Locking: REQUIRED
 * @param pHost: the CHostCacheHost to check.
 * @return true if the host could be found in the cache, false otherwise.
 */
/*bool G2HostCache::check(const SharedHostPtr pHost) const
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "check(const SharedG2HostPtr)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pHost->failures() <= m_nMaxFailures );

	if ( pHost->type() != DiscoveryProtocol::G2 )
		return false;

	G2HostCacheConstIterator it = find( (G2HostCacheHost*)(pHost.data()) );
	return it != getEndIterator();
}*/

/**
 * @brief G2HostCache::clear removes all hosts and frees all memory.
 * Locking: YES
 */
void G2HostCache::clear()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	// Note: Using the system log here might cause problems if that part of the GUI has already been
	// destroyed at call time.
	qDebug() << "[HostCache] clear()";
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	emit clearTriggered();

	m_pSection.lock();

	m_lHosts.clear();

	// Make sure m_pFailures (contains iterators to NULL nodes) stays valid after removal.
	delete[] m_pFailures;
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

	m_pSection.unlock();

	m_nSizeAtomic.store( 0 );
	m_nConnectablesAtomic.store( 0 );
}

/**
 * @brief G2HostCache::save saves the hosts in the cache.
 * Locking: REQUIRED
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void G2HostCache::save( const quint32 tNow ) const
{
#ifndef QUAZAA_SETUP_UNIT_TESTS
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "save(const quint32)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	quint32 nCount = common::securedSaveFile( QuazaaGlobals::DATA_PATH(), "hostcache.dat",
											  Component::HostCache, this,
											  &G2HostCache::writeToFile );
	if ( nCount )
	{
		m_tLastSave = tNow;
		systemLog.postLog( LogSeverity::Debug, Component::HostCache,
						   QObject::tr( "%1 hosts saved." ).arg( nCount ) );
	}
#endif // QUAZAA_SETUP_UNIT_TESTS
}

#ifdef _DEBUG
void G2HostCache::verifyIterators()
{
	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		SharedG2HostPtr pHost = *it;
		if ( pHost )
		{
			Q_ASSERT( pHost->iterator() == it );
		}
	}
}
#endif

/**
  * G2HostCache::writeToFile Helper method for save()
  * Locking: REQUIRED
  * @return the number of hosts written to file
  */
quint32 G2HostCache::writeToFile( const void* const pManager, QFile& oFile )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "writeToFile()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	QDataStream fsFile( &oFile );
	G2HostCache* pHostCache = ( G2HostCache* )pManager;

	ASSUME_LOCK( pHostCache->m_pSection );

	const quint16 nVersion = HOST_CACHE_CODE_VERSION;
	const quint32 nCount   = ( quint32 )pHostCache->m_nSizeAtomic.load();

	fsFile << nVersion;
	fsFile << nCount;

	if ( nCount )
	{
		for ( G2HostCacheConstIterator it = pHostCache->m_lHosts.begin();
			  it != pHostCache->m_lHosts.end(); ++it )
		{
			const SharedG2HostPtr& pHost = *it;
			if ( pHost )
			{
				pHost->save( fsFile );
			}
		}
	}

	return nCount;
}

/**
 * @brief G2HostCache::localAddressChanged needs to be triggered on lokal IP changes.
 * Locking: YES
 */
void G2HostCache::localAddressChanged()
{
#ifndef QUAZAA_SETUP_UNIT_TESTS
	bool bRetry = true;

	// this avoids possible deadlocks with Network.m_pSection
	if ( networkG2.m_pSection.tryLock() )
	{
		if ( m_pSection.tryLock() )
		{
			bRetry          = false;
			m_oLokalAddress = networkG2.localAddress();
			remove( m_oLokalAddress );

			m_pSection.unlock();
		}

		networkG2.m_pSection.unlock();
	}

	if ( bRetry )
	{
		m_pfLocalAddressChanged.invoke( this, Qt::QueuedConnection );
	}
#endif // QUAZAA_SETUP_UNIT_TESTS
}

/**
 * @brief G2HostCache::maintain keeps everything neat and tidy.
 * Locking: YES
 */
void G2HostCache::maintain()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "maintain()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();
	maintainInternal();
	m_pSection.unlock();
}

void G2HostCache::maintainInternal()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "maintainInternal()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

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
		else // e.g. if ( m_nMaxFailures < nNewMaxFailures )
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
	if ( nMaxSize && ( quint32 )m_nSizeAtomic.load() > nMaxSize )
	{
		const quint32 nMax = nMaxSize - nMaxSize / 4;
		quint8 nFailure    = m_nMaxFailures;

		// REMOVE for beta 1
		Q_ASSERT( nMax > 0 );

		// remove 1/4 of all hosts if the cache gets too full - failed and oldest first
		while ( ( quint32 )m_nSizeAtomic.load() > nMax )
		{
			removeWorst( nFailure );
		}

		save( tNow );
	}
	else if ( tNow > m_tLastSave + 600 )
	{
		save( tNow );
	}

	// update m_bConnectable for all hosts
	const quint16 tFailurePenalty = quazaaSettings.Connection.FailurePenalty;
	qint32 tThrottle = ( ( qint32 )quazaaSettings.Gnutella.ConnectThrottle ) - tFailurePenalty;

	int nConnectables = 0;
	bool bConnectable;
	for ( G2HostCacheConstIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		const SharedG2HostPtr& pHost = *it;
		if ( pHost )
		{
			bConnectable = tNow > pHost->lastConnect() + tThrottle;
			pHost->setConnectable( bConnectable );
			nConnectables += bConnectable;
		}
		else // This will be triggered immediately at least once as the first node is always NULL.
		{
			// tThrottle = Gnutella.ConnectThrottle + pHost->failures() * Connection.FailurePenalty
			tThrottle += tFailurePenalty;
		}
	}

	m_nConnectablesAtomic.store( nConnectables );

#if ENABLE_G2_HOST_CACHE_DEBUGGING
	qDebug() << "Number of connectable Hosts: "
			 << QString::number( nConnectables ).toLocal8Bit().data();
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING
}

/**
 * @brief G2HostCache::stopInternal prepares the Host Cache (sub classes) for deletion.
 * Locking: REQUIRED
 */
void G2HostCache::stopInternal()
{
	hostCache.save( common::getTNowUTC() );
}

/**
 * @brief G2HostCache::registerMetaTypesInternal handles registering the necessary meta types of
 * HostCache subclasses.
 */
void G2HostCache::registerMetaTypesInternal()
{
	static int foo    = qRegisterMetaType< SharedG2HostPtr >( "SharedG2HostPtr" );
	static int bar    = qRegisterMetaType< SourceID   >( "SourceID" );
	static int foobar = qRegisterMetaType< InsertList >( "InsertList" );

	Q_UNUSED( foo );
	Q_UNUSED( bar );
	Q_UNUSED( foobar );
}

/**
 * @brief G2HostCache::update updates the timestamp of a EndPoint contained in the Host Cache.
 * Note that the caller needs to make sure the host is actually part of the cache.
 * Locking: REQUIRED
 * @param oHost: the EndPoint
 * @param tTimeStamp: its new timestamp
 * @return the CHostCacheHost pointer pertaining to the updated host.
 */
/*SharedG2HostPtr CHostCache::update(const EndPoint& oHost, const quint32 tTimeStamp)
{
	ASSUME_LOCK( m_pSection );

	HostCacheIterator it = find( oHost );

	if ( it == m_lHosts.end() )
		return SharedG2HostPtr();

	return update( it, tTimeStamp );
}*/

/**
 * @brief G2HostCache::update updates the timestamp of a host reprosented by its HostCacheIterator.
 * Note that the caller needs to make sure the host is actually part of the cache.
 * Locking: REQUIRED
 * @param itHost: must be valid and not the end of the list; invalidated by function call
 * @param tTimeStamp: its timestamp
 * @param nFailures: the new amount of failures. If nFailures > m_nMaxFailures, host will be removed
 * @return the CHostCacheHost pointer pertaining to the updated host.
 */
SharedG2HostPtr G2HostCache::update( G2HostCacheIterator& itHost, quint32 tTimeStamp,
									 SourceID nSource, quint32 nFailures )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "update(G2HostCacheIterator&, const quint32, const quint32)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	Q_ASSERT( itHost !=  m_lHosts.end() );

	SharedG2HostPtr pHost = *itHost;
	SharedG2HostPtr pNew; // (shared) NULL ptr

	// REMOVE for Quazaa 1.0
	Q_ASSERT( pHost->failures() <= m_nMaxFailures );

	if ( nFailures <= m_nMaxFailures )
	{
		// create new host with correct data
		pNew = SharedG2HostPtr( new G2HostCacheHost( *pHost, tTimeStamp, nFailures ) );
	}

	erase( itHost );

#if ENABLE_G2_HOST_CACHE_DEBUGGING
//	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
//					   QString( "pNew->failures(): " ) + QString::number( pNew->failures() ) +
//					   QString( " m_lHosts.size(): " ) + QString::number( m_lHosts.size()  ) +
//					   QString( " m_nMaxFailures: "  ) + QString::number( m_nMaxFailures    ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	if ( pNew )
	{
		// if there isn't a source ID yet...
		if ( !pNew->sourceId() )
		{
			// ...set it
			pNew->setSource( nSource );
		}

		insert( pNew );
	}

	return pNew;
}

/**
 * @brief G2HostCache::addSyncHelper adds synchronously.
 * Locking: REQUIRED
 * @param host: the EndPoint to add
 * @param tTimeStamp: its timestamp
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the EndPoint
 */
SharedG2HostPtr G2HostCache::addSyncHelper( const EndPoint& oHost, quint32 tTimeStamp,
											quint32 tNow, SourceID nSource, quint32 nFailures )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "addSyncHelper()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	Q_ASSERT( oHost.isValid() );
	Q_ASSERT( nFailures <= m_nMaxFailures );

	if ( oHost.isFirewalled() )
	{
		return SharedG2HostPtr();
	}

#ifdef _DEBUG
	// We don't want to call an improperly set up security manager during unit tests.
#ifndef QUAZAA_SETUP_UNIT_TESTS
	// At this point the security check should already have been performed.
	//Q_ASSERT( !securityManager.isDenied( oHost ) );
#endif // QUAZAA_SETUP_UNIT_TESTS
#endif //_DEBUG

	// Don't add own IP to the cache.
	if ( oHost == m_oLokalAddress )
	{
		return SharedG2HostPtr();
	}

	if ( tTimeStamp > tNow )
	{
		tTimeStamp = tNow - 60 ;
	}

	// update existing if such can be found
	G2HostCacheIterator itPrevious = find( oHost );

	if ( itPrevious != m_lHosts.end() )
	{
		SharedG2HostPtr pUpdate = update( itPrevious, tTimeStamp, nSource, nFailures );
		return pUpdate;
	}
	else
	{
		const SourceID nOwnID = m_oIDProvider.aquire();

		// create host, find place in sorted list, insert it there
		SharedG2HostPtr pNew = SharedG2HostPtr( new G2HostCacheHost( oHost, tTimeStamp,
																	 nFailures, nOwnID, nSource ) );
		insert( pNew );

		return pNew;
	}
}

/**
 * @brief G2HostCache::insert inserts a new CHostCacheHost on the correct place into lHosts.
 * Locking: REQUIRED
 * @param pNew: the CHostCacheHost
 * @param tTimeStamp: its timestamp
 * @param lHosts: the list of hosts
 */
void G2HostCache::insert( SharedG2HostPtr pNew )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "insert(SharedG2HostPtr, G2HostCacheIterator&)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pNew );
	Q_ASSERT( pNew->failures() <= m_nMaxFailures );

	G2HostCacheIterator it = m_pFailures[ pNew->failures() ];
	Q_ASSERT( !( *it ) );

	// We got here via a call to m_pFailures, which means the first pointer is a NULL one.
	++it;

	// advance until 1 element past the last element with higher timestamp
	while ( it != m_lHosts.end() && *it && ( *it )->timestamp() > pNew->timestamp() )
	{
		++it;
	}

	Q_ASSERT( it != m_lHosts.begin() && it != m_lHosts.end() );

	// The iterator points now to the element after the last one with
	// (*it)->timestamp() > pNew->timestamp().
	// Insert between the current position and the last one with higher timestamp.
	it = m_lHosts.insert( it, pNew );

	Q_ASSERT( it != m_lHosts.begin() && it != m_lHosts.end() );

	// remember own position in list
	pNew->setIterator( it );

	m_nSizeAtomic.fetchAndAddRelaxed( 1 );

	// Inform GUI
	if ( !m_bLoading )
	{
		emit hostAdded( new HostData( qSharedPointerCast<HostCacheHost>( pNew ) ) );
	}

	// REMOVE for Quazaa 1.0
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
 * @brief G2HostCache::erase removes a host by its iterator and decreases the size and connectable
 * counters.
 * Locking: REQUIRED
 * @param itHost: the iterator - must be valid and not the end of the list.
 * @return the new iterator
 */
G2HostCacheIterator G2HostCache::erase( G2HostCacheIterator& itHost )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	qDebug() << "[HostCache] erase(itHost)";
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	SharedG2HostPtr pHost = *itHost;
	Q_ASSERT( pHost ); // only allow removal of hosts; don't touch m_pFailures[]

	m_nSizeAtomic.fetchAndAddRelaxed( -1 );
	m_nConnectablesAtomic.fetchAndAddRelaxed( -1 * pHost->connectable() );
	Q_ASSERT( m_nConnectablesAtomic.load() >= 0 );

#if ENABLE_G2_HOST_CACHE_DEBUGGING
	qDebug() << QString( "Removed Host by Iterator. Host was connectable: " ) +
			 ( ( ( *itHost )->connectable() ) ? QString( "true" ) : QString( "false" ) ) + "\n" +
			 "      Host Cache Size: "         + QString::number( m_nSizeAtomic.load() ) +
			 "      No of connectable Hosts: " +
			 QString::number( m_nConnectablesAtomic.load() );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	pHost->invalidateIterator();
	G2HostCacheIterator itReturn = m_lHosts.erase( itHost );

	// Inform GUI
	if ( !m_bLoading )
	{
		emit hostRemoved( qSharedPointerCast<HostCacheHost>( pHost ) );
	}

	// REMOVE for Quazaa 1.0
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
 * @brief G2HostCache::pruneOldHosts removes all hosts older than
 * tNow - quazaaSettings.Gnutella2.HostExpire.
 * Locking: REQUIRED
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 */
void G2HostCache::pruneOldHosts( const quint32 tNow )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "pruneOldHosts(const quint32)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	const quint32 tExpire = tNow - quazaaSettings.Gnutella2.HostExpire;

	SharedG2HostPtr pHost;
	G2HostCacheIterator it = m_lHosts.begin();

	// at least m_nMaxFailures + 1
	while ( it != m_lHosts.end() )
	{
		pHost = *it;

		// if an access point or not old enough to remove
		if ( !pHost || pHost->timestamp() > tExpire )
		{
			++it;
		}
		else
		{
			it = erase( it );
		}
	}
}

/**
 * @brief G2HostCache::removeWorst removes the oldest host with a given failure limit - or a smaller
 * one, if there are no hosts with the requested limit.
 * Locking: REQUIRED
 * @param nFailures: the number of failures; this is set to the number of failures of the removed
 * host
 */
void G2HostCache::removeWorst( quint8& nFailures )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "removeWorst()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	// REMOVE for Quazaa 1.0
	Q_ASSERT( !( *m_lHosts.begin() ) );

	if ( nFailures > m_nMaxFailures )
	{
		nFailures = m_nMaxFailures;    // decrease to highest possible value
	}

	G2HostCacheIterator it = m_pFailures[nFailures + 1];
	--it;

	// skip over any failure access points in the way and make sure nFailures is properly decreased
	while ( !( *it ) && it != m_lHosts.begin() )
	{
		--it;
		--nFailures;
	}

	if ( *it ) // if we got a valid host (e.g. if it != m_lHosts.begin() ), remove it
	{
		erase( it );
	}
}

/**
 * @brief G2HostCache::find allows to obtain the list iterator of a given EndPoint.
 * Locking: REQUIRED
 * @param oHost: the given EndPoint
 * @param nFailures: allows the caller to obtain the amount of failures of the host.
 * @return the iterator respectively m_lHosts.end() if not found.
 */
G2HostCacheIterator G2HostCache::find( const EndPoint& oHost )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "find(EndPoint)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	qint16 nFailTest = -1;
	for ( G2HostCacheIterator it = m_lHosts.begin(); it != m_lHosts.end(); ++it )
	{
		if ( *it )
		{
			if ( nFailTest != ( *it )->failures() )
			{
				Q_ASSERT( nFailTest == ( *it )->failures() );
			}

			if ( ( *it )->address() == oHost )
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

G2HostCacheConstIterator G2HostCache::find( const EndPoint& oHost ) const
{
	ASSUME_LOCK( m_pSection );

	// I know this is ugly, but it avoids copy/pasting the previous code. ;)
	G2HostCache* pThis = const_cast<G2HostCache*>( this );
	return ( G2HostCacheConstIterator )( pThis->find( oHost ) );
}

/**
 * @brief G2HostCache::find allows to obtain the list iterator of a given CHostCacheHost.
 * Locking: REQUIRED
 * @param pHost: the given CHostCacheHost
 * @return the iterator respectively m_vlHosts[pHost->failures()].end() if not found.
 */
G2HostCacheIterator G2HostCache::find( const G2HostCacheHost* const pHost )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "find(CHCHost)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	ASSUME_LOCK( m_pSection );

	Q_ASSERT( pHost->iteratorValid() );

	return pHost->iterator();
}

G2HostCacheConstIterator G2HostCache::find( const G2HostCacheHost* const pHost ) const
{
	ASSUME_LOCK( m_pSection );

	// I know this is ugly, but it avoids copy/pasting the previous code. ;)
	G2HostCache* pThis = const_cast<G2HostCache*>( this );
	return ( G2HostCacheConstIterator )( pThis->find( pHost ) );
}

/**
 * @brief G2HostCache::load reads the previously saved hosts from file and adds them to the cache.
 * Locking: YES
 */
void G2HostCache::load()
{
#ifndef QUAZAA_SETUP_UNIT_TESTS
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "load()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();
	m_bLoading = true;

	QString sPath = QuazaaGlobals::DATA_PATH() + "hostcache.dat";

	QFile file( sPath );

	if ( !file.exists() || !file.open( QIODevice::ReadOnly ) )
	{
		m_bLoading = false;
		m_pSection.unlock();
		return;
	}

	QDataStream fsFile( &file );

	quint16 nVersion;
	quint32 nCount;

	fsFile >> nVersion;
	fsFile >> nCount;

	const quint32 tNow   = common::getTNowUTC();

	if ( nVersion == HOST_CACHE_CODE_VERSION ) // else get new hosts from Discovery Manager
	{
		HostCacheHost* pHost;

		while ( nCount )
		{
			pHost = HostCacheHost::load( fsFile, tNow );

			if ( securityManager.isDenied( pHost->address() ) ||
				 pHost->failures() > m_nMaxFailures ||
				 pHost->type() != DiscoveryProtocol::G2 )
			{
				delete pHost;
			}
			else
			{
				insert( SharedG2HostPtr( static_cast<G2HostCacheHost*>( pHost ) ) );
			}

			--nCount; // 1 less to do
		}
	}

	file.close();

	pruneOldHosts( tNow );

	m_bLoading = false;
	m_pSection.unlock();

	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QObject::tr( "Loaded %1 hosts from file: %2"
									).arg( QString::number( m_nSizeAtomic.load() ), sPath ) );

#endif // QUAZAA_SETUP_UNIT_TESTS
}

void G2HostCache::reportSuccess( const EndPoint& rHost )
{
	// TODO: Implement mechanism for reporting a successful connection and base
	//       the rating based on that.
}

/**
 * @brief G2HostCache::addSync adds a given EndPoint synchronously to the cache.
 * Locking: see bLock
 * @param host: the EndPoint
 * @param tTimeStamp: its timestamp
 * @param bLock: does the method need to lock the mutex?
 * @return the CHostCacheHost pointer pertaining to the EndPoint
 */
SharedG2HostPtr G2HostCache::addSync( EndPoint oHost, quint32 tTimeStamp, SourceID nSource )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, "addSync()" );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	const quint32 tNow = common::getTNowUTC();

	m_pSection.lock();

	ASSUME_LOCK( m_pSection );

	SharedG2HostPtr pReturn = addSyncHelper( oHost, tTimeStamp, tNow, nSource );

	m_pSection.unlock();

	return pReturn;
}

SharedG2HostPtr G2HostCache::addSyncSource( EndPoint oHost, quint32 tTimeStamp, EndPoint oSource )
{
//	IDLookup::const_iterator it = m_mSourceIDLookup.find( oSource );
//	const SourceID nID = ( it != m_mSourceIDLookup.end() ) ? (*it).second : 0;

	const SourceID nID = 0;
	return addSync( oHost, tTimeStamp, nID );
}

void G2HostCache::addSyncList( InsertList lHosts, EndPoint oSource )
{
//	IDLookup::const_iterator it = m_mSourceIDLookup.find( oSource );
//	const SourceID nID = ( it != m_mSourceIDLookup.end() ) ? (*it).second : 0;

	const SourceID nID = 0;
	for ( int i = 0, nSize = lHosts.size(); i < nSize; ++i )
	{
		addSync( lHosts[i].first, lHosts[i].second, nID );
	}
}

/**
 * @brief G2HostCache::addSync adds a given EndPoint synchronously to the cache.
 * Locking: YES
 * @param host: the EndPoint
 * @param tTimeStamp: its timestamp
 * @param oKeyHost: the query key host
 * @param nKey: the query key
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the EndPoint
 */
SharedG2HostPtr G2HostCache::addSyncKey( EndPoint host, quint32 tTimeStamp, EndPoint oKeyHost,
										 quint32 nKey, quint32 tNow )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "addSyncKey()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	SharedG2HostPtr pReturn = addSyncHelper( host, tTimeStamp, tNow, 0 );

	if ( pReturn )
	{
		pReturn->setKey( nKey, tNow, oKeyHost );
	}

	m_pSection.unlock();

	return pReturn;
}

/**
 * @brief G2HostCache::addSync adds a given EndPoint synchronously to the cache.
 * Locking: YES
 * @param host: the EndPoint
 * @param tTimeStamp: its timestamp
 * @param tAck: the ack time
 * @param tNow: the current time in sec since 1970-01-01 UTC.
 * @return the CHostCacheHost pointer pertaining to the EndPoint
 */
SharedG2HostPtr G2HostCache::addSyncAck( EndPoint host, quint32 tTimeStamp,
										 quint32 tAck, quint32 tNow )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "addSyncAck()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	SharedG2HostPtr pReturn = addSyncHelper( host, tTimeStamp, tNow, 0 );

	if ( pReturn )
	{
		pReturn->setAck( tAck );
	}

	m_pSection.unlock();

	return pReturn;
}

/**
 * @brief G2HostCache::removeSync removes a EndPoint from the cache.
 * Note: It should never be necessary to remove a host manually from the cache, manual user
 * interaction excepted. Any banned hosts are removed from the cache automatically.
 * Locking: YES
 * @param oHost: the EndPoint
 */
void G2HostCache::removeSync( EndPoint oHost )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "remove(EndPoint)" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();

	G2HostCacheIterator it = find( oHost );

	if ( it != m_lHosts.end() )
	{
		erase( it );
	}

	m_pSection.unlock();
}

/**
 * @brief G2HostCache::startUpInternal helper method for start().
 * Locking: YES
 */
void G2HostCache::startUpInternal()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "startUpInternal()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	// initialize meta methods for for faster asynchronous invoking
	const QMetaObject* pMetaObject = metaObject();
	QByteArray         sNormalizedSignature;
	int                nMethodIndex;

	sNormalizedSignature    = "addSync(EndPoint,quint32,SourceID)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAddSync             = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "addSyncSource(EndPoint,quint32,EndPoint)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAddSyncSource       = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "addSyncList(InsertList,EndPoint)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAddSyncList         = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "addSyncKey(EndPoint,quint32,EndPoint,quint32,quint32)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAddSyncKey          = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "addSyncAck(EndPoint,quint32,quint32,quint32)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAddSyncAck          = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "asyncAddXTry(QString)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncAddXTry        = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "asyncOnFailure(EndPoint)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncOnFailure      = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "asyncPruneByQueryAck(quint32)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncPruneByQueryAck= pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "asyncPruneBySourceID(SourceID)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncPruneBySourceID= pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "asyncUpdateFailures(EndPoint,quint32)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncUpdateFailures = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "localAddressChanged()";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfLocalAddressChanged = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "removeSync(EndPoint)";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfRemoveSync          = pMetaObject->method( nMethodIndex );

	sNormalizedSignature    = "startUpInternal()";
	nMethodIndex            = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfStartUpInternal     = pMetaObject->method( nMethodIndex );

	const QMetaObject* pSecurityMetaObject = securityManager.m_oSanity.metaObject();

	sNormalizedSignature     = "sanityCheckPerformed()";
	nMethodIndex             = pSecurityMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfSanityCheckPerformed = pSecurityMetaObject->method( nMethodIndex );

#ifdef _DEBUG
	// verify HostCache methods
	Q_ASSERT( m_pfAddSync.isValid() );
	Q_ASSERT( m_pfAddSyncSource.isValid() );
	Q_ASSERT( m_pfAddSyncList.isValid() );
	Q_ASSERT( m_pfAddSyncKey.isValid() );
	Q_ASSERT( m_pfAddSyncAck.isValid() );
	Q_ASSERT( m_pfAsyncAddXTry.isValid() );
	Q_ASSERT( m_pfAsyncOnFailure.isValid() );
	Q_ASSERT( m_pfAsyncPruneByQueryAck.isValid() );
	Q_ASSERT( m_pfAsyncPruneBySourceID.isValid() );
	Q_ASSERT( m_pfAsyncUpdateFailures.isValid() );
	Q_ASSERT( m_pfLocalAddressChanged.isValid() );
	Q_ASSERT( m_pfRemoveSync.isValid() );
	Q_ASSERT( m_pfStartUpInternal.isValid() );

	// verify security method
	Q_ASSERT( m_pfSanityCheckPerformed.isValid() );
#endif // _DEBUG

	connect( &securityManager.m_oSanity, &Security::SanityChecker::beginSanityCheck,
			 this, &G2HostCache::sanityCheck, Qt::QueuedConnection );

#ifndef QUAZAA_SETUP_UNIT_TESTS
	connect( &networkG2, &NetworkG2::localAddressChanged,
			 this, &G2HostCache::localAddressChanged );
#endif // QUAZAA_SETUP_UNIT_TESTS

	localAddressChanged();

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

	// inform GUI about new Hosts
	emit loadingFinished();

	signalQueue.push( this, "maintain", 10000, true );
}

/**
 * @brief G2HostCache::asyncAddXTry helper method for addXTry().
 * Locking: YES
 * @param sHeader
 */
void G2HostCache::asyncAddXTry( QString sHeader )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "asyncAddXTry()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	QMutexLocker l( &m_pSection );

	// Example for X-Try-Header:
	// X-Try-Hubs: 86.141.203.14:6346 2010-02-23T16:17Z,91.78.12.117:1164 2010-02-23T16:17Z,89.74.83
	// .103:7972 2010-02-23T16:17Z,93.89.196.113:5649 2010-02-23T16:17Z,24.193.237.252:6346 2010-02-
	// 23T16:17Z,24.226.149.80:6346 2010-02-23T16:17Z,89.142.217.180:9633 2010-02-23T16:17Z,83.219.1
	// 12.111:6346 2010-02-23T16:17Z,201.17.187.205:6346 2010-02-23T16:17Z,213.29.19.41:6346 2010-02
	// -23T16:17Z,78.231.224.180:6346 2010-02-23T16:17Z,213.143.88.92:6346 2010-02-23T16:17Z,77.209.
	// 25.104:1515 2010-02-23T16:17Z,86.220.168.24:59153 2010-02-23T16:17Z,88.183.80.110:6346 2010-0
	// 2-23T16:17Z

	// Note: this method assumes "X-Try-Hubs:" has already been removed from the string

	QStringList lEntries = sHeader.split( "," );

	if ( lEntries.isEmpty() )
	{
		return;
	}

	const quint32 tNow = common::getTNowUTC();
	for ( qint32 i = 0; i < lEntries.size(); ++i )
	{
		// Remove the space as entries might be separated by ", "
		QStringList entry = lEntries.at( i ).trimmed().split( " " );

		if ( entry.size() != 2 )
		{
			continue;
		}

		EndPoint oAddress( entry.at( 0 ) );
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
		addSyncHelper( oAddress, tTimeStamp, tNow, 0 );
	}
}

/**
 * @brief G2HostCache::asyncOnFailure helper method for onFailure().
 * Locking: YES
 * @param addr: the EndPoint with the connection failure
 */
void G2HostCache::asyncOnFailure( EndPoint addr )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "asyncOnFailure()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();
	G2HostCacheIterator itHost = find( addr );

	if ( itHost != m_lHosts.end() )
	{
		SharedG2HostPtr pHost = *itHost;
		erase( itHost );      // remove host with old failure count

		quint8 nFailures = pHost->failures();
		// if failure count may be increased and we have established a connection in the past
		// at least once
		if ( nFailures < m_nMaxFailures && pHost->lastConnectSuccess() )
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

void G2HostCache::asyncPruneByQueryAck( const quint32 tNow )
{
	m_pSection.lock();
	const quint32 tAckExpire = tNow - quazaaSettings.Gnutella2.QueryHostDeadline;
	for ( G2HostCacheIterator itHost = m_lHosts.begin(); itHost != m_lHosts.end(); )
	{
		if ( *itHost && ( *itHost )->ack() && ( *itHost )->ack() < tAckExpire )
		{
			itHost = erase( itHost );
		}
		else
		{
			++itHost;
		}
	}
	m_pSection.unlock();
}

void G2HostCache::asyncPruneBySourceID( const SourceID nSource )
{
	m_pSection.lock();
	for ( G2HostCacheIterator itHost = m_lHosts.begin(); itHost != m_lHosts.end(); )
	{
		if ( *itHost && ( *itHost )->sourceId() == nSource )
		{
			itHost = erase( itHost );
		}
		else
		{
			++itHost;
		}
	}
	m_pSection.unlock();
}

void G2HostCache::asyncUpdateFailures( EndPoint oAddress, quint32 nNewFailures )
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "asyncUpdtFail()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pSection.lock();
	G2HostCacheIterator itHost = find( oAddress );

	if ( itHost != m_lHosts.end() )
	{
		SharedG2HostPtr pHost = *itHost;
		erase( itHost );      // remove host with old failure count

		if ( nNewFailures <= m_nMaxFailures )
		{
			if ( !m_nMaxFailures )
			{
				reportSuccess( oAddress );
			}
			G2HostCacheHost* pNew = new G2HostCacheHost( *pHost, pHost->timestamp(), nNewFailures );
			// insert new host with correct failure count into correct list
			insert( SharedG2HostPtr( pNew ) );
		}
	}

	m_pSection.unlock();
}

void G2HostCache::sanityCheck()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache, QString( "sanityCheck()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

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
			itHost = erase( itHost );
			++nCount;
		}
		else
		{
			++itHost;
		}
	}

	m_pSection.unlock();
	securityManager.m_oSanity.unlock();

	m_pfSanityCheckPerformed.invoke( &securityManager.m_oSanity, Qt::QueuedConnection );

#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::HostCache,
					   QString( "Finished sanity checking. %1 hosts removed." ).arg( nCount ) );
#endif // ENABLE_G2_HOST_CACHE_DEBUGGING
}
