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
Manager::Manager( QObject* parent ) :
	QObject( parent ),
	m_bSaved( true ),
	m_bRunning( 0 ),
	m_pActive( new QAtomicInt[Discovery::ServiceType::NoOfTypes] )
{
	for ( quint8 i = 0; i < ServiceType::NoOfTypes; ++i )
	{
		m_pActive[i].store( 0 );
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

	// Note: m_pIDProvider is managed by HostCache
}

/**
 * @brief count allows you to access the number of working services for a given network.
 * Locking: YES (synchronous)
 * @return the number of services for the specified network. If no type is specified or the type
 * is null, the total number of all services is returned, no matter whether they are working or
 * not.
 */
quint32	Manager::count( const NetworkType& oType )
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
	// initialize meta methods for for faster asynchronous invoking
	const QMetaObject* pMetaObject = metaObject();
	QByteArray         sNormalizedSignature;
	int                nMethodIndex;

	sNormalizedSignature              = "asyncSyncSavingHelper()";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncSyncSavingHelper         = pMetaObject->method( nMethodIndex );

	sNormalizedSignature              = "asyncRequestServiceListHelper()";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncRequestServiceListHelper = pMetaObject->method( nMethodIndex );

	sNormalizedSignature              = "asyncUpdateServiceHelper(NetworkType)";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncUpdateServiceHelper      = pMetaObject->method( nMethodIndex );

	sNormalizedSignature              = "asyncUpdateServiceHelper(ServiceID)";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncUpdateServiceHelperByID  = pMetaObject->method( nMethodIndex );

	sNormalizedSignature              = "asyncQueryServiceHelper(NetworkType)";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncQueryServiceHelper       = pMetaObject->method( nMethodIndex );

	sNormalizedSignature              = "asyncQueryServiceHelper(ServiceID)";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncQueryServiceHelperByID   = pMetaObject->method( nMethodIndex );

	sNormalizedSignature              = "asyncManageDuplicatesHelper(ServiceID)";
	nMethodIndex                      = pMetaObject->indexOfMethod( sNormalizedSignature );
	m_pfAsyncManageDuplicatesHelper   = pMetaObject->method( nMethodIndex );

#ifdef _DEBUG
	Q_ASSERT( m_pfAsyncSyncSavingHelper.isValid() );
	Q_ASSERT( m_pfAsyncRequestServiceListHelper.isValid() );
	Q_ASSERT( m_pfAsyncUpdateServiceHelper.isValid() );
	Q_ASSERT( m_pfAsyncUpdateServiceHelperByID.isValid() );
	Q_ASSERT( m_pfAsyncQueryServiceHelper.isValid() );
	Q_ASSERT( m_pfAsyncQueryServiceHelperByID.isValid() );
	Q_ASSERT( m_pfAsyncManageDuplicatesHelper.isValid() );
#endif // _DEBUG

	// assure worker thread validity
	Q_ASSERT( hostCache.m_pHostCacheDiscoveryThread );
	Q_ASSERT( hostCache.m_pHostCacheDiscoveryThread->isRunning() );

	// move to worker thread
	m_pHostCacheDiscoveryThread = hostCache.m_pHostCacheDiscoveryThread;
	moveToThread( m_pHostCacheDiscoveryThread );

	// use the same ID provider for Host Cache and Discovery
	m_pIDProvider = &hostCache.m_oIDProvider;

	QMetaObject::invokeMethod( this, "startInternal", Qt::QueuedConnection );
}

/**
 * @brief stop prepares the Discovery Services Manager for destruction.
 * Locking: YES (synchronous)
 * @return true if the services have been successfully written to disk.
 */
bool Manager::stop()
{
	m_bRunning.store( 0 );

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
bool Manager::save( bool bForceSaving )
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

#ifdef _DEBUG
			bool bOK =
#endif // _DEBUG
			m_pfAsyncSyncSavingHelper.invoke( this, Qt::QueuedConnection );
#ifdef _DEBUG
			Q_ASSERT( bOK );
#endif // _DEBUG

			// We do not know whether it was successful,
			// so pretend it was not to be on the safe side.
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
ServiceID Manager::add( QString sURL, const ServiceType::Type eSType,
						const NetworkType& oNType, const quint8 nRating )
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

	if ( checkBan( sURL ) )
	{
		m_pSection.unlock();
		postLog( LogSeverity::Warning, tr( "Service URL banned." ) );
		return 0;
	}
	else if ( eSType == ServiceType::Banned )
	{
		if ( manageBan( sURL, eSType, oNType ) )
		{
			m_pSection.unlock();
			return 0;
		}
	}
	else if ( normalizeURL( sURL ) ) // true if URL changes in the process
	{
		if ( checkBan( sURL ) )
		{
			m_pSection.unlock();
			postLog( LogSeverity::Warning, tr( "Service URL banned." ) );
			return 0;
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
			const ServiceID nTmp = pService->m_nID;

			m_pSection.unlock();

#if ENABLE_DISCOVERY_DEBUGGING
			postLog( LogSeverity::Debug, QString( "New discovery service added: " ) + sURL, true );
#endif

			postLog( LogSeverity::Notice, tr( "New discovery service added: " ) + sURL );

			qDebug() << "Service added; ID: "
					 << QString::number( pService->id() ).toLocal8Bit().data();

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
bool Manager::remove( ServiceID nID )
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
void Manager::clear( bool bInformGUI )
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
bool Manager::check( const ConstServicePtr pService )
{
	QMutexLocker l( &m_pSection );

	ConstIterator iService = m_mServices.find( pService->m_nID );

	if ( iService == m_mServices.end() )
	{
		return false;    // Unable to find service by ID
	}

	const ConstServicePtr pExService = ( *iService ).second;

	if ( *pExService == *pService )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief initiateSearchForDuplicates schedules a check for services identical to the service
 * specified by the given ID.
 * Locking: YES (asynchronous)
 * @param nID : the service ID
 */
void Manager::initiateSearchForDuplicates( ServiceID nID )
{
#ifdef _DEBUG
	bool bOK =
#endif // _DEBUG
	m_pfAsyncManageDuplicatesHelper.invoke( this, Qt::QueuedConnection, Q_ARG( ServiceID, nID ) );
#ifdef _DEBUG
	Q_ASSERT( bOK );
#endif // _DEBUG
}

/**
 * @brief isActive allows to find out whether the Discovery Manager is currently active.
 * Locking: YES (synchronous)
 * @param eSType
 * @return true if active; false otherwise
 */
bool Manager::isActive( const ServiceType::Type eSType )
{
	return m_pActive[eSType].load() > 0;
}

/**
 * @brief isOperating allows to find out whether the Manager has finished starting up and not
 * started to shut down.
 * @return true if the Manager is operating; false if not finished starting up/started shutting
 * down
 */
bool Manager::isOperating()
{
	return m_bRunning.load() == 1;
}

/**
 * @brief requestNAMgr provides a shared pointer to the discovery services network
 * access manager. Note that the caller needs to hold his copy of the shared pointer until the
 * network operation has been completed to prevent the manager from being deleted too early.
 * Locking: YES (synchronous)
 * @return
 */
QNAMPtr Manager::requestNAM()
{
	m_pSection.lock();

	QNAMPtr pReturnVal = m_pNetAccessMgr.toStrongRef();
	if ( !pReturnVal )
	{
		// else create a new access manager (will be deleted if nobody is using it anymore)
		pReturnVal = QNAMPtr( new QNetworkAccessManager(), &QObject::deleteLater );
		m_pNetAccessMgr = pReturnVal.toWeakRef();

		// Make sure the networkAccessible state is properly initialized.
		QNetworkConfigurationManager manager;
		manager.updateConfigurations();
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
#ifdef _DEBUG
	bool bOK =
#endif // _DEBUG
	m_pfAsyncRequestServiceListHelper.invoke( this, Qt::QueuedConnection );
#ifdef _DEBUG
	Q_ASSERT( bOK );
#endif // _DEBUG
}

/**
 * @brief updateService updates a service for a given network type with our IP. Note that not
 * all service types might support or require such updates.
 * Locking: YES (asynchronous)
 * @param type
 */
void Manager::updateService( const NetworkType& type )
{
#ifdef _DEBUG
	bool bOK =
#endif // _DEBUG
	m_pfAsyncUpdateServiceHelper.invoke( this, Qt::QueuedConnection,
										 Q_ARG( const NetworkType, type ) );
#ifdef _DEBUG
	Q_ASSERT( bOK );
#endif // _DEBUG
}

void Manager::updateService( ServiceID nID )
{
#ifdef _DEBUG
	bool bOK =
#endif // _DEBUG
	m_pfAsyncUpdateServiceHelperByID.invoke( this, Qt::QueuedConnection,
											 Q_ARG( ServiceID, nID ) );
#ifdef _DEBUG
	Q_ASSERT( bOK );
#endif // _DEBUG
}

/**
 * @brief queryService queries a service for hosts to connect to.
 * Locking: YES (asynchronous)
 * @param type
 */
void Manager::queryService( const NetworkType& type )
{
#ifdef _DEBUG
	bool bOK =
#endif // _DEBUG
	m_pfAsyncQueryServiceHelper.invoke( this, Qt::QueuedConnection,
										Q_ARG( const NetworkType, type ) );
#ifdef _DEBUG
	Q_ASSERT( bOK );
#endif // _DEBUG
}

void Manager::queryService( ServiceID nID )
{
#ifdef _DEBUG
	bool bOK =
#endif // _DEBUG
	m_pfAsyncQueryServiceHelperByID.invoke( this, Qt::QueuedConnection, Q_ARG( ServiceID, nID ) );
#ifdef _DEBUG
	Q_ASSERT( bOK );
#endif // _DEBUG
}

/**
 * @brief getWorkingService
 * Locking: YES (synchronous)
 * @param type
 * @return the URL of a service of the requested type that is known to work
 */
QString Manager::getWorkingService( ServiceType::Type type )
{
	QMutexLocker l( &m_pSection );

	DiscoveryServicesList list;
	quint16 nTotalRating = 0;		// Used to store accumulative probability rating of all services
	// taken under consideration as return value.
	ServicePtr pService;

	for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
		  it != m_mServices.end(); ++it )
	{
		const MapPair& pair = *it;
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
				nTotalRating += pService->m_nProbaMult;
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
		while ( nSelectedRating > pSelected->m_nProbaMult )
		{
			nSelectedRating -= pSelected->m_nProbaMult;
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

/**
 * @brief postLog writes a message to the system log or to the debug output.
 * Requires locking: /
 * @param severity
 * @param message
 * @param bDebug Defaults to false. If set to true, the message is send  to qDebug() instead of
 * to the system log.
 */
void Manager::postLog( LogSeverity severity, QString message,
					   bool bDebug, ServiceID nID )
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
		sMessage = systemLog.msgFromComponent( Component::Discovery ) + sMessage;
		qDebug() << sMessage.toLocal8Bit().constData();
	}
	else
	{
		systemLog.postLog( severity, Component::Discovery, sMessage );
	}
}

/**
 * @brief writeToFile is a helper method for save()
 * Locking: YES (synchronous)
 * @param pManager
 * @param oFile
 * @return The number of services written to the specified file
 */
quint32 Manager::writeToFile( const void* const pManager, QFile& oFile )
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, QString( "Manager::writeToFile()" ) ), true );
#endif

	Manager* pDiscovery = ( Manager* )pManager;

	pDiscovery->m_pSection.lock();

	const quint16 nVersion = DISCOVERY_CODE_VERSION;
	const quint32 nCount   = pDiscovery->doCount();

	try
	{
		QDataStream fsFile( &oFile );

		fsFile << nVersion;
		fsFile << nCount;

		ServicePtr pService;

		// write services to stream
		for ( DiscoveryServicesMap::const_iterator it = pDiscovery->m_mServices.begin();
			  it != pDiscovery->m_mServices.end(); ++it )
		{
			const MapPair& pair = *it;
			pService = pair.second;
			pService->cancelRequest( true );
			DiscoveryService::save( pService.data(), fsFile );
			pService->unlock();
		}
	}
	catch ( ... )
	{
		pDiscovery->m_pSection.unlock();

		postLog( LogSeverity::Error,
				 tr( "Unspecified problem while writing discovery services to disk." ) );

		return 0;
	}

	pDiscovery->m_bSaved = true;
	pDiscovery->m_pSection.unlock();
	return nCount;
}

void Manager::startInternal()
{
	// initialize random number generator
	qsrand ( common::getTNowUTC() );

	// includes its own locking
	load();

	m_bRunning.store( 1 );
}

bool Manager::asyncSyncSavingHelper()
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "asyncSyncSavingHelper()", true );
#endif

	const quint32 nCount = common::securedSaveFile( QuazaaGlobals::DATA_PATH(), "discovery.dat",
													Component::Discovery, this,
													&Manager::writeToFile );

	postLog( LogSeverity::Debug, tr( "%1 services saved." ).arg( nCount ) );

	return nCount;
}

void Manager::asyncRequestServiceListHelper()
{
	m_pSection.lock();
	for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
		  it != m_mServices.end(); ++it )
	{
		const MapPair& pair = *it;
		emit serviceInfo( pair.second );
	}
	m_pSection.unlock();
}

void Manager::asyncUpdateServiceHelper( const NetworkType type )
{
	QSharedPointer<QNetworkAccessManager> pNAM = requestNAM();

	if ( pNAM->networkAccessible() == QNetworkAccessManager::Accessible )
	{
		m_pSection.lock();
		ServicePtr pService = getRandomService( type );
		m_pSection.unlock();

		if ( pService )
		{
			if ( pService.data()->isRunning() )
			{
				qDebug() << "*** Warning: Canceled currently running Discovery request for update!";
				pService.data()->cancelRequest();
			}

			postLog( LogSeverity::Notice, tr( "Updating service: " ) + pService->url() );

			m_pActive[pService.data()->m_nServiceType].fetchAndAddRelaxed( 1 );
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

void Manager::asyncUpdateServiceHelper( ServiceID nID )
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

	ServicePtr pService = ( *iService ).second;
	m_pSection.unlock();

	Q_ASSERT( pService ); // We should always get a valid service from the iterator

	if ( pService.data()->isRunning() )
	{
		qDebug() << "*** Warning: Canceled currently running Discovery request for update!";
		pService.data()->cancelRequest();
	}

	postLog( LogSeverity::Notice, tr( "Updating service: " ) + pService->url() );

	m_pActive[pService.data()->m_nServiceType].fetchAndAddRelaxed( 1 );
	pService->update();
}

void Manager::asyncQueryServiceHelper( const NetworkType type )
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

			if ( pService.data()->isRunning() )
			{
				qDebug() << "*** Warning: Canceled currently running Discovery request for query!";
				pService.data()->cancelRequest();
			}

			m_pActive[pService.data()->serviceType()].fetchAndAddRelaxed( 1 );
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

void Manager::asyncQueryServiceHelper( ServiceID nID )
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

	ServicePtr pService = ( *iService ).second;
	m_pSection.unlock();

	Q_ASSERT( pService ); // We should always get a valid service from the iterator

	if ( pService.data()->isRunning() )
	{
		qDebug() << "*** Warning: Canceled currently running Discovery request for query!";
		pService.data()->cancelRequest();
	}

	postLog( LogSeverity::Notice, tr( "Querying service: " ) + pService->url() );

	m_pActive[pService.data()->serviceType()].fetchAndAddRelaxed( 1 );
	pService->query();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Service Queried.", true );
#endif
}

void Manager::asyncManageDuplicatesHelper( ServiceID nID )
{
	manageDuplicates( ( *m_mServices.find( nID ) ).second, true );
}

/**
 * @brief doCount: Internal helper without locking. See count for documentation.
 */
quint32 Manager::doCount( const NetworkType& oType )
{
	if ( oType.isNull() )
	{
		return ( quint32 )m_mServices.size();
	}
	else
	{
		quint16 nCount = 0;
		ServicePtr pService;

		for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
			  it != m_mServices.end(); ++it )
		{
			const MapPair& pair = *it;
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
void Manager::doClear( bool bInformGUI )
{
	if ( bInformGUI )
	{
		for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
			  it != m_mServices.end(); ++it )
		{
			const MapPair& pair = *it;
			emit serviceRemoved( pair.second->m_nID ); // inform GUI

			// free GUI ID
			m_pIDProvider->release( pair.second->m_nID );
		}
	}

	m_mServices.clear();	// TServicePtr takes care of service deletion
}

/**
 * @brief doRemove: Internal helper without locking. See remove for documentation.
 */
bool Manager::doRemove( ServiceID nID )
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

	ServicePtr pService = ( *iService ).second;

	postLog( LogSeverity::Notice,
			 tr( "Removing discovery service: " ) + pService->m_oServiceURL.toString() );

	// stop it if necessary
	pService->cancelRequest();

	// remove it
	m_mServices.erase( nID );

	// Make sure to reassign the now unused ID.
	m_pIDProvider->release( nID );

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
	QString sPath = QuazaaGlobals::DATA_PATH() + "discovery.dat";

	if ( load( sPath ) )
	{
		postLog( LogSeverity::Debug, tr( "Loaded %1 discovery services from file: "
									   ).arg( QString::number( count() ) ) + sPath );
	}
	else // Unable to load default file. Switch to backup one instead.
	{
		sPath = QuazaaGlobals::DATA_PATH() + "discovery_backup.dat";

		postLog( LogSeverity::Warning,
				 tr( "Failed to load discovery services from primary file. Switching to backup: " )
				 + sPath );

		if ( load( sPath ) )
		{
			postLog( LogSeverity::Debug, tr( "Loaded %1 discovery services from backup file: "
										   ).arg( QString::number( count() ) ) + sPath );
		}
		else
		{
			postLog( LogSeverity::Warning, "Failed secondary attempt on loading services!" );
		}
	}

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

	emit loadingFinished();
}

bool Manager::load( QString sPath )
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Loading discovery services from file: " ) + sPath, true );
#endif

	QFile oFile( sPath );

	if ( ! oFile.open( QIODevice::ReadOnly ) )
	{
		return false;
	}

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
		{
			delete pService;
		}

		clear();
		oFile.close();

		postLog( LogSeverity::Error, tr( "Caught an exception while loading services from file!" ) );

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
bool Manager::add( ServicePtr& pService )
{
	// remove duplicates and handle bans
	if ( manageDuplicates( pService, false ) )
	{
		return false;
	}

	// assign ID to service
	pService->m_nID = m_pIDProvider->aquire();

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
	QString sPath = QDir::toNativeSeparators( QString( "%1/DefaultServices.dat" ).arg( qApp->applicationDirPath() ) );
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

		while ( !fsFile.atEnd() )
		{
			sLine = fsFile.readLine();

			if ( sLine.length() < 7 )	// Blank comment line
			{
				continue;
			}

			cType = sLine.at( 0 );
			sService = sLine.right( sLine.length() - 2 );
#if ENABLE_DISCOVERY_DEBUGGING
			bAdded = false;
#endif

			switch ( cType.toLatin1() )
			{
//			case '1':	// gnutella service
//				break;
			case '2':	// G2 service
#if ENABLE_DISCOVERY_DEBUGGING
				postLog( LogSeverity::Debug, "Parsing Default Service: G2", true );
				bAdded =
#endif
					add( sService, ServiceType::GWC,
						 NetworkType( DiscoveryProtocol::G2 ), DISCOVERY_MAX_PROBABILITY );
				break;

			case 'M':	// Multi-network service
#if ENABLE_DISCOVERY_DEBUGGING
				postLog( LogSeverity::Debug, "Parsing Default Service: Multi Network Cache", true );
				bAdded =
#endif
					add( sService, ServiceType::GWC,
						 NetworkType( DiscoveryProtocol::G2 ), DISCOVERY_MAX_PROBABILITY );
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
					add( sService, ServiceType::Banned, NetworkType( DiscoveryProtocol::None ), 0 );
				break;

			default:	// Comment line or unsupported
#if ENABLE_DISCOVERY_DEBUGGING
				postLog( LogSeverity::Debug, "Parsed comment line or unsupported service type.", true );
#endif
				break;
			}

#if ENABLE_DISCOVERY_DEBUGGING
			if ( bAdded )
			{
				postLog( LogSeverity::Debug, "Service added.", true );
			}
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
 * @brief manageBan handles the checking existing services with the same URL when adding a ban.
 * Requires locking: YES
 * @param sURL : the URL
 * @param eSType : the service type of the banned service
 * @param oNType : the network type of the banned service
 * @return true if the caller needs not to worry about adding the requested ban anymore
 */
bool Manager::manageBan( const QString& sURL, const ServiceType::Type eSType,
						 const NetworkType& oNType )
{
	bool bReturn = false;

	ServicePtr pService = lookupUrl( sURL );

	if ( pService )
	{
		// Note: All service with type stBanned also have m_bBanned set to true.
		// Note also that at the time this is called we have already checked for a duplicate bans.
		if ( eSType == pService->m_nServiceType )
		{
			pService->m_oRWLock.unlock();
			pService->m_oRWLock.lockForWrite();

			// Make sure the existing service is set to handle the networks pService handles, too.
			pService->m_oNetworkType.setNetwork( oNType );
			pService->m_bBanned = true;
			bReturn = true;

			postLog( LogSeverity::Debug,
					 tr( "Detected an already existing service using the same URL. " ) +
					 tr( "Setting that service to banned instead of adding a new one." ) );

			pService->m_oRWLock.unlock();
		}
		else
		{
			pService->m_oRWLock.unlock();

			doRemove( pService->m_nID );

			postLog( LogSeverity::Debug,
					 tr( "Detected an already existing service using the same URL." ) +
					 tr( "Removing that service before adding a new one." ) );
		}
	}

	return bReturn;
}

/**
 * @brief checkBan checks a given URL against the list of banned services.
 * Requires locking: YES
 * @param sURL : the URL
 * @return true if the URL is banned; false otherwise.
 */
bool Manager::checkBan( const QString& sURL ) const
{
	bool bReturn = false;

	ServicePtr pService = lookupUrl( sURL );

	if ( pService )
	{
		bReturn = pService->m_bBanned;

		pService->m_oRWLock.unlock();
	}

	return bReturn;
}

/**
 * @brief lookupUrl finds the service with the specified URL if such a service exists.
 * Requires locking: YES
 * @param sURL : the URL
 * @return the service; note that it is locked for read
 */
Manager::ServicePtr Manager::lookupUrl( const QString& sURL ) const
{
	for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
		  it != m_mServices.end(); ++it )
	{
		const MapPair& pair = *it;
		ServicePtr pService = pair.second;

		pService->m_oRWLock.lockForRead();

		if ( !sURL.compare( pService->m_oServiceURL.toString() ) ||
			 !sURL.compare( pService->m_oRedirectUrl.toString() ) )
		{
			return pService;
		}

		pService->m_oRWLock.unlock();
	}

	return ServicePtr();
}


/**
 * @brief lookupDuplicates hands the caller a list of possible duplicates of a specified service
 * Requires locking: YES
 * @param pService : the service
 * @return a list of pairs of duplicates and bool values; bool == true for dominating service
 */
/*std::list< std::pair<Manager::ServicePtr, bool> > Manager::lookupDuplicates(ServicePtr& pService)
{
	std::list< std::pair<Manager::ServicePtr, bool> > lResult;

	pService->m_oRWLock.lockForRead();
	const QString sURL      = pService->m_oServiceURL.toString();
	const QString sRedirect = pService->m_oRedirectUrl.toString();

	for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin;
		  it != m_mServices.end(); ++it )
	{
		const MapPair& pair = *it;
		// already existing service
		ServicePtr pExService = pair.second;

		pExService->m_oRWLock.lockForRead();
		const QString sExURL      = pExService->m_oServiceURL.toString();
		const QString sExRedirect = pExService->m_oRedirectUrl.toString();

		if ( !sURL.compare( sExURL )           || // url identical
			 !sRedirect.compare( sExRedirect ) || // redirection destination identical
			 !sRedirect.compare( sExURL )      || // the given service redirects to the existing one
			 sURL.startsWith( sExURL ) )          // shorter URL dominates
		{
			lResult.push_back( std::make_pair( pExService, true ) );
		}
		else if ( !sURL.compare( sExRedirect ) || // existing service redirects to this URL
				  sExURL.startsWith( sURL ) )     // existing URL is longer (e.g. dominated)
		{
			lResult.push_back( std::make_pair( pExService, false ) );
		}

		pExService->m_oRWLock.unlock();
	}

	pService->m_oRWLock.unlock();

	return lResult;
}*/

/**
 * @brief manageDuplicates checks if an identical (or very similar) service is (already) present
 * in the manager, decides which service to keep, which service to update etc. Note that this does
 * not treat bans differently from any other service, so ban management needs to be done beforehand.
 * Requires locking: YES
 * @param pService : the service to check
 * @param bNeedRemoval : false - pService has not yet been added; true - otherwise
 * @return true if pService needs not to be added to the manager, false if it needs to be added.
 */
bool Manager::manageDuplicates( ServicePtr& pService, bool bNeedRemoval )
{
	// there is only a valid ID for services that have already been added
	Q_ASSERT( bNeedRemoval == (bool)pService->m_nID );

	/*std::list< std::pair<Manager::ServicePtr, bool> > lDuplicates = lookupDuplicates( pService );


	pService->m_oRWLock.lockForRead();
	for ( std::list< std::pair<Manager::ServicePtr, bool> >::const_iterator i = lDuplicates.begin();
		  i != lDuplicates.end(); ++i )
	{
		ServicePtr pDuplicate = (*i).first;

		if ( (*i).second ) // pDuplicate dominates pService
		{
			bDelete
		}
		else // pService dominates pDuplicate
		{
			if ( bNeedRemoval )
			{
				postLog( LogSeverity::Debug,
						 tr( "Removing duplicate Service:" ) + pDuplicate->m_oServiceURL.toString() );

				pDuplicate->m_oRWLock.unlock();
				doRemove( pService->m_nID );
				pService.clear();
				return false;
			}
			else
			{
				postLog( LogSeverity::Debug,
						 tr( "Service already present. Not going to add duplicate." ) );

				pService->m_oRWLock.unlock();
				pService.clear();
				return false;
			}
		}
	}
	pService->unlock();

	if ( bDelete )
	{
		if ( bNeedRemoval )
		{
			postLog( LogSeverity::Debug,
					 tr( "Removing duplicate Service:" ) + pService->m_oServiceURL.toString() );

			pService->m_oRWLock.unlock();
			doRemove( pService->m_nID );
			pService.clear();
			return true;
		}
		else
		{
			postLog( LogSeverity::Debug,
					 tr( "Service already present. Not going to add duplicate." ) );

			pService->m_oRWLock.unlock();
			pService.clear();
			return true;
		}
	}*/



	pService->m_oRWLock.lockForWrite();
	QString sURL = pService->m_oServiceURL.toString();

	for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
		  it != m_mServices.end(); ++it )
	{
		const MapPair& pair = *it;
		// already existing service
		ServicePtr pExService = pair.second;

		QReadLocker oExServiceReadLock( &pExService->m_oRWLock );

		if ( *pService == *pExService )
		{
			postLog( LogSeverity::Debug,
					 tr( "Service already present. Not going to add duplicate." ) );

			pService->m_oRWLock.unlock();
			pService.clear();
			return true;
		}

		QString sExURL = pExService->m_oServiceURL.toString();

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
bool Manager::normalizeURL( QString& sURL )
{
	const QString sInput = sURL;

	// Check it has a valid protocol
	if ( sURL.startsWith( "http://" ) || sURL.startsWith( "https://" ) )
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
Manager::ServicePtr Manager::getRandomService( const NetworkType& oNType )
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "CDiscovery::getRandomService( const CNetworkType: %1 )" ).arg(
				 oNType.toString() ), true );
#endif

	DiscoveryServicesList list;
	quint16 nTotalRating = 0;		// Used to store accumulative probability rating of all services
	// taken under consideration as return value.
	ServicePtr pService;
	const quint32 tNow = common::getTNowUTC();

	for ( DiscoveryServicesMap::const_iterator it = m_mServices.begin();
		  it != m_mServices.end(); ++it )
	{
		const MapPair& pair = *it;
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
				pService->m_bZero = true;
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

			nTotalRating += pService->m_nProbaMult;
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
		while ( nSelectedRating > pSelected->m_nProbaMult )
		{
			nSelectedRating -= pSelected->m_nProbaMult;
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
