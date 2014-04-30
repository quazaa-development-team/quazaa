/*
** widgetreturnemittextedit.h
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

#ifndef WIDGETRETURNEMITTEXTEDIT_H
#define WIDGETRETURNEMITTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QEvent>
#include <QMimeData>

#include "chatcompleter.h"
class ChatCompleter;

class CWidgetReturnEmitTextEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit CWidgetReturnEmitTextEdit( QWidget* parent = 0 );
	ChatCompleter* completer() const;
	QString textUnderCursor() const;
	int currentWordStartIndex();

private:
	bool emitReturn;
	ChatCompleter* m_oCompleter;
	QStringList m_lHistory;
	QStringList m_lPlainTextHistory;
	int m_iHistoryIndex;

protected:
	bool event( QEvent* event );
	void keyPressEvent( QKeyEvent* event );
	virtual bool focusNextPrevChild( bool next );
	void insertFromMimeData( const QMimeData* source );

signals:
	void returnPressed();
	void tabPressed();
	void textChanged( const QString& text );
	void scrollToTop();
	void scrollToBottom();
	void scrollToNextPage();
	void scrollToPreviousPage();

public slots:
	void onTextChanged();
	void setSkin();
	void addHistory( QTextDocument* document );
	void addHistory( QString* text );
	void resetHistoryIndex();
};

#endif // WIDGETRETURNEMITTEXTEDIT_H
