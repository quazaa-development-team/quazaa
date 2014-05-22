/*
** g2hostcachehost.cpp
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

#include "g2hostcachehost.h"

using namespace HostManagement;

/*
template<>
class qLess <CHostCacheHost*>
{
public:
	inline bool operator()(const CHostCacheHost* l, const CHostCacheHost* r) const
	{
		return l->timestamp() > r->timestamp();
	}
};*/

G2HostCacheHost::G2HostCacheHost( const EndPoint& oAddress, quint32 tTimestamp,
								  quint8 nFailures, SourceID nOwnID, SourceID nSourceID ) :
	HostCacheHost( oAddress, nFailures, tTimestamp, 0, nOwnID, nSourceID ),
	m_nQueryKey(   0          ),
	m_oKeyHost(    EndPoint() ),
	m_nKeyTime(    0          ),
	m_tAck(        0          ),
	m_tLastQuery(  0          ),
	m_tRetryAfter( 0          ),
	m_bIteratorValid( false   )
{
	m_nType = DiscoveryProtocol::G2;
}

G2HostCacheHost::G2HostCacheHost( const G2HostCacheHost& oHost, quint32 tTimestamp,
								  quint8 nFailures ) :
	HostCacheHost( oHost.m_oAddress, nFailures, tTimestamp,
				   oHost.m_tLastConnect, oHost.m_nOwnID, oHost.m_nSourceID ),
	m_nQueryKey(   oHost.m_nQueryKey   ),
	m_oKeyHost(    oHost.m_oKeyHost    ),
	m_nKeyTime(    oHost.m_nKeyTime    ),
	m_tAck(        oHost.m_tAck        ),
	m_tLastQuery(  oHost.m_tLastQuery  ),
	m_tRetryAfter( oHost.m_tRetryAfter ),
	m_bIteratorValid( false            )
{
	m_nType = DiscoveryProtocol::G2;
}

G2HostCacheHost::G2HostCacheHost( const G2HostCacheHost& ) :
	HostCacheHost( EndPoint(), 0, 0, 0, 0, 0 )
{
	Q_ASSERT( false );
}

G2HostCacheHost::~G2HostCacheHost()
{
}

bool G2HostCacheHost::canQuery( const quint32 tNow ) const
{
	if ( m_tAck && m_nQueryKey ) // if waiting for an ack, and we have a query key
	{
		return false;
	}

	Q_ASSERT( tNow - m_tTimeStamp >= 0 );
	if ( tNow - m_tTimeStamp > quazaaSettings.Gnutella2.HostCurrent ) // host is not too old
	{
		return false;
	}

	if ( m_tRetryAfter && tNow < m_tRetryAfter ) // honor retry-after
	{
		return false;
	}

	if ( !m_tLastQuery ) // host not yet been queried
	{
		return true;
	}

	Q_ASSERT( tNow - m_tLastQuery >= 0 );
	return tNow - m_tLastQuery > quazaaSettings.Gnutella2.QueryHostThrottle;
}

void G2HostCacheHost::setKey( quint32 nKey, const quint32 tNow, const EndPoint& rHost )
{
	m_tAck      = 0;
	m_nFailures = 0;
	m_nQueryKey = nKey;
	m_nKeyTime  = tNow;
	m_oKeyHost  = rHost;
}

G2HostCacheIterator G2HostCacheHost::iterator() const
{
	// REMOVE for beta 1
	Q_ASSERT( m_bIteratorValid );
	// iterator is supposed to point back to a shared pointer to this instance
	Q_ASSERT( ( *m_iHostCacheIterator ).data() == this );

	return m_iHostCacheIterator;
}

void G2HostCacheHost::setIterator( const G2HostCacheIterator& it )
{
	// iterator is supposed to point back to a shared pointer to this instance
	Q_ASSERT( ( *it ).data() == this );

	m_iHostCacheIterator = it;
	m_bIteratorValid = true;
}

void G2HostCacheHost::invalidateIterator()
{
	m_bIteratorValid = false;
}
