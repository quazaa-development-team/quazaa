/*
** misscache.h
**
** Copyright © Quazaa Development Team, 2009-2013.
** This file is part of the Quazaa Security Library (quazaa.sourceforge.net)
**
** The Quazaa Security Library is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** The Quazaa Security Library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with the Quazaa Security Library; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef MISSCACHE_H
#define MISSCACHE_H

#include <set>
#include <QHostAddress>

namespace Security
{

class MissCache : QObject
{
	Q_OBJECT

private:
	typedef quint32                       IPv4Addr;

	typedef struct
	{
		quint64 data[2];
	} IPv6Addr;

	typedef std::pair< quint32, IPv4Addr > IPv4Entry; // tInsert, IPv4
	typedef std::pair< quint32, IPv6Addr > IPv6Entry; // tInsert, IPv6

	struct IPv4EntrySort
	{
		bool operator()(const IPv4Entry& first, const IPv4Entry& second) const
		{
			return first.second < second.second;
		}
	};

	struct IPv6EntrySort
	{
		bool operator()(const IPv6Entry& first, const IPv6Entry& second) const
		{
			// second half of IPv6 is to be expected to be more diverse than first half
			return first.second.data[1] < second.second.data[1];
		}
	};

	typedef std::set< IPv4Entry, IPv4EntrySort > IPv4MissCache;
	typedef std::set< IPv6Entry, IPv6EntrySort > IPv6MissCache;

	mutable QMutex  m_oSection;

	IPv4MissCache   m_lsIPv4Cache;
	IPv6MissCache   m_lsIPv6Cache;

	quint32         m_tOldestIP4Entry;
	quint32         m_tOldestIP6Entry;

	quint32         m_nMaxIPsInCache;
	bool            m_bUseMissCache;

	bool            m_bExpiryRequested;

	/**
	 * @brief qHostAddressToIP4 converts an IP and a time to an IPv4Entry
	 * @param oIP : the IP
	 * @param tNow : the time
	 * @return the IPv4Entry
	 */
	static IPv4Entry qHostAddressToIP4(const QHostAddress& oIP, const quint32 tNow = 0);

	/**
	 * @brief qHostAddressToIP6 converts an IP and a time to an IPv6Entry
	 * @param oIP : the IP
	 * @param tNow : the time
	 * @return the IPv6Entry
	 */
	static IPv6Entry qHostAddressToIP6(const QHostAddress& oIP, const quint32 tNow = 0);

	/**
	 * @brief qipv6addrToIPv6Addr onverts an Q_IPV6ADDR struct to an IPv6Addr
	 * @param qip6 : the Q_IPV6ADDR struct
	 * @return the IPv6Addr
	 */
	static IPv6Addr qipv6addrToIPv6Addr(const Q_IPV6ADDR& qip6);

public:
	/**
	 * @brief MissCache creates an empty miss cache.
	 */
	MissCache();

	/**
	 * @brief size allows accessing the size of the cache
	 * @param eProtocol allows to specify the protocol. Defaults to
	 * QAbstractSocket::UnknownNetworkLayerProtocol which will return the total number of IPv4 and
	 * IPv6 entries in the cache.
	 * @return the number of IPs in the cache
	 */
	uint size(QAbstractSocket::NetworkLayerProtocol eProtocol =
			  QAbstractSocket::UnknownNetworkLayerProtocol) const;

	/**
	 * @brief insert allows to insert an IP into the cache
	 * @param oIP : the IP
	 * @param tNow : the current time
	 */
	void insert(const QHostAddress& oIP, const quint32 tNow);

	/**
	 * @brief erase removes a specified IP from the cache
	 * @param oIP : the IP
	 */
	void erase(const QHostAddress& oIP);

	/**
	 * @brief clear removes all IPs from the cache
	 */
	void clear();

	/**
	 * @brief check allows to test whether a specified IP is currently part of the cache.
	 * @param oIP : the IP
	 * @return true if the IP could be found; false otherwise
	 */
	bool check(const QHostAddress& oIP) const;

	/**
	 * @brief evaluateUsage recalculates the maximal cache size etc.
	 */
	void evaluateUsage();				// determines whether it is logical to use the cache or not

private slots:
	/**
	 * @brief expire removes all IPs added prior than (tNow - tOldestIP)/2
	 */
	void expire();

private:
	/**
	 * @brief requestExpiry allows to request a delayed expiry
	 */
	void requestExpiry();
};

}

#endif // MISSCACHE_H
