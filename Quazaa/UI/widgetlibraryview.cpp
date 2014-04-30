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

#include "widgetlibraryview.h"
#include "ui_widgetlibraryview.h"
#include "dialoglibrarysearch.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

CWidgetLibraryView::CWidgetLibraryView( QWidget* parent ) :
	QMainWindow( parent ),
	ui( new Ui::CWidgetLibraryView )
{
	ui->setupUi( this );
	lineEditFind = new QLineEdit();
	lineEditFind->setMaximumWidth( 150 );
	ui->toolBarSearch->insertWidget( ui->actionFind, lineEditFind );
	restoreState( quazaaSettings.WinMain.LibraryToolbar );
	ui->splitterLibraryView->restoreState( quazaaSettings.WinMain.LibraryDetailsSplitter );
	setSkin();
}

CWidgetLibraryView::~CWidgetLibraryView()
{
	delete ui;
}

void CWidgetLibraryView::changeEvent( QEvent* e )
{
	QMainWindow::changeEvent( e );
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CWidgetLibraryView::saveWidget()
{
	quazaaSettings.WinMain.LibraryToolbar = saveState();
	quazaaSettings.WinMain.LibraryDetailsSplitter = ui->splitterLibraryView->saveState();
}

void CWidgetLibraryView::on_actionFind_triggered()
{
	CDialogLibrarySearch* dlgLibrarySearch = new CDialogLibrarySearch( this );
	dlgLibrarySearch->show();
}

void CWidgetLibraryView::on_splitterLibraryView_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterLibraryView->handle( 1 )->underMouse() )
	{
		if ( ui->splitterLibraryView->sizes()[1] > 0 )
		{
			quazaaSettings.WinMain.LibraryDetailsSplitterRestoreTop = ui->splitterLibraryView->sizes()[0];
			quazaaSettings.WinMain.LibraryDetailsSplitterRestoreBottom = ui->splitterLibraryView->sizes()[1];
			QList<int> newSizes;
			newSizes.append( ui->splitterLibraryView->sizes()[0] + ui->splitterLibraryView->sizes()[1] );
			newSizes.append( 0 );
			ui->splitterLibraryView->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.LibraryDetailsSplitterRestoreTop );
			sizesList.append( quazaaSettings.WinMain.LibraryDetailsSplitterRestoreBottom );
			ui->splitterLibraryView->setSizes( sizesList );
		}
	}
}

void CWidgetLibraryView::setSkin()
{

}
