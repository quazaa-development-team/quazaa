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

#include "widgetactivity.h"
#include "ui_widgetactivity.h"
#include "skinsettings.h"

#include "quazaasettings.h"

#include "debug_new.h"

WidgetActivity::WidgetActivity( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::WidgetActivity )
{
	ui->setupUi( this );
	ui->splitterActivity->restoreState( quazaaSettings.WinMain.ActivitySplitter );
	panelNeighbours = new CWidgetNeighbours();
	ui->verticalLayoutNeighbours->addWidget( panelNeighbours );
	panelSystemLog = new WidgetSystemLog();
	ui->verticalLayoutSystemLog->addWidget( panelSystemLog );
	setSkin();
}

WidgetActivity::~WidgetActivity()
{
	delete ui;
}

void WidgetActivity::changeEvent( QEvent* e )
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

void WidgetActivity::saveWidget()
{
	quazaaSettings.WinMain.ActivitySplitter = ui->splitterActivity->saveState();
	panelNeighbours->saveWidget();
	panelSystemLog->saveWidget();
}

void WidgetActivity::on_splitterActivity_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterActivity->handle( 1 )->underMouse() )
	{
		if ( ui->splitterActivity->sizes()[1] > 0 )
		{
			quazaaSettings.WinMain.ActivitySplitterRestoreTop = ui->splitterActivity->sizes()[0];
			quazaaSettings.WinMain.ActivitySplitterRestoreBottom = ui->splitterActivity->sizes()[1];
			QList<int> newSizes;
			newSizes.append( ui->splitterActivity->sizes()[0] + ui->splitterActivity->sizes()[1] );
			newSizes.append( 0 );
			ui->splitterActivity->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.ActivitySplitterRestoreTop );
			sizesList.append( quazaaSettings.WinMain.ActivitySplitterRestoreBottom );
			ui->splitterActivity->setSizes( sizesList );
		}
	}
}

void WidgetActivity::on_toolButtonSystemLogHeader_clicked()
{
	if ( ui->splitterActivity->sizes()[0] > 0 )
	{
		quazaaSettings.WinMain.ActivitySplitterRestoreTop = ui->splitterActivity->sizes()[0];
		quazaaSettings.WinMain.ActivitySplitterRestoreBottom = ui->splitterActivity->sizes()[1];
		QList<int> newSizes;
		newSizes.append( 0 );
		newSizes.append( ui->splitterActivity->sizes()[0] + ui->splitterActivity->sizes()[1] );
		ui->splitterActivity->setSizes( newSizes );
	}
	else
	{
		QList<int> sizesList;
		sizesList.append( quazaaSettings.WinMain.ActivitySplitterRestoreTop );
		sizesList.append( quazaaSettings.WinMain.ActivitySplitterRestoreBottom );
		ui->splitterActivity->setSizes( sizesList );
	}
}

void WidgetActivity::on_toolButtonNeighboursHeader_clicked()
{
	if ( ui->splitterActivity->sizes()[1] > 0 )
	{
		quazaaSettings.WinMain.ActivitySplitterRestoreTop = ui->splitterActivity->sizes()[0];
		quazaaSettings.WinMain.ActivitySplitterRestoreBottom = ui->splitterActivity->sizes()[1];
		QList<int> newSizes;
		newSizes.append( ui->splitterActivity->sizes()[0] + ui->splitterActivity->sizes()[1] );
		newSizes.append( 0 );
		ui->splitterActivity->setSizes( newSizes );
	}
	else
	{
		QList<int> sizesList;
		sizesList.append( quazaaSettings.WinMain.ActivitySplitterRestoreTop );
		sizesList.append( quazaaSettings.WinMain.ActivitySplitterRestoreBottom );
		ui->splitterActivity->setSizes( sizesList );
	}
}

void WidgetActivity::setSkin()
{
	ui->toolButtonNeighboursHeader->setStyleSheet( skinSettings.taskHeader );
	ui->toolButtonSystemLogHeader->setStyleSheet( skinSettings.taskHeader );
}
