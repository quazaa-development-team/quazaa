/*
** widgetsearchtemplate.h
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

#ifndef WIDGETSEARCHTEMPLATE_H
#define WIDGETSEARCHTEMPLATE_H

#include <QDialog>
#include <QMenu>

#include "types.h"
#include "searchtreemodel.h"
#include "searchsortfilterproxymodel.h"
#include "NetworkCore/queryhit.h"

namespace Ui
{
class WidgetSearchTemplate;
}

class ManagedSearch;
class Query;

namespace SearchState
{
enum SearchState { Default, Stopped, Searching, Paused };
}

class WidgetSearchTemplate : public QWidget
{
	Q_OBJECT
private:
	Ui::WidgetSearchTemplate* ui;
	QMenu* searchMenu;

public:
	SearchTreeModel*            m_pSearchModel;
	SearchSortFilterProxyModel* m_pSortModel;
	ManagedSearch*              m_pSearch;

	SearchState::SearchState m_eSearchState;
	QString m_sSearchString;

	int m_nHubs;
	int m_nLeaves;
	int m_nHits;
	int m_nFiles;

public:
	WidgetSearchTemplate( QString searchString = "", QWidget* parent = 0 );
	~WidgetSearchTemplate();

	void startSearch( Query* pQuery );
	void pauseSearch();
	void stopSearch();
	void clearSearch();
	QModelIndex currentItem() const;

	void filter( const SearchFilter::FilterControlData& rData );
	SearchFilter::FilterControlData* getFilterDataCopy() const;
	//void getStats(quint32& nHubs, quint32& nLeaves, quint32& nHits);

protected:
	void changeEvent( QEvent* e );

private:
	const Hash* const getHash() const;

signals:
	void statsUpdated( WidgetSearchTemplate* thisSearch );
	void stateChanged();

private slots:
	void on_treeViewSearchResults_doubleClicked( const QModelIndex& index );
	void on_treeViewSearchResults_customContextMenuRequested( const QPoint& pos );
	void on_actionDownload_triggered();
	void on_actionViewReviews_triggered();
	void on_actionVirusTotalCheck_triggered();
	void setSkin();
	void on_actionBanNode_triggered();

protected slots:
	void onStatsUpdated();
	void onStateChanged();
	void sort();

public slots:
	void saveHeaderState();
	void loadHeaderState();
};

#endif // WIDGETSEARCHTEMPLATE_H
