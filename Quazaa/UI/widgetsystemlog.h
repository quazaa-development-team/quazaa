/*
** widgetsystemlog.h
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

#ifndef WIDGETSYSTEMLOG_H
#define WIDGETSYSTEMLOG_H

#include <QMainWindow>
#include <QTime>
#include "types.h"

namespace Ui
{
class WidgetSystemLog;
}

class WidgetSystemLog : public QMainWindow
{
	Q_OBJECT
public:
	WidgetSystemLog( QWidget* parent = 0 );
	~WidgetSystemLog();
	void saveWidget();

protected:
	void changeEvent( QEvent* e );

private:
	Ui::WidgetSystemLog* ui;
	QMenu*  m_pLogMenu;
	QTime   m_oTimeStamp;

private slots:
	void on_actionCopy_triggered();
	void on_textEditSystemLog_customContextMenuRequested( QPoint pos );
	void on_actionClearBuffer_triggered();

	void appendLog( QString message, LogSeverity severity = LogSeverity::Information );
	void writeLine( const QString& sLine, LogSeverity severity );
	void appendText( const QString& sFontArgs, const QColor& color,
					 const QString& sLine, bool bAddTimeStamp );

	void setSkin();
};

#endif // WIDGETSYSTEMLOG_H
