/*
* Copyright (C) 2008-2013 J-P Nurmi <jpnurmi@gmail.com>
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

#include "userwizardpage.h"
#include "quazaasettings.h"
#include <QRegExpValidator>
#include <QCompleter>

UserWizardPage::UserWizardPage(QWidget* parent) : QWizardPage(parent)
{
    ui.setupUi(this);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/resources/oxygen/64x64/actions/user.png"));
    connect(ui.lineEditNick, SIGNAL(textChanged(QString)), this, SIGNAL(completeChanged()));

	QCompleter* nickCompleter = new QCompleter(quazaaSettings.Chat.NickNames, ui.lineEditNick);
	nickCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	ui.lineEditNick->setCompleter(nickCompleter);
	QRegExpValidator* validator = new QRegExpValidator(ui.lineEditNick);
	validator->setRegExp(QRegExp("\\S+"));
	ui.lineEditNick->setValidator(validator);

	QCompleter* nameCompleter = new QCompleter(quazaaSettings.Chat.RealNames, ui.lineEditName);
	nameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	ui.lineEditName->setCompleter(nameCompleter);
}

UserWizardPage::~UserWizardPage()
{
	if (!quazaaSettings.Chat.NickNames.contains(nickName(), Qt::CaseInsensitive))
		quazaaSettings.Chat.NickNames << nickName();
	if (!quazaaSettings.Chat.RealNames.contains(realName(), Qt::CaseInsensitive))
		quazaaSettings.Chat.RealNames << realName();

	quazaaSettings.saveChatConnectionWizard();
}

QString UserWizardPage::nickName() const
{
    return ui.lineEditNick->text();
}

void UserWizardPage::setNickName(const QString& nickName)
{
    ui.lineEditNick->setText(nickName);
}

QString UserWizardPage::realName() const
{
    return ui.lineEditName->text();
}

void UserWizardPage::setRealName(const QString& realName)
{
    ui.lineEditName->setText(realName);
}

bool UserWizardPage::isComplete() const
{
    return !ui.lineEditNick->text().isEmpty();
}