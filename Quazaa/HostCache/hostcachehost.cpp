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

quint32             HostCacheHost::m_nLastID = 0;
QMutex              HostCacheHost::m_oIDLock;
std::set<quint32>   HostCacheHost::m_idCheck;

HostCacheHost::HostCacheHost(const CEndPoint& oAddress, const quint8 nFailures,
							 const quint32 tTimestamp, const quint32 tLastConnect) :
	m_nType( Protocol::dpNull ),
	m_oAddress(     oAddress     ),
	m_tTimestamp(   tTimestamp   ),
	m_tLastConnect( tLastConnect ),
	m_nFailures(    nFailures    ),
	m_bConnectable( false        )
{
	m_oIDLock.lock();
	static bool bNeedVerify = false;
	bNeedVerify = !(++m_nLastID); // e.g. we got an overflow

	// We only need to start checking the ID after the first overflow of m_nLastID.
	if ( bNeedVerify )
	{
		while ( m_idCheck.find( m_nLastID ) != m_idCheck.end() )
		{
			++m_nLastID;
		}
	}

	m_idCheck.insert( m_nLastID );
	m_nID = m_nLastID;

	m_oIDLock.unlock();
}

HostCacheHost::~HostCacheHost()
{
	// TODO: find out why this causes problems

	/*m_oIDLock.lock();
	m_idCheck.erase( m_nID );
	m_oIDLock.unlock();*/
}
