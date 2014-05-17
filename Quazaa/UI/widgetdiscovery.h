/*
** widgetdiscovery.h
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

#ifndef WIDGETDISCOVERY_H
#define WIDGETDISCOVERY_H

#include <QMainWindow>
#include <QActionGroup>

#include "tableview.h"
#include "discovery.h"

class DiscoveryTableModel;

namespace Ui
{
class CWidgetDiscovery;
}

class CWidgetDiscovery : public QMainWindow
{
	Q_OBJECT

public:
	DiscoveryTableModel* m_pDiscoveryList;
	//QActionGroup* actionGroupDiscoveryNavigation;

private:
	Ui::CWidgetDiscovery* ui;
	QMenu* m_pDiscoveryMenu;
	CTableView* m_pTableViewDiscovery;

public:
	CWidgetDiscovery( QWidget* parent = 0 );
	~CWidgetDiscovery();

	void		setModel( QAbstractItemModel* model );
	QWidget*	tableView();
	void		saveWidget();

protected:
	virtual void changeEvent( QEvent* e );
	virtual void keyPressEvent( QKeyEvent* event );

public slots:
	//void update();

private slots:
	void tableViewDiscovery_customContextMenuRequested( const QPoint& pos );
	void tableViewDiscovery_doubleClicked( const QModelIndex& index );
	void tableViewDiscovery_clicked( const QModelIndex& index );

	void setSkin();

	void on_actionDiscoveryAddService_triggered();
	void on_actionDiscoveryBrowseStatistics_triggered();
	void on_actionDiscoveryRemoveService_triggered();
	void on_actionDiscoveryQueryNow_triggered();
	void on_actionDiscoveryAdvertise_triggered();
	void on_actionDiscoveryProperties_triggered();

};

#endif // WIDGETDISCOVERY_H
