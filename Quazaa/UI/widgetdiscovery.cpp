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

#include "widgetdiscovery.h"
#include "ui_widgetdiscovery.h"

#include "discoverytablemodel.h"
#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

#include <QMenu>
#include <QKeyEvent>

CWidgetDiscovery::CWidgetDiscovery(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::CWidgetDiscovery)
{
	ui->setupUi(this);

	m_pDiscoveryMenu = new QMenu( this );
	m_pDiscoveryMenu->addAction( ui->actionDiscoveryAddService );
	m_pDiscoveryMenu->addAction( ui->actionDiscoveryRemoveService );
	m_pDiscoveryMenu->addAction( ui->actionDiscoveryQueryNow );
	m_pDiscoveryMenu->addAction( ui->actionDiscoveryAdvertise );

	restoreState( quazaaSettings.WinMain.DiscoveryToolbar );

	tableViewDiscovery = new CTableView();
	tableViewDiscovery->verticalHeader()->setVisible( false );
	ui->verticalLayoutDiscoveryTable->addWidget( tableViewDiscovery );

	connect( tableViewDiscovery, &CTableView::clicked,
			 this, &CWidgetDiscovery::tableViewDiscovery_clicked );
	connect( tableViewDiscovery, &CTableView::doubleClicked,
			 this, &CWidgetDiscovery::tableViewDiscovery_doubleClicked );
	connect( tableViewDiscovery, &CTableView::customContextMenuRequested,
			 this, &CWidgetDiscovery::tableViewDiscovery_customContextMenuRequested );

	m_pDiscoveryList = new CDiscoveryTableModel( this, tableView() );
	setModel( m_pDiscoveryList );
	m_pDiscoveryList->sort( tableViewDiscovery->horizontalHeader()->sortIndicatorSection(),
							tableViewDiscovery->horizontalHeader()->sortIndicatorOrder()    );

	tableViewDiscovery->horizontalHeader()->setStretchLastSection( false );
	setSkin();

	enum Column
	{
		TYPE = 0,
		URL,
		ACCESSED,
		HOSTS,
		TOTAL_HOSTS,
		ALTERNATE_SERVICES,
		FAILURES,
#if ENABLE_DISCOVERY_DEBUGGING
		RATING,
		MULTIPLICATOR,
#endif
		PONG,
		_NO_OF_COLUMNS
	};

	// Set up header sizes
	if ( !tableViewDiscovery->horizontalHeader()->restoreState(
			 quazaaSettings.WinMain.DiscoveryHeader ) )
	{
		QFontMetrics fm     = tableViewDiscovery->fontMetrics();
		QHeaderView* header = tableViewDiscovery->horizontalHeader();

		header->resizeSection( CDiscoveryTableModel::TYPE,
							   fm.width( " -Banned GWC- " ) );
		header->resizeSection( CDiscoveryTableModel::URL,
							   fm.width( " -http://cache2.bazookanetworks.com/g2/bazooka.php- " ) );
		header->resizeSection( CDiscoveryTableModel::ACCESSED,
							   fm.width( " ---Mi 20.Mrz 2013 19:21:45--- " ) );
		header->resizeSection( CDiscoveryTableModel::HOSTS,
							   fm.width( " -Hosts- " ) );
		header->resizeSection( CDiscoveryTableModel::TOTAL_HOSTS,
							   fm.width( " -Total Hosts- " ) );
		header->resizeSection( CDiscoveryTableModel::ALTERNATE_SERVICES,
							   fm.width( " -Alt. Services- " ) );
		header->resizeSection( CDiscoveryTableModel::FAILURES,
							   fm.width( " -Failures- " ) );
#if ENABLE_DISCOVERY_DEBUGGING
		header->resizeSection( CDiscoveryTableModel::RATING,
							   fm.width( " -Rating- " ) );
		header->resizeSection( CDiscoveryTableModel::MULTIPLICATOR,
							   fm.width( " -Mult.- " ) );
#endif
		header->resizeSection( CDiscoveryTableModel::PONG,
							   fm.width( " Some space to fill in the pong reply " ) );
	}
}

CWidgetDiscovery::~CWidgetDiscovery()
{
	delete ui; // Note: This does also take care of m_pDiscoveryMenu and m_pDiscoveryList.
	delete tableViewDiscovery; // TODO: check whether this is necessary...
}

void CWidgetDiscovery::setModel(QAbstractItemModel* model)
{
	tableViewDiscovery->setModel( model );
}

QWidget* CWidgetDiscovery::tableView()
{
	return tableViewDiscovery;
}

void CWidgetDiscovery::saveWidget()
{
	quazaaSettings.WinMain.DiscoveryToolbar = saveState();
	quazaaSettings.WinMain.DiscoveryHeader  = tableViewDiscovery->horizontalHeader()->saveState();
}

void CWidgetDiscovery::changeEvent(QEvent* e)
{
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
	{
		ui->retranslateUi( this );
		break;
	}

	default:
		break;
	}

	QMainWindow::changeEvent( e );
}

void CWidgetDiscovery::keyPressEvent(QKeyEvent *e)
{
	switch ( e->key() )
	{
	case Qt::Key_Delete:
	{

		break;
	}

	case Qt::Key_Return:
	{

		break;
	}

	case Qt::Key_F5:
	{
		m_pDiscoveryList->completeRefresh();
		break;
	}
	}

	QMainWindow::keyPressEvent( e );
}

void CWidgetDiscovery::update()
{
	// TODO: improve

	m_pDiscoveryList->updateAll();
}

void CWidgetDiscovery::tableViewDiscovery_customContextMenuRequested(const QPoint& point)
{
	QModelIndex index = tableViewDiscovery->indexAt( point );

	if ( index.isValid() )
	{
		ui->actionDiscoveryAddService->setEnabled( true );
		ui->actionDiscoveryRemoveService->setEnabled( true );

		if ( m_pDiscoveryList->isIndexBanned( index ) )
		{
			ui->actionDiscoveryQueryNow->setEnabled( false );
			ui->actionDiscoveryAdvertise->setEnabled( false );
		}
		else
		{
			ui->actionDiscoveryQueryNow->setEnabled( true );
			ui->actionDiscoveryAdvertise->setEnabled( true );
		}
	}
	else
	{
		ui->actionDiscoveryAddService->setEnabled( true );
		ui->actionDiscoveryRemoveService->setEnabled( false );
		ui->actionDiscoveryQueryNow->setEnabled( false );
		ui->actionDiscoveryAdvertise->setEnabled( false );
	}

	m_pDiscoveryMenu->popup( QCursor::pos() );
}

void CWidgetDiscovery::tableViewDiscovery_doubleClicked(const QModelIndex& index)
{
	if ( index.isValid() )
	{
		on_actionDiscoveryProperties_triggered();
	}
	else
	{
		ui->actionDiscoveryAddService->setEnabled( true );
		ui->actionDiscoveryRemoveService->setEnabled( false );
		ui->actionDiscoveryQueryNow->setEnabled( false );
		ui->actionDiscoveryAdvertise->setEnabled( false );
	}
}

void CWidgetDiscovery::tableViewDiscovery_clicked(const QModelIndex& index)
{
	if ( index.isValid() )
	{
		ui->actionDiscoveryAddService->setEnabled( true );
		ui->actionDiscoveryRemoveService->setEnabled( true );

		if ( m_pDiscoveryList->isIndexBanned( index ) )
		{
			ui->actionDiscoveryQueryNow->setEnabled( false );
			ui->actionDiscoveryAdvertise->setEnabled( false );
		}
		else
		{
			ui->actionDiscoveryQueryNow->setEnabled( true );
			ui->actionDiscoveryAdvertise->setEnabled( true );
		}
	}
	else
	{
		ui->actionDiscoveryAddService->setEnabled( true );
		ui->actionDiscoveryRemoveService->setEnabled( false );
		ui->actionDiscoveryQueryNow->setEnabled( false );
		ui->actionDiscoveryAdvertise->setEnabled( false );
	}
}

void CWidgetDiscovery::setSkin()
{
	tableViewDiscovery->setStyleSheet( skinSettings.listViews );
}

void CWidgetDiscovery::on_actionDiscoveryAddService_triggered()
{
	// TODO: Pop up dialog to add service
}

void CWidgetDiscovery::on_actionDiscoveryBrowseStatistics_triggered()
{
	// TODO: Find out what this is ispposed to do.
}

void CWidgetDiscovery::on_actionDiscoveryRemoveService_triggered()
{
	QModelIndexList selection = tableViewDiscovery->selectionModel()->selectedRows();

	foreach( QModelIndex i, selection )
	{
		if ( i.isValid() )
		{
			Q_ASSERT( discoveryManager.check( m_pDiscoveryList->nodeFromIndex( i ) ) );

			discoveryManager.remove( m_pDiscoveryList->nodeFromIndex( i )->id() );
		}
	}
}

void CWidgetDiscovery::on_actionDiscoveryQueryNow_triggered()
{
	QModelIndex index = tableViewDiscovery->currentIndex();

	if ( index.isValid() )
	{
		Q_ASSERT( discoveryManager.check( m_pDiscoveryList->nodeFromIndex( index ) ) );

		discoveryManager.queryService( m_pDiscoveryList->nodeFromIndex( index )->id() );
	}
}

void CWidgetDiscovery::on_actionDiscoveryAdvertise_triggered()
{
	QModelIndex index = tableViewDiscovery->currentIndex();

	if ( index.isValid() )
	{
		Q_ASSERT( discoveryManager.check( m_pDiscoveryList->nodeFromIndex( index ) ) );

		discoveryManager.updateService( m_pDiscoveryList->nodeFromIndex( index )->id() );
	}
}

void CWidgetDiscovery::on_actionDiscoveryProperties_triggered()
{
	QModelIndex index = tableViewDiscovery->currentIndex();

	if ( index.isValid() )
	{

	//see https://qt-project.org/wiki/New_Signal_Slot_Syntax

// TODO: Pop up dialog to edit service
	}
}
