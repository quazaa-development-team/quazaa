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

#include "dialogsettings.h"
#include "ui_dialogsettings.h"

#include "quazaasettings.h"
#include "skinsettings.h"
#include "commonfunctions.h"

#include "dialogprofile.h"

#include <QSettings>
#include <QInputDialog>
#include <QFileDialog>

#include "debug_new.h"

CDialogSettings::CDialogSettings( QWidget* parent, SettingsPage::settingsPage page ) :
	QDialog( parent ),
	ui( new Ui::CDialogSettings )
{
	ui->setupUi( this );
	ui->comboBoxG2Mode->setView( new QListView() );
	ui->comboBoxOnClose->setView( new QListView() );
	ui->comboBoxQueueLength->setView( new QListView() );
	switchSettingsPage( page );

	newSkinSelected = false;

	// Load Basic Settings
	ui->checkBoxStartWithSystem->setChecked( quazaaSettings.System.StartWithSystem );
	ui->checkBoxConnectOnStart->setChecked( quazaaSettings.System.ConnectOnStartup );
	ui->comboBoxOnClose->setCurrentIndex( quazaaSettings.System.CloseMode );
	ui->checkBoxOnMinimize->setChecked( quazaaSettings.System.MinimizeToTray );
	ui->spinBoxDiskWarn->setValue( quazaaSettings.System.DiskSpaceWarning );
	ui->spinBoxDiskStop->setValue( quazaaSettings.System.DiskSpaceStop );

	// Load Parental Settings
	ui->checkBoxParentalChatFilter->setChecked( quazaaSettings.Parental.ChatAdultCensor );
	ui->checkBoxParentalSearchFilter->setChecked( quazaaSettings.Parental.FilterAdultSearchResults );
	ui->listWidgetParentalFilter->addItems( quazaaSettings.Parental.AdultFilter );

	// Load Library Settings
	ui->checkBoxLibraryRememberViews->setChecked( quazaaSettings.Library.RememberViews );
	ui->checkBoxQuickHashing->setChecked( quazaaSettings.Library.HighPriorityHashing );
	ui->checkBoxDisplayHashingProgress->setChecked( quazaaSettings.Library.HashWindow );
	ui->checkBoxLibraryGhostFiles->setChecked( quazaaSettings.Library.GhostFiles );
	ui->checkBoxVideoSeriesDetection->setChecked( quazaaSettings.Library.SmartSeriesDetection );
	ui->spinBoxFileHistoryRemember->setValue( quazaaSettings.Library.HistoryTotal );
	ui->spinBoxFileHistoryDays->setValue( quazaaSettings.Library.HistoryDays );
	ui->listWidgetFileTypesSafeOpen->addItems( quazaaSettings.Library.SafeExecuteTypes );
	ui->listWidgetFileTypesNeverShare->addItems( quazaaSettings.Library.NeverShareTypes );

	// File Types Settings

	// Load Media Player Settings

	// Load Search Settings
	ui->checkBoxSearchExpandMultiSorce->setChecked( quazaaSettings.Search.ExpandSearchMatches );
	ui->checkBoxSearchSwitchOnDownload->setChecked( quazaaSettings.Search.SwitchOnDownload );
	ui->checkBoxSearchHighlightNewMatches->setChecked( quazaaSettings.Search.HighlightNew );

	// Load Private Messages Settings
	ui->checkBoxPrivateMessagesAres->setChecked( quazaaSettings.PrivateMessages.AresEnable );
	ui->checkBoxPrivateMessagesGnutella->setChecked( quazaaSettings.PrivateMessages.Gnutella2Enable );
	ui->checkBoxPrivateMessagesEDonkey->setChecked( quazaaSettings.PrivateMessages.eDonkeyEnable );
	ui->spinBoxPrivateMessagesIdleMessage->setValue( quazaaSettings.PrivateMessages.AwayMessageIdleTime );
	ui->plainTextEditPrivateMessagesIdleMessage->setPlainText( quazaaSettings.PrivateMessages.AwayMessage );

	// Load Connection Settings
	ui->doubleSpinBoxInSpeed->setValue( ( quazaaSettings.Connection.InSpeed / 1024 ) * 8 );
	ui->doubleSpinBoxOutSpeed->setValue( ( quazaaSettings.Connection.OutSpeed / 1024 ) * 8 );
	ui->spinBoxNetworkPort->setValue( quazaaSettings.Connection.Port );
	ui->checkBoxRandomPort->setChecked( quazaaSettings.Connection.RandomPort );
	ui->spinBoxConnectionTimeout->setValue( quazaaSettings.Connection.TimeoutConnect );

	// Load Web Settings
	ui->checkBoxIntegrationMagnetLinks->setChecked( quazaaSettings.Web.Magnet );
	ui->checkBoxIntegrationAresLinks->setChecked( quazaaSettings.Web.Ares );
	ui->checkBoxIntegrationBitTorrentLinks->setChecked( quazaaSettings.Web.Torrent );
	ui->checkBoxIntegrationPioletLinks->setChecked( quazaaSettings.Web.Piolet );
	ui->checkBoxIntegrationEDonkeyLinks->setChecked( quazaaSettings.Web.ED2K );
	ui->checkBoxManageWebDownloads->setChecked( quazaaSettings.Web.BrowserIntegration );
	ui->listWidgetManageDownloadTypes->addItems( quazaaSettings.Web.ManageDownloadTypes );

	// Load Transfer Settings
	ui->checkBoxOnlyDownloadConnectedNetworks->setChecked( quazaaSettings.Transfers.RequireConnectedNetwork );
	ui->checkBoxSimpleProgress->setChecked( quazaaSettings.Transfers.SimpleProgressBar );

	// Load Download Settings
	ui->checkBoxExpandDownloads->setChecked( quazaaSettings.Downloads.ExpandDownloads );
	ui->lineEditSaveFolder->setText( quazaaSettings.Downloads.CompletePath );
	ui->lineEditTempFolder->setText( quazaaSettings.Downloads.IncompletePath );
	ui->comboBoxQueueLength->setCurrentIndex( quazaaSettings.Downloads.QueueLimit );
	ui->spinBoxMaxFiles->setValue( quazaaSettings.Downloads.MaxFiles );
	ui->spinBoxMaxTransfers->setValue( quazaaSettings.Downloads.MaxTransfers );
	ui->spinBoxTransfersPerFile->setValue( quazaaSettings.Downloads.MaxTransfersPerFile );

	// Load Upload Settings
	ui->checkBoxSharePartials->setChecked( quazaaSettings.Uploads.SharePartials );
	ui->checkBoxSharingLimitHub->setChecked( quazaaSettings.Uploads.HubShareLimiting );
	ui->checkBoxSharePreviews->setChecked( quazaaSettings.Uploads.SharePreviews );
	ui->spinBoxUniqueHostLimit->setValue( quazaaSettings.Uploads.MaxPerHost );

	// Load Security Settings
	ui->checkBoxChatFilterSpam->setChecked( quazaaSettings.Security.ChatFilter );
	ui->checkBoxAllowBrowseProfile->setChecked( quazaaSettings.Security.AllowProfileBrowse );
	ui->checkBoxIrcFloodProtection->setChecked( quazaaSettings.Security.IrcFloodProtection );
	ui->spinBoxChatFloodLimit->setValue( quazaaSettings.Security.IrcFloodLimit );
	ui->checkBoxRemoteEnable->setChecked( quazaaSettings.Security.RemoteEnable );
	ui->lineEditRemoteUserName->setText( quazaaSettings.Security.RemoteUsername );
	ui->lineEditRemotePassword->setText( quazaaSettings.Security.RemotePassword );
	ui->checkBoxIgnoreLocalIP->setChecked( quazaaSettings.Security.IgnorePrivateIP );
	ui->checkBoxEnableUPnP->setChecked( quazaaSettings.Security.EnableUPnP );
	ui->checkBoxAllowBrowseShares->setChecked( quazaaSettings.Security.AllowSharesBrowse );
	ui->listWidgetUserAgents->addItems( quazaaSettings.Security.BlockedAgentUploadFilter );

	// Load Gnutella 2 Settings
	ui->checkBoxConnectG2->setChecked( quazaaSettings.Gnutella2.Enable );
	ui->comboBoxG2Mode->setCurrentIndex( quazaaSettings.Gnutella2.ClientMode );
	ui->spinBoxG2LeafToHub->setValue( quazaaSettings.Gnutella2.NumHubs );
	ui->spinBoxG2HubToLeaf->setValue( quazaaSettings.Gnutella2.NumLeafs );
	ui->spinBoxG2HubToHub->setValue( quazaaSettings.Gnutella2.NumPeers );

	// Load Ares Settings
	ui->checkBoxConnectAres->setChecked( quazaaSettings.Ares.Enable );

	// Load eDonkey 2k Settings
	ui->checkBoxConnectEDonkey->setChecked( quazaaSettings.EDonkey.Enable );
	ui->checkBoxConnectKAD->setChecked( quazaaSettings.EDonkey.EnableKad );
	ui->checkBoxED2kSearchCahedServers->setChecked( quazaaSettings.EDonkey.SearchCachedServers );
	ui->spinBoxED2kMaxResults->setValue( quazaaSettings.EDonkey.MaxResults );
	ui->checkBoxED2kUpdateServerList->setChecked( quazaaSettings.EDonkey.LearnNewServers );
	ui->spinBoxED2kMaxClients->setValue( quazaaSettings.EDonkey.MaxClients );
	ui->checkBoxAutoQueryServerList->setChecked( quazaaSettings.EDonkey.MetAutoQuery );
	ui->lineEditEDonkeyServerListUrl->setText( quazaaSettings.EDonkey.ServerListURL );

	// Load BitTorrent Settings
	ui->checkBoxTorrentSaveDialog->setChecked( quazaaSettings.BitTorrent.UseSaveDialog );
	ui->checkBoxTorrentsStartPaused->setChecked( quazaaSettings.BitTorrent.StartPaused );
	ui->checkBoxTorrentsUseTemp->setChecked( quazaaSettings.BitTorrent.UseTemp );
	ui->checkBoxManagedTorrent->setChecked( quazaaSettings.BitTorrent.Managed );
	ui->checkBoxTorrentsEndgame->setChecked( quazaaSettings.BitTorrent.Endgame );
	ui->spinBoxTorrentsSimultaneous->setValue( quazaaSettings.BitTorrent.DownloadTorrents );
	ui->spinBoxTorrentsClientConnections->setValue( quazaaSettings.BitTorrent.DownloadConnections );
	ui->checkBoxTorrentsClearDownloaded->setChecked( quazaaSettings.BitTorrent.AutoClear );
	ui->spinBoxTorrentsRatioClear->setValue( quazaaSettings.BitTorrent.ClearRatio );
	ui->checkBoxTorrentsPreferTorrent->setChecked( quazaaSettings.BitTorrent.PreferBTSources );
	ui->checkBoxTorrentsUseKademlia->setChecked( quazaaSettings.BitTorrent.UseKademlia );
	ui->lineEditTorrentFolder->setText( quazaaSettings.BitTorrent.TorrentPath );

	// Set Generic Control States
	ui->groupBoxParentalFilter->setVisible( false );
	ui->pushButtonApply->setEnabled( false );

	// System Settings
	connect( ui->checkBoxStartWithSystem, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxConnectOnStart, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->comboBoxOnClose, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxOnMinimize, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxDiskWarn, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxDiskStop, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// Parental Settings
	connect( ui->checkBoxParentalChatFilter, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxParentalSearchFilter, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// Library Settings
	connect( ui->checkBoxLibraryRememberViews, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxQuickHashing, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxDisplayHashingProgress, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxLibraryGhostFiles, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxVideoSeriesDetection, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxFileHistoryRemember, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxFileHistoryDays, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// File Types Settings

	// Media Player Settings

	// Search Settings
	connect( ui->checkBoxSearchExpandMultiSorce, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxSearchSwitchOnDownload, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxSearchHighlightNewMatches, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// Private Messages Settings
	connect( ui->checkBoxPrivateMessagesAres, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->plainTextEditPrivateMessagesIdleMessage, SIGNAL( textChanged() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxPrivateMessagesIdleMessage, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxPrivateMessagesEDonkey, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxPrivateMessagesGnutella, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// Chat Settings
	connect( ui->checkBoxPrivateMessagesGnutella, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxPrivateMessagesIdleMessage, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// Connection Settings
	connect( ui->doubleSpinBoxInSpeed, SIGNAL( valueChanged( double ) ), this, SLOT( enableApply() ) );
	connect( ui->doubleSpinBoxOutSpeed, SIGNAL( valueChanged( double ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxNetworkPort, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxRandomPort, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxConnectionTimeout, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// Web Settings
	connect( ui->checkBoxIntegrationMagnetLinks, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxIntegrationAresLinks, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxIntegrationBitTorrentLinks, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxIntegrationPioletLinks, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxIntegrationEDonkeyLinks, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxManageWebDownloads, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// Transfer Settings
	connect( ui->checkBoxOnlyDownloadConnectedNetworks, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxSimpleProgress, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// Download Settings
	connect( ui->checkBoxExpandDownloads, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->lineEditSaveFolder, SIGNAL( textEdited( QString ) ), this, SLOT( enableApply() ) );
	connect( ui->lineEditTempFolder, SIGNAL( textEdited( QString ) ), this, SLOT( enableApply() ) );
	connect( ui->comboBoxQueueLength, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxMaxFiles, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxMaxTransfers, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxTransfersPerFile, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// Upload Settings
	connect( ui->checkBoxSharePartials, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxSharingLimitHub, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxSharePreviews, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxUniqueHostLimit, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// Security Settings
	connect( ui->checkBoxChatFilterSpam, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxAllowBrowseProfile, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxIrcFloodProtection, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxChatFloodLimit, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxRemoteEnable, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->lineEditRemoteUserName, SIGNAL( textEdited( QString ) ), this, SLOT( enableApply() ) );
	connect( ui->lineEditRemotePassword, SIGNAL( textEdited( QString ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxIgnoreLocalIP, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxEnableUPnP, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxAllowBrowseShares, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// Gnutella 2 Settings
	connect( ui->checkBoxConnectG2, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->comboBoxG2Mode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxG2LeafToHub, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxG2HubToLeaf, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxG2HubToHub, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );

	// Ares Settings
	connect( ui->checkBoxConnectAres, SIGNAL( clicked() ), this, SLOT( enableApply() ) );

	// eDonkey 2k Settings
	connect( ui->checkBoxConnectEDonkey, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxConnectKAD, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxED2kSearchCahedServers, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxED2kMaxResults, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxED2kUpdateServerList, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxED2kMaxClients, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxAutoQueryServerList, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->lineEditEDonkeyServerListUrl, SIGNAL( textEdited( QString ) ), this, SLOT( enableApply() ) );

	// BitTorrent Settings
	connect( ui->checkBoxTorrentSaveDialog, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxTorrentsStartPaused, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxTorrentsUseTemp, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxManagedTorrent, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxTorrentsEndgame, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxTorrentsSimultaneous, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxTorrentsClientConnections, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxTorrentsClearDownloaded, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->spinBoxTorrentsRatioClear, SIGNAL( valueChanged( int ) ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxTorrentsPreferTorrent, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->checkBoxTorrentsUseKademlia, SIGNAL( clicked() ), this, SLOT( enableApply() ) );
	connect( ui->lineEditTorrentFolder, SIGNAL( textEdited( QString ) ), this, SLOT( enableApply() ) );
	setSkin();
}

CDialogSettings::~CDialogSettings()
{
	delete ui;

	// free some extra RAM
	common::getRandomUnusedPort( true );
}

void CDialogSettings::changeEvent( QEvent* e )
{
	QDialog::changeEvent( e );
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CDialogSettings::switchSettingsPage( SettingsPage::settingsPage page )
{
	switch ( page )
	{
	case SettingsPage::System:
		ui->stackedWidgetSettings->setCurrentIndex( 0 );
		ui->listWidgetGeneralTask->setCurrentRow( 0, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Skins:
		ui->stackedWidgetSettings->setCurrentIndex( 1 );
		ui->listWidgetGeneralTask->setCurrentRow( 1, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Library:
		ui->stackedWidgetSettings->setCurrentIndex( 2 );
		ui->listWidgetGeneralTask->setCurrentRow( 2, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::FileTypes:
		ui->stackedWidgetSettings->setCurrentIndex( 3 );
		ui->listWidgetGeneralTask->setCurrentRow( 3, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::MediaPlayer:
		ui->stackedWidgetSettings->setCurrentIndex( 4 );
		ui->listWidgetGeneralTask->setCurrentRow( 4, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Search:
		ui->stackedWidgetSettings->setCurrentIndex( 5 );
		ui->listWidgetGeneralTask->setCurrentRow( 5, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Integration:
		ui->stackedWidgetSettings->setCurrentIndex( 6 );
		ui->listWidgetGeneralTask->setCurrentRow( 6, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Plugins:
		ui->stackedWidgetSettings->setCurrentIndex( 7 );
		ui->listWidgetGeneralTask->setCurrentRow( 7, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::PrivateMessages:
		ui->stackedWidgetSettings->setCurrentIndex( 8 );
		ui->listWidgetCommunityTask->setCurrentRow( 0, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( true );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Chat:
		ui->stackedWidgetSettings->setCurrentIndex( 9 );
		ui->listWidgetCommunityTask->setCurrentRow( 1, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( true );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Security:
		ui->stackedWidgetSettings->setCurrentIndex( 10 );
		ui->listWidgetSecurityTask->setCurrentRow( 0, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( true );
		break;
	case SettingsPage::Parental:
		ui->stackedWidgetSettings->setCurrentIndex( 11 );
		ui->listWidgetSecurityTask->setCurrentRow( 1, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( true );
		break;
	case SettingsPage::Connection:
		ui->stackedWidgetSettings->setCurrentIndex( 12 );
		ui->listWidgetNetworkTask->setCurrentRow( 0, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( true );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Transfers:
		ui->stackedWidgetSettings->setCurrentIndex( 13 );
		ui->listWidgetNetworkTask->setCurrentRow( 1, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( true );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Downloads:
		ui->stackedWidgetSettings->setCurrentIndex( 14 );
		ui->listWidgetNetworkTask->setCurrentRow( 2, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( true );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Uploads:
		ui->stackedWidgetSettings->setCurrentIndex( 15 );
		ui->listWidgetNetworkTask->setCurrentRow( 3, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( true );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Gnutella2:
		ui->stackedWidgetSettings->setCurrentIndex( 16 );
		ui->listWidgetProtocolsTask->setCurrentRow( 0, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( true );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Ares:
		ui->stackedWidgetSettings->setCurrentIndex( 17 );
		ui->listWidgetProtocolsTask->setCurrentRow( 1, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( true );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::EDonkey:
		ui->stackedWidgetSettings->setCurrentIndex( 18 );
		ui->listWidgetProtocolsTask->setCurrentRow( 2, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( true );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::BitTorrent:
		ui->stackedWidgetSettings->setCurrentIndex( 19 );
		ui->listWidgetProtocolsTask->setCurrentRow( 3, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( true );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	case SettingsPage::Protocols:
		ui->stackedWidgetSettings->setCurrentIndex( 20 );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( false );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( true );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	default:
		ui->stackedWidgetSettings->setCurrentIndex( 0 );
		ui->listWidgetGeneralTask->setCurrentRow( 0, QItemSelectionModel::SelectCurrent );
		ui->toolButtonCommunityTask->setChecked( false );
		ui->toolButtonGeneralTask->setChecked( true );
		ui->toolButtonNetworkTask->setChecked( false );
		ui->toolButtonProtocolsTask->setChecked( false );
		ui->toolButtonSecurityTask->setChecked( false );
		break;
	}
}

void CDialogSettings::on_listWidgetGeneralTask_clicked( QModelIndex index )
{
	ui->stackedWidgetSettings->setCurrentIndex( index.row() );
}

void CDialogSettings::on_listWidgetCommunityTask_clicked( QModelIndex index )
{
	ui->stackedWidgetSettings->setCurrentIndex( index.row() + 8 );
}

void CDialogSettings::on_listWidgetSecurityTask_clicked( QModelIndex index )
{
	ui->stackedWidgetSettings->setCurrentIndex( index.row() + 10 );
}

void CDialogSettings::on_listWidgetNetworkTask_clicked( QModelIndex index )
{
	ui->stackedWidgetSettings->setCurrentIndex( index.row() + 12 );
}

void CDialogSettings::on_listWidgetProtocolsTask_clicked( QModelIndex index )
{
	ui->stackedWidgetSettings->setCurrentIndex( index.row() + 16 );
}

void CDialogSettings::on_pushButtonOk_clicked()
{
	if ( ui->pushButtonApply->isEnabled() )
	{
		ui->pushButtonApply->click();
	}
	emit closed();
	close();
}

void CDialogSettings::on_pushButtonCancel_clicked()
{
	emit closed();
	close();
}

void CDialogSettings::on_pushButtonApply_clicked()
{
	// Save Basic Settings
	quazaaSettings.System.StartWithSystem = ui->checkBoxStartWithSystem->isChecked();
	quazaaSettings.System.ConnectOnStartup = ui->checkBoxConnectOnStart->isChecked();
	quazaaSettings.System.CloseMode = ui->comboBoxOnClose->currentIndex();
	quazaaSettings.System.MinimizeToTray = ui->checkBoxOnMinimize->isChecked();
	quazaaSettings.System.DiskSpaceWarning = ui->spinBoxDiskWarn->value();
	quazaaSettings.System.DiskSpaceStop = ui->spinBoxDiskStop->value();

	// Save Parental Settings
	quazaaSettings.Parental.ChatAdultCensor = ui->checkBoxParentalChatFilter->isChecked();
	quazaaSettings.Parental.FilterAdultSearchResults = ui->checkBoxParentalSearchFilter->isChecked();
	quazaaSettings.Parental.AdultFilter.clear();
	for ( int m_iAdultFilterRow = 0; m_iAdultFilterRow < ui->listWidgetParentalFilter->count(); m_iAdultFilterRow++ )
	{
		ui->listWidgetParentalFilter->setCurrentRow( m_iAdultFilterRow );
		quazaaSettings.Parental.AdultFilter.append( ui->listWidgetParentalFilter->currentItem()->text() );
	}

	// Save Library Settings
	quazaaSettings.Library.RememberViews = ui->checkBoxLibraryRememberViews->isChecked();
	quazaaSettings.Library.HighPriorityHashing = ui->checkBoxQuickHashing->isChecked();
	quazaaSettings.Library.HashWindow = ui->checkBoxDisplayHashingProgress->isChecked();
	quazaaSettings.Library.GhostFiles = ui->checkBoxLibraryGhostFiles->isChecked();
	quazaaSettings.Library.SmartSeriesDetection = ui->checkBoxVideoSeriesDetection->isChecked();
	quazaaSettings.Library.HistoryTotal = ui->spinBoxFileHistoryRemember->value();
	quazaaSettings.Library.HistoryDays = ui->spinBoxFileHistoryDays->value();
	quazaaSettings.Library.SafeExecuteTypes.clear();
	for ( int m_iSafeOpenRow = 0; m_iSafeOpenRow < ui->listWidgetFileTypesSafeOpen->count(); m_iSafeOpenRow++ )
	{
		ui->listWidgetFileTypesSafeOpen->setCurrentRow( m_iSafeOpenRow );
		quazaaSettings.Library.SafeExecuteTypes.append( ui->listWidgetFileTypesSafeOpen->currentItem()->text() );
	}
	quazaaSettings.Library.NeverShareTypes.clear();
	for ( int m_iNeverShareRow = 0; m_iNeverShareRow < ui->listWidgetFileTypesNeverShare->count(); m_iNeverShareRow++ )
	{
		ui->listWidgetFileTypesNeverShare->setCurrentRow( m_iNeverShareRow );
		quazaaSettings.Library.NeverShareTypes.append( ui->listWidgetFileTypesNeverShare->currentItem()->text() );
	}

	// Save File Types Settings

	// Save Media Player Settings

	// Save Search Settings
	quazaaSettings.Search.ExpandSearchMatches = ui->checkBoxSearchExpandMultiSorce->isChecked();
	quazaaSettings.Search.SwitchOnDownload = ui->checkBoxSearchSwitchOnDownload->isChecked();
	quazaaSettings.Search.HighlightNew = ui->checkBoxSearchHighlightNewMatches->isChecked();

	// Save Private Messages Settings
	quazaaSettings.PrivateMessages.AresEnable = ui->checkBoxPrivateMessagesAres->isChecked();
	quazaaSettings.PrivateMessages.AwayMessage = ui->plainTextEditPrivateMessagesIdleMessage->toPlainText();
	quazaaSettings.PrivateMessages.AwayMessageIdleTime = ui->spinBoxPrivateMessagesIdleMessage->value();
	quazaaSettings.PrivateMessages.eDonkeyEnable = ui->checkBoxPrivateMessagesEDonkey->isChecked();
	quazaaSettings.PrivateMessages.Gnutella2Enable = ui->checkBoxPrivateMessagesGnutella->isChecked();

	// Save Connection Settings
	quazaaSettings.Connection.InSpeed = ( ui->doubleSpinBoxInSpeed->value() / 8 ) * 1024;
	quazaaSettings.Connection.OutSpeed = ( ui->doubleSpinBoxOutSpeed->value() / 8 ) * 1024;
	quazaaSettings.Connection.Port = ui->spinBoxNetworkPort->value();
	quazaaSettings.Connection.RandomPort = ui->checkBoxRandomPort->isChecked();
	quazaaSettings.Connection.TimeoutConnect = ui->spinBoxConnectionTimeout->value();

	// Save Web Settings
	quazaaSettings.Web.Magnet = ui->checkBoxIntegrationMagnetLinks->isChecked();
	quazaaSettings.Web.Ares = ui->checkBoxIntegrationAresLinks->isChecked();
	quazaaSettings.Web.Torrent = ui->checkBoxIntegrationBitTorrentLinks->isChecked();
	quazaaSettings.Web.Piolet = ui->checkBoxIntegrationPioletLinks->isChecked();
	quazaaSettings.Web.ED2K = ui->checkBoxIntegrationEDonkeyLinks->isChecked();
	quazaaSettings.Web.BrowserIntegration = ui->checkBoxManageWebDownloads->isChecked();
	quazaaSettings.Web.ManageDownloadTypes.clear();
	for ( int m_iDownloadTypesRow = 0; m_iDownloadTypesRow < ui->listWidgetManageDownloadTypes->count();
		  m_iDownloadTypesRow++ )
	{
		ui->listWidgetManageDownloadTypes->setCurrentRow( m_iDownloadTypesRow );
		quazaaSettings.Web.ManageDownloadTypes.append( ui->listWidgetManageDownloadTypes->currentItem()->text() );
	}

	// Save Transfer Settings
	quazaaSettings.Transfers.RequireConnectedNetwork = ui->checkBoxOnlyDownloadConnectedNetworks->isChecked();
	quazaaSettings.Transfers.SimpleProgressBar = ui->checkBoxSimpleProgress->isChecked();

	// Save Download Settings
	quazaaSettings.Downloads.ExpandDownloads = ui->checkBoxExpandDownloads->isChecked();
	quazaaSettings.Downloads.CompletePath = ui->lineEditSaveFolder->text();
	quazaaSettings.Downloads.IncompletePath = ui->lineEditTempFolder->text();
	quazaaSettings.Downloads.QueueLimit = ui->comboBoxQueueLength->currentIndex();
	quazaaSettings.Downloads.MaxFiles = ui->spinBoxMaxFiles->value();
	quazaaSettings.Downloads.MaxTransfers = ui->spinBoxMaxTransfers->value();
	quazaaSettings.Downloads.MaxTransfersPerFile = ui->spinBoxTransfersPerFile->value();

	// Save Upload Settings
	quazaaSettings.Uploads.SharePartials = ui->checkBoxSharePartials->isChecked();
	quazaaSettings.Uploads.HubShareLimiting = ui->checkBoxSharingLimitHub->isChecked();
	quazaaSettings.Uploads.SharePreviews = ui->checkBoxSharePreviews->isChecked();
	quazaaSettings.Uploads.MaxPerHost = ui->spinBoxUniqueHostLimit->value();

	// Save Security Settings
	quazaaSettings.Security.ChatFilter = ui->checkBoxChatFilterSpam->isChecked();
	quazaaSettings.Security.AllowProfileBrowse = ui->checkBoxAllowBrowseProfile->isChecked();
	quazaaSettings.Security.IrcFloodProtection = ui->checkBoxIrcFloodProtection->isChecked();
	quazaaSettings.Security.IrcFloodLimit = ui->spinBoxChatFloodLimit->value();
	quazaaSettings.Security.RemoteEnable = ui->checkBoxRemoteEnable->isChecked();
	quazaaSettings.Security.RemoteUsername = ui->lineEditRemoteUserName->text();
	quazaaSettings.Security.RemotePassword = ui->lineEditRemotePassword->text();
	quazaaSettings.Security.IgnorePrivateIP = ui->checkBoxIgnoreLocalIP->isChecked();
	quazaaSettings.Security.EnableUPnP = ui->checkBoxEnableUPnP->isChecked();
	quazaaSettings.Security.AllowSharesBrowse = ui->checkBoxAllowBrowseShares->isChecked();
	quazaaSettings.Security.BlockedAgentUploadFilter.clear();
	for ( int m_iUserAgentsRow = 0; m_iUserAgentsRow < ui->listWidgetUserAgents->count(); m_iUserAgentsRow++ )
	{
		ui->listWidgetUserAgents->setCurrentRow( m_iUserAgentsRow );
		quazaaSettings.Security.BlockedAgentUploadFilter.append( ui->listWidgetUserAgents->currentItem()->text() );
	}

	// Save Gnutella 2 Settings
	quazaaSettings.Gnutella2.Enable = ui->checkBoxConnectG2->isChecked();
	quazaaSettings.Gnutella2.ClientMode = ui->comboBoxG2Mode->currentIndex();
	quazaaSettings.Gnutella2.NumHubs = ui->spinBoxG2LeafToHub->value();
	quazaaSettings.Gnutella2.NumLeafs = ui->spinBoxG2HubToLeaf->value();
	quazaaSettings.Gnutella2.NumPeers = ui->spinBoxG2HubToHub->value();

	// Save Ares Settings
	quazaaSettings.Ares.Enable = ui->checkBoxConnectAres->isChecked();

	// Save eDonkey 2k Settings
	quazaaSettings.EDonkey.Enable = ui->checkBoxConnectEDonkey->isChecked();
	quazaaSettings.EDonkey.EnableKad = ui->checkBoxConnectKAD->isChecked();
	quazaaSettings.EDonkey.SearchCachedServers = ui->checkBoxED2kSearchCahedServers->isChecked();
	quazaaSettings.EDonkey.MaxResults = ui->spinBoxED2kMaxResults->value();
	quazaaSettings.EDonkey.LearnNewServers = ui->checkBoxED2kUpdateServerList->isChecked();
	quazaaSettings.EDonkey.MaxClients = ui->spinBoxED2kMaxClients->value();
	quazaaSettings.EDonkey.MetAutoQuery = ui->checkBoxAutoQueryServerList->isChecked();
	quazaaSettings.EDonkey.ServerListURL = ui->lineEditEDonkeyServerListUrl->text();

	// Save BitTorrent Settings
	quazaaSettings.BitTorrent.UseSaveDialog = ui->checkBoxTorrentSaveDialog->isChecked();
	quazaaSettings.BitTorrent.StartPaused = ui->checkBoxTorrentsStartPaused->isChecked();
	quazaaSettings.BitTorrent.UseTemp = ui->checkBoxTorrentsUseTemp->isChecked();
	quazaaSettings.BitTorrent.Managed = ui->checkBoxManagedTorrent->isChecked();
	quazaaSettings.BitTorrent.Endgame = ui->checkBoxTorrentsEndgame->isChecked();
	quazaaSettings.BitTorrent.DownloadTorrents = ui->spinBoxTorrentsSimultaneous->value();
	quazaaSettings.BitTorrent.DownloadConnections = ui->spinBoxTorrentsClientConnections->value();
	quazaaSettings.BitTorrent.AutoClear = ui->checkBoxTorrentsClearDownloaded->isChecked();
	quazaaSettings.BitTorrent.ClearRatio = ui->spinBoxTorrentsRatioClear->value();
	quazaaSettings.BitTorrent.PreferBTSources = ui->checkBoxTorrentsPreferTorrent->isChecked();
	quazaaSettings.BitTorrent.UseKademlia = ui->checkBoxTorrentsUseKademlia->isChecked();
	quazaaSettings.BitTorrent.TorrentPath = ui->lineEditTorrentFolder->text();

	// Reset Apply Enabled To False
	quazaaSettings.saveSettings();
	ui->pushButtonApply->setEnabled( false );
}

void CDialogSettings::on_pushButtonProfileEdit_clicked()
{
	CDialogProfile* dlgProfile = new CDialogProfile( this );

	dlgProfile->exec();
}

void CDialogSettings::on_pushButtonShowParentalFilter_clicked()
{
	ui->groupBoxParentalFilter->setVisible( true );
}

void CDialogSettings::on_labelConfigureG2_linkActivated( QString link )
{
	Q_UNUSED( link );
	switchSettingsPage( SettingsPage::Gnutella2 );
}

void CDialogSettings::on_labelConfigureAres_linkActivated( QString link )
{
	Q_UNUSED( link );
	switchSettingsPage( SettingsPage::Ares );
}

void CDialogSettings::on_labelConfigureEDonkey_linkActivated( QString link )
{
	Q_UNUSED( link );
	switchSettingsPage( SettingsPage::EDonkey );
}

void CDialogSettings::on_labelConfigureBitTorrent_linkActivated( QString link )
{
	Q_UNUSED( link );
	switchSettingsPage( SettingsPage::BitTorrent );
}

void CDialogSettings::on_listWidgetSkins_itemClicked( QListWidgetItem* item )
{
	newSkinSelected = true;
	ui->pushButtonApply->setEnabled( true );
	QSettings reader( ( qApp->applicationDirPath() + "/Skin/" + item->text() + "/" + item->text() + ".qsk" ),
					  QSettings::IniFormat );
	skinFile = ( qApp->applicationDirPath() + "/Skin/" + item->text() + "/" + item->text() + ".qsk" );
	tempSkinName = reader.value( "skinName", "" ).toString();
	tempSkinAuthor = reader.value( "skinAuthor", "" ).toString();
	tempSkinVersion = reader.value( "skinVersion", "" ).toString();
	tempSkinDescription = reader.value( "skinDescription", "" ).toString();

	ui->labelSkinAuthor->setText( tr( "Author: %1" ).arg( tempSkinAuthor ) );
	ui->labelSkinVersion->setText( tr( "Version: %1" ).arg( tempSkinVersion ) );
	ui->plainTextEditSkinDescription->setPlainText( tempSkinDescription );
}

void CDialogSettings::on_pushButtonSkinPreview_clicked()
{

}

void CDialogSettings::on_pushButtonFileTypesSafeOpenAdd_clicked()
{
	bool m_bOkPressed;
	QString m_sSafeOpenItem = QInputDialog::getText( this, tr( "Add Safe To Open File Type" ),
													 tr( "Type the extension of the file type to add and press OK. \nWARNING: This file type will always be opened!" ),
													 QLineEdit::Normal, "", &m_bOkPressed );
	if ( m_bOkPressed && ( !m_sSafeOpenItem.isEmpty() ) )
	{
		ui->listWidgetFileTypesSafeOpen->addItem( m_sSafeOpenItem );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonFileTypesNeverShareAdd_clicked()
{
	bool m_bOkPressed;
	QString m_sNeverShareItem = QInputDialog::getText( this, tr( "Add Never Share File Type" ),
													   tr( "Type the extension of the file type to add and press OK. \nWARNING: This file type will never be shared!" ),
													   QLineEdit::Normal, "", &m_bOkPressed );
	if ( m_bOkPressed && ( !m_sNeverShareItem.isEmpty() ) )
	{
		ui->listWidgetFileTypesNeverShare->addItem( m_sNeverShareItem );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonAddManageDownloadTypes_clicked()
{
	bool m_bOkPressed;
	QString m_sDownloadManageItem = QInputDialog::getText( this, tr( "Add Download Types to Manage" ),
														   tr( "Type the extension of the file type you want the download manager to manage and press OK. \nQuazaa will detect downloads of these types and manage them for you." ),
														   QLineEdit::Normal, "", &m_bOkPressed );
	if ( m_bOkPressed && ( !m_sDownloadManageItem.isEmpty() ) )
	{
		ui->listWidgetManageDownloadTypes->addItem( m_sDownloadManageItem );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonUserAgentAdd_clicked()
{
	bool m_bOkPressed;
	QString m_sUserAgentItem = QInputDialog::getText( this, tr( "Add User Agents to Block" ),
													  tr( "Type the vendor code of the client to block and press OK. \nWARNING: Quazaa will block all communications with this client!" ),
													  QLineEdit::Normal, "", &m_bOkPressed );
	if ( m_bOkPressed && ( !m_sUserAgentItem.isEmpty() ) )
	{
		ui->listWidgetUserAgents->addItem( m_sUserAgentItem );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonAddParentalFilter_clicked()
{
	bool m_bOkPressed;
	QString m_sParentalFilterItem = QInputDialog::getText( this, tr( "Add Words to Block" ),
														   tr( "Type the word to block and press OK. \nWARNING: No downloads containing these words will be listed in searches \nand these words will be replaced by \"!%@$#\" in chat!" ),
														   QLineEdit::Normal, "", &m_bOkPressed );
	if ( m_bOkPressed && ( !m_sParentalFilterItem.isEmpty() ) )
	{
		ui->listWidgetParentalFilter->addItem( m_sParentalFilterItem );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonFileTypesSafeOpenRemove_clicked()
{
	if ( ui->listWidgetFileTypesSafeOpen->currentRow() != -1 )
	{
		ui->listWidgetFileTypesSafeOpen->takeItem( ui->listWidgetFileTypesSafeOpen->currentRow() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonFileTypesNeverShareRemove_clicked()
{
	if ( ui->listWidgetFileTypesNeverShare->currentRow() != -1 )
	{
		ui->listWidgetFileTypesNeverShare->takeItem( ui->listWidgetFileTypesNeverShare->currentRow() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonRemoveManageDownloadTypes_clicked()
{
	if ( ui->listWidgetManageDownloadTypes->currentRow() != -1 )
	{
		ui->listWidgetManageDownloadTypes->takeItem( ui->listWidgetManageDownloadTypes->currentRow() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonUserAgentRemove_clicked()
{
	if ( ui->listWidgetUserAgents->currentRow() != -1 )
	{
		ui->listWidgetUserAgents->takeItem( ui->listWidgetUserAgents->currentRow() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_pushButtonRemoveParentalFilter_clicked()
{
	if ( ui->listWidgetParentalFilter->currentRow() != -1 )
	{
		ui->listWidgetParentalFilter->takeItem( ui->listWidgetParentalFilter->currentRow() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_toolButtonSaveBrowse_clicked()
{
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QDir directory( QFileDialog::getExistingDirectory( this,
													   tr( "Select A Location To Save Files" ),
													   ui->lineEditSaveFolder->text(),
													   options ) );
	if ( directory.exists() )
	{
		ui->lineEditSaveFolder->setText( directory.canonicalPath() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_toolButtonTempBrowse_clicked()
{
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QDir directory( QFileDialog::getExistingDirectory( this,
													   tr( "Select A Folder For Temp Files" ),
													   ui->lineEditTempFolder->text(),
													   options ) );
	if ( directory.exists() )
	{
		ui->lineEditTempFolder->setText( directory.canonicalPath() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::on_toolButtonTorrentBrowse_clicked()
{
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QDir directory( QFileDialog::getExistingDirectory( this,
													   tr( "Select A Folder To Store .torrent Files" ),
													   ui->lineEditTorrentFolder->text(),
													   options ) );
	if ( directory.exists() )
	{
		ui->lineEditTorrentFolder->setText( directory.canonicalPath() );
		ui->pushButtonApply->setEnabled( true );
	}
}

void CDialogSettings::enableApply()
{
	ui->pushButtonApply->setEnabled( true );
}

void CDialogSettings::setSkin()
{
	ui->frameSidebarContents->setStyleSheet( skinSettings.sidebarBackground );
	ui->toolButtonCommunityTask->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->frameCommunityTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->toolButtonGeneralTask->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->frameGeneralTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->toolButtonNetworkTask->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->frameNetworkTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->toolButtonProtocolsTask->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->frameProtocolsTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->toolButtonSecurityTask->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->frameSecurityTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->listWidgetCommunityTask->setStyleSheet( skinSettings.listViews );
	ui->listWidgetFileTypesNeverShare->setStyleSheet( skinSettings.listViews );
	ui->listWidgetFileTypesSafeOpen->setStyleSheet( skinSettings.listViews );
	ui->listWidgetGeneralTask->setStyleSheet( skinSettings.listViews );
	ui->listWidgetManageDownloadTypes->setStyleSheet( skinSettings.listViews );
	ui->listWidgetNetworkTask->setStyleSheet( skinSettings.listViews );
	ui->listWidgetParentalFilter->setStyleSheet( skinSettings.listViews );
	ui->listWidgetPlugins->setStyleSheet( skinSettings.listViews );
	ui->listWidgetProtocolsTask->setStyleSheet( skinSettings.listViews );
	ui->listWidgetSecurityTask->setStyleSheet( skinSettings.listViews );
	ui->listWidgetSkins->setStyleSheet( skinSettings.listViews );
	ui->listWidgetUserAgents->setStyleSheet( skinSettings.listViews );
	ui->treeWidgetQueues->setStyleSheet( skinSettings.listViews );
	ui->tableWidgetFileTypesOpenWith->setStyleSheet( skinSettings.listViews );
}

void CDialogSettings::on_toolButton_clicked()
{
	ui->spinBoxNetworkPort->setValue( common::getRandomUnusedPort() );
}
