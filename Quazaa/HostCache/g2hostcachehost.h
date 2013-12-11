/*
** g2hostcachehost.h
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

#ifndef G2HOSTCACHEHOST_H
#define G2HOSTCACHEHOST_H

#include <list>
#include <vector>

#include "hostcachehost.h"

class G2HostCacheHost;

typedef QSharedPointer<G2HostCacheHost> SharedG2HostPtr;

typedef std::list<SharedG2HostPtr>      G2HostCacheList;
typedef G2HostCacheList::iterator       G2HostCacheIterator;

class G2HostCacheHost : public HostCacheHost
{
private:
	quint32     m_nQueryKey;    // QK
	CEndPoint   m_oKeyHost;     // host dla ktorego jest QK
	quint32     m_nKeyTime;     // kiedy odebrano OK?

	quint32     m_tAck;         // czas ostatniej operacji wymagajacej potwierdzenia

	quint32     m_tLastQuery;   // kiedy poslano ostatnie zapytanie?
	quint32     m_tRetryAfter;  // kiedy mozna ponowic?

	bool                m_bIteratorValid;
	G2HostCacheIterator m_iHostCacheIterator;

public:
	G2HostCacheHost(const CEndPoint& oAddress, const quint32 tTimestamp, const quint8 nFailures);
	G2HostCacheHost(const G2HostCacheHost& oHost, const quint32 tTimestamp, const quint8 nFailures);

public:
	~G2HostCacheHost();

	bool canQuery(const quint32 tNow = common::getTNowUTC()) const;
	void setKey(quint32 nKey, const quint32 tNow = common::getTNowUTC(), CEndPoint* pHost = NULL);

	inline quint32   queryKey()      const { return m_nQueryKey;      }
	inline CEndPoint keyHost()       const { return m_oKeyHost;       }
	inline quint32   keyTime()       const { return m_nKeyTime;       }
	inline quint32   ack()           const { return m_tAck;           }
	inline quint32   lastQuery()     const { return m_tLastQuery;     }
	inline quint32   retryAfter()    const { return m_tRetryAfter;    }
	inline bool      iteratorValid() const { return m_bIteratorValid; }

	inline void      setQueryKey(    quint32   nQueryKey    ) { m_nQueryKey    = nQueryKey;    }
	inline void      setKeyHost(     CEndPoint nKeyHost     ) { m_oKeyHost     = nKeyHost;     }
	inline void      setKeyTime(     quint32   nKeyTime     ) { m_nKeyTime     = nKeyTime;     }
	inline void      setAck(         quint32   tAck         ) { m_tAck         = tAck;         }
	inline void      setLastQuery(   quint32   tLastQuery   ) { m_tLastQuery   = tLastQuery;   }
	inline void      setRetryAfter(  quint32   tRetryAfter  ) { m_tRetryAfter  = tRetryAfter;  }

	G2HostCacheIterator iterator() const;
	void setIterator(const G2HostCacheIterator& it);
};

#endif // G2HOSTCACHEHOST_H
