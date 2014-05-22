/*
** hostcachehost.cpp
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

#include "hostcachehost.h"
#include "g2hostcachehost.h"

#include "Misc/networkiconprovider.h"

#include "hostcachetablemodel.h"

using namespace HostManagement;

//IDProvider<quint32> HostCacheHost::m_oIDProvider;
bool                HostCacheHost::m_bShutDownFlag = false;

HostCacheHost::HostCacheHost( const EndPoint& oAddress, quint8 nFailures, quint32 tTimestamp,
							  quint32 tLastConnect, SourceID nOwnID, SourceID nSourceID ) :
	m_nType( DiscoveryProtocol::None ),
	m_oAddress(     oAddress         ),
	m_tTimeStamp(   tTimestamp       ),
	m_nOwnID(       nOwnID           ),
	m_nSourceID(    nSourceID        ),
	m_tLastConnect( tLastConnect     ),
	m_tLastConnectionEstablished( 1  ), // TODO: fill with data
	m_nFailures(    nFailures        ),
	m_bConnectable( false            )
{
//	m_nID = m_oIDProvider.aquire();
	Q_ASSERT( nOwnID ); // each host must have a valid own ID
}

HostCacheHost::~HostCacheHost()
{
	if ( !m_bShutDownFlag )
	{
//		m_oIDProvider.release( m_nID );
	}
}

HostCacheHost* HostCacheHost::load( QDataStream& fsFile, quint32 tNow )
{
	quint8   nType;
	EndPoint oAddress;
	quint8   nFailures;
	quint32  tTimeStamp;
	quint32  tLastConnect;
	quint32  nOwnID;
	quint32  nParentID;

	fsFile >> nType;
	fsFile >> oAddress;
	fsFile >> nFailures;
	fsFile >> tTimeStamp;
	fsFile >> tLastConnect;
	fsFile >> nOwnID;
	fsFile >> nParentID;

	if ( tTimeStamp > tNow )
	{
		tTimeStamp = tNow - 60;
	}

	if ( tLastConnect > tNow )
	{
		tLastConnect = tNow - 60;
	}

	HostCacheHost* pReturn = NULL;

	switch ( nType )
	{
	case DiscoveryProtocol::G2:
		pReturn = new G2HostCacheHost( oAddress, tTimeStamp, nFailures, nOwnID, nParentID );
		pReturn->setLastConnect( tLastConnect );
		break;

	default:
		break;
	}

	return pReturn;
}

void HostCacheHost::save( QDataStream& fsFile )
{
	fsFile << ( quint8 )m_nType;
	fsFile << m_oAddress;
	fsFile << m_nFailures;
	fsFile << m_tTimeStamp;
	fsFile << m_tLastConnect;
	fsFile << ( quint32 )m_nOwnID;
	fsFile << ( quint32 )m_nSourceID;
}

HostData::HostData( SharedHostPtr pHost ) :
	m_pHost(        pHost                ),
	m_oAddress(     pHost->address()     ),
	m_sAddress(     m_oAddress.toStringWithPort() ),
	m_sCountryCode( m_oAddress.country() ),
	m_sCountry(     geoIP.countryNameFromCode( m_sCountryCode ) ),
	m_iCountry(     NetworkIconProvider::icon( m_sCountryCode ) ),
//	m_nID(          pHost->id()          ),
	m_tLastConnect( pHost->lastConnect() ),
	m_sLastConnect( m_tLastConnect ? QDateTime::fromTime_t( m_tLastConnect ).toString() :
									 QObject::tr( "never" ) ),
	m_nFailures(    pHost->failures()    ),
	m_sFailures(    QString::number( m_nFailures ) ),
	m_nType(        pHost->type() )
{
}

/**
 * @brief update refreshes the data within HostData if necessary.
 * Locking: REQUIRES hostCache.m_pSection
 * @param nRow : the row being refreshed
 * @param nSortCol : the currently sorted column
 * @param lToUpdate : the list of indexes that have changed
 * @param pModel : the model
 * @return true if an entry within the column col has been modified
 */
bool HostData::update( int nRow, int nSortCol, QModelIndexList& lToUpdate,
					   HostCacheTableModel* pModel )
{
	Q_ASSERT( !m_pHost.isNull() );

	bool bReturn = false;

	// address and country never change
	if ( m_tLastConnect != m_pHost->lastConnect() )
	{
		lToUpdate.append( pModel->index( nRow, HostCacheTableModel::LASTCONNECT ) );
		m_tLastConnect = m_pHost->lastConnect();
		m_sLastConnect = m_tLastConnect ? QDateTime::fromTime_t( m_tLastConnect ).toString()
						 : QObject::tr( "never" );

		if ( nSortCol == HostCacheTableModel::LASTCONNECT )
		{
			bReturn = true;
		}
	}

	if ( m_nFailures != m_pHost->failures() )
	{
		lToUpdate.append( pModel->index( nRow, HostCacheTableModel::FAILURES ) );
		m_nFailures = m_pHost->failures();
		m_sFailures = QString::number( m_nFailures );

		if ( nSortCol == HostCacheTableModel::FAILURES )
		{
			bReturn = true;
		}
	}

	return bReturn;
}

/**
 * @brief RuleData::data
 * @param col
 * @return
 */
QVariant HostData::data( int col ) const
{
	switch ( col )
	{
	case HostCacheTableModel::ADDRESS:
		return m_sAddress;

	case HostCacheTableModel::LASTCONNECT:
		return m_sLastConnect;

	case HostCacheTableModel::FAILURES:
		return m_sFailures;

	case HostCacheTableModel::COUNTRY:
		return m_sCountry;

	default:
		return QVariant();
	}
}

bool HostData::lessThan( int col, const HostData* const pOther ) const
{
	Q_ASSERT( pOther );

	switch ( col )
	{
	case HostCacheTableModel::ADDRESS:
		return m_sAddress     < pOther->m_sAddress;

	case HostCacheTableModel::LASTCONNECT:
		return m_tLastConnect < pOther->m_tLastConnect;

	case HostCacheTableModel::FAILURES:
		return m_nFailures    < pOther->m_nFailures;

	case HostCacheTableModel::COUNTRY:
		return m_sCountryCode < pOther->m_sCountryCode;

	default:
		return false;
	}
}
