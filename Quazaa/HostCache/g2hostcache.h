/*
** g2hostcache.h
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

#ifndef G2HOSTCACHE_H
#define G2HOSTCACHE_H

#include <QMetaMethod>

#include <unordered_map>

#include "hostcache.h"

#define ENABLE_G2_HOST_CACHE_DEBUGGING    0
#define ENABLE_G2_HOST_CACHE_BENCHMARKING 0

// Increment this if there have been made changes to the way of storing Host Cache Hosts.
#define HOST_CACHE_CODE_VERSION	9
// History:
// 4 - Initial implementation.
// 6 - Fixed Hosts having an early date and changed time storage from QDateTime to quint32.
// 7 - Changed host failure counter from quint32 to quint8.
// 8 - Added Polymorphism and type indicator (m_nType)
// 9 - Added host parent and source ID

// TODO: test changes of m_nMaxFailures under load
// TODO: Test: search manager must request query keys directly from other nodes
//             (use a random search string)

class QFile;

namespace HostManagement
{
class G2HostCacheHost;
}

/**
 * @brief SharedG2HostPtr A shared pointer to a G2HostCacheHost.
 */
typedef QSharedPointer<HostManagement::G2HostCacheHost> SharedG2HostPtr;

namespace HostManagement
{
typedef std::list<SharedG2HostPtr>      G2HostCacheList;
typedef G2HostCacheList::iterator       G2HostCacheIterator;
}

/**
 * @brief G2HostCacheConstIterator In iterator allowing to iterate over the G2HostCache's internal
 * storage container.
 */
typedef HostManagement::G2HostCacheList::const_iterator G2HostCacheConstIterator;

namespace HostManagement
{

typedef quint32 SourceID;
typedef QList< QPair<EndPoint, quint32> > InsertList;
// typedef std::unordered_map<EndPoint, SourceID> IDLookup;

class G2HostCache : public HostCache
{
	Q_OBJECT

private:
	// allows access to 0 .. m_nMaxFailures + 1
	G2HostCacheIterator*    m_pFailures; // = new G2HostCacheIterator[m_nMaxFailures + 2];
	G2HostCacheList         m_lHosts;

#if ENABLE_G2_HOST_CACHE_BENCHMARKING
	QAtomicInt              m_nLockWaitTime;
	QAtomicInt              m_nWorkTime;
#endif // ENABLE_G2_HOST_CACHE_BENCHMARKING

	EndPoint                m_oLokalAddress;

	bool                    m_bLoading;

	// QMetaMethod objects for faster asynchronous method invokation
	QMetaMethod m_pfAddSync;
	QMetaMethod m_pfAddSyncSource;
	QMetaMethod m_pfAddSyncList;
	QMetaMethod m_pfAddSyncKey;
	QMetaMethod m_pfAddSyncAck;
	QMetaMethod m_pfAsyncAddXTry;
	QMetaMethod m_pfAsyncOnFailure;
	QMetaMethod m_pfAsyncPruneByQueryAck;
	QMetaMethod m_pfAsyncPruneBySourceID;
	QMetaMethod m_pfAsyncUpdateFailures;
	QMetaMethod m_pfLocalAddressChanged;
	QMetaMethod m_pfRemoveSync;
	QMetaMethod m_pfStartUpInternal;

	QMetaMethod m_pfSanityCheckPerformed;

public:
	G2HostCache();
	~G2HostCache();

	void add(    const EndPoint& oHost, quint32 tTimeStamp, SourceID nSource = 0 );
	void add(    const EndPoint& oHost, quint32 tTimeStamp, const EndPoint& oSource );
	void add(    const InsertList& lHosts, const EndPoint& oSource );
	void addKey( const EndPoint& oHost, quint32 tTimeStamp,
				 const EndPoint& oKeyHost, quint32 nKey, quint32 tNow );
	void addAck( const EndPoint& oHost, quint32 tTimeStamp, quint32 tAck, quint32 tNow );

	void remove( const EndPoint& oHost );
	void remove( SharedG2HostPtr pHost );

	void pruneByQueryAck( const quint32 tNow );
	void pruneBySourceID( SourceID nSource );

	void addXTry( const QString& sHeader );
	QString getXTry() const;

	void onFailure( const EndPoint& addr );
	void onConnectSuccess( const EndPoint& rHost );
//	void updateFailures( const EndPoint& oAddress, const quint32 nFailures );

	SharedG2HostPtr get( const EndPoint& oHost ) const;
	SharedG2HostPtr getConnectable( const QSet<SharedG2HostPtr>& oExcept = QSet<SharedG2HostPtr>(),
									QString sCountry = QString( "ZZ" ) );

	quint32 requestHostInfo();

	//bool check(const SharedHostPtr pHost) const;

	void clear();
	void save( const quint32 tNow ) const;

	// These allow access to the list without the possibility of being able to edit the list itself
	inline G2HostCacheConstIterator begin() const
	{
		return m_lHosts.begin();
	}
	inline G2HostCacheConstIterator end() const
	{
		return m_lHosts.end();
	}

#ifdef _DEBUG
	// REMOVE for Quazaa 1.0
	void verifyIterators();
#endif

	static quint32 writeToFile( const void* const pManager, QFile& oFile );

public slots:
	void localAddressChanged();
	void maintain();

private:
	void maintainInternal();

	/**
	 * @brief stopInternal prepares the Host Cache (sub classes) for deletion.
	 * Locking: REQUIRED
	 */
	void stopInternal();
	void registerMetaTypesInternal();

//	SharedG2HostPtr update( const EndPoint& oHost,       const quint32 tTimeStamp );
	SharedG2HostPtr update( G2HostCacheIterator& itHost, quint32 tTimeStamp,
							SourceID nSource, quint32 nFailures = 0 );

	SharedG2HostPtr addSyncHelper( const EndPoint& oHost, quint32 tTimeStamp,
								   quint32 tNow, SourceID nSource, quint32 nNewFailures = 0 );

	void insert( SharedG2HostPtr pNew );
	G2HostCacheIterator erase( G2HostCacheIterator& itHost );

	void pruneOldHosts( const quint32 tNow );
	void removeWorst( quint8& nFailures );

	G2HostCacheIterator      find( const EndPoint& oHost );
	G2HostCacheConstIterator find( const EndPoint& oHost ) const;

	G2HostCacheIterator      find( const G2HostCacheHost* const pHost );
	G2HostCacheConstIterator find( const G2HostCacheHost* const pHost ) const;

	void load();

	/**
	 * @brief reportSuccess handles reporting a successful connection to the source of rHost.
	 * <br><b>Locking: REQUIRED</b>
	 *
	 * @param rHost  The host that has been successfully connected to.
	 */
	void reportSuccess( const EndPoint& rHost );

private slots:
	SharedG2HostPtr addSync( EndPoint oHost, quint32 tTimeStamp, SourceID nSource );
	SharedG2HostPtr addSyncSource( EndPoint oHost, quint32 tTimeStamp, EndPoint oSource );
	void addSyncList( InsertList lHosts, EndPoint oSource );
	SharedG2HostPtr addSyncKey( EndPoint host, quint32 tTimeStamp,
								EndPoint oKeyHost, quint32 nKey, quint32 tNow );
	SharedG2HostPtr addSyncAck( EndPoint host, quint32 tTimeStamp,
								quint32 tAck, quint32 tNow );

	void removeSync( EndPoint oHost );

	void startUpInternal();

	void asyncAddXTry( QString sHeader );
	void asyncOnFailure( EndPoint addr );

	/**
	 * @brief asyncPruneByQueryAck removes all hosts with outdated acks (older than
	 * tNow - quazaaSettings.Gnutella2.QueryHostDeadline).
	 * <br><b>Locking: YES</b>
	 *
	 * @param tNow  The current time in sec since 1970-01-01 0:00:00 UTC.
	 */
	void asyncPruneByQueryAck( const quint32 tNow );

	/**
	 * @brief asyncPruneBySourceID removes all hosts with the SourceID nSource from the cache.
	 * <br><b>Locking: YES</b>
	 *
	 * @param nSource  The SourceID to expunge from the cache.
	 */
	void asyncPruneBySourceID( const SourceID nSource );

	/**
	 * @brief asyncUpdateFailures is a helper method for updateFailures().
	 * <br><b>Locking: YES</b>
	 *
	 * @param oAddress   The address for which to update the failure count.
	 * @param nFailures  The new failure count of oAddress.
	 */
	void asyncUpdateFailures( EndPoint oAddress, quint32 nNewFailures );

	/**
	 * @brief sanityCheck performs the sanity check after a new Security::Rule has been added to the
	 * Security::Manager.
	 * <br><b>Locking: YES</b>
	 */
	void sanityCheck();
};

} // namespace HostManagement

extern HostManagement::G2HostCache hostCache;

#endif // G2HOSTCACHE_H
