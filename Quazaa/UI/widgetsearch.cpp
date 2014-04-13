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

#include "widgetsearch.h"
#include "ui_widgetsearch.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "systemlog.h"
#include "suggestedlineedit.h"
#include <QNetworkAccessManager>

QNetworkAccessManager netManager; // TODO: move it to a better place ;)

#include "debug_new.h"

CWidgetSearch::CWidgetSearch(QWidget* parent) :
	QWidget( parent ),
	ui( new Ui::CWidgetSearch )
{
	ui->setupUi( this );
	ui->comboBoxSearchFileType->setView( new QListView() );
	ui->comboBoxSearchSizeMax->setView( new QListView() );
	ui->comboBoxSearchSizeMin->setView( new QListView() );
	ui->splitterSearch->restoreState( quazaaSettings.WinMain.SearchSplitter );
	ui->toolButtonSearchFiletypeTaskHeader->setChecked( quazaaSettings.WinMain.SearchFileTypeTaskVisible );
	ui->toolButtonSearchNetworksTaskHeader->setChecked( quazaaSettings.WinMain.SearchNetworksTaskVisible );
	ui->toolButtonSearchResultsTaskHeader->setChecked( quazaaSettings.WinMain.SearchResultsTaskVisible );
	ui->toolButtonSearchTaskHeader->setChecked( quazaaSettings.WinMain.SearchTaskVisible );
	panelSearchResults = new CWidgetSearchResults();
	ui->verticalLayoutSearchResults->addWidget( panelSearchResults );
	connect( panelSearchResults, SIGNAL( searchTabChanged( WidgetSearchTemplate* ) ), this, SLOT( onSearchTabChanged( WidgetSearchTemplate* ) ) );
	connect( panelSearchResults, SIGNAL( statsUpdated( WidgetSearchTemplate* ) ), this, SLOT( updateStats( WidgetSearchTemplate* ) ) );
	connect( panelSearchResults, SIGNAL( stateChanged() ), this, SLOT( updateButtons() ) );
	panelSearchResults->on_tabWidgetSearch_currentChanged( -1 );
	setSkin();

	// TODO: make some settings to control this thingy
	CSuggestedLineEdit* pSuggest = new CSuggestedLineEdit( ui->lineEditSearch, this );
	pSuggest->setEnableExternal( true );
	pSuggest->setMaxRecentItems( 20 );
	pSuggest->setNetworkAccessManager( &netManager );
}

CWidgetSearch::~CWidgetSearch()
{
	delete ui;
}

void CWidgetSearch::changeEvent(QEvent* e)
{
	QWidget::changeEvent( e );
	switch( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi( this );
			break;
		default:
			break;
	}
}

void CWidgetSearch::saveWidget()
{
	quazaaSettings.WinMain.SearchSplitter = ui->splitterSearch->saveState();
	quazaaSettings.WinMain.SearchFileTypeTaskVisible = ui->toolButtonSearchFiletypeTaskHeader->isChecked();
	quazaaSettings.WinMain.SearchNetworksTaskVisible = ui->toolButtonSearchNetworksTaskHeader->isChecked();
	quazaaSettings.WinMain.SearchResultsTaskVisible = ui->toolButtonSearchResultsTaskHeader->isChecked();
	quazaaSettings.WinMain.SearchTaskVisible = ui->toolButtonSearchTaskHeader->isChecked();
	panelSearchResults->saveWidget();
}

void CWidgetSearch::on_toolButtonSearch_clicked()
{
	if ( currentPage->m_searchState == SearchState::Paused  ||
		 currentPage->m_searchState == SearchState::Stopped ||
		 currentPage->m_searchState == SearchState::Default )
	{
		// TODO: handle additional criteria
		panelSearchResults->startSearch( ui->lineEditSearch->text() );
		updateButtons();
	}
	focusSearchInput();
}

void CWidgetSearch::on_toolButtonSearchClear_clicked()
{
	if ( currentPage->m_searchState == SearchState::Searching ||
		 currentPage->m_searchState == SearchState::Paused )
	{
		panelSearchResults->stopSearch();
	}
	else
	{
		bool cleared = panelSearchResults->clearSearch();
		if ( cleared )
		{
			ui->labelSearchResultsSearching->setText( tr( "Not Currently Searching" ) );
			ui->labelSearchResultsFound->setText( tr( "No Files Found" ) );
		}
	}
	updateButtons();
	focusSearchInput();
}

void CWidgetSearch::startNewSearch(QString* searchString)
{
	panelSearchResults->startNewSearch( searchString );
	focusSearchInput();
}

void CWidgetSearch::on_toolButtonNewSearch_clicked()
{
	panelSearchResults->addSearchTab();
	focusSearchInput();
}

void CWidgetSearch::on_splitterSearch_customContextMenuRequested(QPoint pos)
{
	Q_UNUSED( pos );

	if ( ui->splitterSearch->handle(1)->underMouse() )
	{
		if ( ui->splitterSearch->sizes()[0] > 0 )
		{
			quazaaSettings.WinMain.SearchSplitterRestoreLeft = ui->splitterSearch->sizes()[0];
			quazaaSettings.WinMain.SearchSplitterRestoreRight = ui->splitterSearch->sizes()[1];
			QList<int> newSizes;
			newSizes.append( 0 );
			newSizes.append( ui->splitterSearch->sizes()[0] + ui->splitterSearch->sizes()[1] );
			ui->splitterSearch->setSizes( newSizes );
		}
		else
		{
			QList<int> sizesList;
			sizesList.append( quazaaSettings.WinMain.SearchSplitterRestoreLeft  );
			sizesList.append( quazaaSettings.WinMain.SearchSplitterRestoreRight );
			ui->splitterSearch->setSizes( sizesList );
		}
	}
}

void CWidgetSearch::onSearchTabChanged(WidgetSearchTemplate* searchPage)
{
	currentPage = searchPage;
	ui->lineEditSearch->setText( searchPage->m_sSearchString );

	updateButtons( searchPage->m_pSearch == 0 );
	focusSearchInput();
}

void CWidgetSearch::updateStats(WidgetSearchTemplate* searchWidget)
{
	ui->labelSearchResultsSearching->setText( tr( "%1 hubs,%2 leaves." ).arg( searchWidget->m_nHubs ).arg( searchWidget->m_nLeaves ) );
	ui->labelSearchResultsFound->setText( tr( "%1 files in %2 hits." ).arg( searchWidget->m_nFiles ).arg( searchWidget->m_nHits ) );
	if ( searchWidget->m_nHubs == 0 && searchWidget->m_nLeaves == 0 && searchWidget->m_searchState != SearchState::Searching )
	{
		ui->labelSearchResultsSearching->setText( tr( "Not Currently Searching" ) );
	}
	if ( searchWidget->m_nFiles == 0 && searchWidget->m_nHits == 0 )
	{
		ui->labelSearchResultsFound->setText( tr( "No Files Found" ) );
	}
}

void CWidgetSearch::updateButtons(bool bInitial)
{
	WidgetSearchTemplate* searchPage = currentPage;

	switch( searchPage->m_searchState )
	{
	case SearchState::Searching:
		ui->toolButtonSearch->setText( tr( "Searching" ) );
		ui->toolButtonSearch->setEnabled( false );
		ui->toolButtonSearchClear->setText( tr( "Pause" ) );
		ui->toolButtonSearchClear->setEnabled( true );
		break;
	case SearchState::Paused:
		ui->toolButtonSearch->setText( tr( "More" ) );
		ui->toolButtonSearch->setEnabled( true );
		ui->toolButtonSearchClear->setText( tr( "Stop" ) );
		break;
	case SearchState::Stopped:
		if ( bInitial )
		{
			ui->toolButtonSearch->setText( tr( "Search" ) );
			ui->toolButtonSearchClear->setEnabled( false );
		}
		else
		{
			ui->toolButtonSearch->setText( tr( "More" ) );
			ui->toolButtonSearchClear->setEnabled( true );
		}
		ui->toolButtonSearch->setEnabled( true );
		ui->toolButtonSearchClear->setText( tr( "Clear" ) );
		ui->labelSearchResultsSearching->setText( tr( "Not Currently Searching" ) );
		ui->labelSearchResultsFound->setText( tr( "No Files Found" ) );
		break;
	default:
		ui->toolButtonSearch->setText( tr( "Search" ) );
		ui->toolButtonSearch->setEnabled( true );
		ui->toolButtonSearchClear->setText( tr( "Clear" ) );
		ui->toolButtonSearchClear->setEnabled( false );
		ui->labelSearchResultsSearching->setText( tr( "Not Currently Searching" ) );
		ui->labelSearchResultsFound->setText( tr( "No Files Found" ) );
		break;
	}
}

void CWidgetSearch::focusSearchInput()
{
	ui->lineEditSearch->setFocus();
}

void CWidgetSearch::setSkin()
{
	ui->frameSearchTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->frameSearchNetworksTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->frameSearchResultsTask->setStyleSheet(  skinSettings.sidebarTaskBackground );
	ui->frameSearchFiletypeTask->setStyleSheet( skinSettings.sidebarTaskBackground );
	ui->scrollAreaSearchSidebar->setStyleSheet( skinSettings.sidebarBackground );
	ui->toolButtonSearchFiletypeTaskHeader->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->toolButtonSearchNetworksTaskHeader->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->toolButtonSearchResultsTaskHeader->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->toolButtonSearchTaskHeader->setStyleSheet( skinSettings.sidebarTaskHeader );
	ui->listViewSearchFiletype->setStyleSheet( skinSettings.listViews );
	ui->toolButtonNewSearch->setStyleSheet( skinSettings.addSearchButton );
}
