/*
* Copyright (C) 2008-2013 J-P Nurmi <jpnurmi@gmail.com>
* Copyright (C) 2010-2013 SmokeX <smokexjc@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include "chatcompleter.h"
#include "historylineedit.h"
#include "ircusermodel.h"
#include "irc.h"

#include <QDebug>

ChatCompleter::ChatCompleter(QObject* parent) : QCompleter(parent)
{
	d.lineEdit = 0;
	d.textEdit = 0;
	d.userModel = 0;
	d.commandModel = 0;
	d.channelPrefixes = QLatin1String("#");
	d.commandPrefixes = QLatin1String("/");
	setCaseSensitivity(Qt::CaseInsensitive);
	setCompletionMode(InlineCompletion);
	connect(this, SIGNAL(highlighted(QString)), this, SLOT(insertCompletion(QString)));
}

HistoryLineEdit* ChatCompleter::lineEdit() const
{
	return d.lineEdit;
}

void ChatCompleter::setLineEdit(HistoryLineEdit* lineEdit)
{
	if (d.lineEdit != lineEdit) {
		if (d.lineEdit)
			disconnect(d.lineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));

		if (lineEdit)
			connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited()));

		d.lineEdit = lineEdit;
	}

	d.textEdit = 0;
}

CWidgetReturnEmitTextEdit *ChatCompleter::textEdit() const
{
	return d.textEdit;
}

void ChatCompleter::setTextEdit(CWidgetReturnEmitTextEdit *textEdit)
{
	if (d.textEdit != textEdit)
	{
		if (d.textEdit)
			disconnect(d.textEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextEdited()));

		if (textEdit)
			connect(textEdit, SIGNAL(textChanged(QString)), this, SLOT(onTextEdited()));

		d.textEdit = textEdit;
	}

	d.lineEdit = 0;
}

IrcUserModel* ChatCompleter::userModel() const
{
	return d.userModel;
}

void ChatCompleter::setUserModel(IrcUserModel* model)
{
	d.userModel = model;
}

QAbstractItemModel *ChatCompleter::channelModel() const
{
	return d.channelModel;
}

void ChatCompleter::setChannelModel(QAbstractItemModel *model)
{
	d.channelModel = model;
}

QAbstractItemModel* ChatCompleter::commandModel() const
{
	return d.commandModel;
}

void ChatCompleter::setCommandModel(QAbstractItemModel* model)
{
	d.commandModel = model;
}

QString ChatCompleter::channelPrefixes() const
{
	return d.channelPrefixes;
}

void ChatCompleter::setChannelPrefixes(const QString& prefixes)
{
	d.channelPrefixes = prefixes;
}

QString ChatCompleter::commandPrefixes() const
{
	return d.commandPrefixes;
}

void ChatCompleter::setCommandPrefixes(const QString& prefixes)
{
	d.commandPrefixes = prefixes;
}

void ChatCompleter::onTabPressed()
{
	if (!d.lineEdit && !d.textEdit)
		return;

	if(d.lineEdit)
	{
		// store selection
		int pos = d.lineEdit->cursorPosition();
		int start = d.lineEdit->selectionStart();
		QString selected = d.lineEdit->selectedText();

		// select current word
		d.lineEdit->cursorWordForward(false);
		d.lineEdit->cursorWordBackward(true);
		QString word = d.lineEdit->selectedText();

		// choose model
		if (!word.isEmpty() && d.commandPrefixes.contains(word.at(0))) {
			if (model() != d.commandModel) {
				setModel(d.commandModel);
				setCompletionRole(Qt::DisplayRole);
			}
		} else if (!word.isEmpty() && d.channelPrefixes.contains(word.at(0))) {
			if (model() != d.channelModel) {
				setModel(d.channelModel);
				setCompletionRole(Qt::DisplayRole);
			}
		} else {
			if (model() != d.userModel)
				setModel(d.userModel);
				setCompletionRole(Irc::NameRole);
		}

		bool repeat = true;
		QString prefix = completionPrefix();
		if (prefix.isEmpty() || !word.startsWith(prefix, Qt::CaseInsensitive)) {
			repeat = false;
			setCompletionPrefix(word);
		}

		// restore selection
		d.lineEdit->setCursorPosition(pos);
		if (start != -1)
			d.lineEdit->setSelection(start, selected.length());

		// complete
		if (!word.isEmpty()) {
			complete();

			int count = completionCount();
			if (count > 1) {
				int next = currentRow() + 1;
				setCurrentRow(next % count);
			} else if (count == 1 && repeat && word.startsWith('/')) {
				emit commandCompletion(word.mid(1));
			}
		}
	} else if(d.textEdit) {
		QString word = d.textEdit->textUnderCursor();

		// choose model
		if (!word.isEmpty() && d.commandPrefixes.contains(word.at(0))) {
			if (model() != d.commandModel) {
				setModel(d.commandModel);
				setCompletionRole(Qt::DisplayRole);
			}
		} else if (!word.isEmpty() && d.channelPrefixes.contains(word.at(0))) {
			if (model() != d.channelModel) {
				setModel(d.channelModel);
				setCompletionRole(Qt::DisplayRole);
			}
		} else {
			if (model() != d.userModel)
				setModel(d.userModel);
				setCompletionRole(Irc::NameRole);
		}

		QString prefix = completionPrefix();
		if (prefix.isEmpty() || !word.startsWith(prefix, Qt::CaseInsensitive))
			setCompletionPrefix(word);

		// complete
		if (!word.isEmpty()) {
			complete();

			int count = completionCount();
			if (count > 1)
			{
				int next = currentRow() + 1;
				setCurrentRow(next % count);
			}
		}
	}
}

void ChatCompleter::onTextEdited()
{
	setCompletionPrefix(QString());
}

void ChatCompleter::insertCompletion(const QString& completion)
{
	if ((!d.lineEdit && !d.textEdit) || completion.isEmpty())
		return;

	if (d.lineEdit)
	{
		int pos = d.lineEdit->cursorPosition();
		QString text = d.lineEdit->text();
		if (pos > 0 && pos < text.length() && !text.at(pos - 1).isSpace())
			d.lineEdit->cursorWordForward(false);
		d.lineEdit->cursorWordBackward(true);
		pos = d.lineEdit->cursorPosition();

		QString tmp = completion;
		if (pos == 0 && !completion.startsWith("/"))
			tmp.append(":");
		d.lineEdit->insert(tmp);

		text = d.lineEdit->text();
		int cursor = d.lineEdit->cursorPosition();
		if (!text.at(cursor - 1).isSpace())
			d.lineEdit->insert(" ");
	} else if(d.textEdit) {
		int startIndex = d.textEdit->currentWordStartIndex();
		QTextCursor tc = d.textEdit->textCursor();
		tc.movePosition(QTextCursor::StartOfWord);
		if(completion.startsWith("/"))
		{
			tc.movePosition(QTextCursor::PreviousCharacter);
			tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
		}
		tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
		tc.removeSelectedText();
		tc.insertText(completion);

		if(!tc.atEnd())
		{
			tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			if(tc.selectedText() != ":") {
				tc.movePosition(QTextCursor::PreviousCharacter);
				if ( !completion.startsWith("/") && (startIndex == 0) )
					tc.insertText(":");
				tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
				if(tc.selectedText() != " ") {
					tc.movePosition(QTextCursor::PreviousCharacter);
					tc.insertText(" ");
				}
			}
		} else {
			if ( !completion.startsWith("/") && (startIndex == 0) )
				tc.insertText(":");
			tc.insertText(" ");
		}
		d.textEdit->setTextCursor(tc);
	}
}
