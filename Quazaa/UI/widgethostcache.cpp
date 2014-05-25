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

#include <QMenu>
#include <QKeyEvent>

#include "widgethostcache.h"
#include "ui_widgethostcache.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

WidgetHostCache::WidgetHostCache( QWidget* parent ) :
	QMainWindow( parent ),
	ui( new Ui::WidgetHostCache )
{
	ui->setupUi( this );

	m_pHostCacheMenu =  new QMenu( this );
	m_pHostCacheMenu->addAction( ui->actionConnectTo  );
	m_pHostCacheMenu->addAction( ui->actionRemove     );

	restoreState( quazaaSettings.WinMain.HostCacheToolbar );
	ui->splitterHostCache->restoreState( quazaaSettings.WinMain.HostCacheSplitter );

	m_pTableViewG2Cache = new CTableView();

	ui->splitterHostCache->addWidget( m_pTableViewG2Cache );

	connect( m_pTableViewG2Cache, SIGNAL( customContextMenuRequested( QPoint ) ), this,
			 SLOT( tableViewG2Cache_customContextMenuRequested( QPoint ) ) );
	connect( m_pTableViewG2Cache, SIGNAL( clicked( QModelIndex ) ), this, SLOT( tableViewG2Cache_clicked( QModelIndex ) ) );
	connect( m_pTableViewG2Cache, SIGNAL( doubleClicked( QModelIndex ) ), this,
			 SLOT( tableViewG2Cache_doubleClicked( QModelIndex ) ) );

	m_pHosts = new HostCacheTableModel( this );

	m_pTableViewG2Cache->setModel( m_pHosts );

	m_pHosts->sort( m_pTableViewG2Cache->horizontalHeader()->sortIndicatorSection(),
					m_pTableViewG2Cache->horizontalHeader()->sortIndicatorOrder() );

	m_pTableViewG2Cache->horizontalHeader()->setStretchLastSection( false );

	setSkin();
}

WidgetHostCache::~WidgetHostCache()
{
	delete ui;
}

void WidgetHostCache::changeEvent( QEvent* e )
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

void WidgetHostCache::keyPressEvent( QKeyEvent* e )
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
		m_pHosts->completeRefresh();
		break;
	}
	}

	QMainWindow::keyPressEvent( e );
}

void WidgetHostCache::saveWidget()
{
	quazaaSettings.WinMain.HostCacheToolbar = saveState();
	quazaaSettings.WinMain.HostCacheSplitter = ui->splitterHostCache->saveState();
}

void WidgetHostCache::on_splitterHostCache_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterHostCache->handle( 1 )->underMouse() )
	{
		if ( ui->splitterHostCache->sizes()[0] > 0 )
		{
			quazaaSettings.WinMain.HostCacheSplitterRestoreLeft = ui->splitterHostCache->sizes()[0];
			quazaaSettings.WinMain.HostCacheSplitterRestoreRight = ui->splitterHostCache->sizes()[1];
			QList<int> newSizes;
			newSizes.append( 0 );
			newSizes.append( ui->splitterHostCache->sizes()[0] + ui->splitterHostCache->sizes()[1] );
			ui->splitterHostCache->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.HostCacheSplitterRestoreLeft );
			sizesList.append( quazaaSettings.WinMain.HostCacheSplitterRestoreRight );
			ui->splitterHostCache->setSizes( sizesList );
		}
	}
}

void WidgetHostCache::setSkin()
{
	m_pTableViewG2Cache->setStyleSheet( skinSettings.listViews );
	m_pTableViewG2Cache->setStyleSheet( skinSettings.listViews );
}

void WidgetHostCache::tableViewG2Cache_doubleClicked( const QModelIndex& index )
{
	// TODO: implement
	if ( index.isValid() )
	{
	}
	else
	{
	}
}

void WidgetHostCache::tableViewG2Cache_clicked( const QModelIndex& index )
{
	// TODO: enable/disable actions
	if ( index.isValid() )
	{
	}
	else
	{
	}
}

void WidgetHostCache::tableViewG2Cache_customContextMenuRequested( const QPoint& pos )
{
	QModelIndex index = m_pTableViewG2Cache->indexAt( pos );

	// TODO: enable/disable actions
	if ( index.isValid() )
	{
	}
	else
	{
	}

	m_pHostCacheMenu->popup( QCursor::pos() );
}

void WidgetHostCache::on_actionConnectTo_triggered()
{
	// TODO: implement
}

void WidgetHostCache::on_actionDisconnect_triggered()
{
	// TODO: implement
}

void WidgetHostCache::on_actionRemove_triggered()
{
	// TODO: implement
}
