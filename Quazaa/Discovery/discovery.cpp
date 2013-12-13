/*
** discovery.cpp
**
** Copyright © Quazaa Development Team, 2012.
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

#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>

#include <QNetworkConfigurationManager>

#include "discovery.h"
#include "discoveryservice.h"

#include "quazaaglobals.h"
#include "quazaasettings.h"
#include "debug_new.h"

Discovery::Manager discoveryManager;

using namespace Discovery;

/**
 * @brief Discovery: Constructs a new discovery services manager.
 * @param parent
 */
Manager::Manager(QObject *parent) :
	QObject( parent ),
	m_bSaved( true ),
	m_nLastID( 0 ),
	m_pActive( new quint16[Discovery::ServiceType::NoOfTypes] )
{
	for ( quint8 i = 0; i < Discovery::ServiceType::NoOfTypes; ++i )
	{
		m_pActive[i] = 0;
	}

	// reg. meta types
	static int foo = qRegisterMetaType<ServiceID>( "ServiceID" );
	static int bar = qRegisterMetaType<ConstServicePtr>( "ConstServicePtr" );

	Q_UNUSED( foo );
	Q_UNUSED( bar );
}

/**
 * @brief ~CDiscovery: Destructor. Make sure you have stopped the magager befor destroying it.
 */
Manager::~Manager()
{
	delete[] m_pActive;

	Q_ASSERT( m_mServices.empty() );
}

/**
 * @brief count allows you to access the number of working services for a given network.
 * Locking: YES (synchronous)
 * @return the number of services for the specified network. If no type is specified or the type
 * is null, the total number of all services is returned, no matter whether they are working or
 * not.
 */
quint32	Manager::count(const CNetworkType& oType)
{
	QMutexLocker l( &m_pSection );
	return doCount( oType );
}

/**
 * @brief start initializes the Discovery Services Manager. Make sure this is called after
 * QApplication is instantiated.
 * Locking: YES
 */
void Manager::start()
{
	m_pHostCacheDiscoveryThread = hostCache.m_pHostCacheDiscoveryThread;
	Q_ASSERT( !m_pHostCacheDiscoveryThread.isNull() );

	moveToThread( m_pHostCacheDiscoveryThread.data() );

	Q_ASSERT( m_pHostCacheDiscoveryThread.data()->isRunning() );

	// Initialize random number generator.
	qsrand ( common::getTNowUTC() );

	// Includes its own locking.
	load();
}

/**
 * @brief stop prepares the Discovery Services Manager for destruction.
 * Locking: YES (synchronous)
 * @return true if the services have been successfully written to disk.
 */
bool Manager::stop()
{
	bool bSaved = save( true );
	clear();

	return bSaved;
}

/**
 * @brief save saves all discovery services to disk, if there have been important modifications
 * to at least one service or bForceSaving is set to true.
 * Locking: YES (synchronous/asynchronous)
 * @param bForceSaving: Set this to true to force saving even if there have been no important
 * service modifications, for example to make sure the hosts count statistics from the current
 * session are saved properly.
 * Setting this parameter will also result in synchronous execution. If this parameter is set to
 * false (or omitted), saving will be done asynchronously.
 * @return true if saving to file is known to have been successful; false otherwise (e.g. error
 * or asynchronous execution)
 */
bool Manager::save(bool bForceSaving)
{
	if ( bForceSaving )
	{
		// synchronous saving required
		return asyncSyncSavingHelper();
	}
	else
	{
		m_pSection.lock();
		if ( m_bSaved )
		{
			m_pSection.unlock();
			return true; // Saving not required ATM.
		}
		else
		{
			m_pSection.unlock();
			QMetaObject::invokeMethod( this, "asyncSyncSavingHelper", Qt::QueuedConnection );

		// We do not know whether it was successful, so pretend it was not to be on the safe side.
			return false;
		}
	}
}

/**
 * @brief add adds a new Service with a given URL to the manager.
 * Locking: YES (synchronous)
 * @param sURL
 * @param eSType
 * @param oNType
 * @param nRating
 * @return the service ID used to identify the service internally; 0 if the service has not been
 * added.
 */
ServiceID Manager::add(QString sURL, const ServiceType::Type eSType,
						 const CNetworkType& oNType, const quint8 nRating)
{
#if ENABLE_DISCOVERY_DEBUGGING
	QString s = QString( "add( <Service>, Service Type: " ) + QString::number( eSType )  +
						 ", Network Type: "                 + oNType.toString()          +
						 ", Service Rating: "               + QString::number( nRating ) +
						 " ) Service: "                     + sURL;
	postLog( LogSeverity::Debug, s, true );
#endif

	// set URL to lowercase
	sURL = sURL.toLower().trimmed();

	m_pSection.lock();

	if ( eSType == ServiceType::Banned )
	{
		if ( manageBan( sURL, eSType, oNType ) )
		{
			m_pSection.unlock();
			return 0;
		}
	}
	else
	{
		if ( checkBan( sURL ) )
		{
			m_pSection.unlock();
			postLog( LogSeverity::Warning, tr( "Service URL banned." ) );
			return 0;
		}

		if ( normalizeURL( sURL ) )
		{
			if ( checkBan( sURL ) )
			{
				m_pSection.unlock();
				postLog( LogSeverity::Warning, tr( "Service URL banned." ) );
				return 0;
			}
		}
	}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, QString( "Adding " ) + sURL, true );
#endif

	ServicePtr pService;

	if ( !sURL.isEmpty() )
	{
		pService = ServicePtr( DiscoveryService::createService( sURL, eSType, oNType, nRating ) );

		if ( pService && add( pService ) )
		{
			// make sure to return the right ID
			ServiceID nTmp = pService->m_nID;

			m_pSection.unlock();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, QString( "New discovery service added: " ) + sURL, true );
#endif

			postLog( LogSeverity::Notice, tr( "New discovery service added: " ) + sURL );

			// inform GUI about new service
			emit serviceAdded( pService );
			return nTmp;
		}
		else
		{
			// Error messages handled within add()
			m_pSection.unlock();
			return 0;
		}
	}

	m_pSection.unlock();

	// sURL empty. This is the sign of a previous error, probably within normalizeURL()
	postLog( LogSeverity::Warning, tr( "Error adding service." ) );
	return 0;
}

/**
 * @brief remove removes a service by ID.
 * Locking: YES (synchronous)
 * @param nID
 * @return true if the removal was successful (e.g. the service could be found), false
 * otherwise.
 */
bool Manager::remove(ServiceID nID)
{
	if ( !nID )
	{
		postLog( LogSeverity::Error, tr( "Internal error: Got request to remove invalid ID: 0" ) );
		return false; // invalid ID
	}

	m_pSection.lock();
	bool bReturn = doRemove( nID );
	m_pSection.unlock();

	return bReturn;
}

/**
 * @brief clear removes all services from the manager.
 * Locking: YES (synchronous)
 * @param bInformGUI: Set this to true if the GUI shall be informed about the removal of the
 * services. The default value is false, which represents the scenario on shutdown, where the
 * GUI will be removed shortly anyway.
 */
void Manager::clear(bool bInformGUI)
{
	m_pSection.lock();
	doClear( bInformGUI );
	m_pSection.unlock();
}

/**
 * @brief check verifies whether the given service is managed by the manager.
 * Locking: YES (synchronous)
 * @return true if managed; false otherwise
 */
bool Manager::check(const ConstServicePtr pService)
{
	QMutexLocker l( &m_pSection );

	ConstIterator iService = m_mServices.find( pService->m_nID );

	if ( iService == m_mServices.end() )
		return false; // Unable to find service by ID

	const ConstServicePtr pExService = (*iService).second;

	if ( *pExService == *pService )
		return true;
	else
		return false;
}

/**
 * @brief isActive allows to find out whether the Discovery Manager is currently active.
 * Locking: YES (synchronous)
 * @param eSType
 * @return true if active; false otherwise
 */
bool Manager::isActive(const ServiceType::Type eSType)
{
	QMutexLocker l( &m_pSection );
	return m_pActive[eSType];
}

/**
 * @brief CDiscovery::requestNAMgr provides a shared pointer to the discovery services network
 * access manager. Note that the caller needs to hold his copy of the shared pointer until the
 * network operation has been completed to prevent the manager from being deleted too early.
 * Locking: YES (synchronous)
 * @return
 */
QSharedPointer<QNetworkAccessManager> Manager::requestNAM()
{
	m_pSection.lock();

	QSharedPointer<QNetworkAccessManager> pReturnVal = m_pNetAccessMgr.toStrongRef();
	if ( !pReturnVal )
	{
		// else create a new access manager (will be deleted if nobody is using it anymore)
		pReturnVal = QSharedPointer<QNetworkAccessManager>( new QNetworkAccessManager(),
															&QObject::deleteLater );
		m_pNetAccessMgr = pReturnVal.toWeakRef();

		// Make sure the networkAccessible state is properly initialized.
		QNetworkConfigurationManager manager;
		pReturnVal->setConfiguration( manager.defaultConfiguration() );
		// QNetworkAccessManager::networkAccessible is not explicitly set when the
		// QNetworkAccessManager is created. It is only set after the network session is
		// initialized. The session is initialized automatically when a network request is made or
		// it can be initialized manually beforehand with QNetworkAccessManager::setConfiguration()
		// or by setting the QNetworkConfigurationManager::NetworkSessionRequired flag.
		// http://www.qtcentre.org/threads/37514-use-of-QNetworkAccessManager-networkAccessible
		// ?s=171a7f69eccb2459cf1cc38507347ead&p=188372#post188372
	}

	m_pSection.unlock();

	return pReturnVal;
}

/**
 * @brief requestServiceList can be used to obtain a complete list of all currently managed
 * services. Connect to the serviceInfo() signal to recieve them.
 * Locking: YES (asynchronous)
 */
void Manager::requestServiceList()
{
	QMetaObject::invokeMethod( this, "asyncRequestServiceListHelper", Qt::QueuedConnection );
}

/**
 * @brief updateService updates a service for a given network type with our IP. Note that not
 * all service types might support or require such updates.
 * Locking: YES (asynchronous)
 * @param type
 */
void Manager::updateService(const CNetworkType& type)
{
	QMetaObject::invokeMethod( this, "asyncUpdateServiceHelper",
							   Qt::QueuedConnection, Q_ARG( const CNetworkType, type ) );
}

void Manager::updateService(ServiceID nID)
{
	QMetaObject::invokeMethod( this, "asyncUpdateServiceHelper",
							   Qt::QueuedConnection, Q_ARG( ServiceID, nID ) );
}

/**
 * @brief queryService queries a service for hosts to connect to.
 * Locking: YES (asynchronous)
 * @param type
 */
void Manager::queryService(const CNetworkType& type)
{
	QMetaObject::invokeMethod( this, "asyncQueryServiceHelper",
							   Qt::QueuedConnection, Q_ARG( const CNetworkType, type ) );
}

void Manager::queryService(ServiceID nID)
{
	QMetaObject::invokeMethod( this, "asyncQueryServiceHelper",
							   Qt::QueuedConnection, Q_ARG( ServiceID, nID ) );
}

/**
 * @brief getWorkingService
 * Locking: YES (synchronous)
 * @param type
 * @return the URL of a service of the requested type that is known to work
 */
QString Manager::getWorkingService(ServiceType::Type type)
{
	QMutexLocker l( &m_pSection );

	DiscoveryServicesList list;
	quint16 nTotalRating = 0;		// Used to store accumulative rating of all services
									// taken under consideration as return value.
	ServicePtr pService;

	foreach ( MapPair pair, m_mServices )
	{
		pService = pair.second;

		// Only consider services that are currently not in use. This is faster and prevents
		// deadlocks as the caller of this method is probably a locked service himself.
		if ( pService->m_oRWLock.tryLockForRead( 100 ) )
		{
			if ( pService->m_bBanned )
			{
				pService->m_oRWLock.unlock();
				continue; // skip banned services
			}

			// if the service has the correct type and the last access was successful...
			if ( pService->m_nServiceType == type && !pService->m_nFailures &&
				 pService->m_tLastSuccess )
			{
				// ...add the service to the list to select from
				list.push_back( pService );
				nTotalRating += pService->m_nRating;
			}
			else
			{
				// won't use it, so no need to lock
				pService->m_oRWLock.unlock();
			}
		}
	}

	if ( list.empty() )
	{
		return QString();
	}
	else
	{
		// Make sure our selection is within [1 ; nTotalRating]
		quint16 nSelectedRating = ( qrand() % nTotalRating ) + 1;
		ListIterator   current = list.begin();
		ServicePtr   pSelected = *current;

		// Iterate threw list until the selected service has been found.
		while ( nSelectedRating > pSelected->m_nRating )
		{
			nSelectedRating -= pSelected->m_nRating;
			pSelected->m_oRWLock.unlock();
			pSelected = *( ++current );
		}

		while ( current != list.end() )
		{
			pService = *( current++ );
			pService->m_oRWLock.unlock();
		}

		return pSelected->m_oServiceURL.toString();
	}
}

bool Manager::asyncSyncSavingHelper()
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "asyncSyncSavingHelper()", true );
#endif

	postLog( LogSeverity::Notice, tr( "Saving Discovery Services Manager state." ) );

	QString sPath          = CQuazaaGlobals::DATA_PATH() + "discovery.dat";
	QString sBackupPath    = CQuazaaGlobals::DATA_PATH() + "discovery_backup.dat";
	QString sTemporaryPath = sBackupPath + "_tmp";

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "defined paths", true );
#endif

	if ( QFile::exists( sTemporaryPath ) && !QFile::remove( sTemporaryPath ) )
	{
		postLog( LogSeverity::Error,
				 tr( "Could not free space required for data backup: " ) + sPath );

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "error temp path used...", true );
#endif
		return false;
	}

	QFile oFile( sTemporaryPath );

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "created file", true );
#endif

	if ( !oFile.open( QIODevice::WriteOnly ) )
	{
		postLog( LogSeverity::Error, tr( "Could open data file for write: " ) + sTemporaryPath );

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "could not write to file", true );
#endif
		return false;
	}

	m_pSection.lock();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "got mutex lock", true );
#endif

	quint16 nVersion = DISCOVERY_CODE_VERSION;
	quint32 nCount   = doCount();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "before try", true );
#endif

	try
	{
		QDataStream fsFile( &oFile );

		fsFile << nVersion;
		fsFile << nCount;

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "before writing services to file", true );
#endif

		ServicePtr pService;

		// write services to stream
		foreach (  MapPair pair, m_mServices )
		{
			pService = pair.second;
			pService->cancelRequest( true );
			DiscoveryService::save( pService.data(), fsFile );
			pService->unlock();
		}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "saved all services", true );
#endif
	}
	catch ( ... )
	{
		m_pSection.unlock();

		postLog( LogSeverity::Error,
				 tr( "Unspecified problem while writing discovery services to disk." ) );

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Caught exception!", true );
#endif
		return false;
	}

	m_bSaved = true;
	m_pSection.unlock();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "mutex unlocked", true );
#endif

	oFile.close();

	if ( QFile::exists( sPath ) && !QFile::remove( sPath ) )
	{
		postLog( LogSeverity::Error, tr( "Could not remove old data file: " ) + sPath );
		return false;
	}

	if ( !QFile::rename( sTemporaryPath, sPath ) )
	{
		postLog( LogSeverity::Error, tr( "Could not rename data file: " ) + sPath );
		return false;
	}

	if ( QFile::exists( sBackupPath ) && !QFile::remove( sBackupPath ) )
	{
		postLog( LogSeverity::Warning, tr( "Could not remove old backup file: " ) + sBackupPath );
	}

	if ( !QFile::copy( sPath, sBackupPath ) )
	{
		postLog( LogSeverity::Warning, tr( "Could not create create new backup file: " )
						   + sBackupPath );
	}

	postLog( LogSeverity::Debug, tr( "Saved %1 services to file." ).arg( nCount ) );

	return true;
}

void Manager::asyncRequestServiceListHelper()
{
	m_pSection.lock();
	foreach ( MapPair pair, m_mServices )
	{
		emit serviceInfo( pair.second );
	}
	m_pSection.unlock();
}

void Manager::asyncUpdateServiceHelper(const CNetworkType type)
{
	QSharedPointer<QNetworkAccessManager> pNAM = requestNAM();

	if ( pNAM->networkAccessible() == QNetworkAccessManager::Accessible )
	{
		m_pSection.lock();
		ServicePtr pService = getRandomService( type );
		m_pSection.unlock();

		if ( pService )
		{
			postLog( LogSeverity::Notice, tr( "Updating service: " ) + pService->url() );

			++m_pActive[pService.data()->m_nServiceType];
			pService->update();
		}
		else
		{
			postLog( LogSeverity::Warning,
					 tr( "Unable to update service for network: " ) + type.toString() );
		}
	}
	else
	{
		postLog( LogSeverity::Error,
		tr( "Could not update service because the network connection is currently unavailable." ) );
	}
}

void Manager::asyncUpdateServiceHelper(ServiceID nID)
{
	// We do not prevent users from manually querying services even if the network connection is
	// reported to be down. Maybe they know better than we do.

	m_pSection.lock();

	Iterator iService = m_mServices.find( nID );

	if ( iService == m_mServices.end() )
	{
		m_pSection.unlock();

		// Unable to find service by ID (should never happen)
		Q_ASSERT( false );

		return;
	}

	ServicePtr pService = (*iService).second;
	m_pSection.unlock();

	Q_ASSERT( pService ); // We should always get a valid service from the iterator

	postLog( LogSeverity::Notice, tr( "Updating service: " ) + pService->url() );

	++m_pActive[pService.data()->m_nServiceType];
	pService->update();
}

void Manager::asyncQueryServiceHelper(const CNetworkType type)
{
#if ENABLE_DISCOVERY_DEBUGGING
	QString s = QString ( "CDiscovery::asyncQueryServiceHelper( " ) +
				QString ( "const CNetworkType: %1 )" ).arg( type.toString() );
	postLog( LogSeverity::Debug, s, true );
#endif

	QSharedPointer<QNetworkAccessManager> pNAM = requestNAM();

	if ( pNAM->networkAccessible() == QNetworkAccessManager::Accessible )
	{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Network accessible.", true );
#endif

		m_pSection.lock();
		ServicePtr pService = getRandomService( type );
		m_pSection.unlock();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Got service pointer.", true );
#endif

		if ( pService )
		{
			postLog( LogSeverity::Notice, tr( "Querying service: " ) + pService->url() );

#if ENABLE_DISCOVERY_DEBUGGING
			postLog( LogSeverity::Debug, "Service pointer OK.", true );
#endif

			++m_pActive[pService.data()->serviceType()];
			pService->query();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Service Queried.", true );
#endif
		}
		else
		{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Error, "Service pointer NULL!", true );
#endif

			postLog( LogSeverity::Warning,
					 tr( "Unable to query service for network: " ) + type.toString() );
		}
	}
	else
	{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Error, "Network inaccessible!", true );
	postLog( LogSeverity::Error,
			 QString( "Network status: " ) + QString::number( pNAM->networkAccessible() ), true );
#endif

		postLog( LogSeverity::Error,
		tr( "Could not query service because the network connection is currently unavailable." ) );
	}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 "End of CDiscovery::asyncQueryServiceHelper( const CNetworkType )", true );
#endif
}

void Manager::asyncQueryServiceHelper(ServiceID nID)
{
	// We do not prevent users from manually querying services even if the network connection is
	// reported to be down. Maybe they know better than we do.

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "CDiscovery::asyncQueryServiceHelper( TServiceID )", true );
#endif

	m_pSection.lock();

	Iterator iService = m_mServices.find( nID );

	if ( iService == m_mServices.end() )
	{
		m_pSection.unlock();

		// Unable to find service by ID (should never happen)
		Q_ASSERT( false );

		return; // Unable to find service by ID
	}

	ServicePtr pService = (*iService).second;
	m_pSection.unlock();

	Q_ASSERT( pService ); // We should always get a valid service from the iterator

	postLog( LogSeverity::Notice, tr( "Querying service: " ) + pService->url() );

	++m_pActive[pService.data()->serviceType()];
	pService->query();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Service Queried.", true );
#endif
}

/**
 * @brief doCount: Internal helper without locking. See count for documentation.
 */
quint32 Manager::doCount(const CNetworkType& oType)
{
	if ( oType.isNull() )
	{
		return (quint32)m_mServices.size();
	}
	else
	{
		quint16 nCount = 0;
		ServicePtr pService;

		foreach ( MapPair pair, m_mServices )
		{
			pService = pair.second;
			pService->m_oRWLock.lockForRead();

			// Count all services that have the correct type, are not blocked and have a rating > 0
			if ( pService->m_oNetworkType.isNetwork( oType ) &&
				 !pService->m_bBanned &&
				 pService->m_nRating )
			{
				++nCount;
			}

			pService->m_oRWLock.unlock();
		}

		return nCount;
	}
}

/**
 * @brief doClear: Internal helper without locking. See clear for documentation.
 */
void Manager::doClear(bool bInformGUI)
{
	if ( bInformGUI )
	{
		foreach ( MapPair pair, m_mServices )
		{
			emit serviceRemoved( pair.second->m_nID ); // inform GUI
		}
	}

	m_mServices.clear();	// TServicePtr takes care of service deletion
	m_nLastID = 0;			// make sure to recycle the IDs.
}

/**
 * @brief doRemove: Internal helper without locking. See remove for documentation.
 */
bool Manager::doRemove(ServiceID nID)
{
	Iterator iService = m_mServices.find( nID );

	if ( iService == m_mServices.end() )
	{
		postLog( LogSeverity::Error,
				 tr( "Got request to remove invalid ID: " ) + QString::number( nID ) );
		return false; // Unable to find service by ID
	}

	// inform GUI about service removal
	emit serviceRemoved( nID );

	ServicePtr pService = (*iService).second;

	postLog( LogSeverity::Notice,
			 tr( "Removing discovery service: " ) + pService->m_oServiceURL.toString() );

	// stop it if necessary
	pService->cancelRequest();

	// remove it
	m_mServices.erase( nID );

	// Make sure to reassign the now unused ID.
	if ( --nID < m_nLastID )
		m_nLastID = nID;

	return true;
}

/**
 * @brief load retrieves stored services from the HDD.
 * Locking: YES (synchronous)
 * @return true if loading from file was successful; false otherwise.
 */
// Called only from within startup sequence
void Manager::load()
{
	QString sPath = CQuazaaGlobals::DATA_PATH() + "discovery.dat";

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Started loading services.", true );
#endif

	if ( load( sPath ) )
	{
		postLog( LogSeverity::Debug, tr( "Loaded discovery services from file: " ) + sPath );
	}
	else // Unable to load default file. Switch to backup one instead.
	{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Failed primary attempt on loading services.", true );
#endif

		sPath = CQuazaaGlobals::DATA_PATH() + "discovery_backup.dat";

		postLog( LogSeverity::Warning,
				 tr( "Failed to load discovery services from primary file. Switching to backup: " )
				 + sPath );

		if ( !load( sPath ) )
		{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Failed secondary attempt on loading services.", true );
#endif

			postLog( LogSeverity::Error, tr( "Failed to load discovery services!" ) );
		}
	}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, QString( "Number of services: %1" ).arg( count() ), true );
#endif

	if ( count() < 5 )
	{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Adding defaults.", true );
#endif

		addDefaults();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Finished adding defaults. New number of services: %1" ).arg( count() ),
			 true );
#endif
	}
}

bool Manager::load( QString sPath )
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Loading discovery services from file: " ) + sPath, true );
#endif

	QFile oFile( sPath );

	if ( ! oFile.open( QIODevice::ReadOnly ) )
		return false;

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Prepared file.", true );
#endif

	DiscoveryService* pService = NULL;

	try
	{
		clear();

		QDataStream fsFile( &oFile );

		quint16 nVersion;
		quint32 nCount;

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Discovery code version:      " ) +
			 QString::number( DISCOVERY_CODE_VERSION ), true );
#endif

		fsFile >> nVersion;
		fsFile >> nCount;

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "File data structure version: " ) + QString::number( nVersion ), true );
	postLog( LogSeverity::Debug,
			 QString( "Number of services stored in file: " ) + QString::number( nCount ), true );
#endif

		if ( nVersion == DISCOVERY_CODE_VERSION ) // else do load defaults
		{
			QMutexLocker l( &m_pSection );
			while ( nCount > 0 )
			{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Still %1 services to load." ).arg( QString::number( nCount ) ),
			 true );
#endif

				DiscoveryService::load( pService, fsFile, nVersion );

				if ( pService )
				{
					// This needs two lines for compilation under MingW to succeed.
					ServicePtr pManagedService = ServicePtr( pService );
					add( pManagedService );
					pService = NULL;
				}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Service entry %1 processed." ).arg( QString::number( nCount ) ), true );
#endif

				--nCount;
			}
		}
	}
	catch ( ... )
	{
		if ( pService )
			delete pService;

		clear();
		oFile.close();

		postLog( LogSeverity::Error, tr( "Caught an exception while loading services from file!" ));

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Error while loading services.", true );
#endif
		return false;
	}
	oFile.close();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Finished loading services.", true );
#endif
	return true;
}

/**
 * @brief add... obvious... Note: if a duplicate is detected, the CDiscoveryService passed to
 * the method is deleted within.
 * Requires locking: YES
 * @param pService
 * @return true if the service was added; false if not (e.g. duplicate was detected).
 */
bool Manager::add(ServicePtr& pService)
{
	// remove duplicates and handle bans
	if ( manageDuplicates( pService ) )
	{
		return false;
	}

	// check for already existing ID
	if ( pService->m_nID && m_mServices.find( pService->m_nID ) != m_mServices.end() )
	{
		// We need to assign a new ID, as the previous ID of this service is already in use.

		// This should not happen as the services from file should be loaded into the manager before
		// any new services that might be added during the session.
		Q_ASSERT( false );

		pService->m_nID = 0; // set ID to invalid.
	}

	// assign valid ID if necessary
	if ( !pService->m_nID )
	{
		ConstIterator it;
		do
		{
			// Check the next ID until a free one is found.
			it = m_mServices.find( ++m_nLastID );
		}
		while ( !m_nLastID // overflow is allowed and expected once all IDs have been used once
				|| it != m_mServices.end() );

		// assign ID to service
		pService->m_nID = m_nLastID;
	}

	// push to map
	m_mServices[pService->m_nID] = pService;

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "[Discovery] Service added to manager: [%1] " ).arg( pService->type() ) +
			 pService->m_oServiceURL.toString(), true );
#endif
	return true;
}

/**
 * @brief addDefaults loads the DefaultServices.dat file (compatible with Shareaza) into the
 * manager.
 * Locking: YES (synchronous)
 */
// Note: When modifying this method, compatibility to Shareaza should be maintained.
void Manager::addDefaults()
{
	QString sPath = QDir::toNativeSeparators(QString("%1/DefaultServices.dat").arg(qApp->applicationDirPath()));
	QFile oFile( sPath );

	postLog( LogSeverity::Debug, tr( "Loading default services from file." ) );

	if ( !oFile.open( QIODevice::ReadOnly ) )
	{
		postLog( LogSeverity::Error, tr( "Error: Could not open default services file: " ) + sPath );
		return;
	}

	try
	{
		QTextStream fsFile( &oFile );
		QString     sLine, sService;
		QChar       cType;
#if ENABLE_DISCOVERY_DEBUGGING
		bool        bAdded;
#endif

		while( !fsFile.atEnd() )
		{
			sLine = fsFile.readLine();

			if ( sLine.length() < 7 )	// Blank comment line
				continue;

			cType = sLine.at( 0 );
			sService = sLine.right( sLine.length() - 2 );
#if ENABLE_DISCOVERY_DEBUGGING
			bAdded = false;
#endif

			switch( cType.toLatin1() )
			{
//			case '1':	// gnutella service
//				break;
			case '2':	// G2 service
#if ENABLE_DISCOVERY_DEBUGGING
				postLog( LogSeverity::Debug, "Parsing Default Service: G2", true );
				bAdded =
#endif
				add( sService, ServiceType::GWC, CNetworkType( dpG2 ), DISCOVERY_MAX_PROBABILITY );
				break;

			case 'M':	// Multi-network service
#if ENABLE_DISCOVERY_DEBUGGING
				postLog( LogSeverity::Debug, "Parsing Default Service: Multi Network Cache", true );
				bAdded =
#endif
				add( sService, ServiceType::GWC, CNetworkType( dpG2 ), DISCOVERY_MAX_PROBABILITY );
				break;

//			case 'D':	// eDonkey service
//				break;
//			case 'U':	// Bootstrap and UDP Discovery Service
//				break;

			case 'X':	// Blocked service
#if ENABLE_DISCOVERY_DEBUGGING
				postLog( LogSeverity::Debug, "Parsing Default Service: Banned Service", true );
				bAdded =
#endif
				add( sService, ServiceType::Banned, CNetworkType( dpNull ), 0 );
				break;

			default:	// Comment line or unsupported
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Parsed comment line or unsupported service type.", true );
#endif
				break;
			}

#if ENABLE_DISCOVERY_DEBUGGING
			if ( bAdded )
				postLog( LogSeverity::Debug, "Service added.", true );
#endif
		}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Finished parsing Default Services file.", true );
#endif
	}
	catch ( ... )
	{
#if ENABLE_DISCOVERY_DEBUGGING
		postLog( LogSeverity::Debug, "Caught something ugly!", true );
#endif
		postLog( LogSeverity::Error, tr( "Error while loading default servers from file." ) );
	}

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Closing Default Services file.", true );
#endif
	oFile.close();
}

/**
 * @brief manageBan handles the checking for duplicates when adding a new ban.
 * Requires locking: YES
 * @param sURL
 * @param eSType
 * @param oNType
 * @return true if the caller needs not to worry about adding the banned service anymore
 */
bool Manager::manageBan(QString& sURL, const ServiceType::Type eSType, const CNetworkType& oNType)
{
	bool bReturn = false;

	foreach ( MapPair pair, m_mServices )
	{
		// already existing service
		ServicePtr pService = pair.second;
		pService->m_oRWLock.lockForWrite();

		// check for services with the same URL; Note: all URLs should be lower case, so case
		// sensitivity is not a problem here.
		if ( !pService->m_oServiceURL.toString().compare( sURL ) )
		{
			ServiceID nID = 0;

			// All service with type stBanned have m_bBanned set to true.
			if ( /*pService->m_nServiceType == stBanned ||*/ pService->m_bBanned )
			{
				// existing bans do override everything

				postLog( LogSeverity::Debug,
						 tr( "URL already banned. Adding it twice would be a waste." ) );
				bReturn = true;
				sURL = "";
			}
			else // existing service not yet banned
			{
				if ( eSType == pService->m_nServiceType )
				{
					// Make sure the existing service is set to handle the networks pService
					// handles, too.
					pService->m_oNetworkType.setNetwork( oNType );
					pService->m_bBanned = true;
					bReturn = true;
					sURL = "";

					postLog( LogSeverity::Debug,
							 tr( "Detected an already existing service using the same URL. " ) +
							 tr( "Setting that service to banned instead of adding a new one." ) );
				}
				else
				{
					nID = pService->m_nID;
					postLog( LogSeverity::Debug,
							 tr( "Detected an already existing service using the same URL." ) +
							 tr( "Removing that service before adding a new one." ) );
				}
			}

			pService->m_oRWLock.unlock();

			if ( !bReturn )
				doRemove( nID );

			break; // There can only be one service with the same URL, so we needn't search further.
		}
		else
		{
			pService->m_oRWLock.unlock();
		}
	}

	return bReturn;
}

/**
 * @brief checkBan checks a given URL against the list of banned services.
 * Requires locking: YES
 * @param sURL
 * @return true if the URL is already present and has been banned; false otherwise.
 */
bool Manager::checkBan(QString sURL) const
{
	bool bReturn = false;

	foreach ( MapPair pair, m_mServices )
	{
		// already existing service
		ServicePtr pService = pair.second;
		pService->m_oRWLock.lockForRead();

		// check for banned services with the same URL
		// All service with type stBanned have m_bBanned set to true.
		if ( ( pService->m_bBanned /*|| pService->m_nServiceType == stBanned*/ ) &&
			 !pService->m_oServiceURL.toString().compare( sURL ) )
		{
			bReturn = true;

			pService->m_oRWLock.unlock();
			break; // There can only be one service with the same URL, so we needn't search further.
		}

		pService->m_oRWLock.unlock();
	}

	return bReturn;
}

/**
 * @brief manageDuplicates checks if an identical (or very similar) service is already present
 * in the manager, decides which service to keep, which service to update etc. Note that this does
 * not treat bans differently from any other service, so ban management needs to be done beforehand.
 * Requires locking: YES
 * @param pService
 * @return true if pService needs not to be added to the manager, false if it needs to be added.
 */
// TODO: Further improve this
// TODO: implement bans overwriting existing services
// TODO: lookup service domain and keep it updated when accessing service to detect duplicates
bool Manager::manageDuplicates(ServicePtr& pService)
{
	QString sURL = pService->m_oServiceURL.toString();

	pService->m_oRWLock.lockForWrite();

	foreach ( MapPair pair, m_mServices )
	{
		// already existing service
		ServicePtr pExService = pair.second;

		QReadLocker oExServiceReadLock( &pExService->m_oRWLock );
		QString sExURL = pExService->m_oServiceURL.toString();

		if ( *pService == *pExService )
		{
			postLog( LogSeverity::Debug,
					 tr( "Service already present. Not going to add duplicate." ) );

			pService->m_oRWLock.unlock();
			pService.clear();
			return true;
		}

		// check for services with the same URL; Note: all URLs should be case insesitive due to
		// normalizeURLs(), so case sensitivity is not a problem here.
		if ( !sExURL.compare( sURL ) )
		{
			// All service with type stBanned have m_bBanned set to true.
			if ( /*pExService->m_nServiceType == stBanned ||*/ pExService->m_bBanned )
			{
				// existing bans do override everything

				postLog( LogSeverity::Debug, tr( "URL already banned. Not going do add it." ) );
			}
			// All service with type stBanned have m_bBanned set to true.
			else if ( /*pService->m_nServiceType == stBanned ||*/ pService->m_bBanned )
			{
				oExServiceReadLock.unlock();

				// ban existing service
				pExService->m_oRWLock.lockForWrite();
				pExService->m_bBanned = true;
				pExService->m_oRWLock.unlock();

				postLog( LogSeverity::Debug,
						 tr( "Setting existing service with same URL to \"banned\"." ) );
			}
			else // neither service is banned
			{
				if ( pService->m_nServiceType == pExService->m_nServiceType )
				{
					// Make sure the existing service is set to handle the networks pService
					// handles, too.
					pExService->m_oNetworkType.setNetwork( pService->m_oNetworkType );

					postLog( LogSeverity::Debug,
							 tr( "Detected a duplicate service. Not going to add the new one." ) );
				}
				else
				{
					postLog( LogSeverity::Debug,
						 tr( "Detected a service of different type already using the same URL." ) );
				}
			}

			pService->m_oRWLock.unlock();
			pService.clear();
			return true;
		}

		// check for services with only part of the URL of an other service
		// this filters out "www.example.com"/"www.example.com/gwc.php" pairs
/*		if ( sExURL.startsWith( sURL ) )
		{
			if ( pExService->m_nServiceType == stBanned || pExService->m_bBanned )
			{
				// existing bans do override everything

				postLog( LogSeverity::Debug, tr( "URL already banned. Not going do add it." ) );
			}
			else if ( pService->m_nServiceType == stBanned || pService->m_bBanned )
			{
				oExServiceReadLock.unlock();

				// ban existing service
				pExService->m_oRWLock.lockForWrite();
				pExService->m_bBanned = true;
				pExService->m_oRWLock.unlock();

				postLog( LogSeverity::Debug,
						 tr( "Setting exinting service with same URL to \"banned\"." ) );
			}
			else
			{

			}
		}

		if ( sURL.startsWith( sExURL ) )
		{
			if ( pExService->m_nServiceType == stBanned || pExService->m_bBanned )
			{
				// existing bans do override everything

				postLog( LogSeverity::Debug, tr( "URL already banned. Not going do add it." ) );
			}
			else if ( pService->m_nServiceType == stBanned || pService->m_bBanned )
			{
				oExServiceReadLock.unlock();

				// ban existing service
				pExService->m_oRWLock.lockForWrite();
				pExService->m_bBanned = true;
				pExService->m_oRWLock.unlock();

				postLog( LogSeverity::Debug,
						 tr( "Setting exinting service with same URL to \"banned\"." ) );
			}
			else
			{

			}
		}*/
	}

	pService->m_oRWLock.unlock();
	return false;
}

/**
 * @brief normalizeURL transforms a given URL string into a standard form to ease the detection
 * of duplicates, filter out websites caching a service etc.
 * Requires locking: NO
 * @param sURL
 * @return true if sURL has been changed; false if sURL has not seen a change
 */
bool Manager::normalizeURL(QString& sURL)
{
	const QString sInput = sURL;

	// Check it has a valid protocol
	if ( sURL.startsWith( "http://" ) || sURL.startsWith("https://") )
	{
		sURL.remove( QRegularExpression( "/*$" ) );
		sURL.remove( QRegularExpression( "\\.nyud\\.net:[0-9]+" ) );

		// First check whether the URL can be parsed at all.
		QUrl oURL( sURL, QUrl::StrictMode );
		if ( !oURL.isValid() )
		{
			postLog( LogSeverity::Error,
					 tr( "Could not add invalid URL as a discovery service: " ) + sURL );
		}

		sURL = oURL.toString();
	}

	// stuff that's going to be supported in the future
//	else if ( sURL.startsWith( "gnutella1:host:" ) )
//	{
//	}
//	else if ( sURL.startsWith( "gnutella2:host:" ) )
//	{
//	}
//	else if ( sURL.startsWith( "uhc:" ) )
//	{
//	}
//	else if ( sURL.startsWith( "ukhl:" ) )
//	{
//	}
	else // not supported
	{
		sURL.clear();
	}

	return sInput != sURL;
}

/**
 * @brief getRandomService: Helper method. Allows to get a random service for a specified
 * network.
 * Requires locking: YES
 * @param oNType
 * @return A discovery service for the specified network; Null if no working service could be
 * found for the specified network.
 */
Manager::ServicePtr Manager::getRandomService(const CNetworkType& oNType)
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "CDiscovery::getRandomService( const CNetworkType: %1 )" ).arg(
			 oNType.toString() ), true );
#endif

	DiscoveryServicesList list;
	quint16 nTotalRating = 0;		// Used to store accumulative rating of all services
									// taken under consideration as return value.
	ServicePtr pService;
	const quint32 tNow = common::getTNowUTC();

	foreach ( MapPair pair, m_mServices )
	{
		pService = pair.second;

#if ENABLE_DISCOVERY_DEBUGGING
	QString s = QString( "[Discovery] Service : [%1:%2] %3"
						 ).arg( pService->type(),
								pService->m_oNetworkType.toString(),
								pService->m_oServiceURL.toString() );
	postLog( LogSeverity::Debug, s, true );
#endif

		pService->m_oRWLock.lockForRead();

		if ( pService->m_bBanned )
		{
			pService->m_oRWLock.unlock();
			continue; // skip banned services
		}

		bool bRatingEnabled = pService->m_nRating;

		if ( !bRatingEnabled )
		{
			if ( pService->m_tLastAccessed + quazaaSettings.Discovery.ZeroRatingRevivalInterval
				 > tNow )
			{
				pService->m_oRWLock.unlock();
				pService->m_oRWLock.lockForWrite();

				// Revive service
				pService->setRating( DISCOVERY_MAX_PROBABILITY );
				++pService->m_nZeroRevivals;
				bRatingEnabled = true;

				pService->m_oRWLock.unlock();
				pService->m_oRWLock.lockForRead();
			}
		}

		// Consider all services that...
		if ( pService->m_oNetworkType.isNetwork( oNType ) &&     // have the correct type
			 bRatingEnabled &&              // have a rating > 0 or are considered for revival
			 pService->m_tLastAccessed + quazaaSettings.Discovery.AccessThrottle
			 < tNow &&                      // are not recently used
			 !pService->m_bRunning )        // are not in use
		{
			list.push_back( pService );

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Service accepted!", true );
#endif

			nTotalRating += pService->m_nRating;
		}
		else
		{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Service rejected!", true );
#endif

			// We don't need to hold a lock to the services we're not going to use.
			pService->m_oRWLock.unlock();
		}
	}

	if ( list.empty() )
	{
		return ServicePtr();
	}
	else
	{
		// Make sure our selection is within [1 ; nTotalRating]
		quint16 nSelectedRating = ( qrand() % nTotalRating ) + 1;
		ListIterator   current  = list.begin();
		ServicePtr   pSelected  = *current;

		// Iterate threw list until the selected service has been found.
		while ( nSelectedRating > pSelected->m_nRating )
		{
			nSelectedRating -= pSelected->m_nRating;
			pSelected->m_oRWLock.unlock();
			pSelected = *( ++current );
		}

		while ( current != list.end() )
		{
			pService = *( current++ );
			pService->m_oRWLock.unlock();
		}

#if ENABLE_DISCOVERY_DEBUGGING
	QString s = QString( "[Discovery] Service selected: [%1:%2] %3"
						 ).arg( pSelected->type(),
								pSelected->m_oNetworkType.toString(),
								pSelected->m_oServiceURL.toString() );
	postLog( LogSeverity::Debug, s, true );
#endif

		return pSelected;
	}
}

/**
 * @brief postLog writes a message to the system log or to the debug output.
 * Requires locking: /
 * @param severity
 * @param message
 * @param bDebug Defaults to false. If set to true, the message is send  to qDebug() instead of
 * to the system log.
 */
void Manager::postLog(LogSeverity::Severity severity, QString message,
						 bool bDebug, ServiceID nID)
{
	QString sMessage;

	if ( nID )
	{
		sMessage += tr( "ID:" ) += QString::number( nID ) += " ";
	}

	switch ( severity )
	{
	case LogSeverity::Warning:
		sMessage += tr ( "Warning: " );
		break;

	case LogSeverity::Error:
		sMessage += tr ( "Error: " );
		break;

	case LogSeverity::Critical:
		sMessage += tr ( "Critical Error: " );
		break;

	default:
		break; // do nothing
	}

	sMessage += message;

	if ( bDebug )
	{
		sMessage = systemLog.msgFromComponent( Components::Discovery ) + sMessage;
		qDebug() << sMessage.toLocal8Bit().constData();
	}
	else
	{
		systemLog.postLog( severity, Components::Discovery, sMessage );
	}
}
