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

#include "dialogscheduler.h"
#include "ui_dialogscheduler.h"
#include "skinsettings.h"

#include <QListView>

#include "debug_new.h"

CDialogScheduler::CDialogScheduler( QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::CDialogScheduler )
{
	ui->setupUi( this );
	ui->comboBoxAction->setView( new QListView() );
	ui->comboBoxOnceWeekly->setView( new QListView() );
	setSkin();
}

CDialogScheduler::~CDialogScheduler()
{
	delete ui;
}

void CDialogScheduler::changeEvent( QEvent* e )
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

void CDialogScheduler::on_comboBoxAction_currentIndexChanged( int index )
{
	switch ( index )
	{
	case 0:
		ui->spinBoxBandwidth->setEnabled( true );
		ui->labelNetworks->setEnabled( true );
		ui->checkBoxAres->setEnabled( true );
		ui->checkBoxBitTorrent->setEnabled( true );
		ui->checkBoxEDonkey->setEnabled( true );
		ui->checkBoxG2->setEnabled( true );
		ui->labelStop->setEnabled( true );
		ui->dateTimeEditStop->setEnabled( true );
		break;
	case 1:
		ui->spinBoxBandwidth->setEnabled( false );
		ui->labelNetworks->setEnabled( false );
		ui->checkBoxAres->setEnabled( false );
		ui->checkBoxBitTorrent->setEnabled( false );
		ui->checkBoxEDonkey->setEnabled( false );
		ui->checkBoxG2->setEnabled( false );
		ui->labelStop->setEnabled( true );
		ui->dateTimeEditStop->setEnabled( true );
		break;
	case 2:
		ui->spinBoxBandwidth->setEnabled( false );
		ui->labelNetworks->setEnabled( false );
		ui->checkBoxAres->setEnabled( false );
		ui->checkBoxBitTorrent->setEnabled( false );
		ui->checkBoxEDonkey->setEnabled( false );
		ui->checkBoxG2->setEnabled( false );
		ui->labelStop->setEnabled( false );
		ui->dateTimeEditStop->setEnabled( false );
		break;
	default:
		ui->spinBoxBandwidth->setEnabled( true );
		ui->labelNetworks->setEnabled( true );
		ui->checkBoxAres->setEnabled( true );
		ui->checkBoxBitTorrent->setEnabled( true );
		ui->checkBoxEDonkey->setEnabled( true );
		ui->checkBoxG2->setEnabled( true );
		ui->labelStop->setEnabled( true );
		ui->dateTimeEditStop->setEnabled( true );
		break;
	}
}

void CDialogScheduler::on_pushButtonOK_clicked()
{
	if ( ui->pushButtonApply->isEnabled() )
	{
		ui->pushButtonApply->click();
	}
	emit closed();
	close();
}

void CDialogScheduler::on_pushButtonCancel_clicked()
{
	emit closed();
	close();
}

void CDialogScheduler::setSkin()
{

}
