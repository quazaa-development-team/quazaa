/*
** widgetsmileylist.h
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

#ifndef WIDGETSMILEYLIST_H
#define WIDGETSMILEYLIST_H

#include <QMenu>
#include <QListWidgetItem>

namespace Ui
{
class CWidgetSmileyList;
}

class CWidgetSmileyList : public QMenu
{
	Q_OBJECT
public:
	CWidgetSmileyList( QWidget* parent = 0 );
	~CWidgetSmileyList();

protected:
	void changeEvent( QEvent* e );

private:
	Ui::CWidgetSmileyList* ui;

signals:
	void smileyClicked( QString smiley );

private slots:
	void on_listWidget_itemClicked( QListWidgetItem* item );
	void setSkin();
};

#endif // WIDGETSMILEYLIST_H
