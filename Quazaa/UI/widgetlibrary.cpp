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

#include "widgetlibrary.h"
#include "ui_widgetlibrary.h"
#include "dialogeditshares.h"
#include <QtWidgets>

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

CWidgetLibrary::CWidgetLibrary(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::CWidgetLibrary)
{
	ui->setupUi(this);
	ui->comboBoxLibraryNavigatorSharesFilter->setView(new QListView());
	ui->tabWidgetLibraryNavigator->setCurrentIndex(quazaaSettings.WinMain.LibraryNavigatorTab);
	ui->splitterLibrary->restoreState(quazaaSettings.WinMain.LibrarySplitter);
	panelLibraryView = new CWidgetLibraryView();
	ui->verticalLayoutLibraryView->addWidget(panelLibraryView);
	setSkin();
}

CWidgetLibrary::~CWidgetLibrary()
{
	delete ui;
}

void CWidgetLibrary::changeEvent(QEvent* e)
{
	QWidget::changeEvent(e);
	switch(e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void CWidgetLibrary::on_toolButtonLibraryEditShares_clicked()
{
	CDialogEditShares* dlgEditShares = new CDialogEditShares(this);
	dlgEditShares->show();
}

void CWidgetLibrary::saveWidget()
{
	quazaaSettings.WinMain.LibraryNavigatorTab = ui->tabWidgetLibraryNavigator->currentIndex();
	quazaaSettings.WinMain.LibrarySplitter = ui->splitterLibrary->saveState();
	panelLibraryView->saveWidget();
}

void CWidgetLibrary::on_splitterLibrary_customContextMenuRequested(QPoint pos)
{
	Q_UNUSED(pos);

	if(ui->splitterLibrary->handle(1)->underMouse())
	{
		if(ui->splitterLibrary->sizes()[0] > 0)
		{
			quazaaSettings.WinMain.LibrarySplitterRestoreLeft = ui->splitterLibrary->sizes()[0];
			quazaaSettings.WinMain.LibrarySplitterRestoreRight = ui->splitterLibrary->sizes()[1];
			QList<int> newSizes;
			newSizes.append(0);
			newSizes.append(ui->splitterLibrary->sizes()[0] + ui->splitterLibrary->sizes()[1]);
			ui->splitterLibrary->setSizes(newSizes);
		}
		else
		{
			QList<int> sizesList;
			sizesList.append(quazaaSettings.WinMain.LibrarySplitterRestoreLeft);
			sizesList.append(quazaaSettings.WinMain.LibrarySplitterRestoreRight);
			ui->splitterLibrary->setSizes(sizesList);
		}
	}
}

void CWidgetLibrary::initializeLibrary()
{

}

void CWidgetLibrary::setSkin()
{
	ui->frameLibraryNavigator->setStyleSheet(skinSettings.sidebarBackground);
	ui->treeViewLibraryNavigatorOrganiser->setStyleSheet(skinSettings.listViews);
	ui->treeViewLibraryNavigatorShares->setStyleSheet(skinSettings.listViews);
}
