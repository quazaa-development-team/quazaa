﻿/*
** gwc.cpp
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

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "network.h"
#include "quazaaglobals.h"
#include "version.h"

#include "gwc.h"

using namespace Discovery;

CGWC::CGWC(const QUrl& oURL, const CNetworkType& oNType, quint8 nRating) :
	CDiscoveryService( oURL, oNType, nRating ),
	m_pRequest( NULL ),
	m_bGnutella( true ),
	m_bG2( true )
{
	m_nServiceType = stGWC;
}

CGWC::~CGWC()
{
	if ( m_pRequest )
	{
		delete m_pRequest;
		m_pRequest = NULL;

		// if there is a request, there must be a manager
		Q_ASSERT( m_pNAMgr );

		m_pNAMgr.clear();
	}
}

QString CGWC::type() const
{
	return isBanned() ? QString( "Banned GWC" ) : QString( "GWC" );
}

// caller needs to make sure the cache is not currently running
void CGWC::doQuery() throw()
{
	QUrl oURL = m_oServiceURL;
	QString sVersion = QString::number( Version::MAJOR ) + "." + QString::number( Version::MINOR );

	// build query
#if QT_VERSION >= 0x050000
	{
		QUrlQuery query;
		query.addQueryItem( "ping", "1" );
		query.addQueryItem( "get", "1" );
		query.addQueryItem( "net", "gnutella2" );
		query.addQueryItem( "client", QuazaaGlobals::VENDOR_CODE() );
		query.addQueryItem( "version", sVersion.toLocal8Bit().data() );
		oURL.setQuery(query);
	}
#else
	oURL.addQueryItem( "ping", "1" );
	oURL.addQueryItem( "get", "1" );
	oURL.addQueryItem( "net", "gnutella2" );
	oURL.addQueryItem( "client", QuazaaGlobals::VENDOR_CODE() );
	oURL.addQueryItem( "version", sVersion.toLocal8Bit().data() );
#endif

	// inform user
	postLog( LogSeverity::Debug, QString( "Querying GWC: %1" ).arg( oURL.toString() ) );

	quint32 tNow = static_cast< quint32 >( QDateTime::currentDateTimeUtc().toTime_t() );
	setLastAccessed( tNow );

	// generate request
	m_pRequest = new QNetworkRequest( oURL );
	m_pRequest->setRawHeader( "User-Agent", QuazaaGlobals::USER_AGENT_STRING().toLocal8Bit() );

	// obtain network access manager for query
	m_pNAMgr = discoveryManager.requestNAM();

#if QT_VERSION >= 0x050000
	connect( m_pNAMgr.data(), &QNetworkAccessManager::finished,
			 this, &CGWC::requestCompleted );
#else
	connect( m_pNAMgr.data(), SIGNAL( finished(QNetworkReply*) ),
			 this, SLOT( requestCompleted(QNetworkReply*) ) );
#endif

	// do query
	m_pNAMgr->get( *m_pRequest );
}

void CGWC::doUpdate() throw()
{
	QUrl oURL = m_oServiceURL;
	QString sVersion = QString::number( Version::MAJOR ) + "." + QString::number( Version::MINOR );

	Network.m_pSection.lock();
	QString sOwnIP = Network.GetLocalAddress().toStringWithPort();
	Network.m_pSection.unlock();

	QString sPromoteURL = discoveryManager.getWorkingService( stGWC );

	// build query
#if QT_VERSION >= 0x050000
	{
		QUrlQuery query;
		query.addQueryItem( "ping", "1" );
		query.addQueryItem( "update", "1" );
		query.addQueryItem( "net", "gnutella2" );
		query.addQueryItem( "ip", sOwnIP );
		query.addQueryItem( "client", QuazaaGlobals::VENDOR_CODE() );
		query.addQueryItem( "version", sVersion );

		if ( !sPromoteURL.isEmpty() )
		{
			query.addQueryItem( "url", QUrl::toPercentEncoding( sPromoteURL ) );
		}

		oURL.setQuery(query);
	}
#else
	oURL.addQueryItem( "ping", "1" );
	oURL.addQueryItem( "update", "1" );
	oURL.addQueryItem( "net", "gnutella2" );
	oURL.addQueryItem( "ip", sOwnIP );
	oURL.addQueryItem( "client", QuazaaGlobals::VENDOR_CODE() );
	oURL.addQueryItem( "version", sVersion );

	if ( !sPromoteURL.isEmpty() )
	{
		oURL.addQueryItem( "url", QUrl::toPercentEncoding( sPromoteURL ) );
	}
#endif

	// inform user
	postLog( LogSeverity::Debug, QString( "Updating GWC: %1" ).arg( oURL.toString() ) );

	quint32 tNow = static_cast< quint32 >( QDateTime::currentDateTimeUtc().toTime_t() );
	setLastAccessed( tNow );

	// generate request
	m_pRequest = new QNetworkRequest( oURL );
	m_pRequest->setRawHeader( "User-Agent", QuazaaGlobals::USER_AGENT_STRING().toLocal8Bit() );

	// obtain network access manager for update
	m_pNAMgr = discoveryManager.requestNAM();

#if QT_VERSION >= 0x050000
	connect( m_pNAMgr.data(), &QNetworkAccessManager::finished,
			 this, &CGWC::requestCompleted );
#else
	connect( m_pNAMgr.data(), SIGNAL( finished(QNetworkReply*) ),
			 this, SLOT( requestCompleted(QNetworkReply*) ) );
#endif

	// do query
	m_pNAMgr->get( *m_pRequest );
}

void CGWC::doCancelRequest() throw()
{
#if QT_VERSION >= 0x050000
	disconnect( m_pNAMgr.data(), &QNetworkAccessManager::finished,
				this, &CGWC::requestCompleted );
#else
	disconnect( m_pNAMgr.data(), SIGNAL( finished(QNetworkReply*) ),
				this, SLOT( requestCompleted(QNetworkReply*) ) );
#endif

	delete m_pRequest;
	m_pRequest = NULL;
	m_pNAMgr.clear();     // we don't need the network access manager anymore

	resetRunning();
}

void CGWC::requestCompleted(QNetworkReply* pReply)
{
	QWriteLocker oGWCLock( &m_oRWLock );

	if ( !pReply || !isRunning() || !m_pRequest ) // we got cancelled while waiting for the lock
	{
		postLog( LogSeverity::Warning, QString( "The request got probably cancelled while " ) +
		         QString( "requestCompleted() was waiting for a lock on the service." ), true );

		resetRunning();

		if ( pReply )
			pReply->deleteLater();

		if ( m_pRequest )
		{
			delete m_pRequest;
			m_pRequest = NULL;
		}

		m_pNAMgr.clear();

		postLog( LogSeverity::Warning,
		         QString( "Finished cleanup after unusual termination." ),
		         true );

		return;
	}

	if ( pReply->request() != *m_pRequest )
	{
		return; // reply was meant for sb else
	}

	postLog( LogSeverity::Debug, tr( "Recieved answer from GWC." ) );

	// used for statistics update at a later time
	quint16 nHosts = 0;
	quint16 nURLs  = 0;

	bool bUpdateOK = false;     // in case we did update, was it successful?
	QList<QString>   lURLList;  // alternate services returned by the GWC
	QList<CEndPoint> lHostList; // hosts returned by the GWC

	if ( pReply->error() == QNetworkReply::NoError )
	{
		postLog( LogSeverity::Debug, tr( "Parsing GWC response:" ) );

		// get first piece of data from reply
		QString ln = pReply->readLine( 2048 );
		QString sWarning; // used for error messages/warnings the GWC might return

		// parse data
		while ( !ln.isEmpty() )
		{
			postLog( LogSeverity::Debug, QString( "Line: %1" ).arg( ln.replace( '\n', "" ) ) );

			QStringList lp = ln.split( "|" );
			if ( lp.size() > 1 )
			{
				if ( lp[0] == "I" || lp[0] == "i" )
				{
					if ( lp.size() > 2 )
					{
						if ( lp[1].toLower() == "pong" )
						{
							m_sPong = lp[2];
							if ( lp.size() > 3 )
							{
								QStringList lNetworks = lp[3].split( "-" );

								m_bGnutella = false;
								m_bG2       = false;

								for ( int i = 0; i < lNetworks.size(); ++i )
								{
									if ( !lNetworks[i].toLower().compare( "gnutella" ) )
									{
										m_bGnutella = true;
									}

									if ( !lNetworks[i].toLower().compare( "gnutella2" ) )
									{
										m_bG2       = true;
									}
								}
							}
						}
						else if ( lp[1].toLower() == "update" )
						{
							if ( lp[2].toLower() == "ok" )
							{
								bUpdateOK = true;
								if ( lp.size() > 4 && lp[3].toLower() == "warning" )
								{
									sWarning = lp[4];
								}
							}
							else if ( lp[2].toLower() == "warning" )
							{
								if ( lp.size() > 3 )
								{
									sWarning = lp[3];
								}
							}
						}
					}
				}
				else if ( lp[0] == "H" || lp[0] == "h" )
				{
					// found host
					lHostList.push_back( CEndPoint( lp[1] ) );
					++nHosts;
				}
				else if ( lp[0] == "U" || lp[0] == "u" )
				{
					// found URL
					lURLList.push_back( lp[1] );
					++nURLs;
				}
			}
			else
			{
				postLog( LogSeverity::Debug, tr( "Parsing error!" ) );
			}

			if ( !sWarning.isEmpty() )
			{
				if ( bUpdateOK )
				{
					postLog( LogSeverity::Warning, sWarning );
				}
				else
				{
					postLog( LogSeverity::Error, sWarning );
				}
			}

			// get next piece of data
			ln = pReply->readLine( 2048 );
		}
	}
	else
	{
		postLog( LogSeverity::Error, tr( "While querying GWC: " ) + url() );
	}

	// make sure all statistics and failure counters are updated
	updateStatistics( nHosts, nURLs, bUpdateOK );

	// clean up
	pReply->deleteLater();
	delete m_pRequest;
	m_pRequest = NULL;
	m_pNAMgr.clear();

	// make sure the service is not reported as running anymore
	resetRunning();

	TServiceType oServiceType = m_nServiceType;

	// finished accessing GWC variables
	oGWCLock.unlock();

	// add new services to manager
	while ( lURLList.size() )
	{
		discoveryManager.add( lURLList.back(), oServiceType, CNetworkType( dpG2 ) );
		lURLList.pop_back();
	}

	// prepare for adding new hosts
	QMutexLocker l( &hostCache.m_pSection );

	while ( lHostList.size() )
	{
		hostCache.add( lHostList.back() );
		lHostList.pop_back();
	}

}
