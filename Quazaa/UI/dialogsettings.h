/*
** dialogsettings.h
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

#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QListWidgetItem>

namespace SettingsPage
{
enum settingsPage
{
	System, Skins, Library, FileTypes, MediaPlayer, Search, Integration, Plugins, PrivateMessages, Chat, Security,
	Parental, Connection, Transfers, Downloads, Uploads, Gnutella2, Ares, EDonkey, BitTorrent, Protocols
};
}

namespace Ui
{
class CDialogSettings;
}

class CDialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit CDialogSettings( QWidget* parent = 0, SettingsPage::settingsPage page = SettingsPage::System );
	~CDialogSettings();

public slots:
	void enableApply();

protected:
	void changeEvent( QEvent* e );

private slots:
	void switchSettingsPage( SettingsPage::settingsPage page );
	void on_listWidgetGeneralTask_clicked( QModelIndex index );
	void on_listWidgetCommunityTask_clicked( QModelIndex index );
	void on_listWidgetSecurityTask_clicked( QModelIndex index );
	void on_listWidgetNetworkTask_clicked( QModelIndex index );
	void on_listWidgetProtocolsTask_clicked( QModelIndex index );
	void on_pushButtonOk_clicked();
	void on_pushButtonCancel_clicked();
	void on_pushButtonApply_clicked();

	void on_pushButtonProfileEdit_clicked();
	void on_pushButtonShowParentalFilter_clicked();
	void on_labelConfigureG2_linkActivated( QString link );
	void on_labelConfigureAres_linkActivated( QString link );
	void on_labelConfigureEDonkey_linkActivated( QString link );
	void on_labelConfigureBitTorrent_linkActivated( QString link );
	void on_listWidgetSkins_itemClicked( QListWidgetItem* item );
	void on_pushButtonSkinPreview_clicked();
	void on_pushButtonFileTypesSafeOpenAdd_clicked();
	void on_pushButtonFileTypesNeverShareAdd_clicked();
	void on_pushButtonAddManageDownloadTypes_clicked();
	void on_pushButtonUserAgentAdd_clicked();
	void on_pushButtonAddParentalFilter_clicked();
	void on_pushButtonFileTypesSafeOpenRemove_clicked();
	void on_pushButtonFileTypesNeverShareRemove_clicked();
	void on_pushButtonRemoveManageDownloadTypes_clicked();
	void on_pushButtonUserAgentRemove_clicked();
	void on_pushButtonRemoveParentalFilter_clicked();
	void on_toolButtonSaveBrowse_clicked();
	void on_toolButtonTempBrowse_clicked();
	void on_toolButtonTorrentBrowse_clicked();
	void setSkin();

	void on_toolButton_clicked();

private:
	Ui::CDialogSettings* ui;
	QString skinFile;
	QString tempSkinName;
	QString tempSkinAuthor;
	QString tempSkinVersion;
	QString tempSkinDescription;
	bool newSkinSelected;

signals:
	void closed();
};

#endif // DIALOGSETTINGS_H
