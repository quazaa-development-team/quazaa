/*
** hostcachehost.h
**
** Copyright © Quazaa Development Team, 2013-2013.
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

#include "types.h"
#include "network.h"
#include "quazaasettings.h"

class HostCacheHost
{
protected:
	Protocol    m_nType;        // The network type

	CEndPoint   m_oAddress;     // Hub address

	quint32     m_tTimestamp;   // Kiedy ostatnio widziany

	quint32     m_nID;          // GUI ID

	quint32     m_tLastConnect; // kiedy ostatnio sie polaczylismy?
	quint8      m_nFailures;    // Connection failures in a row.

	bool        m_bConnectable;

private:
	// mechanism for allocating GUI IDs
	static quint32              m_nLastID;
	static QMutex               m_oIDLock;
	static std::set<quint32>    m_idCheck;

public:
	HostCacheHost(const CEndPoint& oAddress, const quint8 nFailures,
				  const quint32 tTimestamp,  const quint32 tLastConnect);
	virtual ~HostCacheHost();

private:
	HostCacheHost(const HostCacheHost&) {} // make sure to avoid unintentional copies

public:
	virtual bool canQuery(const quint32) const { return true; }

	inline Protocol  type()          const { return m_nType;          }
	inline CEndPoint address()       const { return m_oAddress;       }
	inline quint32   timestamp()     const { return m_tTimestamp;     }
	inline quint32   id()            const { return m_nID;            }
	inline quint32   lastConnect()   const { return m_tLastConnect;   }
	inline quint8    failures()      const { return m_nFailures;      }
	inline bool      connectable()   const { return m_bConnectable;   }

	// There is no setTimestamp() as the timestamp needs to be maintained by the Host Cache.
	// The same goes for failures and the GUI ID.
	inline void      setAddress(     CEndPoint oAddress     ) { m_oAddress     = oAddress;     }
	inline void      setLastConnect( quint32   tLastConnect ) { m_tLastConnect = tLastConnect; }
	inline void      setConnectable( bool      bConnectable ) { m_bConnectable = bConnectable; }
};

#endif // HOSTCACHEHOST_H
