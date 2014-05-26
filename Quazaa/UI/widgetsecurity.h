/*
** widgetsecurity.h
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

#ifndef WIDGETSECURITY_H
#define WIDGETSECURITY_H

#include <QMainWindow>

#include "tableview.h"
#include "securityfiltermodel.h"

class SecurityTableModel;

namespace Ui
{
class CWidgetSecurity;
}

class CWidgetSecurity : public QMainWindow
{
	Q_OBJECT

public:
	SecurityTableModel*  m_lSecurity;

private:
	Ui::CWidgetSecurity* ui;
	QMenu*               m_pSecurityMenu;
	CTableView*          m_pTableViewSecurity;
	CTableView*          m_pTableViewSecurityAuto;
	SecurityFilterModel* m_lManual;
	SecurityFilterModel* m_lAutomatic;

public:
	CWidgetSecurity( QWidget* parent = 0 );
	virtual ~CWidgetSecurity();

	void		saveWidget();

protected:
	virtual void changeEvent( QEvent* e );
	virtual void keyPressEvent( QKeyEvent* event );

private:
	QModelIndexList getSelectedItems( SecurityFilterModel*& pFilterModel ) const;
	QModelIndexList getSelectedItems( SecurityFilterModel*& pFilterModel,
									  CTableView*& pTableView ) const;

private slots:
	void on_actionSecurityAddRule_triggered();
	void on_actionSecurityRemoveRule_triggered();
	void on_actionSecurityModifyRule_triggered();
	void on_actionSecurityImportRules_triggered();
	void on_actionSecurityExportRules_triggered();
	void on_actionSubscribeSecurityList_triggered();

	void tableViewSecurity_doubleClicked( const QModelIndex& index );
	void tableViewSecurity_clicked( const QModelIndex& index );

	void setSkin();
	void tableViewSecurity_customContextMenuRequested( const QPoint& pos );
	void tableViewSecurityAuto_customContextMenuRequested( const QPoint& pos );
};

#endif // WIDGETSECURITY_H
