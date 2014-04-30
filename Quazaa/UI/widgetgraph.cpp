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

#include "widgetgraph.h"
#include "ui_widgetgraph.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

CWidgetGraph::CWidgetGraph( QWidget* parent ) :
	QMainWindow( parent ),
	ui( new Ui::CWidgetGraph )
{
	ui->setupUi( this );
	restoreState( quazaaSettings.WinMain.GraphToolbar );
	ui->splitterGraph->restoreState( quazaaSettings.WinMain.GraphSplitter );
	setSkin();
}

CWidgetGraph::~CWidgetGraph()
{
	delete ui;
}

void CWidgetGraph::changeEvent( QEvent* e )
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

void CWidgetGraph::saveWidget()
{
	quazaaSettings.WinMain.GraphToolbar = saveState();
	quazaaSettings.WinMain.GraphSplitter = ui->splitterGraph->saveState();
}

void CWidgetGraph::on_splitterGraph_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterGraph->handle( 1 )->underMouse() )
	{
		if ( ui->splitterGraph->sizes()[0] > 0 )
		{
			quazaaSettings.WinMain.GraphSplitterRestoreLeft = ui->splitterGraph->sizes()[0];
			quazaaSettings.WinMain.GraphSplitterRestoreRight = ui->splitterGraph->sizes()[1];
			QList<int> newSizes;
			newSizes.append( 0 );
			newSizes.append( ui->splitterGraph->sizes()[0] + ui->splitterGraph->sizes()[1] );
			ui->splitterGraph->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.GraphSplitterRestoreLeft );
			sizesList.append( quazaaSettings.WinMain.GraphSplitterRestoreRight );
			ui->splitterGraph->setSizes( sizesList );
		}
	}
}

void CWidgetGraph::setSkin()
{

}
