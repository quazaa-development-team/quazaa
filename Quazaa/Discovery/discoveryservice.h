/*
** discoveryservice.h
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

#ifndef DISCOVERYSERVICE_H
#define DISCOVERYSERVICE_H

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QUrlQuery>
#include <QDataStream>
#include <QReadWriteLock>
#include <QUrl>

//#include "debugrwlock.h"
#include "discovery.h"

// Requirements of all subclasses
#include "g2hostcache.h"
#include "commonfunctions.h"
#include "Misc/timedsignalqueue.h"
#include "quazaaglobals.h"

namespace Discovery
{

class DiscoveryService : public QObject
{
	Q_OBJECT

	/* ========================================================================================== */
	/* ======================================= Attributes ======================================= */
	/* ========================================================================================== */
protected:
	mutable QReadWriteLock m_oRWLock;      // Service access lock.
//	mutable CDebugRWLock   m_oRWLock;      // Enable this for debugging purposes.
	ServiceType::Type      m_nServiceType; // GWC, UKHL, ...
	NetworkType            m_oNetworkType; // could be several in case of GWC for instance
	QUrl                   m_oServiceURL;
	QUrl                   m_oRedirectUrl;
	QString                m_sPong;        // The service's reply to a ping request

	bool                   m_bQuery;       // last request was a query (false: [...]an update)
	bool                   m_bBanned;      // service URL is blocked

private:
	quint8          m_nRating;      // 0: bad; 10: very good
	quint8          m_nProbaMult;   // probability multiplicator: [0-5] based on rating

	bool            m_bZero;        // service probability has just been increased from zero or
	// service is new. On access failure, this service will be set
	// to 0 probability no matter its previous proba. For banned
	// hosts, this indicates the host has been banned because of too
	// many failures.
	ServiceID       m_nID;          // ID used by the manager to identify the service; 0:invalid

	quint16         m_nLastHosts;   // number of hosts returned by the service on last query
	quint32         m_nTotalHosts;  // all hosts we ever got from the service
	quint16         m_nAltServices; // alternate services known to the service passed on to us when
	// last we queried
	quint32         m_tLastAccessed;// last time we queried/updated the service
	// Note: for banned services, this holds the ban time
	quint32         m_tLastSuccess; // last time we accessed the service successfully
	quint8          m_nFailures;    // query failures in a row
	quint8          m_nZeroRevivals;// counts number of times this service has been revived from a
	// 0 rating.

	quint8          m_nRedirectCount;

	bool            m_bRunning;     // service is currently doing network communication

	QUuid           m_oSQCancelRequestID; // ID of cancel request (signal queue)

	/* ========================================================================================== */
	/* ====================================== Construction ====================================== */
	/* ========================================================================================== */
public:
	/**
	 * @brief CDiscoveryService: Constructor. Creates a new service.
	 * @param oURL
	 * @param oNType
	 * @param nRating
	 */
	DiscoveryService( const QUrl& oURL, const NetworkType& oNType,
					  quint8 nRating = DISCOVERY_MAX_PROBABILITY );

	/**
	 * @brief CDiscoveryService: Copy constructor. Copies all but the list of registered pointers.
	 * @param pService
	 */
	DiscoveryService( const DiscoveryService& pService );

	/**
	 * @brief ~CDiscoveryService
	 */
	virtual ~DiscoveryService(); /** Must be implemented by subclasses. */

private:
	/* ========================================================================================== */
	/* ======================================= Operators  ======================================= */
	/* ========================================================================================== */
	/**
	 * @brief operator ==: Allows to compare two services. Services are considered to be equal if
	 * they are of the same service type, serve the same networks and have the same URL.
	 * Requires locking: R (both services)
	 * @param pService
	 * @return
	 */
	virtual bool	operator==( const DiscoveryService& pService ) const;

	/**
	 * @brief operator !=: Allows to compare two services. Services are considered to be equal if
	 * they are of the same service type, serve the same networks and have the same URL.
	 * Requires locking: R (both services)
	 * @param pService
	 * @return
	 */
	bool			operator!=( const DiscoveryService& pService ) const;

	/* ========================================================================================== */
	/* ===================================== Static Methods ===================================== */
	/* ========================================================================================== */
	/**
	 * @brief load reads a service from the provided QDataStream and creates a new Object from the
	 * data. Note that if a non-NULL pointer is given to the function, that object is deleted.
	 * Locking: / (static member)
	 * @param pService
	 * @param fsStream
	 * @param nVersion
	 */
	static void load( DiscoveryService*& pService, QDataStream& fsFile, const int nVersion );

	/**
	 * @brief save writes pService to given QDataStream.
	 * Locking: / (static member)
	 * @param pService
	 * @param fsFile
	 */
	static void save( const DiscoveryService* const pService, QDataStream& fsFile );

	/**
	 * @brief createService allows to create valid services. Note that new services are marked as
	 * having been zero so they will be downgraded fast if found non functional.
	 * Locking: / (static member)
	 * @param sURL
	 * @param eSType
	 * @param oNType
	 * @param nRating
	 * @return
	 */
	/** Must be modified when writing subclasses. */
	static DiscoveryService* createService( const QString& sURL, ServiceType::Type eSType,
											const NetworkType& oNType, quint8 nRating );

	/* ========================================================================================== */
	/* ======================================= Operations ======================================= */
	/* ========================================================================================== */
	/**
	 * @brief update sends our own IP to service if the service supports the operation (e.g. if it
	 * is a GWC).
	 * Locking: RW
	 * @param oOwnIP
	 */
	void update();

	/**
	 * @brief query accesses the service to recieve network hosts for initial connection and/or
	 * alternative service URLs.
	 * Locking: RW
	 */
	void query();

private slots:
	/**
	 * @brief cancelRequest stops any update or query operation currently in progress.
	 * Locking: RW
	 */
	void cancelRequest();
	void cancelRequest( bool );

signals:
	/**
	 * @brief updated informs the GUI about a noticable change in this service.
	 */
	void updated( ServiceID nID );

	/* ========================================================================================== */
	/* ==================================== Attribute Access ==================================== */
	/* ========================================================================================== */
public:
	/**
	 * @brief lockForRead allows a reader to lock this service for read from within a constant
	 * context.
	 * Sets locking: R
	 */
	void lockForRead() const;

	/**
	 * @brief unlock allows a reader to unlock this service after having finished the respective
	 * read operations.
	 */
	void unlock() const;

	/**
	 * @brief ServiceType::Type
	 * Requires locking: R
	 * @return
	 */
	inline ServiceType::Type serviceType() const;

	/**
	 * @brief networkType
	 * Requires locking: R
	 * @return
	 */
	inline NetworkType networkType() const;

	/**
	 * @brief type
	 * Requires locking: R
	 * @return a string representation of the full type of this service for usage in the GUI
	 */
	virtual QString type() const = 0; /** Must be implemented by subclasses. */

	/**
	 * @brief url
	 * Requires locking: R
	 * @return
	 */
	inline QString url() const;

	/**
	 * @brief ping
	 * Requires locking: R
	 * @return
	 */
	inline QString pong() const;

	/**
	 * @brief rating
	 * Requires locking: R
	 * @return
	 */
	inline quint8 rating() const;

	/**
	 * @brief isBanned
	 * Requires locking: R
	 * @return
	 */
	inline bool isBanned() const;


#if ENABLE_DISCOVERY_DEBUGGING
	/**
	 * @brief probaMult
	 * Requires locking: R
	 * @return
	 */
	inline quint8 probaMult() const;
#endif

	/**
	 * @brief id
	 * Requires locking: R
	 * @return
	 */
	inline ServiceID id() const;

	/**
	 * @brief lastHosts
	 * Requires locking: R
	 * @return
	 */
	inline quint32 lastHosts() const;

	/**
	 * @brief totalHosts
	 * Requires locking: R
	 * @return
	 */
	inline quint32 totalHosts() const;

	/**
	 * @brief altServices
	 * Requires locking: R
	 * @return
	 */
	inline quint16 altServices() const;

	/**
	 * @brief lastQueried
	 * Requires locking: R
	 * @return
	 */
	inline quint32 lastAccessed() const;

protected:
	/**
	 * @brief setLastQueried sets the lastQueried attribute to tNow
	 * Requires locking: RW
	 * @param tNow
	 */
	inline void setLastAccessed( quint32 tNow );

public:
	/**
	 * @brief lastSuccess
	 * Requires locking: R
	 * @return
	 */
	inline quint32 lastSuccess() const;

	/**
	 * @brief failures: Allows access to the number this service failed in a row. After a successful
	 * query or update, 0 should be returned.
	 * Requires locking: R
	 * @return
	 */
	inline quint8 failures() const;

	/**
	 * @brief isRunning
	 * Requires locking: R
	 * @return
	 */
	inline bool isRunning() const;

protected:
	/**
	 * @brief resetRunning sets the running boolean to false again.
	 * Requires locking: RW
	 */
	inline void resetRunning();

	/* ========================================================================================== */
	/* ============================= Specialized Attribute Setters  ============================= */
	/* ========================================================================================== */

	/**
	 * @brief updateStatistics: Updates statistics, failure counters etc.
	 * Requires locking: RW
	 * @param bCanceled
	 * @param nHosts
	 * @param nURLs
	 * @param bUpdateOK
	 */
	void updateStatistics( bool bCanceled, quint16 nHosts = 0, quint16 nURLs = 0,
						   bool bUpdateOK = false );

private:
	/**
	 * @brief setRating: Helper method to set rating and probability multiplicator simultaneously.
	 * Requires locking: RW
	 * @param nRating
	 */
	void setRating( quint8 nRating );

	/* ========================================================================================== */
	/* =================================== Protected Helpers  =================================== */
	/* ========================================================================================== */
protected:
	/**
	 * @brief postLog writes a message to the system log or to the debug output.
	 * Requires locking: /
	 * @param severity
	 * @param message
	 * @param bDebug Defaults to false. If set to true, the message is send  to qDebug() instead of
	 * to the system log.
	 */
	void postLog( LogSeverity severity, QString message, bool bDebug = false );

	/**
	 * @brief handleRedirect detects redirects on network requests and follows them.
	 * @param pReply : the server reply
	 * @param pRequest : the old request - will be changed if redirection is detected
	 * @return true on redirect; false otherwise
	 */
	bool handleRedirect( QNAMPtr pNAMgr, QNetworkReply* pReply, QNetworkRequest*& pRequest );

	/* ========================================================================================== */
	/* ==================================== Private Virtuals ==================================== */
	/* ========================================================================================== */
private:
	/**
	 * @brief doQuery: Helper method for query(). Performs the actual querying.
	 * Note: Make sure to call this only after having assured the service io currently not running.
	 * Requires locking: RW
	 */
	virtual void doQuery()  throw() = 0;	/** Must be implemented by subclasses. */

	/**
	 * @brief doUpdate: Helper method for update(). Performs the actual updating.
	 * Note: Make sure to call this only after having assured the service io currently not running.
	 * Requires locking: RW
	 */
	virtual void doUpdate() throw() = 0;	/** Must be implemented by subclasses. */

	/**
	 * @brief doCancelRequest: Helper method for cancelRequest(). Performs the actual cancelling.
	 * Requires locking: RW
	 */
	virtual void doCancelRequest() throw() = 0;	/** Must be implemented by subclasses. */

	/* ========================================================================================== */
	/* ===================================== Friend Classes ===================================== */
	/* ========================================================================================== */
	friend class Manager;
};

ServiceType::Type DiscoveryService::serviceType() const
{
	return m_nServiceType;
}

NetworkType DiscoveryService::networkType() const
{
	return m_oNetworkType;
}

QString DiscoveryService::url() const
{
	return m_oServiceURL.toString();
}

QString DiscoveryService::pong() const
{
	return m_sPong;
}

quint8 DiscoveryService::rating() const
{
	return m_nRating;
}

bool DiscoveryService::isBanned() const
{
	return m_bBanned;
}

#if ENABLE_DISCOVERY_DEBUGGING
quint8 DiscoveryService::probaMult() const
{
	return m_nProbaMult;
}
#endif

ServiceID DiscoveryService::id() const
{
	return m_nID;
}

quint32 DiscoveryService::lastHosts() const
{
	return m_nLastHosts;
}

quint32 DiscoveryService::totalHosts() const
{
	return m_nTotalHosts;
}

quint16 DiscoveryService::altServices() const
{
	return m_nAltServices;
}

quint32 DiscoveryService::lastAccessed() const
{
	return m_tLastAccessed;
}

void DiscoveryService::setLastAccessed( quint32 tNow )
{
	m_tLastAccessed = tNow;
}

quint32 DiscoveryService::lastSuccess() const
{
	return m_tLastSuccess;
}

quint8 DiscoveryService::failures() const
{
	return m_nFailures;
}

bool DiscoveryService::isRunning() const
{
	return m_bRunning;
}

void DiscoveryService::resetRunning()
{
	m_bRunning = false;
}
}

#endif // DISCOVERYSERVICE_H
