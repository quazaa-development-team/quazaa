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

#ifdef _DEBUG
#include "g2hostcache.h"
#endif // _DEBUG

bool                HostCacheHost::m_bShutDown = false;
quint32             HostCacheHost::m_nLastID = 0;
QMutex              HostCacheHost::m_oIDLock;
std::set<quint32>   HostCacheHost::m_lsIdCheck;

#ifdef _DEBUG
uint                HostCacheHost::m_nHostCount = 0;
#endif // _DEBUG

HostCacheHost::HostCacheHost(const CEndPoint& oAddress, const quint8 nFailures,
							 const quint32 tTimestamp, const quint32 tLastConnect) :
	m_nType( DiscoveryProtocol::None ),
	m_oAddress(     oAddress     ),
	m_tTimeStamp(   tTimestamp   ),
	m_tLastConnect( tLastConnect ),
	m_nFailures(    nFailures    ),
	m_bConnectable( false        )
{
	m_nID = generateID();
}

HostCacheHost::~HostCacheHost()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	qDebug() << "Deleting Host Cache Host No "
			 << QString::number( m_nHostCount ).toLocal8Bit().data()
			 << " with GUI ID "
			 << QString::number( m_nID ).toLocal8Bit().data();
#endif // _DEBUG

	releaseID( m_nID );
}

HostCacheHost* HostCacheHost::load(QDataStream& fsFile, quint32 tNow)
{
	quint8    nType;
	CEndPoint oAddress;
	quint8    nFailures;
	quint32   tTimeStamp;
	quint32   tLastConnect;

	fsFile >> nType;
	fsFile >> oAddress;
	fsFile >> nFailures;
	fsFile >> tTimeStamp;
	fsFile >> tLastConnect;

	if ( tTimeStamp > tNow )
		tTimeStamp = tNow - 60;

	if ( tLastConnect > tNow )
		tLastConnect = tNow - 60;

	HostCacheHost* pReturn = NULL;

	switch ( nType )
	{
	case DiscoveryProtocol::G2:
		pReturn = new G2HostCacheHost( oAddress, tTimeStamp, nFailures );
		pReturn->setLastConnect( tLastConnect );
		break;

	default:
		break;
	}

	return pReturn;
}

void HostCacheHost::save(QDataStream& fsFile)
{
	fsFile << (quint8)m_nType;
	fsFile << m_oAddress;
	fsFile << m_nFailures;
	fsFile << m_tTimeStamp;
	fsFile << m_tLastConnect;
}

quint32 HostCacheHost::generateID()
{
	m_oIDLock.lock();
#ifdef _DEBUG
	++m_nHostCount;
#endif // _DEBUG

	static bool bNeedVerify = false;
	bNeedVerify = !(++m_nLastID); // e.g. we got an overflow

	// We only need to start checking the ID after the first overflow of m_nLastID.
	if ( bNeedVerify )
	{
		while ( m_lsIdCheck.find( m_nLastID ) != m_lsIdCheck.end() )
		{
			++m_nLastID;
		}
	}

	m_lsIdCheck.insert( m_nLastID );
	quint32 nReturn = m_nLastID;

	Q_ASSERT( m_lsIdCheck.find( m_nLastID ) != m_lsIdCheck.end() );

	m_oIDLock.unlock();

	return nReturn;
}

void HostCacheHost::releaseID(quint32 nID)
{
	m_oIDLock.lock();
#ifdef _DEBUG
	--m_nHostCount;
#endif // _DEBUG

	if ( !m_bShutDown ) // don't access the set on shutdown
		if ( !m_lsIdCheck.erase( nID ) )
			Q_ASSERT( false );

	m_oIDLock.unlock();
}
