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

#include "winmain.h"
#include "ui_winmain.h"
#include "dialogclosetype.h"
#include "dialogsplash.h"
#include "wizardquickstart.h"
#include "dialogabout.h"
#include "dialogopentorrent.h"
#include "dialogcreatetorrent.h"
#include "dialogsettings.h"
#include "dialogeditshares.h"
#include "dialogadddownload.h"
#include "dialogdownloadsimport.h"
#include "dialoglanguage.h"
#include "dialogscheduler.h"
#include "dialogprofile.h"
#include "dialogmodifyrule.h"
#include "dialogsecuritysubscriptions.h"
#include "dialoglibrarysearch.h"
#include "dialogfiltersearch.h"
#include "dialogconnectto.h"
#include "dialoghashprogress.h"
#include "skinsettings.h"

#include "quazaasettings.h"
#include "quazaaglobals.h"

#include "commonfunctions.h"
#include "handshakes.h"
#include "network.h"
#include "neighbours.h"
#include "datagrams.h"
#include "geoiplist.h"
#include "sharemanager.h"
#include "transfers.h"
#include "hostcache.h"

#include "chatsession.h"
#include "chatsessiong2.h"
#include "chatcore.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif //Q_OS_WIN

#include <QTimer>
#include <QDesktopServices>
#include <QClipboard>
#include <QMessageBox>

#include "debug_new.h"

CWinMain* MainWindow = 0;

void CWinMain::quazaaStartup()
{
}

CWinMain::CWinMain(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::CWinMain)
{
	ui->setupUi(this);

	//Initialize vaiables
	bypassCloseEvent = false;
	interfaceLoaded = false;

	ui->actionAres->setChecked(quazaaSettings.Ares.Enable);
	ui->actionEDonkey->setChecked(quazaaSettings.EDonkey.Enable);
	ui->actionGnutella2->setChecked(quazaaSettings.Gnutella2.Enable);

	//Load And Set Up User Interface
	quazaaSettings.loadWindowSettings(this);
	restoreState(quazaaSettings.WinMain.MainToolbar);

	//Set up the status bar
	labelIPAddress = new QLabel(tr("Current IP:"));
	ui->statusbar->addWidget(labelIPAddress);
	labelCurrentIPAddress = new QLabel(tr("Unknown"));
	ui->statusbar->addWidget(labelCurrentIPAddress);
	toolButtonAddressToClipboard = new QToolButton(this);
	toolButtonAddressToClipboard->setText(tr("Copy Address"));
	toolButtonAddressToClipboard->setAutoRaise(true);
	connect(toolButtonAddressToClipboard, SIGNAL(clicked()), this, SLOT(onCopyIP()));
	ui->statusbar->addWidget(toolButtonAddressToClipboard);

	tcpFirewalled = ":/Resource/Network/ShieldRed.png";
	udpFirewalled = ":/Resource/Network/ShieldRed.png";
	labelFirewallStatus = new QLabel(tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"> <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Segoe UI'; font-size:10pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">TCP: <img src=\":/Resource/Network/ShieldRed.png\" /> UDP: <img src=\":/Resource/Network/ShieldRed.png\" /></p></body></html>"));
	ui->statusbar->addPermanentWidget(labelFirewallStatus);
	labelBandwidthTotals = new QLabel();
	ui->statusbar->addPermanentWidget(labelBandwidthTotals);

	//Add the tabs
	pageHome = new CWidgetHome();
	ui->stackedWidgetMain->addWidget(pageHome);
	pageLibrary = new CWidgetLibrary();
	ui->stackedWidgetMain->addWidget(pageLibrary);
	pageMedia = new CWidgetMedia();
	ui->stackedWidgetMain->addWidget(pageMedia);
	pageSearch = new CWidgetSearch();
	ui->stackedWidgetMain->addWidget(pageSearch);
	pageTransfers = new CWidgetTransfers();
	ui->stackedWidgetMain->addWidget(pageTransfers);
	pageSecurity = new CWidgetSecurity();
	ui->stackedWidgetMain->addWidget(pageSecurity);
	pageActivity = new CWidgetActivity();
	ui->stackedWidgetMain->addWidget(pageActivity);
	pageIrcMain = new CWidgetIrcMain();
	ui->stackedWidgetMain->addWidget(pageIrcMain);
	pageHostCache = new CWidgetHostCache;
	ui->stackedWidgetMain->addWidget(pageHostCache);
	pageDiscovery = new CWidgetDiscovery;
	ui->stackedWidgetMain->addWidget(pageDiscovery);
	pageScheduler = new CWidgetScheduler;
	ui->stackedWidgetMain->addWidget(pageScheduler);
	pageGraph = new CWidgetGraph;
	ui->stackedWidgetMain->addWidget(pageGraph);
	pagePacketDump = new CWidgetPacketDump;
	ui->stackedWidgetMain->addWidget(pagePacketDump);
	pageSearchMonitor = new CWidgetSearchMonitor;
	ui->stackedWidgetMain->addWidget(pageSearchMonitor);
	pageHitMonitor = new CWidgetHitMonitor;
	ui->stackedWidgetMain->addWidget(pageHitMonitor);

	dlgPrivateMessages = 0;

	// Set up the navigation toolbar
	actionGroupMainNavigation = new QActionGroup(this);
	actionGroupMainNavigation->addAction(ui->actionHome);
	actionGroupMainNavigation->addAction(ui->actionLibrary);
	actionGroupMainNavigation->addAction(ui->actionMedia);
	actionGroupMainNavigation->addAction(ui->actionSearch);
	actionGroupMainNavigation->addAction(ui->actionTransfers);
	actionGroupMainNavigation->addAction(ui->actionSecurity);
	actionGroupMainNavigation->addAction(ui->actionActivity);
	actionGroupMainNavigation->addAction(ui->actionChat);
	actionGroupMainNavigation->addAction(ui->actionHostCache);
	actionGroupMainNavigation->addAction(ui->actionDiscovery);
	actionGroupMainNavigation->addAction(ui->actionScheduler);
	actionGroupMainNavigation->addAction(ui->actionGraph);
	actionGroupMainNavigation->addAction(ui->actionPacketDump);
	actionGroupMainNavigation->addAction(ui->actionSearchMonitor);
	actionGroupMainNavigation->addAction(ui->actionHitMonitor);
	ui->stackedWidgetMain->setCurrentIndex(quazaaSettings.WinMain.ActiveTab);
	switch(quazaaSettings.WinMain.ActiveTab)
	{
	case 0:
		ui->actionHome->setChecked(true);
		break;
	case 1:
		ui->actionLibrary->setChecked(true);
		break;
	case 2:
		ui->actionMedia->setChecked(true);
		break;
	case 3:
		ui->actionSearch->setChecked(true);
		pageSearch->focusSearchInput();
		break;
	case 4:
		ui->actionTransfers->setChecked(true);
		break;
	case 5:
		ui->actionSecurity->setChecked(true);
		break;
	case 6:
		ui->actionActivity->setChecked(true);
		break;
	case 7:
		ui->actionChat->setChecked(true);
		break;
	case 8:
		ui->actionHostCache->setChecked(true);
		break;
	case 9:
		ui->actionDiscovery->setChecked(true);
		break;
	case 10:
		ui->actionScheduler->setChecked(true);
		break;
	case 11:
		ui->actionGraph->setChecked(true);
		break;
	case 12:
		ui->actionPacketDump->setChecked(true);
		break;
	case 13:
		ui->actionSearchMonitor->setChecked(true);
		break;
	case 14:
		ui->actionHitMonitor->setChecked(true);
		break;
	default:
		ui->actionHome->setChecked(true);
		break;
	}

	connect(ui->actionNewSearch, SIGNAL(triggered()), pageSearch, SLOT(on_toolButtonNewSearch_clicked()));
	connect(pageHome, SIGNAL(requestSearch(QString*)), this, SLOT(startNewSearch(QString*)));
	connect(pageHome, SIGNAL(triggerLibrary()), this, SLOT(on_actionLibrary_triggered()));
	connect(pageHome, SIGNAL(triggerSecurity()), this, SLOT(on_actionSecurity_triggered()));
	connect(pageHome, SIGNAL(triggerTransfers()), this, SLOT(on_actionTransfers_triggered()));

	neighboursRefresher = new QTimer(this);
	connect(neighboursRefresher, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
	connect(neighboursRefresher, SIGNAL(timeout()), pageActivity->panelNeighbours, SLOT(onTimer()));

	update();
	qApp->processEvents();

	interfaceLoaded = true;

	connect(&ChatCore, SIGNAL(openChatWindow(CChatSession*)), this, SLOT(OpenChat(CChatSession*)));
	connect(&Network, SIGNAL(localAddressChanged()), this, SLOT(localAddressChanged()));
	connect(&ShareManager, SIGNAL(hasherStarted(int)), this, SLOT(onHasherStarted(int)));
	setSkin();
}

CWinMain::~CWinMain()
{
	delete ui;
}

void CWinMain::loadTrayIcon()
{
	// Create the system tray right click menu.
	trayMenu = new QMenu(this);
	trayMenu->addAction(ui->actionShowOrHide);
	trayMenu->addSeparator();
	trayMenu->addAction(ui->actionNewSearch);
	trayMenu->addAction(ui->actionURLDownload);
	trayMenu->addSeparator();
	trayMenu->addAction(ui->actionMediaPlay);
	trayMenu->addAction(ui->actionMediaStop);
	trayMenu->addAction(ui->actionMediaOpen);
	trayMenu->addSeparator();
	trayMenu->addAction(ui->actionMediaRewind);
	trayMenu->addAction(ui->actionMediaNextTrack);
	trayMenu->addSeparator();
	trayMenu->addAction(ui->actionExitAfterTransfers);
	trayMenu->addAction(ui->actionExit);
	// Create the system tray icon
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(QIcon(":/Resource/Quazaa.png"));
	trayIcon->setToolTip(tr("Quazaa"));
	trayIcon->setContextMenu(trayMenu);
	// Connect an event handler to the tray icon so we can handle mouse events
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(icon_activated(QSystemTrayIcon::ActivationReason)));
	trayIcon->show();
}

bool CWinMain::event(QEvent* e)
{
	QMainWindow::event(e);
	switch(e->type())
	{
	case QEvent::Close:
		if(!bypassCloseEvent)
		{
			if(quazaaSettings.System.CloseMode == 0)
			{
				CDialogCloseType* dlgCloseType = new CDialogCloseType(this);
				dlgCloseType->exec();
			}

			switch(quazaaSettings.System.CloseMode)
			{
			case 1:
				quazaaShutdown();
				return false;
			case 2:
				hide();
				e->ignore();
				return true;
			case 3:
				quazaaShutdown();
				return false;
			default:
				quazaaShutdown();
				return false;
			}
		}
		else
		{
			quazaaShutdown();
			return false;
		}
	case QEvent::Show:
		neighboursRefresher->start(1000);
		break;
	case QEvent::Hide:
		if(neighboursRefresher)
		{
			neighboursRefresher->stop();
		}
		break;
	default:
		return false;
	}

	return false;
}

void CWinMain::changeEvent(QEvent* e)
{
	QMainWindow::changeEvent(e);
	switch(e->type())
	{
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		switch(quazaaSettings.WinMain.ActiveTab)
		{
		case 0:
			ui->actionHome->setChecked(true);
			break;
		case 1:
			ui->actionLibrary->setChecked(true);
			break;
		case 2:
			ui->actionMedia->setChecked(true);
			break;
		case 3:
			ui->actionSearch->setChecked(true);
			break;
		case 4:
			ui->actionTransfers->setChecked(true);
			break;
		case 5:
			ui->actionSecurity->setChecked(true);
			break;
		case 6:
			ui->actionActivity->setChecked(true);
			break;
		case 7:
			ui->actionChat->setChecked(true);
			break;
		case 8:
			ui->actionHostCache->setChecked(true);
			break;
		case 9:
			ui->actionDiscovery->setChecked(true);
			break;
		case 10:
			ui->actionScheduler->setChecked(true);
			break;
		case 11:
			ui->actionGraph->setChecked(true);
			break;
		case 12:
			ui->actionPacketDump->setChecked(true);
			break;
		case 13:
			ui->actionSearchMonitor->setChecked(true);
			break;
		case 14:
			ui->actionHitMonitor->setChecked(true);
			break;
		default:
			ui->actionHome->setChecked(true);
			break;
		}
		break;
	case QEvent::WindowStateChange:
		{
			QWindowStateChangeEvent *event = (QWindowStateChangeEvent*)e;
			// make sure we only do this for minimize events
			if ((event->oldState() != Qt::WindowMinimized) && isMinimized() && quazaaSettings.System.MinimizeToTray)
			{
				QTimer::singleShot(0, this, SLOT(hide()));
				e->ignore();
			}
		}
	default:
		break;
	}
}

void CWinMain::quazaaShutdown()
{
	CDialogSplash* dlgSplash = new CDialogSplash();
	dlgSplash->show();

	dlgSplash->updateProgress(95, tr("Saving UI..."));
	qApp->processEvents();
	quazaaSettings.WinMain.MainToolbar = saveState();
	pageHome->saveWidget();
	pageLibrary->saveWidget();
	pageMedia->saveWidget();
	pageSearch->saveWidget();
	pageTransfers->saveWidget();
	pageSecurity->saveWidget();
	pageActivity->saveWidget();
	pageIrcMain->saveWidget();
	pageHostCache->saveWidget();
	pageDiscovery->saveWidget();
	pageScheduler->saveWidget();
	pageGraph->saveWidget();
	pagePacketDump->saveWidget();
	pageSearchMonitor->saveWidget();
	pageHitMonitor->saveWidget();
	quazaaSettings.saveWindowSettings(this);
	emit closing();

	dlgSplash->updateProgress(90, tr("Saving Settings..."));
	qApp->processEvents();
	quazaaSettings.saveSettings();

	dlgSplash->updateProgress(85, tr("Closing Networks..."));
	qApp->processEvents();
	neighboursRefresher->stop();
	delete neighboursRefresher;
	neighboursRefresher = 0;
	Network.stop();
	ShareManager.stop();

	dlgSplash->updateProgress(65, tr("Saving Security Manager..."));
	qApp->processEvents();
	securityManager.stop(); // Prepare Security Manager for shutdown (this includes saving the security rules to disk)

	dlgSplash->updateProgress(55, tr("Saving Discovery Manager..."));
	qApp->processEvents();
	discoveryManager.stop();

	dlgSplash->updateProgress(50, tr("Saving Host Cache..."));
	qApp->processEvents();
	hostCache.m_pSection.lock();
	hostCache.save( common::getTNowUTC() );
	hostCache.m_pSection.unlock();

	dlgSplash->updateProgress(30, tr("Removing Tray Icon..."));
	qApp->processEvents();
	trayIcon->hide();
	trayIcon->deleteLater();

	dlgSplash->updateProgress(15, tr("Stopping transfers..."));
	qApp->processEvents();
	Transfers.stop();

	qApp->processEvents();

	dlgSplash->close();

	delete this;
}

void CWinMain::on_actionHome_triggered()
{
	ui->actionHome->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(0);
	quazaaSettings.WinMain.ActiveTab = 0;
}

void CWinMain::on_actionLibrary_triggered()
{
	ui->actionLibrary->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(1);
	quazaaSettings.WinMain.ActiveTab = 1;
}

void CWinMain::on_actionMedia_triggered()
{
	ui->actionMedia->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(2);
	quazaaSettings.WinMain.ActiveTab = 2;
}

void CWinMain::on_actionSearch_triggered()
{
	ui->actionSearch->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(3);
	quazaaSettings.WinMain.ActiveTab = 3;
	pageSearch->focusSearchInput();
}

void CWinMain::on_actionTransfers_triggered()
{
	ui->actionTransfers->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(4);
	quazaaSettings.WinMain.ActiveTab = 4;
}

void CWinMain::on_actionSecurity_triggered()
{
	ui->actionSecurity->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(5);
	quazaaSettings.WinMain.ActiveTab = 5;
}

void CWinMain::on_actionActivity_triggered()
{
	ui->actionActivity->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(6);
	quazaaSettings.WinMain.ActiveTab = 6;
}

void CWinMain::on_actionChat_triggered()
{
	ui->actionChat->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(7);
	quazaaSettings.WinMain.ActiveTab = 7;
}

void CWinMain::on_actionHostCache_triggered()
{
	ui->actionHostCache->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(8);
	quazaaSettings.WinMain.ActiveTab = 8;
}

void CWinMain::on_actionDiscovery_triggered()
{
	ui->actionDiscovery->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(9);
	quazaaSettings.WinMain.ActiveTab = 9;
}

void CWinMain::on_actionScheduler_triggered()
{
	ui->actionScheduler->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(10);
	quazaaSettings.WinMain.ActiveTab = 10;
}

void CWinMain::on_actionGraph_triggered()
{
	ui->actionGraph->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(11);
	quazaaSettings.WinMain.ActiveTab = 11;
}

void CWinMain::on_actionPacketDump_triggered()
{
	ui->actionPacketDump->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(12);
	quazaaSettings.WinMain.ActiveTab = 12;
}

void CWinMain::on_actionSearchMonitor_triggered()
{
	ui->actionSearchMonitor->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(13);
	quazaaSettings.WinMain.ActiveTab = 13;
}

void CWinMain::on_actionHitMonitor_triggered()
{
	ui->actionHitMonitor->setChecked(true);
	ui->stackedWidgetMain->setCurrentIndex(14);
	quazaaSettings.WinMain.ActiveTab = 14;
}

void CWinMain::icon_activated(QSystemTrayIcon::ActivationReason reason)
{
	switch(reason)
	{
	case QSystemTrayIcon::Unknown:
		break;
	case QSystemTrayIcon::DoubleClick:
		ui->actionShowOrHide->trigger();
		break;
	case QSystemTrayIcon::Trigger:
		break;
	case QSystemTrayIcon::MiddleClick:
		break;
	default:
		break;
	}
}

void CWinMain::on_actionShowOrHide_triggered()
{
	if(isVisible())
	{
		hide();
	}
	else
	{
		showOnTop();
	}
}

void CWinMain::showOnTop()
{
#ifdef Q_OS_WIN
	// THIS IS A HACK:
	// from QT documentation:
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// void QWidget::activateWindow ()
	// ...
	// On Windows, if you are calling this when the application is not currently
	// the active one then it will not make it the active window. It will change
	// the color of the taskbar entry to indicate that the window has changed in
	// some way. This is because Microsoft do not allow an application to
	// interrupt what the user is currently doing in another application.
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// This hack does not give the focus to the app but brings it to front so
	// the user sees it.
	SetWindowPos((HWND)effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	SetWindowPos((HWND)effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		// HACK END
#endif
		raise();
		if (isMaximized())
			showMaximized();
		else
			showNormal();
		activateWindow();
		raise();
}

void CWinMain::on_actionExit_triggered()
{
	bypassCloseEvent = true;
	QCloseEvent* ev = new QCloseEvent();
	QApplication::postEvent(this, ev, Qt::HighEventPriority);
}

void CWinMain::on_actionAbout_triggered()
{
	CDialogAbout* dlgAbout = new CDialogAbout(this);

	dlgAbout->show();
}

void CWinMain::on_actionSettings_triggered()
{
	CDialogSettings* dlgSettings = new CDialogSettings(this);
	dlgSettings->show();
}

void CWinMain::on_actionCreateTorrent_triggered()
{
	CDialogCreateTorrent* dlgCreateTorrent = new CDialogCreateTorrent(this);
	dlgCreateTorrent->show();
}

void CWinMain::on_actionSeedTorrent_triggered()
{

}

void CWinMain::on_actionOpenTorrent_triggered()
{
	CDialogOpenTorrent* dlgOpenTorrent = new CDialogOpenTorrent(this);
	dlgOpenTorrent->show();
}

void CWinMain::on_actionShares_triggered()
{
	CDialogEditShares* dlgEditShares = new CDialogEditShares(this);
	dlgEditShares->show();
}

void CWinMain::on_actionOpenDownloadFolder_triggered()
{
	common::folderOpen(quazaaSettings.Downloads.CompletePath);
}

void CWinMain::on_actionURLDownload_triggered()
{
	CDialogAddDownload* dlgAddDownload = new CDialogAddDownload(this);

	dlgAddDownload->show();
}

void CWinMain::on_actionImportPartials_triggered()
{
	CDialogDownloadsImport* dlgDownloadsImport = new CDialogDownloadsImport(this);
	dlgDownloadsImport->show();
}

void CWinMain::on_actionChooseSkin_triggered()
{
	CDialogSettings* dlgSettings = new CDialogSettings(this, SettingsPage::Skins);
	dlgSettings->show();
}

void CWinMain::on_actionChooseLanguage_triggered()
{
	CDialogLanguage* dlgLanguage = new CDialogLanguage(this);
	dlgLanguage->exec();
}

void CWinMain::on_actionQuickstartWizard_triggered()
{
	CWizardQuickStart* wzrdQuickStart = new CWizardQuickStart(this);
	wzrdQuickStart->exec();
}

void CWinMain::on_actionUsersGuide_triggered()
{
	QDesktopServices::openUrl(QUrl("http://quazaa.sourceforge.net/wiki/Manual", QUrl::TolerantMode));
}

void CWinMain::on_actionFAQ_triggered()
{
	QDesktopServices::openUrl(QUrl("http://quazaa.sourceforge.net/wiki/FAQ", QUrl::TolerantMode));
}

void CWinMain::on_actionConnectionTest_triggered()
{
	QDesktopServices::openUrl(QUrl(QString("http://jlh.no-ip.org/connectiontest/index.php?port=%1&lang=%2&test=1").arg(quazaaSettings.Connection.Port).arg("en"), QUrl::TolerantMode));
}

void CWinMain::on_actionCheckForNewVersion_triggered()
{

}

void CWinMain::on_actionDonate_triggered()
{
	QDesktopServices::openUrl(QUrl("https://sourceforge.net/donate/index.php?group_id=286623", QUrl::TolerantMode));
}

void CWinMain::on_actionQuazaaForums_triggered()
{
	QDesktopServices::openUrl(QUrl("http://quazaa.sourceforge.net/forums/", QUrl::TolerantMode));
}

void CWinMain::on_actionEditMyProfile_triggered()
{
	CDialogProfile* dlgProfile = new CDialogProfile(this);
	dlgProfile->show();
}

void CWinMain::on_actionNewSearch_triggered()
{
	ui->actionSearch->trigger();
}

void CWinMain::on_actionConnect_triggered()
{
	ui->actionConnect->setEnabled(false);
	ui->actionDisconnect->setEnabled(true);
	Network.start();
}

void CWinMain::on_actionDisconnect_triggered()
{
	ui->actionConnect->setEnabled(true);
	ui->actionDisconnect->setEnabled(false);
	Network.stop();
}

void CWinMain::on_actionEDonkey_triggered(bool checked)
{
	quazaaSettings.EDonkey.Enable = checked;
}

void CWinMain::on_actionGnutella2_triggered(bool checked)
{
	quazaaSettings.Gnutella2.Enable = checked;
}

void CWinMain::on_actionAres_triggered(bool checked)
{
	quazaaSettings.Ares.Enable = checked;
}

void CWinMain::startNewSearch(QString* searchString)
{
	ui->stackedWidgetMain->setCurrentIndex(3);
	ui->actionSearch->setChecked(true);
	pageSearch->startNewSearch(searchString);
}

void CWinMain::updateStatusBar()
{
	quint16 nTCPInSpeed = 0;
	quint16 nTCPOutSpeed = 0;
	quint16 nUDPInSpeed = 0;
	quint16 nUDPOutSpeed = 0;

	if(!Network.m_bActive)
	{
		return;
	}

	if(Handshakes.m_pSection.tryLock(50))
	{
		if(!Handshakes.isFirewalled())
		{
			tcpFirewalled = ":/Resource/Network/CheckedShieldGreen.png";
		}
		else
		{
			tcpFirewalled = ":/Resource/Network/ShieldRed.png";
		}
		Handshakes.m_pSection.unlock();
	}

	if(Neighbours.m_pSection.tryLock(50))
	{
		nTCPInSpeed = Neighbours.downloadSpeed();
		nTCPOutSpeed = Neighbours.uploadSpeed();
		Neighbours.m_pSection.unlock();
	}

	if(Datagrams.m_pSection.tryLock(50))
	{
		if(!Datagrams.isFirewalled())
		{
			udpFirewalled = ":/Resource/Network/CheckedShieldGreen.png";
		}
		else
		{
			udpFirewalled = ":/Resource/Network/ShieldRed.png";
		}

		nUDPInSpeed = Datagrams.downloadSpeed();
		nUDPOutSpeed = Datagrams.uploadSpeed();

		Datagrams.m_pSection.unlock();
	}

	labelFirewallStatus->setText(tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"> <html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'Segoe UI'; font-size:10pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">TCP: <img src=\"%1\" /> UDP: <img src=\"%2\" /></p></body></html>").arg(tcpFirewalled).arg(udpFirewalled));
	labelBandwidthTotals->setText(tr("TCP: %1/s / %2/s UDP: %3/s / %4/s").arg(common::formatBytes(nTCPInSpeed)).arg(common::formatBytes(nTCPOutSpeed)).arg(common::formatBytes(nUDPInSpeed)).arg(common::formatBytes(nUDPOutSpeed)));
}

void CWinMain::on_actionConnectTo_triggered()
{
	CDialogConnectTo* dlgConnectTo = new CDialogConnectTo(this);
	bool accepted = dlgConnectTo->exec();

	if(accepted)
	{
		CEndPoint ip(dlgConnectTo->getAddressAndPort());

		switch(dlgConnectTo->getConnectNetwork())
		{
		case CDialogConnectTo::G2:
			Neighbours.m_pSection.lock();
			Neighbours.connectTo(ip, dpG2, false);
			Neighbours.m_pSection.unlock();
			break;
		case CDialogConnectTo::eDonkey:
			break;
		case CDialogConnectTo::Ares:
			break;
		default:
			break;
		}
	}
}

void CWinMain::OpenChat(CChatSession* pSess)
{
	if(dlgPrivateMessages == 0)
	{
		dlgPrivateMessages = new CDialogPrivateMessages(0);
		connect(dlgPrivateMessages, SIGNAL(destroyed()), this, SLOT(onLastChatClosed()));
		connect(this, SIGNAL(closing()), dlgPrivateMessages, SLOT(close()));
	}

	dlgPrivateMessages->OpenChat(pSess);
	dlgPrivateMessages->show();
}

void CWinMain::on_actionChatWith_triggered()
{
	CDialogConnectTo* dlgConnectTo = new CDialogConnectTo(this);
	bool accepted = dlgConnectTo->exec();

	if(accepted)
	{
		CEndPoint ip(dlgConnectTo->getAddressAndPort());

		switch(dlgConnectTo->getConnectNetwork())
		{
		case CDialogConnectTo::G2:
		{
			CChatSessionG2* pSession = new CChatSessionG2(ip);
			pSession->connectNode();
			break;
		}
		case CDialogConnectTo::eDonkey:
			break;
		case CDialogConnectTo::Ares:
			break;
		default:
			break;
		}
	}
}
void CWinMain::localAddressChanged()
{
	Network.m_pSection.lock();
	labelCurrentIPAddress->setText(Network.getLocalAddress().toStringWithPort());
	Network.m_pSection.unlock();
}

void CWinMain::onCopyIP()
{
	if(labelCurrentIPAddress->text() != tr("Unknown"))
		QApplication::clipboard()->setText(labelCurrentIPAddress->text());
	else
		QMessageBox::information(this, tr("Unknown IP Address"), tr("The current IP Address is unknown and cannot be copied to the clipboard."), QMessageBox::Ok);
}

void CWinMain::onHasherStarted(int nId)
{
	static CDialogHashProgress* pDialog = 0;
	if( !pDialog )
	{
		pDialog = new CDialogHashProgress(this);
		connect(&ShareManager, SIGNAL(hasherStarted(int)), pDialog, SLOT(onHasherStarted(int)));
		connect(&ShareManager, SIGNAL(hasherFinished(int)), pDialog, SLOT(onHasherFinished(int)));
		connect(&ShareManager, SIGNAL(hashingProgress(int,QString,double,int)), pDialog, SLOT(onHashingProgress(int,QString,double,int)));
		connect(&ShareManager, SIGNAL(remainingFilesChanged(qint32)), pDialog, SLOT(onRemainingFilesChanged(qint32)));
	}
	pDialog->onHasherStarted(nId);
	pDialog->show();
}

void CWinMain::on_actionAbout_Qt_triggered()
{
	qApp->aboutQt();
}

void CWinMain::setSkin()
{
	setStyleSheet(skinSettings.standardItems);
	ui->toolBarNavigation->setStyleSheet(skinSettings.navigationToolbar);
}
