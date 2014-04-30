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

#include "dialogtransferprogresstooltip.h"
#include "ui_dialogtransferprogresstooltip.h"
#include "skinsettings.h"

#include "debug_new.h"

CDialogTransferProgressTooltip::CDialogTransferProgressTooltip( QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::CDialogTransferProgressTooltip )
{
	ui->setupUi( this );
	setSkin();
}

CDialogTransferProgressTooltip::~CDialogTransferProgressTooltip()
{
	delete ui;
}

void CDialogTransferProgressTooltip::changeEvent( QEvent* e )
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

void CDialogTransferProgressTooltip::updateProgress( int percent, QString transferSpeed, QString timeRemaining,
													 QString volumeDownloaded, QString numberSources, QPixmap icon,
													 QString file, QString size, QString type, QStringList hashes )
{
	ui->progressBarProgress->setValue( percent );
	ui->labelTransferSpeed->setText( transferSpeed );
	ui->labelTimeRemaining->setText( timeRemaining );
	ui->labelVolumeDownloaded->setText( volumeDownloaded );
	ui->labelNumberSources->setText( numberSources );
	ui->labelFileIcon->setPixmap( icon );
	ui->labelFileName->setText( file );
	ui->labelSize->setText( size );
	ui->labelType->setText( type );
	ui->labelSHA1Hash->setText( hashes.at( 0 ) );
	ui->labelTigerTreeHash->setText( hashes.at( 1 ) );
	ui->labelED2KHash->setText( hashes.at( 2 ) );
	ui->labelMD5Hash->setText( hashes.at( 3 ) );
}

void CDialogTransferProgressTooltip::setSkin()
{

}
