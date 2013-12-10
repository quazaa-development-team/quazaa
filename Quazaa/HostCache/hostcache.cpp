/*
** hostcache.cpp
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

#include "hostcache.h"

HostCache::HostCache() :
	m_tLastSave( 0 ),
	m_nMaxFailures( 0 ),
	m_nSizeAtomic( 0 )
{
}

HostCache::~HostCache()
{
}

/**
 * @brief start initializes the Host Cache. Make sure this is called after QApplication is
 * instantiated.
 * Locking: YES (asynchronous)
 */
void HostCache::start()
{
#if ENABLE_G2_HOST_CACHE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Components::HostCache, QString( "start()" ) );
#endif //ENABLE_G2_HOST_CACHE_DEBUGGING

	m_pHostCacheDiscoveryThread = SharedThreadPtr( new QThread() );

	moveToThread( m_pHostCacheDiscoveryThread.data() );
	m_pHostCacheDiscoveryThread.data()->start( QThread::LowPriority );

	QMetaObject::invokeMethod( this, "asyncStartUpHelper", Qt::QueuedConnection );
}
