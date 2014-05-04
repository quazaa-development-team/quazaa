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

#include "widgetdownloads.h"
#include "ui_widgetdownloads.h"

#include "quazaasettings.h"
#include "quazaaglobals.h"
#include "downloadstreemodel.h"
#include "skinsettings.h"
#include "dialogopentorrent.h"

#include <QFontMetrics>
#include <QModelIndex>

#include "debug_new.h"

CWidgetDownloads::CWidgetDownloads( QWidget* parent ) :
	QMainWindow( parent ),
	ui( new Ui::CWidgetDownloads )
{
	ui->setupUi( this );
	downloadMenu = new QMenu( this );
	downloadMenu->addAction( ui->actionPreview );
	downloadMenu->addAction( ui->actionAddToPlaylist );
	downloadMenu->addAction( ui->actionShowMonitor );
	downloadMenu->addSeparator();
	downloadMenu->addAction( ui->actionResume );
	downloadMenu->addAction( ui->actionPause );
	downloadMenu->addAction( ui->actionCancel );
	downloadMenu->addSeparator();
	downloadMenu->addAction( ui->actionFindMoreSources );
	downloadMenu->addSeparator();
	downloadMenu->addAction( ui->actionViewReviews );
	downloadMenu->addAction( ui->actionVirusTotalCheck );
	downloadMenu->addSeparator();
	downloadMenu->addAction( ui->actionHelp );

	m_pModel = new DownloadsTreeModel();
	ui->tableViewDownloads->setModel( m_pModel );
	ui->tableViewDownloads->setColumnHidden( DownloadsTreeModel::COUNTRY, true );
	ui->tableViewDownloads->setItemDelegate( new DownloadsItemDelegate( this ) );
	setSkin();

	// Set up header sizes
	if ( !ui->tableViewDownloads->horizontalHeader()->restoreState( quazaaSettings.WinMain.DownloadsHeader ) )
	{
		QFontMetrics fm = ui->tableViewDownloads->fontMetrics();
		QHeaderView* header = ui->tableViewDownloads->horizontalHeader();
		header->resizeSection( DownloadsTreeModel::NAME,
							   fm.width( "a-typical-name-of-mp3-or-movie-or-xxx-or-other-porn.avi" ) );
		header->resizeSection( DownloadsTreeModel::SIZE, fm.width( " 123.45 MB " ) );
		header->resizeSection( DownloadsTreeModel::COMPLETED, fm.width( " 1234.45 MB " ) );
		header->resizeSection( DownloadsTreeModel::BANDWIDTH, fm.width( " 1234.45 MB/s " ) );
		header->resizeSection( DownloadsTreeModel::CLIENT, fm.width( QuazaaGlobals::USER_AGENT_STRING() ) );
		header->resizeSection( DownloadsTreeModel::PROGRESS, 200 );
		header->resizeSection( DownloadsTreeModel::PRIORITY, fm.width( "    Priority    " ) );
		header->resizeSection( DownloadsTreeModel::STATUS, fm.width( "Tracker Down" ) );
	}

	restoreState( quazaaSettings.WinMain.DownloadsToolbar );
}

CWidgetDownloads::~CWidgetDownloads()
{
	delete m_pModel;
	delete ui;
}

void CWidgetDownloads::changeEvent( QEvent* e )
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

void CWidgetDownloads::saveWidget()
{
	quazaaSettings.WinMain.DownloadsToolbar = saveState();
	quazaaSettings.WinMain.DownloadsHeader = ui->tableViewDownloads->horizontalHeader()->saveState();
}


void CWidgetDownloads::on_tableViewDownloads_customContextMenuRequested( const QPoint& pos )
{
	QModelIndex currIndex = ui->tableViewDownloads->indexAt( pos );
	if ( currIndex.isValid() )
	{
		downloadMenu->exec( QCursor::pos() );
	}
}

void CWidgetDownloads::setSkin()
{
	ui->tableViewDownloads->setStyleSheet( skinSettings.listViews );
}

void CWidgetDownloads::on_actionOpenTorrent_triggered()
{
	CDialogOpenTorrent* dlgOpenTorrent = new CDialogOpenTorrent( this );

	dlgOpenTorrent->show();
}

void CWidgetDownloads::on_tableViewDownloads_clicked( const QModelIndex& index )
{
	emit onItemChange( index );
	ui->tableViewDownloads->reset();
}
