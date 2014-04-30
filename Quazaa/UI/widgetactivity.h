/*
** widgetactivity.h
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

#ifndef WIDGETACTIVITY_H
#define WIDGETACTIVITY_H

#include <QWidget>
#include "widgetneighbours.h"
#include "widgetsystemlog.h"

namespace Ui
{
class WidgetActivity;
}

class WidgetActivity : public QWidget
{
	Q_OBJECT
public:
	WidgetActivity( QWidget* parent = 0 );
	~WidgetActivity();
	CWidgetNeighbours* panelNeighbours;
	WidgetSystemLog* panelSystemLog;
	void saveWidget();

protected:
	void changeEvent( QEvent* e );

private:
	Ui::WidgetActivity* ui;

private slots:
	void on_toolButtonNeighboursHeader_clicked();
	void on_toolButtonSystemLogHeader_clicked();
	void on_splitterActivity_customContextMenuRequested( QPoint pos );
	void setSkin();
};

#endif // WIDGETACTIVITY_H
