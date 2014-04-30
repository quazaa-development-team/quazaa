/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2012.
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

#include "wizardquickstart.h"
#include "ui_wizardquickstart.h"
#include "quazaasettings.h"
#include "skinsettings.h"

#include "commonfunctions.h"

#include <QFileDialog>

#include "debug_new.h"

CWizardQuickStart::CWizardQuickStart( QWidget* parent ) :
	QWizard( parent ),
	ui( new Ui::CWizardQuickStart )
{
	ui->setupUi( this );
	this->setPixmap( QWizard::LogoPixmap, QPixmap( ":/Resource/Quazaa48.png" ) );
	ui->comboBoxGender->setView( new QListView() );
	ui->doubleSpinBoxInSpeed->setValue( ( quazaaSettings.Connection.InSpeed / 1024 ) * 8 );
	ui->doubleSpinBoxOutSpeed->setValue( ( quazaaSettings.Connection.OutSpeed / 1024 ) * 8 );
	ui->spinBoxPort->setValue( quazaaSettings.Connection.Port );
	ui->checkBoxRandomPort->setChecked( quazaaSettings.Connection.RandomPort );
	ui->checkBoxUPnP->setChecked( quazaaSettings.Security.EnableUPnP );
	ui->checkBoxAllowBrowseShares->setChecked( quazaaSettings.Security.AllowSharesBrowse );
	ui->listWidgetShares->addItems( quazaaSettings.Library.Shares );
	ui->checkBoxAllowBrowseProfile->setChecked( quazaaSettings.Security.AllowProfileBrowse );
	ui->lineEditIrcNick->setText( quazaaSettings.Profile.IrcNickname );
	ui->lineEditIrcAlternateNick->setText( quazaaSettings.Profile.IrcAlternateNickname );
	ui->lineEditGnutellaScreenName->setText( quazaaSettings.Profile.GnutellaScreenName );
	ui->comboBoxGender->setCurrentIndex( quazaaSettings.Profile.Gender );
	ui->spinBoxAge->setValue( quazaaSettings.Profile.Age );
	ui->lineEditCountry->setText( quazaaSettings.Profile.Country );
	ui->lineEditStateProvince->setText( quazaaSettings.Profile.StateProvince );
	ui->lineEditCity->setText( quazaaSettings.Profile.City );
	ui->checkBoxG2Connect->setChecked( quazaaSettings.Gnutella2.Enable );
	ui->checkBoxAresConnect->setChecked( quazaaSettings.Ares.Enable );
	ui->checkBoxEDonkeyConnect->setChecked( quazaaSettings.EDonkey.Enable );
	ui->checkBoxKADConnect->setChecked( quazaaSettings.EDonkey.EnableKad );
	ui->checkBoxBitTorrent->setChecked( quazaaSettings.Web.Torrent );
	ui->checkBoxBitTorrentKademliaConnect->setChecked( quazaaSettings.BitTorrent.UseKademlia );
	ui->checkBoxConnectNetworksAlways->setChecked( quazaaSettings.System.ConnectOnStartup );
	ui->checkBoxOSStart->setChecked( quazaaSettings.System.StartWithSystem );
	ui->checkBoxSimpleProgress->setChecked( quazaaSettings.Transfers.SimpleProgressBar );
	ui->checkBoxSlowHashing->setChecked( !quazaaSettings.Library.HighPriorityHashing );
	setSkin();
}

CWizardQuickStart::~CWizardQuickStart()
{
	delete ui;
}

void CWizardQuickStart::changeEvent( QEvent* e )
{
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CWizardQuickStart::accept()
{
	quazaaSettings.Connection.InSpeed = ( ui->doubleSpinBoxInSpeed->value() / 8 ) * 1024;
	quazaaSettings.Connection.OutSpeed = ( ui->doubleSpinBoxOutSpeed->value() / 8 ) * 1024;
	quazaaSettings.Connection.Port = ui->spinBoxPort->value();
	quazaaSettings.Connection.RandomPort = ui->checkBoxRandomPort->isChecked();
	quazaaSettings.Security.EnableUPnP = ui->checkBoxUPnP->isChecked();
	quazaaSettings.Security.AllowSharesBrowse = ui->checkBoxAllowBrowseShares->isChecked();
	quazaaSettings.Library.Shares.clear();
	for ( int m_iSharesRow = 0; m_iSharesRow < ui->listWidgetShares->count(); ++m_iSharesRow )
	{
		ui->listWidgetShares->setCurrentRow( m_iSharesRow );
		quazaaSettings.Library.Shares.append( ui->listWidgetShares->currentItem()->text() );
	}
	quazaaSettings.Security.AllowProfileBrowse = ui->checkBoxAllowBrowseProfile->isChecked();
	quazaaSettings.Profile.IrcNickname = ui->lineEditIrcNick->text();
	quazaaSettings.Profile.IrcAlternateNickname = ui->lineEditIrcAlternateNick->text();
	quazaaSettings.Profile.GnutellaScreenName = ui->lineEditGnutellaScreenName->text();
	quazaaSettings.Profile.Gender = ui->comboBoxGender->currentIndex();
	quazaaSettings.Profile.Age = ui->spinBoxAge->value();
	quazaaSettings.Profile.Country = ui->lineEditCountry->text();
	quazaaSettings.Profile.StateProvince = ui->lineEditStateProvince->text();
	quazaaSettings.Profile.City = ui->lineEditCity->text();
	quazaaSettings.Gnutella2.Enable = ui->checkBoxG2Connect->isChecked();
	quazaaSettings.Ares.Enable = ui->checkBoxAresConnect->isChecked();
	quazaaSettings.EDonkey.Enable = ui->checkBoxEDonkeyConnect->isChecked();
	quazaaSettings.EDonkey.EnableKad = ui->checkBoxKADConnect->isChecked();
	quazaaSettings.Web.Torrent = ui->checkBoxBitTorrent->isChecked();
	quazaaSettings.BitTorrent.UseKademlia = ui->checkBoxBitTorrentKademliaConnect->isChecked();
	quazaaSettings.System.ConnectOnStartup = ui->checkBoxConnectNetworksAlways->isChecked();
	quazaaSettings.System.StartWithSystem = ui->checkBoxOSStart->isChecked();
	quazaaSettings.Transfers.SimpleProgressBar = ui->checkBoxSimpleProgress->isChecked();
	quazaaSettings.Library.HighPriorityHashing = !ui->checkBoxSlowHashing->isChecked();
	quazaaSettings.saveProfile();
	quazaaSettings.saveSettings();

	// Free RAM occupied by port storage set.
	common::getRandomUnusedPort( true );

	QDialog::accept();
}

void CWizardQuickStart::on_toolButtonSharesAdd_clicked()
{
	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QDir directory( QFileDialog::getExistingDirectory( this,
													   tr( "Select A Folder To Share" ),
													   quazaaSettings.Downloads.CompletePath,
													   options ) );
	if ( directory.exists() )
	{
		ui->listWidgetShares->addItem( directory.canonicalPath() );
	}
}

void CWizardQuickStart::on_toolButtonSharesRemove_clicked()
{
	if ( ui->listWidgetShares->currentRow() != -1 )
	{
		ui->listWidgetShares->takeItem( ui->listWidgetShares->currentRow() );
	}
}

void CWizardQuickStart::setSkin()
{
}

void CWizardQuickStart::on_checkBoxUPnP_stateChanged( int arg1 )
{
	if ( arg1 == Qt::Checked )
	{
		ui->progressBarUPnP->setEnabled( true );
	}
	else
	{
		ui->progressBarUPnP->setEnabled( false );
	}
}

void CWizardQuickStart::on_pushButtonRandomizePort_clicked()
{
	ui->spinBoxPort->setValue( common::getRandomUnusedPort() );
}
