/*
** winmain.h
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

#ifndef WINMAIN_H
#define WINMAIN_H

#include <QMainWindow>
#include <QActionGroup>
#include <QSystemTrayIcon>
#include <QHideEvent>

#include "widgethome.h"
#include "widgetlibrary.h"
#include "widgetmedia.h"
#include "widgetsearch.h"
#include "widgettransfers.h"
#include "widgetsecurity.h"
#include "widgetactivity.h"
#include "widgethostcache.h"
#include "widgetdiscovery.h"
#include "widgetscheduler.h"
#include "widgetgraph.h"
#include "widgetpacketdump.h"
#include "widgetsearchmonitor.h"
#include "widgethitmonitor.h"

#include "widgetircmain.h"

#include "dialogprivatemessages.h"
#include <QList>
#include <QUuid>
#include <QHBoxLayout>

class ChatSession;

namespace Ui
{
class CWinMain;
}

class CWinMain : public QMainWindow
{
	Q_OBJECT
public:
	CWinMain( QWidget* parent = 0 );
	~CWinMain();
	void loadTrayIcon();

	bool interfaceLoaded;
	QTimer* neighboursRefresher;
	QActionGroup* actionGroupMainNavigation;
	QLabel* labelBandwidthTotals;
	QLabel* labelFirewallStatus;
	QLabel* labelIPAddress;
	QLabel* labelCurrentIPAddress;
	QToolButton* toolButtonAddressToClipboard;
	QFrame* frameStatusSpacer;
	QHBoxLayout* layoutStatusSpacer;

	QSystemTrayIcon* trayIcon;
	QMenu* trayMenu;
	CWidgetHome* pageHome;
	CWidgetLibrary* pageLibrary;
	CWidgetMedia* pageMedia;
	WidgetSearch* pageSearch;
	CWidgetTransfers* pageTransfers;
	CWidgetSecurity* pageSecurity;
	WidgetActivity* pageActivity;
	CWidgetIrcMain* pageIrcMain;
	WidgetHostCache* pageHostCache;
	CWidgetDiscovery* pageDiscovery;
	CWidgetScheduler* pageScheduler;
	CWidgetGraph* pageGraph;
	CWidgetPacketDump* pagePacketDump;
	CWidgetSearchMonitor* pageSearchMonitor;
	CWidgetHitMonitor* pageHitMonitor;
	CDialogPrivateMessages* dlgPrivateMessages;

signals:
	void Show();
	void shutDown();

public slots:
	void OpenChat( ChatSession* pSess );
	void showOnTop();

protected:
	void changeEvent( QEvent* e );
	bool event( QEvent* e );

private:
	Ui::CWinMain* ui;
	bool bypassCloseEvent;
	QString tcpFirewalled;
	QString udpFirewalled;

private slots:
	void on_actionChatWith_triggered();
	void on_actionConnectTo_triggered();
	void on_actionAres_triggered( bool checked );
	void on_actionGnutella2_triggered( bool checked );
	void on_actionEDonkey_triggered( bool checked );
	void on_actionDisconnect_triggered();
	void on_actionConnect_triggered();
	void on_actionNewSearch_triggered();
	void on_actionEditMyProfile_triggered();
	void on_actionQuazaaForums_triggered();
	void on_actionDonate_triggered();
	void on_actionCheckForNewVersion_triggered();
	void on_actionConnectionTest_triggered();
	void on_actionFAQ_triggered();
	void on_actionUsersGuide_triggered();
	void on_actionQuickstartWizard_triggered();
	void on_actionChooseLanguage_triggered();
	void on_actionChooseSkin_triggered();
	void on_actionImportPartials_triggered();
	void on_actionURLDownload_triggered();
	void on_actionOpenDownloadFolder_triggered();
	void on_actionShares_triggered();
	void on_actionOpenTorrent_triggered();
	void on_actionSeedTorrent_triggered();
	void on_actionCreateTorrent_triggered();
	void on_actionSettings_triggered();
	void on_actionAbout_triggered();
	void on_actionExit_triggered();
	void on_actionShowOrHide_triggered();
	void on_actionHitMonitor_triggered();
	void on_actionSearchMonitor_triggered();
	void on_actionPacketDump_triggered();
	void on_actionGraph_triggered();
	void on_actionScheduler_triggered();
	void on_actionDiscovery_triggered();
	void on_actionHostCache_triggered();
	void on_actionChat_triggered();
	void on_actionSecurity_triggered();
	void on_actionActivity_triggered();
	void on_actionTransfers_triggered();
	void on_actionSearch_triggered();
	void on_actionMedia_triggered();
	void on_actionLibrary_triggered();
	void on_actionHome_triggered();
	void quazaaShutdown();
	void quazaaStartup();
	void icon_activated( QSystemTrayIcon::ActivationReason reason );
	void startNewSearch( QString* searchString );
	void updateStatusBar();
	void localAddressChanged();
	void onCopyIP();
	void onHasherStarted( int nId );
	void on_actionAbout_Qt_triggered();
	void setSkin();
};

extern CWinMain* mainWindow;
#endif // WINMAIN_H
