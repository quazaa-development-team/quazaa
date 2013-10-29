/*
** hostcache.h
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

#ifndef HOSTCACHE_H
#define HOSTCACHE_H

#include <QMutex>
#include <QAtomicInt>

#include "hostcachehost.h"

#define ENABLE_HOST_CACHE_DEBUGGING    0
#define ENABLE_HOST_CACHE_BENCHMARKING 0

// Increment this if there have been made changes to the way of storing Host Cache Hosts.
#define HOST_CACHE_CODE_VERSION	7
// History:
// 4 - Initial implementation.
// 6 - Fixed Hosts having an early date and changed time storage from QDateTime to quint32.
// 7 - Changed host failure counter from quint32 to quint8.

// TODO: purge QMetaObject::invokeMethod
// TODO: in canQuery there are calls to .isNull
// TODO: things to test
//  12:58:39 brov: 1. does it use hosts from host cache b4 GWC query succeeds
//  12:59:25 brov: 2. search manager must request query keys directly from other nodes (use a random search string)

class QFile;

class CG2HostCache : public QObject
{
	Q_OBJECT

public:
	// allows access to 0 .. m_nMaxFailures + 1
	TG2HostCacheIterator*   m_pFailures; // = new TG2HostCacheIterator[m_nMaxFailures + 2];
	TG2HostCacheList        m_lHosts;

	//THCLVector              m_vlHosts; // vector of (nFailures, QLinkedList);
									   // QLinkedList sorted by timestamp (descending)

	CEndPoint               m_oLokalAddress;

#if ENABLE_HOST_CACHE_BENCHMARKING
	QAtomicInt              m_nLockWaitTime;
	QAtomicInt              m_nWorkTime;
#endif

	mutable QMutex          m_pSection;
	mutable quint32         m_tLastSave; //TODO: use qatomicint

	quint8                  m_nMaxFailures;
	QAtomicInt              m_nSizeAtomic;

	// Thread used by the Host Cache
	TSharedThreadPtr        m_pHostCacheDiscoveryThread;

public:
	CG2HostCache();
	~CG2HostCache();

	void start();

	void add(const CEndPoint host, const quint32 tTimeStamp);
	void addKey(const CEndPoint host, const quint32 tTimeStamp,
				CEndPoint* pKeyHost, const quint32 nKey, const quint32 tNow);
	void addAck(const CEndPoint host, const quint32 tTimeStamp,
				const quint32 tAck, const quint32 tNow);

	inline CG2HostCacheHost* get(const CEndPoint& oHost);
	inline bool check(const CG2HostCacheHost* const pHost);

	void updateFailures(const CEndPoint& oAddress, const quint32 nFailures);

private: // remove this private if this is ever required...
//	CHostCacheHost* update(const CEndPoint& oHost,     const quint32 tTimeStamp);
	CG2HostCacheHost* update(TG2HostCacheIterator& itHost, const quint32 tTimeStamp,
							 const quint32 nFailures);

public:
	void remove(const CEndPoint& oHost);
	void remove(CG2HostCacheHost*& pHost);

	void addXTry(QString sHeader);
	QString getXTry() const;

	void onFailure(const CEndPoint& addr);
	CG2HostCacheHost* getConnectable(const QSet<CG2HostCacheHost*>& oExcept = QSet<CG2HostCacheHost*>(),
								   QString sCountry = QString("ZZ"));
	bool hasConnectable();

	void clear();
	void save(const quint32 tNow) const;

private:
	void pruneOldHosts(const quint32 tNow);

public:
	void pruneByQueryAck(const quint32 tNow);

	static quint32 writeToFile(const void* const pManager, QFile& oFile);

	inline quint32 count() const;
	inline bool isEmpty() const;

public slots:
	void localAddressChanged();

	CG2HostCacheHost* addSync(CEndPoint host, quint32 tTimeStamp, bool bLock);
	CG2HostCacheHost* addSyncKey(CEndPoint host, quint32 tTimeStamp, CEndPoint* pKeyHost,
							   const quint32 nKey, const quint32 tNow, bool bLock);
	CG2HostCacheHost* addSyncAck(CEndPoint host, quint32 tTimeStamp, const quint32 tAck,
							   const quint32 tNow, bool bLock);

	void removeSync(CEndPoint oHost);

	void sanityCheck();
	void maintain();

private:
	CG2HostCacheHost* addSyncHelper(const CEndPoint& host, quint32 tTimeStamp, const quint32 tNow,
									quint32 nNewFailures = 0);
	void insert(CG2HostCacheHost* pNew, TG2HostCacheIterator& it);

	TG2HostCacheIterator remove(TG2HostCacheIterator& itHost);
	void removeWorst(quint8& nFailures);

	TG2HostCacheIterator      find(const CEndPoint& oHost);
	TG2HostCacheIterator      find(const CG2HostCacheHost* const pHost);
//	THostCacheConstIterator find(const CHostCacheHost* const pHost) const;

	void load();

private slots:
	void asyncStartUpHelper();
	void asyncUpdateFailures(CEndPoint oAddress, quint32 nNewFailures);
	void asyncAddXTry(QString sHeader);
	void asyncOnFailure(CEndPoint addr);
};

/**
 * @brief CHostCache::get allows you to access the CHostCacheHost object pertaining to a given CEndPoint.
 * Locking: REQUIRED
 * @param oHost: The CEndPoint.
 * @return the CHostCacheHost; NULL if the CEndPoint has not been found in the cache.
 */
CG2HostCacheHost* CG2HostCache::get(const CEndPoint& oHost)
{
	ASSUME_LOCK( m_pSection );

	TG2HostCacheIterator it = find( oHost );
	return ( ( it == m_lHosts.end() ) ? NULL : *it );
}

/**
 * @brief CHostCache::check allows to verify if a given CHostCacheHost is part of the cache. The
 * information is guaranteed to stay valid as long as the mutex is held.
 * Locking: REQUIRED
 * @param pHost: the CHostCacheHost to check.
 * @return true if the host could be found in the cache, false otherwise.
 */
bool CG2HostCache::check(const CG2HostCacheHost* const pHost)
{
	ASSUME_LOCK( m_pSection );
	Q_ASSERT( pHost->failures() <= m_nMaxFailures );

	TG2HostCacheIterator it = find( pHost );
	return it != m_lHosts.end();
}

/**
 * @brief CHostCache::count allows access to the size of the Host Cache.
 * Locking: YES
 * @return the number of hosts in the cache.
 */
quint32 CG2HostCache::count() const
{
	return m_nSizeAtomic.loadAcquire();
}

/**
 * @brief CHostCache::isEmpty allows to check whether the Host Cache is empty.
 * Locking: YES
 * @return true if the cache contains no hosts; false otherwise.
 */
bool CG2HostCache::isEmpty() const
{
	return !m_nSizeAtomic.loadAcquire();
}

extern CG2HostCache hostCache;

#endif // HOSTCACHE_H
