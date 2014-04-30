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

#include "widgethome.h"
#include "ui_widgethome.h"
#include "dialogadddownload.h"
#include "dialogopentorrent.h"
#include "dialogsettings.h"
#include "wizardquickstart.h"
#include "widgetsearchtemplate.h"
#include "systemlog.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

#include <QDesktopServices>
#include <QUrl>

CWidgetHome::CWidgetHome( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::CWidgetHome )
{
	ui->setupUi( this );
	ui->comboBoxHomeFileType->setView( new QListView() );
	connect( ui->labelWelcomeSecurityLink, SIGNAL( linkActivated( QString ) ), this, SIGNAL( triggerSecurity() ) );
	ui->lineEditWelcomeSearch->setText( quazaaSettings.WinMain.HomeSearchString );
	setSkin();
}

CWidgetHome::~CWidgetHome()
{
	delete ui;
}

void CWidgetHome::changeEvent( QEvent* e )
{
	QWidget::changeEvent( e );
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CWidgetHome::on_labelWelcomeURLDownloadLink_linkActivated( QString link )
{
	Q_UNUSED( link );
	CDialogAddDownload* dlgAddDownload = new CDialogAddDownload( this );
	dlgAddDownload->show();
}

void CWidgetHome::on_labelWelcomeOpenTorrentLink_linkActivated( QString link )
{
	Q_UNUSED( link );
	CDialogOpenTorrent* dlgOpenTorrent = new CDialogOpenTorrent( this );
	dlgOpenTorrent->show();
}

void CWidgetHome::on_labelWelcomeSkinLink_linkActivated( QString link )
{
	Q_UNUSED( link );
	CDialogSettings* dlgSettings = new CDialogSettings( this, SettingsPage::Skins );
	dlgSettings->show();
}

void CWidgetHome::on_labelWelcomeWizardLink_linkActivated( QString link )
{
	Q_UNUSED( link );
	CWizardQuickStart* wzrdQuickStart = new CWizardQuickStart( this );
	wzrdQuickStart->exec();
}

void CWidgetHome::on_labelWelcomeUserGuideLink_linkActivated( QString link )
{
	Q_UNUSED( link );

	QDesktopServices::openUrl( QUrl( "https://sourceforge.net/apps/mediawiki/quazaa/index.php?title=Manual",
									 QUrl::TolerantMode ) );
}

void CWidgetHome::saveWidget()
{
	quazaaSettings.WinMain.HomeSearchString = ui->lineEditWelcomeSearch->text();
}

void CWidgetHome::on_toolButtonWelcomeSearch_clicked()
{
	QString m_sSearchString = ui->lineEditWelcomeSearch->text();
	emit requestSearch( &m_sSearchString );
}

void CWidgetHome::setSkin()
{

}
