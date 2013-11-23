/*
** widgetchatinput.h
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

#ifndef WIDGETCHATINPUT_H
#define WIDGETCHATINPUT_H

#include <QMainWindow>
#include <QCheckBox>
#include <QToolButton>
#include <QLabel>
#include "widgetreturnemittextedit.h"
#include "widgetsmileylist.h"
#include "chatcompleter.h"

#include "network.h"

namespace Ui {
	class CWidgetChatInput;
}

class CWidgetChatInput : public QMainWindow {
	Q_OBJECT
public:
	CWidgetChatInput(QWidget *parent = 0, bool isIrc = false);
	~CWidgetChatInput();
	CWidgetReturnEmitTextEdit* textEdit();
	QLabel* helpLabel();
	QLabel* lagLabel();

signals:
	void messageSent(QTextDocument *text);
	void messageSent(QString text);
	void closing();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::CWidgetChatInput *ui;
	QToolButton *toolButtonSmilies;
	CWidgetSmileyList *widgetSmileyList;
	QColor defaultColor;
	QToolButton *toolButtonPickColor;
	QLabel *labelLag;
	bool bIsIrc;

public slots:
	void setText(QString text);
	void setLag(qint64 lag);

private slots:
	void on_actionUnderline_toggled(bool checked);
	void on_actionItalic_toggled(bool checked);
	void on_actionBold_toggled(bool checked);
	void on_toolButtonSend_clicked();
	void onTextFormatChange(QTextCharFormat newFormat);
	void pickColor();
	void updateToolbar();
	void applySettings();
};

#endif // WIDGETCHATINPUT_H
