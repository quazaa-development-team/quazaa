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

#include "widgetmedia.h"
#include "ui_widgetmedia.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

#include <QDebug>
#include <QtMultimedia>
#include <QDesktopWidget>

#include <QFileDialog>

// The Media Tab's Base Widget

CWidgetMedia::CWidgetMedia( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::CWidgetMedia )
{
	ui->setupUi( this );

	ui->splitterMedia->restoreState( quazaaSettings.WinMain.MediaSplitter );

	player = new QMediaPlayer( this );
	// owned by PlaylistModel
	playlist = new QMediaPlaylist();
	player->setPlaylist( playlist );
	videoContainer = new VideoContainer( this );

	player->setVideoOutput( videoContainer->videoWidget() );

	connect( videoContainer, SIGNAL( doubleClicked() ), SLOT( toggleFullScreen() ) );
	connect( videoContainer->mediaControls()->fullScreenButton(), SIGNAL( clicked() ), SLOT( toggleFullScreen() ) );

	ui->verticalLayoutMedia->addWidget( videoContainer );

	connect( videoContainer->mediaControls()->openButton(), SIGNAL( clicked() ), SLOT( openMedia() ) );

	setSkin();
}

CWidgetMedia::~CWidgetMedia()
{
	delete ui;
}

void CWidgetMedia::changeEvent( QEvent* e )
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

void CWidgetMedia::saveWidget()
{
	quazaaSettings.WinMain.MediaSplitter = ui->splitterMedia->saveState();
}

void CWidgetMedia::on_splitterMedia_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterMedia->handle( 1 )->underMouse() )
	{
		if ( ui->splitterMedia->sizes()[1] > 0 )
		{
			quazaaSettings.WinMain.MediaSplitterRestoreLeft = ui->splitterMedia->sizes()[0];
			quazaaSettings.WinMain.MediaSplitterRestoreRight = ui->splitterMedia->sizes()[1];
			QList<int> newSizes;
			newSizes.append( ui->splitterMedia->sizes()[0] + ui->splitterMedia->sizes()[1] );
			newSizes.append( 0 );
			ui->splitterMedia->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.MediaSplitterRestoreLeft );
			sizesList.append( quazaaSettings.WinMain.MediaSplitterRestoreRight );
			ui->splitterMedia->setSizes( sizesList );
		}
	}
}

void CWidgetMedia::on_toolButtonMediaPlaylistTaskHeader_clicked()
{
	if ( ui->splitterMedia->sizes()[0] > 0 )
	{
		quazaaSettings.WinMain.MediaSplitterRestoreLeft = ui->splitterMedia->sizes()[0];
		quazaaSettings.WinMain.MediaSplitterRestoreRight = ui->splitterMedia->sizes()[1];
		QList<int> newSizes;
		newSizes.append( 0 );
		newSizes.append( ui->splitterMedia->sizes()[0] + ui->splitterMedia->sizes()[1] );
		ui->splitterMedia->setSizes( newSizes );
	}
	else
	{
		QList<int> sizesList;
		sizesList.append( quazaaSettings.WinMain.MediaSplitterRestoreLeft );
		sizesList.append( quazaaSettings.WinMain.MediaSplitterRestoreRight );
		ui->splitterMedia->setSizes( sizesList );
	}
}

void CWidgetMedia::setSkin()
{
	ui->toolButtonMediaPlaylistTaskHeader->setStyleSheet( skinSettings.taskHeader );
	ui->frameMediaPlaylistTask->setStyleSheet( skinSettings.sidebarBackground );
	ui->listViewMediaPlaylist->setStyleSheet( skinSettings.listViews );
}

void CWidgetMedia::openMedia()
{
	QString file = QFileDialog::getOpenFileName( this, tr( "Open File" ) );

	QUrl url = QUrl::fromLocalFile( file );

	playlist->addMedia( url );

	player->play();
}

void CWidgetMedia::toggleFullScreen()
{
	if ( !ui->videoContainerWidget->isFullScreen() )
	{
		ui->videoContainerWidget->setParent( 0 );
		ui->videoContainerWidget->showFullScreen();
		videoContainer->mediaControls()->fullScreenButton()->setIcon( QIcon( ":/Resource/Media/NoFullScreen.png" ) );
	}
	else
	{
		ui->videoContainerWidget->setParent( this );
		ui->splitterMedia->insertWidget( 0, ui->videoContainerWidget );
		videoContainer->mediaControls()->fullScreenButton()->setIcon( QIcon( ":/Resource/Media/FullScreen.png" ) );
	}
}
