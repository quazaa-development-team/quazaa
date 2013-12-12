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

quint32             HostCacheHost::m_nLastID = 0;
QMutex              HostCacheHost::m_oIDLock;
std::set<quint32>   HostCacheHost::m_lsIdCheck;

HostCacheHost::HostCacheHost(const CEndPoint& oAddress, const quint8 nFailures,
							 const quint32 tTimestamp, const quint32 tLastConnect) :
	m_nType( Protocol::dpNull ),
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
	// TODO: find out why this causes trouble on shutdown
	//releaseID( m_nID );
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
	case Protocol::dpG2:
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
	Q_ASSERT( m_lsIdCheck.erase( nID ) );
	m_oIDLock.unlock();
}
