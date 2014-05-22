/*
** hostcachehost.h
**
** Copyright © Quazaa Development Team, 2013-2014.
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

#include "quazaasettings.h"
#include "idprovider.h"

#include "endpoint.h"
#include "networktype.h"

#ifndef QUAZAA_SETUP_UNIT_TESTS
#include "network.h"
#endif

namespace HostManagement
{

/**
 * @brief SourceID: ID type used to relate a HostCacheHost with the source it has been obtained
 * from. All IDs are positive. 0 indicates an invalid ID.
 */
typedef quint32 SourceID;

class HostCacheHost
{
protected:
	DiscoveryProtocol::Protocol    m_nType;        // The network type

	EndPoint    m_oAddress;     // Hub address

	quint32     m_tTimeStamp;   // Kiedy ostatnio widziany

//	quint32     m_nID;          // GUI ID

	SourceID    m_nOwnID;
	SourceID    m_nSourceID;

	quint32     m_tLastConnect; // kiedy ostatnio sie polaczylismy?
	quint32     m_tLastConnectionEstablished;

	quint8      m_nFailures;    // Connection failures in a row.

	bool        m_bConnectable;

protected:
	// mechanism for allocating GUI IDs
//	static IDProvider<quint32>  m_oIDProvider;
	static bool                 m_bShutDownFlag;

public:
	HostCacheHost( const EndPoint& oAddress, quint8 nFailures, quint32 tTimestamp,
				   quint32 tLastConnect, SourceID nOwnID, SourceID nSourceID );
	virtual ~HostCacheHost();

private:
	HostCacheHost( const HostCacheHost& )
	{
		Q_ASSERT( false );    // avoid unintentional copies
	}

public:
	virtual bool canQuery( const quint32 ) const
	{
		return true;
	}

	static HostCacheHost* load( QDataStream& fsFile, quint32 tNow );
	virtual void save( QDataStream& fsFile );

	inline DiscoveryProtocol::Protocol type() const
	{
		return m_nType;
	}

	inline EndPoint address()       const
	{
		return m_oAddress;
	}
	inline quint32  timestamp()     const
	{
		return m_tTimeStamp;
	}
//	inline quint32  id()            const
//	{
//		return m_nID;
//	}
	inline SourceID ownId()         const
	{
		return m_nOwnID;
	}
	inline SourceID sourceId()      const
	{
		return m_nSourceID;
	}
	inline void setSource( SourceID nSource )
	{
		m_nSourceID = nSource;
	}
	inline quint32  lastConnect()   const
	{
		return m_tLastConnect;
	}
	inline quint32  lastConnectSuccess() const
	{
		return m_tLastConnectionEstablished;
	}
	inline quint8   failures()      const
	{
		return m_nFailures;
	}
	inline bool     connectable()   const
	{
		return m_bConnectable;
	}

	// There is no setTimestamp() as the timestamp needs to be maintained by the Host Cache.
	// The same goes for failures and the GUI ID.
	inline void      setAddress(     EndPoint oAddress     )
	{
		m_oAddress     = oAddress;
	}
	inline void      setLastConnect( quint32   tLastConnect )
	{
		m_tLastConnect = tLastConnect;
	}

	// Important: setConnectable() is only to be used by maintain(), else the connectables counter
	// is messed up.
	inline void      setConnectable( bool      bConnectable )
	{
		m_bConnectable = bConnectable;
	}

	inline static void setShutDownFlag()
	{
		m_bShutDownFlag = true;
	}
};

} // namespace HostManagement

typedef QSharedPointer<HostManagement::HostCacheHost> SharedHostPtr;

#include <QIcon>
#include <QAbstractTableModel>

class HostCacheTableModel;

class HostData
{
public:
	SharedHostPtr   m_pHost;

	const EndPoint  m_oAddress;
	const QString   m_sAddress;
	const QString   m_sCountryCode;
	const QString   m_sCountry;
	const QIcon     m_iCountry;
//	const quint32   m_nID;
	quint32         m_tLastConnect;
	QString         m_sLastConnect;
	quint8          m_nFailures;
	QString         m_sFailures;

	DiscoveryProtocol::Protocol m_nType;

//	QString         m_sUserAgent;
//	QIcon           m_iNetwork;

	HostData( SharedHostPtr pHost );
	bool update( int row, int col, QModelIndexList& to_update, HostCacheTableModel* model );
	QVariant data( int col ) const;
	bool lessThan( int col, const HostData* const pOther ) const;
};

#endif // HOSTCACHEHOST_H
