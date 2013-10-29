/*
** hostcachehost.h
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

#ifndef HOSTCACHEHOST_H
#define HOSTCACHEHOST_H

#include <list>
#include <vector>
#include <QThread>

#include "types.h"
#include "network.h"
#include "quazaasettings.h"

class CG2HostCacheHost;

typedef QSharedPointer<QThread>           TSharedThreadPtr;

typedef std::list<CG2HostCacheHost*>      TG2HostCacheList;
typedef TG2HostCacheList::iterator        TG2HostCacheIterator;
//typedef TG2HostCacheList::const_iterator  TG2HostCacheConstIterator;

typedef std::vector<TG2HostCacheList>     THCLVector;

class CG2HostCacheHost
{
private:
	CEndPoint   m_oAddress;     // Hub address

	// TODO: WTF are we using these timestamps for?
	quint32     m_tTimestamp;   // Kiedy ostatnio widziany

	quint32     m_nQueryKey;    // QK
	CEndPoint   m_oKeyHost;     // host dla ktorego jest QK
	quint32     m_nKeyTime;     // kiedy odebrano OK?

	quint32     m_tAck;         // czas ostatniej operacji wymagajacej potwierdzenia

	quint32     m_tLastQuery;   // kiedy poslano ostatnie zapytanie?
	quint32     m_tRetryAfter;  // kiedy mozna ponowic?
	quint32     m_tLastConnect; // kiedy ostatnio sie polaczylismy?
	quint8      m_nFailures;    // Connection failures in a row.

	bool        m_bConnectable;

	bool               m_bIteratorValid;
	TG2HostCacheIterator m_iHostCacheIterator;

public:
	CG2HostCacheHost(const CEndPoint& oAddress, const quint32 tTimestamp, const quint8 nFailures);
	CG2HostCacheHost(const CG2HostCacheHost& oHost, const quint32 tTimestamp, const quint8 nFailures);

private:
	CG2HostCacheHost(const CG2HostCacheHost&) {} // make sure to avoid unintentional copies

public:
	~CG2HostCacheHost();

	bool canQuery(const quint32 tNow = common::getTNowUTC());
	void setKey(quint32 nKey, const quint32 tNow = common::getTNowUTC(), CEndPoint* pHost = NULL);

	inline CEndPoint address()       const { return m_oAddress;       }
	inline quint32   timestamp()     const { return m_tTimestamp;     }
	inline quint32   queryKey()      const { return m_nQueryKey;      }
	inline CEndPoint keyHost()       const { return m_oKeyHost;       }
	inline quint32   keyTime()       const { return m_nKeyTime;       }
	inline quint32   ack()           const { return m_tAck;           }
	inline quint32   lastQuery()     const { return m_tLastQuery;     }
	inline quint32   retryAfter()    const { return m_tRetryAfter;    }
	inline quint32   lastConnect()   const { return m_tLastConnect;   }
	inline quint8    failures()      const { return m_nFailures;      }
	inline bool      connectable()   const { return m_bConnectable;   }
	inline bool      iteratorValid() const { return m_bIteratorValid; }

	inline void      setAddress(     CEndPoint oAddress     ) { m_oAddress     = oAddress;     }
// There is no setTimestamp() as the timestamp needs to be maintained by the Host Cache.
	inline void      setQueryKey(    quint32   nQueryKey    ) { m_nQueryKey    = nQueryKey;    }
	inline void      setKeyHost(     CEndPoint nKeyHost     ) { m_oKeyHost     = nKeyHost;     }
	inline void      setKeyTime(     quint32   nKeyTime     ) { m_nKeyTime     = nKeyTime;     }
	inline void      setAck(         quint32   tAck         ) { m_tAck         = tAck;         }
	inline void      setLastQuery(   quint32   tLastQuery   ) { m_tLastQuery   = tLastQuery;   }
	inline void      setRetryAfter(  quint32   tRetryAfter  ) { m_tRetryAfter  = tRetryAfter;  }
	inline void      setLastConnect( quint32   tLastConnect ) { m_tLastConnect = tLastConnect; }
// There is no setFailures() as the number of failures needs to be maintained by the Host Cache.
	inline void      setConnectable( bool      bConnectable ) { m_bConnectable = bConnectable; }

	TG2HostCacheIterator iterator() const;
	void setIterator(const TG2HostCacheIterator& it);
};

#endif // HOSTCACHEHOST_H
