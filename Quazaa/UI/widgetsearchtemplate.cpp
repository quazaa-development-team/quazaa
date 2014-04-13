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

#include "widgetsearchtemplate.h"
#include "ui_widgetsearchtemplate.h"

#include "NetworkCore/searchmanager.h"
#include "NetworkCore/managedsearch.h"
#include "NetworkCore/query.h"
#include "NetworkCore/queryhit.h"
#include "NetworkCore/Hashes/hash.h"
#include "downloads.h"
#include "securitymanager.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "debug_new.h"

#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>

WidgetSearchTemplate::WidgetSearchTemplate(QString searchString, QWidget* parent) :
	QWidget(parent),
	ui(new Ui::WidgetSearchTemplate)
{
	ui->setupUi(this);
	searchMenu = new QMenu(this);
	searchMenu->addAction(ui->actionDownload);
	searchMenu->addSeparator();
	searchMenu->addAction(ui->actionBanNode);
	searchMenu->addAction(ui->actionMarkAsJunk);
	searchMenu->addAction(ui->actionClearResults);
	searchMenu->addSeparator();
	searchMenu->addAction(ui->actionViewReviews);
	searchMenu->addAction(ui->actionVirusTotalCheck);

	m_sSearchString = searchString;
	m_pSearch = 0;
	m_nFiles = 0;
	m_nHits = 0;
	m_nHubs = 0;
	m_nLeaves = 0;
	m_eSearchState = SearchState::Default;
	m_pSortModel = new QSortFilterProxyModel(this);
	m_pSearchModel = new SearchTreeModel();
	m_pSortModel->setSourceModel(m_pSearchModel);
	ui->treeViewSearchResults->setModel(m_pSortModel);
	m_pSortModel->setDynamicSortFilter(false);
	connect(m_pSearchModel, SIGNAL(sort()), this, SLOT(sort()));
	connect(m_pSearchModel, SIGNAL(updateStats()), this, SLOT(onStatsUpdated()));
	loadHeaderState();
	connect(ui->treeViewSearchResults->header(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(saveHeaderState()));
	connect(ui->treeViewSearchResults->header(), SIGNAL(sectionResized(int,int,int)), this, SLOT(saveHeaderState()));
	connect(ui->treeViewSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(saveHeaderState()));
	setSkin();
}

WidgetSearchTemplate::~WidgetSearchTemplate()
{
	if(m_pSearch != 0)
	{
		stopSearch();
	}

	delete m_pSearchModel;
	delete ui;
}

void WidgetSearchTemplate::changeEvent(QEvent* e)
{
	QWidget::changeEvent(e);
	switch(e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void WidgetSearchTemplate::startSearch(Query* pQuery)
{
	if ( m_pSearch && m_pSearch->m_pQuery != pQuery )
	{
		delete m_pSearch;
		m_pSearch = 0;
	}

	if ( !m_pSearch )
	{
		m_pSearch = new ManagedSearch( pQuery );
		connect( m_pSearch, SIGNAL( onHit( QueryHitSharedPtr ) ),
				 m_pSearchModel, SLOT( addQueryHit( QueryHitSharedPtr ) ) );
		connect( m_pSearch, SIGNAL( statsUpdated() ), this, SLOT( onStatsUpdated() ) );
		connect( m_pSearch, SIGNAL( stateChanged() ), this, SLOT( onStateChanged() ) );
	}

	m_eSearchState = SearchState::Searching;
	m_pSearch->start();
	m_sSearchString = m_pSearch->m_pQuery->descriptiveName();
}

void WidgetSearchTemplate::stopSearch()
{
	Q_ASSERT(m_pSearch != 0);

	m_eSearchState = SearchState::Stopped;
	m_pSearch->stop();
	delete m_pSearch;
	m_pSearch = 0;
}

void WidgetSearchTemplate::pauseSearch()
{
	Q_ASSERT(m_pSearch != 0);

	m_eSearchState = SearchState::Paused;
	m_pSearch->pause();
}

void WidgetSearchTemplate::clearSearch()
{
	//qDebug() << "Clear search captured in widget search template.";
		m_eSearchState = SearchState::Default;
	m_pSearchModel->clear();
	qApp->processEvents();
}

void WidgetSearchTemplate::onStatsUpdated()
{
	m_nFiles = m_pSearchModel->fileCount();

	if ( m_pSearch )
	{
		m_nHits = m_pSearch->m_nHits;
		m_nHubs = m_pSearch->m_nHubs;
		m_nLeaves = m_pSearch->m_nLeaves;
	}
	emit statsUpdated(this);
}

QModelIndex WidgetSearchTemplate::currentItem()
{
	QModelIndex idx = ui->treeViewSearchResults->currentIndex();
	const QSortFilterProxyModel* pModel = static_cast<const QSortFilterProxyModel*>(idx.model());
	return pModel->mapToSource(idx);
}

void WidgetSearchTemplate::onStateChanged()
{
	if( m_pSearch )
	{
		if( m_pSearch->m_bPaused )
		{
			m_eSearchState = SearchState::Paused;
		}
		else if( m_pSearch->m_bActive && !m_pSearch->m_bPaused )
		{
			m_eSearchState = SearchState::Searching;
		}
		else
		{
			m_eSearchState = SearchState::Stopped;
		}
	}
	else
	{
				m_eSearchState = SearchState::Stopped;
	}

	emit stateChanged();
}

void WidgetSearchTemplate::sort()
{
	m_pSortModel->sort(m_pSortModel->sortColumn(), m_pSortModel->sortOrder());
}

void WidgetSearchTemplate::saveHeaderState()
{
	quazaaSettings.WinMain.SearchHeader = ui->treeViewSearchResults->header()->saveState();
}

void WidgetSearchTemplate::loadHeaderState()
{
	ui->treeViewSearchResults->header()->restoreState(quazaaSettings.WinMain.SearchHeader);
}


void WidgetSearchTemplate::on_treeViewSearchResults_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	SearchTreeItem* itemSearch = m_pSearchModel->topLevelItemFromIndex(currentItem());

	if( itemSearch != NULL )
	{
		QueryHit* pHits = 0;
		QueryHit* pLast = 0;

		for(int i = 0; i < itemSearch->childCount(); ++i)
		{
			if( pLast )
			{
				pLast->m_pNext = new QueryHit(itemSearch->child(i)->m_oHitData.pQueryHit.data());
				pLast = pLast->m_pNext;
			}
			else
			{
				pHits = new QueryHit(itemSearch->child(i)->m_oHitData.pQueryHit.data());
				pLast = pHits;
			}
		}

		Downloads.m_pSection.lock();
		Downloads.add(pHits);
		Downloads.m_pSection.unlock();

		delete pHits;
	}
}

void WidgetSearchTemplate::on_treeViewSearchResults_customContextMenuRequested(const QPoint &pos)
{
	QModelIndex currIndex = ui->treeViewSearchResults->indexAt(pos);
	if( currIndex.isValid() )
	{
		if(m_pSearchModel->parent(currentItem()).isValid())
			ui->actionBanNode->setVisible(true);
		else
			ui->actionBanNode->setVisible(false);

		searchMenu->exec(QCursor::pos());
	}
}

void WidgetSearchTemplate::on_actionDownload_triggered()
{
	SearchTreeItem* itemSearch = m_pSearchModel->topLevelItemFromIndex(currentItem());

	if( itemSearch != NULL )
	{
		on_treeViewSearchResults_doubleClicked(currentItem());
	}
}

void WidgetSearchTemplate::on_actionViewReviews_triggered()
{
	SearchTreeItem* itemSearch = m_pSearchModel->topLevelItemFromIndex(currentItem());

	if( itemSearch != NULL )
	{
		QDesktopServices::openUrl( QUrl(QString("http://bitzi.com/lookup/%1?v=detail&ref=quazaa").arg(itemSearch->m_oHitData.lHashes.at(0).toString()), QUrl::TolerantMode) );
	}
}

void WidgetSearchTemplate::on_actionVirusTotalCheck_triggered()
{
	SearchTreeItem* itemSearch = m_pSearchModel->topLevelItemFromIndex(currentItem());

	if( itemSearch != NULL )
	{
		QDesktopServices::openUrl( QUrl(QString("www.virustotal.com/latest-report.html?resource=%1").arg(QString(itemSearch->m_oHitData.lHashes.at(0).rawValue().toHex())), QUrl::TolerantMode) );
	}
}

void WidgetSearchTemplate::setSkin()
{
	ui->treeViewSearchResults->setStyleSheet(skinSettings.listViews);
}

void WidgetSearchTemplate::on_actionBanNode_triggered()
{
	SearchTreeItem* itemSearch = m_pSearchModel->itemFromIndex(currentItem());

	if( itemSearch != NULL )
	{
		bool ok;
		QString reason = QInputDialog::getText( this, tr("Ban Reason"), tr("Please enter a ban reason."), QLineEdit::Normal, "", &ok );
		if ( ok && !reason.isEmpty() )
		{
			CEndPoint address = itemSearch->m_oHitData.pQueryHit.data()->m_pHitInfo.data()->m_oNodeAddress;
			securityManager.ban( address, Security::RuleTime::SixMonths, true, reason, false );
			m_pSearchModel->removeQueryHit(currentItem().row(), m_pSearchModel->parent(currentItem()));
		}
	}
}
