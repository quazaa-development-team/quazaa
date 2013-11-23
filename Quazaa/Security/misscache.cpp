/*
** misscache.cpp
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

#include "misscache.h"
#include "securitymanager.h"

using namespace Security;

MissCache::IPv4Entry MissCache::qHostAddressToIP4(const QHostAddress& oIP, const quint32 tNow)
{
	return IPv4Entry( tNow, oIP.toIPv4Address() );
}

MissCache::IPv6Entry MissCache::qHostAddressToIP6(const QHostAddress& oIP, const quint32 tNow)
{
	return IPv6Entry( tNow, qipv6addrToIPv6Addr( oIP.toIPv6Address() ) );
}

MissCache::IPv6Addr MissCache::qipv6addrToIPv6Addr(const Q_IPV6ADDR& qip6)
{
	IPv6Addr ipReturn;

	ipReturn.data[0] = ((quint64*)&qip6)[0];
	ipReturn.data[1] = ((quint64*)&qip6)[1];

	return ipReturn;
}

MissCache::MissCache() :
	m_tOldestIP4Entry( 0 ),
	m_tOldestIP6Entry( 0 ),
	m_nMaxIPsInCache( 0 ),
	m_bUseMissCache( false ),
	m_bExpiryRequested( false )
{
}

uint MissCache::size(QAbstractSocket::NetworkLayerProtocol eProtocol) const
{
	uint nReturn;

	m_oSection.lock();
	switch( eProtocol )
	{
	case QAbstractSocket::IPv4Protocol:
	{
		nReturn = (uint)m_lsIPv4Cache.size();
	}
	case QAbstractSocket::IPv6Protocol:
	{
		nReturn = (uint)m_lsIPv6Cache.size();
	}
	case QAbstractSocket::UnknownNetworkLayerProtocol:
	{
		nReturn = (uint)m_lsIPv4Cache.size() + (uint)m_lsIPv6Cache.size();
	}
	default:
		qDebug() << QString( "Cannot handle protocol %1 in miss cache." ).arg( eProtocol );
		nReturn = 0;
	}
	m_oSection.unlock();

	return nReturn;
}

void MissCache::insert(const QHostAddress& oIP, const quint32 tNow)
{
	if ( m_bUseMissCache )
	{
		m_oSection.lock();

		switch( oIP.protocol() )
		{
		case QAbstractSocket::IPv4Protocol:
		{
			if ( !m_tOldestIP4Entry )
				m_tOldestIP4Entry = tNow;

			m_lsIPv4Cache.insert( qHostAddressToIP4( oIP, tNow ) );

			if ( !m_bExpiryRequested && m_lsIPv4Cache.size() > m_nMaxIPsInCache )
			{
				requestExpiry();
			}
			break;
		}
		case QAbstractSocket::IPv6Protocol:
		{
			if ( !m_tOldestIP6Entry )
				m_tOldestIP6Entry = tNow;

			m_lsIPv6Cache.insert( qHostAddressToIP6( oIP, tNow ) );

			if ( !m_bExpiryRequested && m_lsIPv6Cache.size() > m_nMaxIPsInCache )
			{
				requestExpiry();
			}
			break;
		}
		default:
			qDebug() << QString( "Cannot handle protocol %1 in miss cache." ).arg( oIP.protocol() );
		}

		m_oSection.unlock();
	}
}

void MissCache::erase(const QHostAddress& oIP)
{
	if ( m_bUseMissCache )
	{
		m_oSection.lock();

		if ( oIP.protocol() == QAbstractSocket::IPv4Protocol )
		{
			m_lsIPv4Cache.erase( qHostAddressToIP4( oIP ) );
		}
		else if ( oIP.protocol() == QAbstractSocket::IPv6Protocol )
		{
			m_lsIPv6Cache.erase( qHostAddressToIP6( oIP ) );
		}
		else
		{
			qDebug() << QString( "Cannot handle protocol %1 in miss cache." ).arg( oIP.protocol() );
		}

		m_oSection.unlock();
	}
}

void MissCache::clear()
{
	m_oSection.lock();

	m_lsIPv4Cache.clear();
	m_lsIPv6Cache.clear();

	m_tOldestIP4Entry = 0;
	m_tOldestIP6Entry = 0;

	m_oSection.unlock();
}

bool MissCache::check(const QHostAddress& oIP) const
{
	bool bReturn = false;

	m_oSection.lock();

	switch( oIP.protocol() )
	{
	case QAbstractSocket::IPv4Protocol:
	{
		bReturn = m_lsIPv4Cache.find( qHostAddressToIP4( oIP ) ) != m_lsIPv4Cache.end();
		break;
	}
	case QAbstractSocket::IPv6Protocol:
	{
		bReturn = m_lsIPv6Cache.find( qHostAddressToIP6( oIP ) ) != m_lsIPv6Cache.end();
		break;
	}
	default:
		qDebug() << QString( "Cannot handle protocol %1 in miss cache." ).arg( oIP.protocol() );
	}

	m_oSection.unlock();

	return bReturn;
}

void MissCache::evaluateUsage()
{
	// Note: The size of the country map is considered to be negligible here, it it will not
	//       contain a large number of rules in 99% of the use cases.
	//       Also, we don't need a lock here as the negative impasct of an erroneous value at this
	//       place is negligible.
	uint nIPMap       = (uint)securityManager.m_lmIPs.size();
	uint nIPRanges    = (uint)securityManager.m_vIPRanges.size();

	m_oSection.lock();

	// ln( nCache ) < ln ( nIPMap ) + ln ( nIPRanges )
	m_nMaxIPsInCache = nIPMap * nIPRanges;

	if ( m_bUseMissCache )
	{
		m_bUseMissCache = m_nMaxIPsInCache > SECURITY_MIN_RULES_TO_ENABLE_CACHE;
		m_oSection.unlock();
	}
	else
	{
		if ( m_nMaxIPsInCache > SECURITY_MIN_RULES_TO_ENABLE_CACHE )
		{
			m_bUseMissCache = true;
			m_oSection.unlock();

			// we didn't use it for some time, so the IPs are probably expired anyway
			clear();
		}
		else
		{
			m_oSection.unlock();
		}
	}
}

void MissCache::requestExpiry()
{
	m_bExpiryRequested = true;
	QMetaObject::invokeMethod( this, "expire", Qt::QueuedConnection );
}

void MissCache::expire()
{
	const quint32 tNow = QDateTime::currentDateTimeUtc().toTime_t();

	m_oSection.lock();

	if ( m_tOldestIP4Entry && m_lsIPv4Cache.size() )
	{
		m_tOldestIP4Entry = ( m_tOldestIP4Entry + tNow ) / 2;

		IPv4MissCache::iterator it = m_lsIPv4Cache.begin();

		while ( it != m_lsIPv4Cache.end() )
		{
			if ( (*it).first < m_tOldestIP4Entry )
				it = m_lsIPv4Cache.erase( it );
		}
	}

	if ( m_tOldestIP6Entry && m_lsIPv6Cache.size() )
	{
		m_tOldestIP6Entry = ( m_tOldestIP6Entry + tNow ) / 2;

		IPv6MissCache::iterator it = m_lsIPv6Cache.begin();

		while ( it != m_lsIPv6Cache.end() )
		{
			if ( (*it).first < m_tOldestIP6Entry )
				it = m_lsIPv6Cache.erase( it );
		}
	}

	m_bExpiryRequested = false;

	m_oSection.unlock();
}

