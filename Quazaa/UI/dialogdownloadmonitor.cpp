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

#include "dialogdownloadmonitor.h"
#include "ui_dialogdownloadmonitor.h"
#include "skinsettings.h"

#include "debug_new.h"

CDialogDownloadMonitor::CDialogDownloadMonitor( QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::CDialogDownloadMonitor )
{
	ui->setupUi( this );
	setSkin();
}

CDialogDownloadMonitor::~CDialogDownloadMonitor()
{
	delete ui;
}

void CDialogDownloadMonitor::changeEvent( QEvent* e )
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

void CDialogDownloadMonitor::on_pushButtonHide_clicked()
{
	emit closed();
	close();
}

void CDialogDownloadMonitor::on_pushButton_clicked()
{
	emit closed();
	close();
}

void CDialogDownloadMonitor::updateProgress( int percent, QString transferSpeed, QString timeRemaining,
											 QString volumeDownloaded, QString numberSources, QPixmap icon,
											 QString status, QString file )
{
	ui->progressBarStatus->setValue( percent );
	ui->labelEstimatedTimeRemaining->setText( timeRemaining );
	ui->labelTransferSpeed->setText( transferSpeed );
	ui->labelVolumeDownloaded->setText( volumeDownloaded );
	ui->labelNumberSources->setText( numberSources );
	ui->labelFileIcon->setPixmap( icon );
	ui->labelStatus->setText( status );
	ui->labelFileName->setText( file );
}

void CDialogDownloadMonitor::setSkin()
{

}
