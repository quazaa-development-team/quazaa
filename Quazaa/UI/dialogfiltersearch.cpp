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

#include "debug_new.h"

DialogFilterSearch::DialogFilterSearch( SearchFilter::FilterControlData& rData, QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::DialogFilterSearch ),
	m_rFilterData( rData ),
	m_oWorkingData( rData ),
	m_pFilters( rData.m_pFilters ),
	m_sSelection( rData.m_sFilterName )
{
	ui->setupUi( this );
	setSkin();

	QRegularExpression pattern =
			QRegularExpression( "\\A(|[0-9]+\\s*(|B|KB|KiB|MB|MiB|GB|GiB|TB|TiB))\\z",
								QRegularExpression::CaseInsensitiveOption );

	QValidator* pValidator = new QRegularExpressionValidator( pattern, this );
	ui->lineEditMinimumSize->setValidator( pValidator );
	ui->lineEditMaximumSize->setValidator( pValidator );

	m_pFilters->repopulate( *ui->comboBoxFilters, rData.m_sFilterName );
	// Note: triggers on_comboBoxFilters_currentIndexChanged()
}

DialogFilterSearch::~DialogFilterSearch()
{
	delete ui;
}

void DialogFilterSearch::changeEvent( QEvent* e )
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

void DialogFilterSearch::updateGUIFromData()
{
	ui->lineEditWords->setText(                m_oWorkingData.m_sMatchString );
	ui->checkBoxRegularExpression->setChecked( m_oWorkingData.m_bRegExp      );

	const quint64 MAX_VAL = 18446744073709551615; // max value of 64 bit int
	QString sMinSize = m_oWorkingData.m_nMinSize ?
						   common::writeSizeInWholeBytes( m_oWorkingData.m_nMinSize ) : "";
	QString sMaxSize = m_oWorkingData.m_nMaxSize != MAX_VAL ?
						   common::writeSizeInWholeBytes( m_oWorkingData.m_nMaxSize ) : "";

	ui->lineEditMinimumSize->setText( sMinSize );
	ui->lineEditMaximumSize->setText( sMaxSize );

	ui->spinBoxMinimumSources->setValue( m_oWorkingData.m_nMinSources );

	ui->checkBoxAdultResults       ->setChecked( m_oWorkingData.m_bAdultAllowed            );
	ui->checkBoxBogusResults       ->setChecked( m_oWorkingData.m_bBogusAllowed            );
	ui->checkBoxBusyHosts          ->setChecked( m_oWorkingData.m_bBusyAllowed             );
	ui->checkBoxDRMFiles           ->setChecked( m_oWorkingData.m_bDRMAllowed              );
	ui->checkBoxFilesAlreadyHave   ->setChecked( m_oWorkingData.m_bExistsInLibraryAllowed  );
	ui->checkBoxFirewalledPushHosts->setChecked( m_oWorkingData.m_bFirewalledAllowed       );
	ui->checkBoxIncompleteFiles    ->setChecked( m_oWorkingData.m_bIncompleteAllowed       );
	ui->checkBoxNonMatchingFiles   ->setChecked( m_oWorkingData.m_bNonMatchingAllowed      );
	ui->checkBoxSuspiciousFiles    ->setChecked( m_oWorkingData.m_bSuspiciousAllowed       );
	ui->checkBoxUnreachableHosts   ->setChecked( m_oWorkingData.m_bUnstableAllowed         );

	ui->checkBoxDefaultFilter      ->setChecked( m_sSelection == m_pFilters->defaultName() );
}

void DialogFilterSearch::updateDataFromGUI()
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

	m_oWorkingData.m_sMatchString = ui->lineEditWords->text();
	m_oWorkingData.m_bRegExp      = ui->checkBoxRegularExpression->isChecked();

	bool bOK = true;
	m_oWorkingData.m_nMinSize     = common::readSizeInBytes( ui->lineEditMinimumSize->text(), bOK );
	m_oWorkingData.m_nMinSize     = bOK ? m_oWorkingData.m_nMinSize : 0;

	m_oWorkingData.m_nMaxSize     = common::readSizeInBytes( ui->lineEditMaximumSize->text(), bOK );
	m_oWorkingData.m_nMaxSize     = bOK ? m_oWorkingData.m_nMaxSize : MAX_VAL;

	m_oWorkingData.m_nMinSources  = ui->spinBoxMinimumSources->value();

	m_oWorkingData.m_bAdultAllowed           = ui->checkBoxAdultResults->isChecked();
	m_oWorkingData.m_bBogusAllowed           = ui->checkBoxBogusResults->isChecked();
	m_oWorkingData.m_bBusyAllowed            = ui->checkBoxBusyHosts->isChecked();
	m_oWorkingData.m_bDRMAllowed             = ui->checkBoxDRMFiles->isChecked();
	m_oWorkingData.m_bExistsInLibraryAllowed = ui->checkBoxFilesAlreadyHave->isChecked();
	m_oWorkingData.m_bFirewalledAllowed      = ui->checkBoxFirewalledPushHosts->isChecked();
	m_oWorkingData.m_bIncompleteAllowed      = ui->checkBoxIncompleteFiles->isChecked();
	m_oWorkingData.m_bNonMatchingAllowed     = ui->checkBoxNonMatchingFiles->isChecked();
	m_oWorkingData.m_bSuspiciousAllowed      = ui->checkBoxSuspiciousFiles->isChecked();
	m_oWorkingData.m_bUnstableAllowed        = ui->checkBoxUnreachableHosts->isChecked();
}

void DialogFilterSearch::on_pushButtonFilter_clicked()
{
	// update m_oWorkingData
	updateDataFromGUI();

	// update filter data in the parent
	m_rFilterData = m_oWorkingData;

	emit filterClicked();
	close();
}

void DialogFilterSearch::on_pushButtonCancel_clicked()
{
	close();
}

void DialogFilterSearch::setSkin()
{
}

void DialogFilterSearch::on_pushButtonSave_clicked()
{
	updateDataFromGUI();

	QString sNewName = ui->comboBoxFilters->currentText();

	if ( m_sSelection == "" )
	{
		// duplicate the entry without a name to allow adding more filters in the future
		Q_ASSERT( m_oWorkingData.m_sFilterName == "" );
		m_pFilters->insert( m_oWorkingData );

		if ( ui->comboBoxFilters->count() == 1 )
		{
			ui->pushButtonDelete->setDisabled( false );
		}
	}
	else if ( m_sSelection != sNewName )
	{
		// rename the previously saved filter
		m_pFilters->rename( m_sSelection, sNewName );
	}

	// add or update the entry
	m_oWorkingData.m_sFilterName = sNewName;
	m_pFilters->insert( m_oWorkingData );

	if ( ui->checkBoxDefaultFilter->isChecked() )
	{
		m_pFilters->setDefault( sNewName );
	}

	m_pFilters->repopulate( *ui->comboBoxFilters, sNewName );
	// Note: automatically trigers on_comboBoxFilters_currentIndexChanged()
}

void DialogFilterSearch::on_pushButtonDelete_clicked()
{
	if ( !m_sSelection.isEmpty() ) // don't remove item with empty name.
	{
		QComboBox* pBox = ui->comboBoxFilters;

		m_pFilters->remove( m_sSelection );

		int nNewTextPos = pBox->currentIndex();
		if ( ++nNewTextPos == pBox->count() )
		{
			nNewTextPos -= 2;
		}

		// select the previous item - which always exists as the empty item is always the first and
		// cannot be removed
		m_pFilters->repopulate( *pBox, pBox->itemText( nNewTextPos ) );
		// Note: automatically trigers on_comboBoxFilters_currentIndexChanged()

		if ( ui->comboBoxFilters->count() == 1 )
		{
			ui->pushButtonDelete->setDisabled( true );
		}
	}
}

void DialogFilterSearch::on_comboBoxFilters_currentIndexChanged( int )
{
	m_sSelection = ui->comboBoxFilters->currentText();
	m_pFilters->select( m_sSelection, m_oWorkingData );
	updateGUIFromData();
}
