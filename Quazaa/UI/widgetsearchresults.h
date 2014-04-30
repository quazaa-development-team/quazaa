/*
** widgetsearchresults.h
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

#ifndef WIDGETSEARCHRESULTS_H
#define WIDGETSEARCHRESULTS_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include "widgetsearchtemplate.h"

class WidgetSearch;

namespace Ui
{
class WidgetSearchResults;
}

class WidgetSearchResults : public QMainWindow
{
	Q_OBJECT
private:
	Ui::WidgetSearchResults* ui;

	QLabel* labelFilter;
	QLineEdit* m_pLineEditFilter;

	SearchFilter::FilterControlData* m_pFilterData;

public:
	WidgetSearchResults( QWidget* parent = 0 );
	~WidgetSearchResults();
	void saveWidget();

protected:
	void changeEvent( QEvent* e );

private:
	void updateSearchFilter();

signals:
	void searchTabChanged( WidgetSearchTemplate* searchPage );
	void statsUpdated( WidgetSearchTemplate* searchPage );
	void stateChanged();

public slots:
	void startSearch( QString searchString );
	void startNewSearch( QString* searchString );
	void addSearchTab();
	void stopSearch();
	bool clearSearch();
	void onStatsUpdated( WidgetSearchTemplate* searchWidget );

private slots:
	void on_tabWidgetSearch_currentChanged( int index );
	void on_splitterSearchDetails_customContextMenuRequested( QPoint pos );
	void on_actionFilterMore_triggered();
	void on_tabWidgetSearch_tabCloseRequested( int index );
	void on_actionSearchDownload_triggered();
	void setSkin();

	void lineEditSearchChanged();
	void advancedSearchFilteringChanged();

	friend class WidgetSearch;
};

#endif // WIDGETSEARCHRESULTS_H
