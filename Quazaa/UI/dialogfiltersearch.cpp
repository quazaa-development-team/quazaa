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
	ui(new Ui::CDialogFilterSearch),
	m_pData( pData )
{
	Q_ASSERT( pData );

	ui->setupUi(this);
	setSkin();
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

	m_pData->m_sMatchString = ui->lineEditWords->text();
	m_pData->m_bRegExp      = ui->checkBoxRegularExpression->isChecked();
	m_pData->m_nMinSize     = ui->lineEditMinimumSize->text().toLongLong();
	m_pData->m_nMaxSize     = ui->lineEditMaximumSize->text().toLongLong();

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
