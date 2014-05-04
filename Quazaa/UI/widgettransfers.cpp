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

#include "widgettransfers.h"
#include "ui_widgettransfers.h"

#include "downloadstreemodel.h"
#include "downloadspeermodel.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

#include <QDebug>

CWidgetTransfers::CWidgetTransfers( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::CWidgetTransfers )
{
	ui->setupUi( this );
	ui->splitterTransfers->restoreState( quazaaSettings.WinMain.TransfersSplitter );
	panelDownloads = new CWidgetDownloads( this );
	m_pPeerModel = new DownloadsPeerModel( this );
	m_pPeerModel->setDynamicSortFilter( true );
	m_pPeerModel->setSourceModel( panelDownloads->m_pModel );
	ui->tableViewPeers->setModel( m_pPeerModel );
	ui->verticalLayoutDownloads->addWidget( panelDownloads );
	panelUploads = new CWidgetUploads( this );
	ui->verticalLayoutUploads->addWidget( panelUploads );
	ui->splitterDownloads->restoreState( quazaaSettings.WinMain.DownloadsSplitter );
	ui->stackedWidgetTransfers->setCurrentIndex( 0 );
	ui->tabWidgetDownloadDetails->setCurrentIndex( 0 );
	setSkin();

	connect( panelDownloads, SIGNAL( onItemChange( QModelIndex ) ), this,
			 SLOT( onPanelDownloads_itemChanged( QModelIndex ) ) );
}

CWidgetTransfers::~CWidgetTransfers()
{
	delete ui;
}

void CWidgetTransfers::changeEvent( QEvent* e )
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

void CWidgetTransfers::saveWidget()
{
	quazaaSettings.WinMain.TransfersSplitter = ui->splitterTransfers->saveState();
	quazaaSettings.WinMain.TransfersNavigationSplitter = ui->splitterTransfersNavigation->saveState();
	quazaaSettings.WinMain.DownloadsSplitter = ui->splitterDownloads->saveState();
	panelDownloads->saveWidget();
	panelUploads->saveWidget();
}

void CWidgetTransfers::on_splitterTransfers_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterTransfers->handle( 1 )->underMouse() )
	{
		if ( ui->splitterTransfers->sizes()[0] > 0 )
		{
			quazaaSettings.WinMain.TransfersSplitterRestoreLeft = ui->splitterTransfers->sizes()[0];
			quazaaSettings.WinMain.TransfersSplitterRestoreRight = ui->splitterTransfers->sizes()[1];
			QList<int> newSizes;
			newSizes.append( 0 );
			newSizes.append( ui->splitterTransfers->sizes()[0] + ui->splitterTransfers->sizes()[1] );
			ui->splitterTransfers->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.TransfersSplitterRestoreLeft );
			sizesList.append( quazaaSettings.WinMain.TransfersSplitterRestoreRight );
			ui->splitterTransfers->setSizes( sizesList );
		}
	}
}

void CWidgetTransfers::setSkin()
{

}

void CWidgetTransfers::onPanelDownloads_itemChanged( const QModelIndex index )
{
	m_pPeerModel->setCurrentRoot( index );
	ui->tableViewPeers->setRootIndex( m_pPeerModel->mapFromSource( index ) );
}

void CWidgetTransfers::on_splitterDownloads_customContextMenuRequested( const QPoint& pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterDownloads->handle( 1 )->underMouse() )
	{
		if ( ui->splitterDownloads->sizes()[1] > 0 )
		{
			quazaaSettings.WinMain.DownloadsSplitterRestoreTop = ui->splitterDownloads->sizes()[0];
			quazaaSettings.WinMain.DownloadsSplitterRestoreBottom = ui->splitterDownloads->sizes()[1];
			QList<int> newSizes;
			newSizes.append( ui->splitterDownloads->sizes()[0] + ui->splitterDownloads->sizes()[1] );
			newSizes.append( 0 );
			ui->splitterDownloads->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.DownloadsSplitterRestoreTop );
			sizesList.append( quazaaSettings.WinMain.DownloadsSplitterRestoreBottom );
			ui->splitterDownloads->setSizes( sizesList );
		}
	}
}

void CWidgetTransfers::on_treeWidgetTransfers_currentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
	Q_UNUSED( previous );

	if ( current->statusTip( 0 ) == "Downloads" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 0 );
	}

	if ( current->statusTip( 0 ) == "CompletedDownloads" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 0 );
	}

	if ( current->statusTip( 0 ) == "Paused" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 0 );
	}

	if ( current->statusTip( 0 ) == "Active" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 0 );
	}

	if ( current->statusTip( 0 ) == "Inactive" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 0 );
	}

	if ( current->statusTip( 0 ) == "Uploads" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 1 );
	}

	if ( current->statusTip( 0 ) == "CompletedUploads" )
	{
		ui->stackedWidgetTransfers->setCurrentIndex( 1 );
	}
}

void CWidgetTransfers::on_splitterTransfersNavigation_customContextMenuRequested( const QPoint& pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterTransfersNavigation->handle( 1 )->underMouse() )
	{
		if ( ui->splitterTransfersNavigation->sizes()[1] > 0 )
		{
			quazaaSettings.WinMain.TransfersNavigationSplitterRestoreTop = ui->splitterTransfersNavigation->sizes()[0];
			quazaaSettings.WinMain.TransfersNavigationSplitterRestoreBottom = ui->splitterTransfersNavigation->sizes()[1];
			QList<int> newSizes;
			newSizes.append( ui->splitterTransfersNavigation->sizes()[0] + ui->splitterTransfersNavigation->sizes()[1] );
			newSizes.append( 0 );
			ui->splitterTransfersNavigation->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.TransfersNavigationSplitterRestoreTop );
			sizesList.append( quazaaSettings.WinMain.TransfersNavigationSplitterRestoreBottom );
			ui->splitterTransfersNavigation->setSizes( sizesList );
		}
	}
}

