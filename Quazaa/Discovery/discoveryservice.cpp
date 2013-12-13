/*
** discoveryservice.cpp
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

#include "discoveryservice.h"
#include "gwc.h"
#include "banneddiscoveryservice.h"

#include "quazaasettings.h"

using namespace Discovery;

/**
 * @brief CDiscoveryService: Constructor. Creates a new service.
 * @param oURL
 * @param oNType
 * @param nRating
 */
DiscoveryService::DiscoveryService(const QUrl& oURL, const CNetworkType& oNType, quint8 nRating) :
	m_nServiceType( ServiceType::Null ),
	m_oNetworkType( oNType ),
	m_oServiceURL( oURL ),
	m_bQuery( true ),
	m_bBanned( false ),
	m_nRating( nRating ),
	m_nProbaMult( ( nRating > DISCOVERY_MAX_PROBABILITY ) ? DISCOVERY_MAX_PROBABILITY : nRating ),
	m_bZero( true ),    // mark service as having been zero -> gets downrated fast if non functional
	m_nID( 0 ),         // invalid ID
	m_nLastHosts( 0 ),
	m_nTotalHosts( 0 ),
	m_nAltServices( 0 ),
	m_tLastAccessed( 0 ),
	m_tLastSuccess( 0 ),
	m_nFailures( 0 ),
	m_nZeroRevivals( 0 ),
	m_bRunning( false )
{
}

/**
 * @brief CDiscoveryService: Copy constructor. Copies all but the list of registered pointers.
 * @param pService
 */
DiscoveryService::DiscoveryService(const DiscoveryService& pService) :
	QObject(),
	m_bRunning( false )
{
	// The usage of a custom copy constructor makes sure the list of registered
	// pointers is NOT forwarded to a copy of this service.

	m_nServiceType  = pService.m_nServiceType;
	m_oNetworkType  = pService.m_oNetworkType;
	m_oServiceURL   = pService.m_oServiceURL;
	m_sPong         = pService.m_sPong;
	m_nRating       = pService.m_nRating;
	m_bBanned       = pService.m_bBanned;
	m_bZero         = pService.m_bZero;
	m_nID           = pService.m_nID;
	m_nLastHosts    = pService.m_nLastHosts;
	m_nTotalHosts   = pService.m_nTotalHosts;
	m_nAltServices  = pService.m_nAltServices;
	m_tLastAccessed = pService.m_tLastAccessed;
	m_tLastSuccess  = pService.m_tLastSuccess;
	m_nFailures     = pService.m_nFailures;

	m_nZeroRevivals = pService.m_nZeroRevivals;
	m_nProbaMult    = pService.m_nProbaMult;
}

/**
 * @brief ~CDiscoveryService
 */
DiscoveryService::~DiscoveryService()
{

}

/**
 * @brief operator ==: Allows to compare two services. Services are considered to be equal if
 * they are of the same service type, serve the same networks and have the same URL.
 * Requires locking: R (both services)
 * @param pService
 * @return
 */
bool DiscoveryService::operator==(const DiscoveryService& pService) const
{
	return ( m_nServiceType == pService.m_nServiceType &&
			 m_oNetworkType == pService.m_oNetworkType &&
			 m_oServiceURL  == pService.m_oServiceURL  );
}

/**
 * @brief operator !=: Allows to compare two services. Services are considered to be equal if
 * they are of the same service type, serve the same networks and have the same URL.
 * Requires locking: R (both services)
 * @param pService
 * @return
 */
bool DiscoveryService::operator!=(const DiscoveryService& pService) const
{
	return !( *this == pService );
}

/**
 * @brief load reads a service from the provided QDataStream and creates a new Object from the
 * data. Note that if a non-NULL pointer is given to the function, that object is deleted.
 * Locking: / (static member)
 * @param pService
 * @param fsStream
 * @param nVersion
 */
void DiscoveryService::load(DiscoveryService*& pService, QDataStream &fsFile, int)
{
	quint8     nServiceType;        // GWC, UKHL, ...
	quint16    nNetworkType;        // could be several in case of GWC for instance
	QString    sURL;
	QString    sPong;               // answer to ping
	quint8     nRating;             // 0: bad; 10: very good
	bool       bBanned;             // service URL is blocked
	bool       bZero;
	ServiceID nID;                 // ID used by the manager to identify the service
	quint16    nLastHosts;          // hosts returned by the service on last query
	quint32    nTotalHosts;         // all hosts we ever got from the service
	quint16    nAltServices;        // number of URLs we got on query
	quint32    tLastAccessed;       // last time we accessed the host
	quint32    tLastSuccess;        // last time we queried the service successfully
	quint8     nFailures;
	quint8     nZeroRatingFailures;

	fsFile >> nServiceType;
	fsFile >> nNetworkType;
	fsFile >> sURL;
	fsFile >> sPong;
	fsFile >> nRating;
	fsFile >> bBanned;
	fsFile >> bZero;
	fsFile >> nID;
	fsFile >> nLastHosts;
	fsFile >> nTotalHosts;
	fsFile >> nAltServices;
	fsFile >> tLastAccessed;
	fsFile >> tLastSuccess;
	fsFile >> nFailures;
	fsFile >> nZeroRatingFailures;

	pService = createService( sURL, (ServiceType::Type)nServiceType,
							  CNetworkType( nNetworkType ), nRating );

	if ( pService )
	{
		pService->m_sPong         = sPong;
		pService->m_bBanned       = bBanned;
		pService->m_bZero         = bZero;
		pService->m_nID           = nID;
		pService->m_nLastHosts    = nLastHosts;
		pService->m_nTotalHosts   = nTotalHosts;
		pService->m_nAltServices  = nAltServices;
		pService->m_tLastAccessed = tLastAccessed;
		pService->m_tLastSuccess  = tLastSuccess;
		pService->m_nFailures     = nFailures;
		pService->m_nZeroRevivals = nZeroRatingFailures;

#if ENABLE_DISCOVERY_DEBUGGING
		QString s = QString( "Rating: " )         + QString::number( pService->m_nRating ) +
					QString( " Multiplicator: " ) + QString::number( pService->m_nProbaMult );
		pService->postLog( LogSeverity::Debug, s, true );
#endif
	}
}

/**
 * @brief save writes pService to given QDataStream.
 * Locking: / (static member)
 * @param pService
 * @param fsFile
 */
void DiscoveryService::save(const DiscoveryService* const pService, QDataStream &fsFile)
{
	fsFile << (quint8)(pService->m_nServiceType);
	fsFile << (quint16)(pService->m_oNetworkType.toQuint16());
	fsFile << pService->m_oServiceURL.toString();
	fsFile << pService->m_sPong;
	fsFile << (quint8)(pService->m_nRating);
	fsFile << pService->m_bBanned;
	fsFile << pService->m_bZero;
	fsFile << pService->m_nID;
	fsFile << pService->m_nLastHosts;
	fsFile << pService->m_nTotalHosts;
	fsFile << pService->m_nAltServices;
	fsFile << pService->m_tLastAccessed;
	fsFile << pService->m_tLastSuccess;
	fsFile << pService->m_nFailures;
	fsFile << pService->m_nZeroRevivals;
}

/**
 * @brief createService allows to create valid services.
 * Locking: / (static member)
 * @param sURL
 * @param eSType
 * @param oNType
 * @param nRating
 * @return
 */
DiscoveryService* DiscoveryService::createService(const QString& sURL, ServiceType::Type eSType,
													const CNetworkType& oNType,	quint8 nRating )
{
	DiscoveryService* pService = NULL;

	switch ( eSType )
	{
	case ServiceType::Null:
#if ENABLE_DISCOVERY_DEBUGGING
		qDebug() << "[Discovery] Service Type: Null";
#endif
		break;

	case ServiceType::Banned:
	{
#if ENABLE_DISCOVERY_DEBUGGING
		qDebug() << "[Discovery] Service Type: Banned";
#endif
		pService = new BannedDiscoveryService( sURL, oNType, nRating );
		break;
	}

	case ServiceType::GWC:
	{
#if ENABLE_DISCOVERY_DEBUGGING
		qDebug() << "[Discovery] Service Type: GWC";
#endif
		pService = new GWC( sURL, oNType, nRating );
		break;
	}

	default:
#if ENABLE_DISCOVERY_DEBUGGING
		qDebug() << "[Discovery] Service Type: Unknown";
#endif
		systemLog.postLog( LogSeverity::Error, Components::Discovery,
						   tr( "Internal error: Creation of service with unknown type requested: Type " )
						   + QString( eSType ) );

		Q_ASSERT( false ); // unsupported service type
	}

	return pService;
}

/**
 * @brief update sends our own IP to service if the service supports the operation (e.g. if it
 * is a GWC).
 * Locking: RW
 * @param oOwnIP
 */
void DiscoveryService::update()
{
	m_oRWLock.lockForWrite();

	Q_ASSERT( !m_bRunning );

	m_bRunning = true;
	m_bQuery   = false;

	doUpdate();

	m_oRWLock.unlock();

	m_oSQCancelRequestID = signalQueue.push( this, "cancelRequest",
											 quazaaSettings.Discovery.ServiceTimeout );

	emit updated( m_nID ); // notify GUI
}

/**
 * @brief query accesses the service to recieve network hosts for initial connection and/or
 * alternative service URLs.
 * Locking: RW
 */
void DiscoveryService::query()
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Querying service.", true );
#endif

	m_oRWLock.lockForWrite();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Got service lock.", true );
#endif

	Q_ASSERT( !m_bRunning );

	m_bRunning = true;
	m_bQuery   = true;

	doQuery();

	m_oRWLock.unlock();

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Released service lock.", true );
#endif

	m_oSQCancelRequestID = signalQueue.push( this, "cancelRequest",
											 quazaaSettings.Discovery.ServiceTimeout );

	emit updated( m_nID ); // notify GUI

#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, "Finished querying service.", true );
#endif
}

/**
 * @brief cancelRequest stops any update or query operation currently in progress.
 * Locking: RW
 */
void DiscoveryService::cancelRequest()
{
	cancelRequest( false );
}
void DiscoveryService::cancelRequest(bool bKeepLocked)
{
	m_oRWLock.lockForWrite();

	if ( m_bRunning )
	{
		doCancelRequest();
		updateStatistics( true ); // also removes cancel request from signal queue if still in there
	}

	if ( !bKeepLocked )
		m_oRWLock.unlock();
}

/**
 * @brief lockForRead allows a reader to lock this service for read from within a constant
 * context.
 * Sets locking: R
 */
void DiscoveryService::lockForRead() const
{
	m_oRWLock.lockForRead();
}

/**
 * @brief unlock allows a reader to unlock this service after having finished the respective
 * read operations.
 */
void DiscoveryService::unlock() const
{
	m_oRWLock.unlock();
}

/**
 * @brief updateStatistics updates statistics, failure counters etc.
 * Requires locking: RW
 * @param nHosts
 */
void DiscoveryService::updateStatistics(bool bCanceled, quint16 nHosts, quint16 nURLs,
										 bool bUpdateOK)
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug,
			 QString( "Updating Statistics: Query %1, Hosts %2, URLs %3, UpdateOK %4"
					  ).arg( QString::number( m_bQuery ), QString::number( nHosts ),
							 QString::number( nURLs ),    QString::number( bUpdateOK ) ), true );
#endif

	postLog( LogSeverity::Debug, tr( "Updating statistics." ), true );

	if ( bCanceled )
	{
		Q_ASSERT( !m_oSQCancelRequestID.isNull() );
	}
	else
	{
		// remove cancel request from signal queue
		Q_ASSERT( signalQueue.pop( m_oSQCancelRequestID ) );
	}
	m_oSQCancelRequestID = QUuid();

	if ( m_bQuery || nHosts )//in case of an update, we still count hosts we got but did not request
		m_nLastHosts = nHosts;

	if ( m_bQuery || nURLs ) //same as above
		m_nAltServices = nURLs;

	m_nTotalHosts += nHosts;

	if ( ( m_bQuery && nHosts ) || ( !m_bQuery && bUpdateOK ) ) // access successful
	{
		// increase rating
		setRating( m_nRating + 1 );

		m_tLastSuccess = m_tLastAccessed;
		m_nFailures = 0;

		// eventual revival try successful
		m_bZero = false;
		m_nZeroRevivals = 0;
	}
	else // fail
	{
#if ENABLE_DISCOVERY_NAM
		// Check network connected status and skip this if network is not connected
		QSharedPointer<QNetworkAccessManager> pNAM = discoveryManager.requestNAM();

		if ( pNAM->networkAccessible() == QNetworkAccessManager::Accessible )
		{
			++m_nFailures;

			if ( m_bZero ) // We're dealing with a newly revived service
			{              // that has been known to fail in the past.
				// revival failed
				setRating( 0 );

				if ( m_nZeroRevivals >= quazaaSettings.Discovery.ZeroRatingRevivalTries )
				{
					// Maximum allowed revival tries reached. Assume service no longer operational.
					m_bBanned = true;
				}
			}
			else
			{
				if ( quazaaSettings.Discovery.FailureLimit &&
					 m_nFailures >= quazaaSettings.Discovery.FailureLimit )
				{
					// maximum allowed failures reached
					setRating( 0 );
				}
				else
				{
					// decrease rating by 1
					setRating( (m_nRating > 0) ? m_nRating - 1 : 0 );
				}

				if ( !m_nRating )
				{
					m_bZero = true;

					// if no retrial at a later time is wanted, ban the service for good
					if ( !quazaaSettings.Discovery.ZeroRatingRevivalTries )
					{
						m_bBanned = true;
					}
				}
			}
		}
#endif // ENABLE_DISCOVERY_NAM
	}

	if ( discoveryManager.m_pActive[m_nServiceType] )
		--discoveryManager.m_pActive[m_nServiceType];
	else
		Q_ASSERT( false );

	emit updated( m_nID );
}

/**
 * @brief setRating: Helper method to set rating and probability multiplicator simultaneously.
 * Requires locking: RW
 * @param nRating
 */
void DiscoveryService::setRating(quint8 nRating)
{
	m_nRating    = ( nRating > quazaaSettings.Discovery.MaximumServiceRating ) ?
					   quazaaSettings.Discovery.MaximumServiceRating : nRating;
	m_nProbaMult = ( nRating > DISCOVERY_MAX_PROBABILITY ) ? DISCOVERY_MAX_PROBABILITY : nRating;
}

/**
 * @brief postLog writes a message to the system log or to the debug output.
 * Requires locking: /
 * @param severity
 * @param message
 * @param bDebug Defaults to false. If set to true, the message is send  to qDebug() instead of
 * to the system log.
 */
void DiscoveryService::postLog(LogSeverity::Severity severity, QString message, bool bDebug)
{
	Manager::postLog( severity, message, bDebug, m_nID );
}
