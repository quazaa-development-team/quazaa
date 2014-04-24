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

#include "dialogfiltersearch.h"
#include "ui_dialogfiltersearch.h"
#include "skinsettings.h"

#include "searchfilter.h"

#include "debug_new.h"

DialogFilterSearch::DialogFilterSearch(SearchFilter::FilterControlData*& pData, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogFilterSearch),
	m_pData( pData )
{
	Q_ASSERT( pData );

	ui->setupUi(this);
	setSkin();

	// TODO: replace with QRegularExpression oMatch( "\\A...\\z", QRegularExpression::
	// CaseInsensitiveOption ); once Qt has finised integrating new RegularExpression class.
	QRegExp pattern = QRegExp( "^(|[0-9]+\\s*(|B|KB|KiB|MB|MiB|GB|GiB|TB|TiB))$" );
	pattern.setCaseSensitivity( Qt::CaseInsensitive );

	QValidator* pValidator = new QRegExpValidator( pattern, this );
	ui->lineEditMinimumSize->setValidator( pValidator );
	ui->lineEditMaximumSize->setValidator( pValidator );

	ui->lineEditWords->setText( m_pData->m_sMatchString );
	ui->checkBoxRegularExpression->setChecked( m_pData->m_bRegExp );

	const quint64 MAX_VAL = 18446744073709551615; // max value of 64 bit int
	QString sMinSize = m_pData->m_nMinSize ? QString::number( m_pData->m_nMinSize ) + "B" : "";
	QString sMaxSize = m_pData->m_nMaxSize != MAX_VAL ?
												  QString::number( m_pData->m_nMaxSize ) + "B" : "";

	ui->lineEditMinimumSize->setText( sMinSize );
	ui->lineEditMaximumSize->setText( sMaxSize );

	ui->spinBoxMinimumSources->setValue( m_pData->m_nMinSources );

	ui->checkBoxAdultResults       ->setChecked( m_pData->m_bAdultAllowed          );
	ui->checkBoxBogusResults       ->setChecked( m_pData->m_bBogusAllowed          );
	ui->checkBoxBusyHosts          ->setChecked( m_pData->m_bBusyAllowed           );
	ui->checkBoxDRMFiles           ->setChecked( m_pData->m_bDRMAllowed            );
	ui->checkBoxFilesAlreadyHave   ->setChecked( m_pData->m_bExistsInLibraryAllowed);
	ui->checkBoxFirewalledPushHosts->setChecked( m_pData->m_bFirewalledAllowed     );
	ui->checkBoxIncompleteFiles    ->setChecked( m_pData->m_bIncompleteAllowed     );
	ui->checkBoxNonMatchingFiles   ->setChecked( m_pData->m_bNonMatchingAllowed    );
	ui->checkBoxSuspiciousFiles    ->setChecked( m_pData->m_bSuspiciousAllowed     );
	ui->checkBoxUnreachableHosts   ->setChecked( m_pData->m_bUnstableAllowed       );
}

DialogFilterSearch::~DialogFilterSearch()
{
	delete ui;
}

void DialogFilterSearch::changeEvent(QEvent* e)
{
	QDialog::changeEvent(e);
	switch(e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void DialogFilterSearch::on_pushButtonFilter_clicked()
{
	/*
	// filter attributes
	QString m_sMatchString;
	bool m_bRegExp;

	quint64 m_nMinSize;
	quint64 m_nMaxSize;
	quint16 m_nMinSources;

	// bools: state allowed
	bool m_bBusy;
	bool m_bFirewalled;
	bool m_bUnstable;
	bool m_bDRM;
	bool m_bSuspicious;
	bool m_bNonMatching;
	bool m_bExistsInLibrary;
	bool m_bBogus;
	bool m_bAdult; */

	const quint64 MAX_VAL = 18446744073709551615; // max value of 64 bit int

	m_pData->m_sMatchString     = ui->lineEditWords->text();
	m_pData->m_bRegExp          = ui->checkBoxRegularExpression->isChecked();

	bool bOK = true;
	m_pData->m_nMinSize         = common::readSizeInBytes( ui->lineEditMinimumSize->text(), bOK );
	m_pData->m_nMinSize         = bOK ? m_pData->m_nMinSize : 0;

	m_pData->m_nMaxSize         = common::readSizeInBytes( ui->lineEditMaximumSize->text(), bOK );
	m_pData->m_nMaxSize         = bOK ? m_pData->m_nMaxSize : MAX_VAL;

	m_pData->m_nMinSources      = ui->spinBoxMinimumSources->value();

	m_pData->m_bAdultAllowed           = ui->checkBoxAdultResults->isChecked();
	m_pData->m_bBogusAllowed           = ui->checkBoxBogusResults->isChecked();
	m_pData->m_bBusyAllowed            = ui->checkBoxBusyHosts->isChecked();
	m_pData->m_bDRMAllowed             = ui->checkBoxDRMFiles->isChecked();
	m_pData->m_bExistsInLibraryAllowed = ui->checkBoxFilesAlreadyHave->isChecked();
	m_pData->m_bFirewalledAllowed      = ui->checkBoxFirewalledPushHosts->isChecked();
	m_pData->m_bIncompleteAllowed      = ui->checkBoxIncompleteFiles->isChecked();
	m_pData->m_bNonMatchingAllowed     = ui->checkBoxNonMatchingFiles->isChecked();
	m_pData->m_bSuspiciousAllowed      = ui->checkBoxSuspiciousFiles->isChecked();
	m_pData->m_bUnstableAllowed        = ui->checkBoxUnreachableHosts->isChecked();

	emit closed();
	close();
}

void DialogFilterSearch::on_pushButtonCancel_clicked()
{
	delete m_pData;
	m_pData = NULL;
	emit closed();
	close();
}

void DialogFilterSearch::setSkin()
{

}

void DialogFilterSearch::on_pushButtonSaveAs_clicked()
{

}

void DialogFilterSearch::on_pushButtonDelete_clicked()
{

}
