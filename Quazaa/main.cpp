/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2013.
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

#include "singleapplication.h"

#include "winmain.h"
#include "quazaaglobals.h"
#include "quazaasettings.h"

#include "geoiplist.h"
#include "network.h"
#include "queryhashmaster.h"
#include "dialogsplash.h"
#include "dialoglanguage.h"
#include "wizardquickstart.h"
#include "sharemanager.h"
#include "commonfunctions.h"
#include "transfers.h"

#include "Misc/timedsignalqueue.h"
#include "Security/securitymanager.h"
#include "HostCache/g2hostcache.h"
#include "Discovery/discovery.h"

#include <QNetworkProxy>
#include <QFont>
#include <QtPlugin>
#include <QUrl>
#include <QIcon>

#ifdef Q_OS_LINUX
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif // Q_OS_LINUX

#include <stdio.h>
#include <stdlib.h>

#include "debug_new.h"

#ifdef _SNAPSHOT_BUILD
#include <QMessageBox>
#include "version.h"
#endif

static void setApplicationProxy( QUrl url )
{
	if ( !url.isEmpty() )
	{
		if ( url.port() == -1 )
		{
			url.setPort( 8080 );
		}
		QNetworkProxy proxy( QNetworkProxy::HttpProxy, url.host(), url.port(),
							 url.userName(), url.password() );
		QNetworkProxy::setApplicationProxy( proxy );
	}
}

//QuazaaGlobals quazaaGlobals;

int main( int argc, char* argv[] )
{
#ifdef Q_OS_MAC
	// QTBUG-32789 - GUI widgets use the wrong font on OS X Mavericks
	QFont::insertSubstitution( ".Lucida Grande UI", "Lucida Grande" );
#endif

	SingleApplication theApp( argc, argv );

	if ( !theApp.shouldContinue() )
	{
		return 0;
	}

	QStringList args = theApp.arguments();

	QUrl proxy;
	int index = args.indexOf( "-proxy" );
	if ( index != -1 )
	{
		proxy = QUrl( args.value( index + 1 ) );
	}
	else
	{
		proxy = QUrl( qgetenv( "http_proxy" ) );
	}
	if ( !proxy.isEmpty() )
	{
		setApplicationProxy( proxy );
	}

	QByteArray encoding;
	index = args.indexOf( "-encoding" );
	if ( index != -1 )
	{
		encoding = args.value( index + 1 ).toLocal8Bit();
	}
	else if ( !qgetenv( "COMMUNI_ENCODING" ).isEmpty() )
	{
		encoding = qgetenv( "COMMUNI_ENCODING" );
	}
	if ( !encoding.isEmpty() )
	{
		SingleApplication::setEncoding( encoding );
	}


// To enable this, run qmake with "DEFINES+=_SNAPSHOT_BUILD"
#ifdef _SNAPSHOT_BUILD
	QDate oExpire = QDate::fromString( Version::BUILD_DATE, Qt::ISODate ).addDays( 60 );

	if ( QDate::currentDate() > oExpire )
	{
		QMessageBox::information( NULL,
								  QObject::tr( "Cool Software, but..." ),
								  QObject::tr( "This build is expired. If you wish to continue using this "
											   "Cool Software you must download either latest stable releas"
											   "e or latest snapshot build from http://quazaa.sf.net/.\n\n"
											   "The program will now terminate." ) );
		return 0;
	}

	if ( !args.contains( "--no-alpha-warning" ) )
	{
		int ret = QMessageBox::warning( NULL,
										QObject::tr( "Snapshot/Debug Build Warning" ),
										QObject::tr( "WARNING: This is a SNAPSHOT BUILD of Quazaa. \n"
													 "It is NOT meant for GENERAL USE, and is only for testi"
													 "ng specific features in a controlled environment.\n"
													 "It will frequently stop running, or will display debug"
													 "information to assist testing.\n"
													 "This build will expire on %1.\n\n"
													 "Do you wish to continue?"
												   ).arg( oExpire.toString( Qt::SystemLocaleLongDate ) ),
										QMessageBox::Yes | QMessageBox::No );
		if ( ret == QMessageBox::No )
		{
			return 0;
		}
	}
#endif

	qsrand( time( 0 ) );

#ifdef Q_OS_LINUX

	rlimit sLimit;
	memset( &sLimit, 0, sizeof( rlimit ) );
	getrlimit( RLIMIT_NOFILE, &sLimit );

	sLimit.rlim_cur = sLimit.rlim_max;

	if ( setrlimit( RLIMIT_NOFILE, &sLimit ) == 0 )
	{
		qDebug() << "Successfully raised resource limits";
	}
	else
	{
		qDebug() << "Cannot set resource limits";
	}

#endif // Q_OS_LINUX

	theApp.setApplicationName(    QuazaaGlobals::APPLICATION_NAME() );
	theApp.setApplicationVersion( QuazaaGlobals::APPLICATION_VERSION_STRING() );
	theApp.setOrganizationDomain( QuazaaGlobals::APPLICATION_ORGANIZATION_DOMAIN() );
	theApp.setOrganizationName(   QuazaaGlobals::APPLICATION_ORGANIZATION_NAME() );
	theApp.setApplicationSlogan(  QObject::tr( "World class file sharing." ) );

	QIcon icon;
	icon.addFile( ":/Resource/Quazaa16.png" );
	icon.addFile( ":/Resource/Quazaa24.png" );
	icon.addFile( ":/Resource/Quazaa32.png" );
	icon.addFile( ":/Resource/Quazaa48.png" );
	icon.addFile( ":/Resource/Quazaa64.png" );
	icon.addFile( ":/Resource/Quazaa128.png" );
	qApp->setWindowIcon( icon );

	// Initialize system log component translations
	systemLog.start();

	// Setup Qt elements of signal queue necessary for operation
	signalQueue.setup();

	//Initialize multilanguage support
	quazaaSettings.loadLanguageSettings();
	quazaaSettings.translator.load( quazaaSettings.Language.File );
	qApp->installTranslator( &quazaaSettings.translator );

	// If the MainWindow object is not created and shown before any other dialogs,
	// the media player will break input event processing
	mainWindow = new CWinMain();
	mainWindow->show();
	mainWindow->hide();

	//Create splash window
	CDialogSplash* dlgSplash = new CDialogSplash();
	dlgSplash->show();
	qApp->processEvents();

	dlgSplash->updateProgress( 1, QObject::tr( "Loading settings..." ) );
	qApp->processEvents();

	//Initialize Settings
	quazaaSettings.loadSettings();

	//Check if this is Quazaa's first run
	dlgSplash->updateProgress( 8, QObject::tr( "Checking for first run..." ) );
	qApp->processEvents();
	bool bFirstRun = quazaaSettings.isFirstRun();
	if ( bFirstRun )
	{
		CDialogLanguage* dlgLanguage = new CDialogLanguage();
		dlgLanguage->exec();

		dlgSplash->updateProgress( 10, QObject::tr( "Running Quick Start wizard..." ) );
		quazaaSettings.saveFirstRun( false );
		quazaaSettings.saveSettings();
		quazaaSettings.saveProfile();

		CWizardQuickStart* wzrdQuickStart = new CWizardQuickStart();
		wzrdQuickStart->exec();
	}

	// Initialize GeoIP list
	dlgSplash->updateProgress( 12, QObject::tr( "Loading GeoIP..." ) );
	qApp->processEvents();
	geoIP.loadGeoIP();

	// Load Security Manager
	dlgSplash->updateProgress( 17, QObject::tr( "Loading Security Manager..." ) );
	qApp->processEvents();
	if ( !securityManager.start() )
		systemLog.postLog( LogSeverity::Information,
						   QObject::tr( "Security data file was not available." ) );

	//Load Host Cache
	dlgSplash->updateProgress( 22, QObject::tr( "Loading Host Cache..." ) );
	qApp->processEvents();
	hostCache.start();

	// Load Discovery Services Manager
	dlgSplash->updateProgress( 32, QObject::tr( "Loading Discovery Services Manager..." ) );
	qApp->processEvents();
	discoveryManager.start();

	//Load profile
	dlgSplash->updateProgress( 38, QObject::tr( "Loading Profile..." ) );
	qApp->processEvents();
	quazaaSettings.loadProfile();

	//Load the library
	dlgSplash->updateProgress( 40, QObject::tr( "Loading Library..." ) );
	qApp->processEvents();
	queryHashMaster.create();
	shareManager.start();

	// Load Download Manager
	dlgSplash->updateProgress( 60, QObject::tr( "Loading Transfer Manager..." ) );
	qApp->processEvents();
	transfers.start();

	dlgSplash->updateProgress( 80, QObject::tr( "Loading User Interface..." ) );
	qApp->processEvents();

	if ( quazaaSettings.WinMain.Visible )
	{
		if ( bFirstRun )
		{
			mainWindow->showMaximized();
		}
		else
		{
			mainWindow->show();
		}
	}

	dlgSplash->updateProgress( 90, QObject::tr( "Loading Tray Icon..." ) );
	qApp->processEvents();
	mainWindow->loadTrayIcon();

	dlgSplash->updateProgress( 100, QObject::tr( "Welcome to Quazaa!" ) );
	qApp->processEvents();

	dlgSplash->deleteLater();
	dlgSplash = 0;

	// Start networks if needed
	if ( quazaaSettings.System.ConnectOnStartup )
	{
		if ( quazaaSettings.Gnutella2.Enable )
		{
			networkG2.start();
		}
	}

	return theApp.exec();
}

