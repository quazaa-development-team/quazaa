/*
** widgetneighbours.h
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

#ifndef WIDGETNEIGHBORS_H
#define WIDGETNEIGHBORS_H

#include <QMainWindow>
#include <QAbstractItemModel>
#include <QLabel>

class NeighboursTableModel;

namespace Ui
{
class CWidgetNeighbours;
}

class CWidgetNeighbours : public QMainWindow
{
	Q_OBJECT
public:
	QLabel* labelG2StatsIcon;
	QLabel* labelG2Stats;
	NeighboursTableModel* neighboursList;

public:
	CWidgetNeighbours( QWidget* parent = 0 );
	~CWidgetNeighbours();

	void		setModel( QAbstractItemModel* model );
	QWidget*	treeView();
	void		saveWidget();

protected:
	void changeEvent( QEvent* e );

private:
	Ui::CWidgetNeighbours* ui;
	QMenu* neighboursMenu;

private slots:
	void on_actionNetworkChatWith_triggered();
	void on_tableViewNeighbours_customContextMenuRequested( QPoint pos );
	void on_actionNeighbourDisconnect_triggered();
	void on_actionNeighbourConnectTo_triggered();
	void on_actionSettings_triggered();

	void onTimer();

	void updateG2();
	void updateAres();
	void updateEDonkey();
	void setSkin();
	void on_tableViewNeighbours_doubleClicked( const QModelIndex& index );
	void on_actionNetworkBan_triggered();
};

#endif // WIDGETNEIGHBORS_H
