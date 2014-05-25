/*
** widgethostcache.h
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

#ifndef WIDGETHOSTCACHE_H
#define WIDGETHOSTCACHE_H

#include <QMainWindow>

#include "tableview.h"
#include "hostcachetablemodel.h"

namespace Ui
{
class WidgetHostCache;
}

class WidgetHostCache : public QMainWindow
{
	Q_OBJECT
public:
	HostCacheTableModel*    m_pHosts;

private:
	Ui::WidgetHostCache*    ui;

	QMenu*                  m_pHostCacheMenu;
	CTableView*             m_pTableViewG2Cache;

public:
	WidgetHostCache( QWidget* parent = 0 );
	~WidgetHostCache();
	void saveWidget();

protected:
	void changeEvent( QEvent* e );
	virtual void keyPressEvent( QKeyEvent* event );

private slots:
	void on_splitterHostCache_customContextMenuRequested( QPoint pos );
	void setSkin();

	void tableViewG2Cache_doubleClicked( const QModelIndex& index );
	void tableViewG2Cache_clicked( const QModelIndex& index );
	void tableViewG2Cache_customContextMenuRequested( const QPoint& pos );
	void on_actionConnectTo_triggered();
	void on_actionDisconnect_triggered();
	void on_actionRemove_triggered();
};

#endif // WIDGETHOSTCACHE_H
