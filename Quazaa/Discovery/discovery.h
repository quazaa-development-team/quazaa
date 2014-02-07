/*
** discovery.h
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

#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QMutex>
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QString>
#include <QThread>

#include <map>
#include <list>

#include "systemlog.h"
#include "networktype.h"

// Increment this if there have been made changes to the way of storing discovery services.
#define DISCOVERY_CODE_VERSION	1
// History:
// 0 - Initial implementation
// 1 - Full implementation of GWC spec 2.0.

#define DISCOVERY_MAX_PROBABILITY 5

// Set this to 1 to enable additionnal debug output for the Discovery Manager.
#define ENABLE_DISCOVERY_DEBUGGING 0

// TODO: Implement rating based on successful connection to obtained hosts
// TODO: Implement quazaaSettings.Discovery.AccessThrottle
// TODO: Check if already active ( see method )
// TODO: Ask brov whether callback on unsuccessful service query shall be implemented
// TODO: http://www.tartc.ru/classes/gwc?ping=1&get=1&net=gnutella2&client=QAZB&version=0.1 fails
//       http://www.tartc.ru/classes/gwc/?ping=1&amp;get=1&amp;net=gnutella2&amp;client=QAZB&amp;version=0.1 works
// Line: <p>The document has moved <a href="http://www.tartc.ru/classes/gwc/?ping=1&amp;get=1&amp;net=gnutella2&amp;client=QAZB&amp;version=0.1">here</a>.</p>
//typedef QSharedPointer<QThread> SharedThreadPtr;

namespace Discovery
{
// Notes:
// 1. locking: The documentation of each method contains information on the locks accessed within
//             or required for calling the method. It also contains information on whether the
//             locked section resides inside the Discovery thread ("asynchronous") or the calling
//             thread ("synchronous").
// 2. times:   All times within the manager and its attached classes are UTC.
class DiscoveryService;

/**
 * @brief ServiceType::Type: Must be updated when implementing new subclasses of DiscoveryService.
 * Each subclass must implement (exactly) one ServiceType::Type.
 */
namespace ServiceType
{
	enum Type
	{
		Null = 0, Banned = 1, GWC = 2, NoOfTypes = 3
	};
}

/**
 * @brief DiscoveryID: ID type used to identify and manage discovery services. All IDs are
 * positive. 0 indicates an invalid ID.
 */
typedef quint16 ServiceID;

/**
 * @brief ConstServicePtr is a shared pointer (strong reference) for allowing classes external to
 * the manager to access information on a discovery service.
 */
typedef QSharedPointer< const DiscoveryService > ConstServicePtr;

class Manager : public QObject
{
	Q_OBJECT

private:
	/**
	 * @brief ServicePtr is a shared pointer class for internal use.
	 */
	typedef QSharedPointer< DiscoveryService > ServicePtr;

	/**
	 * @brief DiscoveryServicesMap: Used to store and retrieve services based on their ID.
	 */
	typedef std::map < ServiceID, ServicePtr  > DiscoveryServicesMap;

	/**
	 * @brief MapPair: pair of an ID and a discovery service pointer for use in the map.
	 */
	typedef std::pair< ServiceID, ServicePtr  > MapPair;

	/**
	 * @brief DiscoveryServicesList: Used to temorarily store services.
	 */
	typedef std::list < ServicePtr  > DiscoveryServicesList;

	/**
	 * @brief ConstIterator: Constant map iterator.
	 */
	typedef DiscoveryServicesMap::const_iterator ConstIterator;

	/**
	 * @brief Iterator: Modifiable map iterator
	 */
	typedef DiscoveryServicesMap::iterator Iterator;

	/**
	 * @brief ListIterator: Constant list iterator
	 */
	typedef DiscoveryServicesList::const_iterator ListIterator;

private:
	// handles web requests
	QWeakPointer<QNetworkAccessManager> m_pNetAccessMgr;

	// access lock
	QMutex                  m_pSection;

	// discovery service map
	DiscoveryServicesMap    m_mServices;

	// true if current Discovery Manager state has already been saved to file, false otherwise.
	bool                    m_bSaved;

	// last ID to assigned by the manager.
	ServiceID               m_nLastID;

	// thread used by the manager
	QThread*                m_pHostCacheDiscoveryThread;

public:
	quint16*                m_pActive;

	/* ========================================================================================== */
	/* ====================================== Construction ====================================== */
	/* ========================================================================================== */
public:
	/**
	 * @brief CDiscovery: Constructs a new discovery services manager.
	 * @param parent
	 */
	Manager(QObject* parent = NULL);

	/**
	 * @brief ~CDiscovery: Destructor. Make sure you have stopped the magager befor destroying it.
	 */
	~Manager();

public:
	/**
	 * @brief count allows you to access the number of working services for a given network.
	 * Locking: YES (synchronous)
	 * @return the number of services for the specified network. If no type is specified or the type
	 * is null, the total number of all services is returned, no matter whether they are working or
	 * not.
	 */
	quint32 count(const CNetworkType& oType = CNetworkType());

	/**
	 * @brief start initializes the Discovery Services Manager. Make sure this is called after
	 * QApplication is instantiated.
	 * Locking: YES
	 */
	void start();

	/**
	 * @brief stop prepares the Discovery Services Manager for destruction.
	 * Locking: YES (synchronous)
	 * @return true if the services have been successfully written to disk.
	 */
	bool stop();

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
	bool save(bool bForceSaving = false);

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
	ServiceID add(QString sURL, const ServiceType::Type eSType,
				  const CNetworkType& oNType, const quint8 nRating = DISCOVERY_MAX_PROBABILITY);

	/**
	 * @brief remove removes a service by ID.
	 * Locking: YES (synchronous)
	 * @param nID
	 * @return true if the removal was successful (e.g. the service could be found), false
	 * otherwise.
	 */
	bool remove(ServiceID nID);

	/**
	 * @brief clear removes all services from the manager.
	 * Locking: YES (synchronous)
	 * @param bInformGUI: Set this to true if the GUI shall be informed about the removal of the
	 * services. The default value is false, which represents the scenario on shutdown, where the
	 * GUI will be removed shortly anyway.
	 */
	void clear(bool bInformGUI = false);

	/**
	 * @brief check verifies whether the given service is managed by the manager.
	 * Locking: YES (synchronous)
	 * @return true if managed; false otherwise
	 */
	bool check(const ConstServicePtr pService);

	/**
	 * @brief isActive allows to find out whether the Discovery Manager is currently active.
	 * Locking: YES (synchronous)
	 * @param eSType
	 * @return true if active; false otherwise
	 */
	bool isActive(const ServiceType::Type eSType);

	/**
	 * @brief requestNAM provides a shared pointer to the discovery services network
	 * access manager. Note that the caller needs to hold his copy of the shared pointer until the
	 * network operation has been completed to prevent the manager from being deleted too early.
	 * Locking: YES (synchronous)
	 * @return
	 */
	QSharedPointer<QNetworkAccessManager> requestNAM();

	/**
	 * @brief requestServiceList can be used to obtain a complete list of all currently managed
	 * services. Connect to the serviceInfo() signal to recieve them.
	 * Locking: YES (asynchronous)
	 */
	void requestServiceList();

	/**
	 * @brief updateService updates a service for a given network type with our IP. Note that not
	 * all service types might support or require such updates.
	 * Locking: YES (asynchronous)
	 * @param type
	 */
	void updateService(const CNetworkType& type); // Random service access
	void updateService(ServiceID nID);           // Manual service access

	/**
	 * @brief queryService queries a service for hosts to connect to.
	 * Locking: YES (asynchronous)
	 * @param type
	 */
	void queryService(const CNetworkType& type); // Random service access
	void queryService(ServiceID nID);           // Manual service access

	/**
	 * @brief getWorkingService
	 * Locking: YES (synchronous)
	 * @param type
	 * @return the URL of a service of the requested type that is known to work
	 */
	QString getWorkingService(ServiceType::Type type);

public:
	/**
	 * @brief postLog writes a message to the system log or to the debug output.
	 * Requires locking: NO
	 * @param severity
	 * @param message
	 * @param bDebug Defaults to false. If set to true, the message is send  to qDebug() instead of
	 * to the system log.
	 */
	static void postLog(LogSeverity::Severity severity, QString message,
						bool bDebug = false, ServiceID nID = 0);

signals:
	/**
	 * @brief serviceAdded is emitted each time a new service is added to the manager.
	 * @param pService
	 */
	void serviceAdded(ConstServicePtr pService);

	/**
	 * @brief serviceRemoved is emitted (almost) each time a service is removed from the manager.
	 * @param nServiceID
	 */
	void serviceRemoved(ServiceID nServiceID);

	/**
	 * @brief serviceInfo is emitted once for each service on request via requestServiceList().
	 * @param pService
	 */
	void serviceInfo(ConstServicePtr pService);

	/**
	 * @brief loadingFinished is emitted after new rules have been loaded.
	 */
	void loadingFinished();

private slots:
	// All methods in this section are helpers to do certain tasks asynchronously. See their
	// respective callers for documentation.
	bool asyncSyncSavingHelper();
	void asyncRequestServiceListHelper();
	void asyncUpdateServiceHelper(const CNetworkType type);
	void asyncUpdateServiceHelper(ServiceID nID);
	void asyncQueryServiceHelper(const CNetworkType type);
	void asyncQueryServiceHelper(ServiceID nID);

private:
	/**
	 * @brief doCount: Internal helper without locking. See count for documentation.
	 */
	quint32 doCount(const CNetworkType& oType = CNetworkType());

	/**
	 * @brief doClear: Internal helper without locking. See clear for documentation.
	 */
	void doClear(bool bInformGUI = false);

	/**
	 * @brief doRemove: Internal helper without locking. See remove for documentation.
	 */
	bool doRemove(ServiceID nID);

	/**
	 * @brief load retrieves stored services from the HDD.
	 * Locking: YES (synchronous)
	 * @return true if loading from file was successful; false otherwise.
	 */
	void load();
	bool load(QString sPath);

	/**
	 * @brief add... obvious... Note: if a duplicate is detected, the CDiscoveryService passed to
	 * the method is deleted within.
	 * Requires locking: YES
	 * @param pService
	 * @return true if the service was added; false if not (e.g. duplicate was detected).
	 */
	bool add(ServicePtr& pService);

	/**
	 * @brief addDefaults loads the DefaultServices.dat file (compatible with Shareaza) into the
	 * manager.
	 * Locking: YES (synchronous)
	 */
	void addDefaults();

	/**
	 * @brief manageBan handles the checking for duplicates when adding a new ban.
	 * Requires locking: YES
	 * @param sURL
	 * @param eSType
	 * @param oNType
	 * @return true if the caller needs not to worry about adding the banned service anymore
	 */
	bool manageBan(QString& sURL, const ServiceType::Type eSType, const CNetworkType& oNType);

	/**
	 * @brief checkBan checks a given URL against the list of banned services.
	 * Requires locking: YES
	 * @param sURL
	 * @return true if the URL is already present and has been banned; false otherwise.
	 */
	bool checkBan(QString sURL) const;

	/**
	 * @brief manageDuplicates checks if an identical (or very similar) service is alreads present
	 * in the manager, decides which service to remove and frees unnecessary data.
	 * Requires locking: YES
	 * @param pService
	 * @return true if a duplicate was detected. pService is deleted and set to nullptr in that
	 * case. false otherwise.
	 */
	bool manageDuplicates(ServicePtr &pService);

	/**
	 * @brief normalizeURL transforms a given URL string into a standard form to ease the detection
	 * of duplicates, filter out websites caching a service etc.
	 * Requires locking: NO
	 * @param sURL
	 * @return true if sURL has been changed; false if sURL has not seen a change
	 */
	bool normalizeURL(QString& sURL);

	/**
	 * @brief getRandomService: Helper method. Allows to get a random service for a specified
	 * network.
	 * Requires locking: YES
	 * @param oNType
	 * @return A discovery service for the specified network; Null if no working service could be
	 * found for the specified network.
	 */
	ServicePtr getRandomService(const CNetworkType& oNType);
};

} // namespace Discovery

/**
 * @brief discoveryManager is the static instance of this class used to manage discovery services
 * system wide.
 */
extern Discovery::Manager discoveryManager;

#endif // DISCOVERY_H
