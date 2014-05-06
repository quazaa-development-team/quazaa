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

#include "widgetsearchresults.h"
#include "ui_widgetsearchresults.h"
#include "dialogfiltersearch.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "NetworkCore/query.h"
#include "queryhit.h"
#include "searchtreemodel.h"
#include "NetworkCore/managedsearch.h"
#include "downloads.h"

#include "systemlog.h"

#include <QMessageBox>

#include "debug_new.h"

WidgetSearchResults::WidgetSearchResults( QWidget* parent ) :
	QMainWindow( parent ),
	m_pFilterData( NULL ),
	ui( new Ui::WidgetSearchResults )
{
	ui->setupUi( this );
	labelFilter = new QLabel();
	labelFilter->setText( "Filter: " );
	m_pLineEditFilter = new QLineEdit();
	m_pLineEditFilter->setMaximumWidth( 150 );
	ui->toolBarFilter->insertWidget( ui->actionFilterMore, labelFilter );
	ui->toolBarFilter->insertWidget( ui->actionFilterMore, m_pLineEditFilter );
	restoreState( quazaaSettings.WinMain.SearchToolbar );

	addSearchTab();

	connect( m_pLineEditFilter, &QLineEdit::returnPressed,
			 this, &WidgetSearchResults::lineEditSearchChanged );

	ui->splitterSearchDetails->restoreState( quazaaSettings.WinMain.SearchDetailsSplitter );
	setSkin();
}

WidgetSearchResults::~WidgetSearchResults()
{
	delete ui;
	delete m_pFilterData;
}

void WidgetSearchResults::changeEvent( QEvent* e )
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

void WidgetSearchResults::updateSearchFilter()
{
	WidgetSearchTemplate* tabSearch =
		qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );

	tabSearch->filter( *m_pFilterData );
}

void WidgetSearchResults::saveWidget()
{
	quazaaSettings.WinMain.SearchToolbar = saveState();
	quazaaSettings.WinMain.SearchDetailsSplitter = ui->splitterSearchDetails->saveState();
}

void WidgetSearchResults::startSearch( QString searchString )
{
	if ( !searchString.isEmpty() )
	{
		WidgetSearchTemplate* tabSearch = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );
		if ( tabSearch )
		{
			if ( tabSearch->m_pSearch )
			{
				if ( tabSearch->m_sSearchString == searchString )
				{
					tabSearch->startSearch( tabSearch->m_pSearch->m_pQuery );
					connect( tabSearch, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
					emit searchTabChanged( tabSearch );
					emit statsUpdated( tabSearch );
				}
				else
				{
					int result = QMessageBox::question( this, tr( "You have started a new search." ),
														tr( "Would you like to start this search in a new tab?\n\nIf you perform this search in the current tab, you will mix the results of %1 search with the results of the new %2 search." ).arg(
															tabSearch->m_sSearchString ).arg( searchString ), QMessageBox::Yes | QMessageBox::No );
					if ( result == QMessageBox::Yes )
					{
						addSearchTab();
						tabSearch = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );
					}

					if ( tabSearch )
					{
						Query* pQuery = new Query();
						pQuery->setDescriptiveName( searchString );
						tabSearch->startSearch( pQuery );
						connect( tabSearch, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
						ui->tabWidgetSearch->setTabText( ui->tabWidgetSearch->currentIndex(), searchString );
						emit searchTabChanged( tabSearch );
						emit statsUpdated( tabSearch );
					}
				}
			}
			else
			{
				Query* pQuery = new Query();
				pQuery->setDescriptiveName( searchString );
				tabSearch->startSearch( pQuery );
				connect( tabSearch, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
				ui->tabWidgetSearch->setTabText( ui->tabWidgetSearch->currentIndex(), searchString );
			}
		}
	}
}

// Makes a new search tab and starts a search in it
void WidgetSearchResults::startNewSearch( QString* searchString )
{
	if ( searchString != QString( "" ) )
	{
		addSearchTab();
		WidgetSearchTemplate* tabSearch = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );
		if ( tabSearch )
		{
			Query* pQuery = new Query();
			pQuery->setDescriptiveName( QString( *searchString ) );
			tabSearch->startSearch( pQuery );
			connect( tabSearch, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
			ui->tabWidgetSearch->setTabText( ui->tabWidgetSearch->currentIndex(), QString( *searchString ) );
			emit searchTabChanged( tabSearch );
			emit statsUpdated( tabSearch );
		}
	}
}



void WidgetSearchResults::on_tabWidgetSearch_tabCloseRequested( int index )
{
	WidgetSearchTemplate* tabSearch = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->widget( index ) );
	ui->tabWidgetSearch->removeTab( index );
	delete tabSearch;
	if ( ui->tabWidgetSearch->count() == 1 )
	{
		ui->tabWidgetSearch->setTabsClosable( false );
	}
}

void WidgetSearchResults::addSearchTab()
{
	WidgetSearchTemplate* tabSearch = new WidgetSearchTemplate();
	int nTab = ui->tabWidgetSearch->addTab( tabSearch, QIcon( ":/Resource/Generic/Search.png" ),
											tr( "Search" ) );
	ui->tabWidgetSearch->setCurrentIndex( nTab );

	if ( ui->tabWidgetSearch->count() == 1 )
	{
		ui->tabWidgetSearch->setTabsClosable( false );
	}
	else
	{
		ui->tabWidgetSearch->setTabsClosable( true );
	}

	connect( tabSearch, &WidgetSearchTemplate::statsUpdated,
			 this, &WidgetSearchResults::onStatsUpdated );

	emit searchTabChanged( tabSearch );
}

void WidgetSearchResults::stopSearch()
{
	WidgetSearchTemplate* tabSearch =
		qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );

	if ( tabSearch )
	{
		if ( tabSearch->m_eSearchState == SearchState::Searching )
		{
			tabSearch->pauseSearch();
		}
		else if ( tabSearch->m_eSearchState == SearchState::Paused )
		{
			tabSearch->stopSearch();
		}
	}
}

bool WidgetSearchResults::clearSearch()
{
	WidgetSearchTemplate* tabSearch =
		qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );

	if ( tabSearch )
	{
		//qDebug() << "Clear search captured in WidgetSearchResults.";
		tabSearch->clearSearch();
		ui->tabWidgetSearch->setTabText( ui->tabWidgetSearch->currentIndex(), tr( "Search" ) );
		return true;
	}
	return false;
}

void WidgetSearchResults::on_actionFilterMore_triggered()
{
	Q_ASSERT( m_pFilterData );

	DialogFilterSearch* dlgFilterSearch = new DialogFilterSearch( *m_pFilterData, this );
	connect( dlgFilterSearch, &DialogFilterSearch::filterClicked,
			 this, &WidgetSearchResults::advancedSearchFilteringChanged );

	dlgFilterSearch->show();
}

void WidgetSearchResults::on_splitterSearchDetails_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );

	if ( ui->splitterSearchDetails->handle( 1 )->underMouse() )
	{
		if ( ui->splitterSearchDetails->sizes()[1] > 0 )
		{
			quazaaSettings.WinMain.SearchResultsSplitterRestoreTop = ui->splitterSearchDetails->sizes()[0];
			quazaaSettings.WinMain.SearchResultsSplitterRestoreBottom = ui->splitterSearchDetails->sizes()[1];
			QList<int> newSizes;
			newSizes.append( ui->splitterSearchDetails->sizes()[0] + ui->splitterSearchDetails->sizes()[1] );
			newSizes.append( 0 );
			ui->splitterSearchDetails->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.SearchResultsSplitterRestoreTop );
			sizesList.append( quazaaSettings.WinMain.SearchResultsSplitterRestoreBottom );
			ui->splitterSearchDetails->setSizes( sizesList );
		}
	}
}

void WidgetSearchResults::on_tabWidgetSearch_currentChanged( int index )
{
	Q_UNUSED( index );

	WidgetSearchTemplate* tabSearch = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );
	emit searchTabChanged( tabSearch );
	emit statsUpdated( tabSearch );
	tabSearch->loadHeaderState();

	delete m_pFilterData;
	m_pFilterData = tabSearch->getFilterDataCopy();

	m_pLineEditFilter->setText( m_pFilterData->m_sMatchString );
}

void WidgetSearchResults::onStatsUpdated( WidgetSearchTemplate* searchWidget )
{
	ui->tabWidgetSearch->setTabText( ui->tabWidgetSearch->indexOf( searchWidget ),
									 ( QString( "%1 [%2,%3]" ).arg( searchWidget->m_sSearchString ).arg( searchWidget->m_nFiles ).arg(
										   searchWidget->m_nHits ) ) );
	if ( ( searchWidget = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() ) ) )
	{
		emit statsUpdated( searchWidget );
	}
}

void WidgetSearchResults::on_actionSearchDownload_triggered()
{
	if ( ui->tabWidgetSearch->currentIndex() != -1 )
	{
		WidgetSearchTemplate* tabSearch = qobject_cast<WidgetSearchTemplate*>( ui->tabWidgetSearch->currentWidget() );

		if ( tabSearch )
		{
			const SearchTreeItem* itemSearch = tabSearch->m_pSearchModel->topLevelItemFromIndex( tabSearch->currentItem() );

			if ( itemSearch != NULL )
			{
				QueryHit* pHits = 0;
				QueryHit* pLast = 0;

				for ( int i = 0; i < itemSearch->childCount(); ++i )
				{
					if ( pLast )
					{
						pLast->m_pNext = new QueryHit( itemSearch->child( i )->m_oHitData.pQueryHit.data() );
						pLast = pLast->m_pNext;
					}
					else
					{
						pHits = new QueryHit( itemSearch->child( i )->m_oHitData.pQueryHit.data() );
						pLast = pHits;
					}
				}

				downloads.m_pSection.lock();
				downloads.add( pHits );
				downloads.m_pSection.unlock();

				delete pHits;
			}
		}
	}
}

void WidgetSearchResults::setSkin()
{
}

void WidgetSearchResults::lineEditSearchChanged()
{
	m_pFilterData->m_sMatchString = m_pLineEditFilter->text();
	updateSearchFilter();
}

void WidgetSearchResults::advancedSearchFilteringChanged()
{
	m_pLineEditFilter->setText( m_pFilterData->m_sMatchString );
	updateSearchFilter();
}
