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

#ifndef COMPLETER_H
#define COMPLETER_H

#include <QCompleter>
#include <widgetreturnemittextedit.h>

class HistoryLineEdit;
class CWidgetReturnEmitTextEdit;
class IrcUserModel;

class ChatCompleter : public QCompleter
{
	Q_OBJECT

public:
	ChatCompleter(QObject* parent = 0);

	HistoryLineEdit* lineEdit() const;
	void setLineEdit(HistoryLineEdit* lineEdit);

	CWidgetReturnEmitTextEdit* textEdit() const;
	void setTextEdit(CWidgetReturnEmitTextEdit* textEdit);

	IrcUserModel *userModel() const;
	void setUserModel(IrcUserModel* model);

	QAbstractItemModel* channelModel() const;
	void setChannelModel(QAbstractItemModel* model);

	QAbstractItemModel* commandModel() const;
	void setCommandModel(QAbstractItemModel* model);

	QString channelPrefixes() const;
	void setChannelPrefixes(const QString& prefixes);

	QString commandPrefixes() const;
	void setCommandPrefixes(const QString& prefixes);

signals:
	void commandCompletion(const QString& command);

private slots:
	void onTabPressed();
	void onTextEdited();
	void insertCompletion(const QString& completion);

private:
	struct CompleterData {
		CWidgetReturnEmitTextEdit* textEdit;
		HistoryLineEdit* lineEdit;
		IrcUserModel* userModel;
		QAbstractItemModel* channelModel;
		QAbstractItemModel* commandModel;
		QString channelPrefixes;
		QString commandPrefixes;
	} d;
};

#endif // COMPLETER_H
