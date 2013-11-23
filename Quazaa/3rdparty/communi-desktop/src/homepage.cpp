/*
* Copyright (C) 2008-2013 The Communi Project
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

#include "homepage.h"
#include "singleapplication.h"
#include "dialogircsettings.h"
#include <QCommandLinkButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QLabel>
#include <QMenu>
#include <QFile>

static QString readHtmlFile(const QString& filePath)
{
	QFile file(filePath);
	file.open(QIODevice::ReadOnly);
	return QString::fromUtf8(file.readAll());
}

HomePage::HomePage(QWidget* parent) : QWidget(parent)
{
	bg.load(":/Resource/chatbackground.png");

	header = new QLabel(this);
	header->setMargin(2);
	header->setObjectName("headerLabel");
	header->setOpenExternalLinks(true);
	header->setWordWrap(true);
	header->setText(readHtmlFile(":/Resource/welcome_header.html").arg(SingleApplication::applicationName()).arg(SingleApplication::applicationSlogan()));

	slogan = new QLabel(this);
	slogan->setMargin(2);
	slogan->setObjectName("sloganLabel");
	slogan->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	slogan->setText("<small>Please respect others while using this service.</small>");

	footer = new QLabel(this);
	footer->setObjectName("footerLabel");
	footer->setMargin(2);
	footer->setOpenExternalLinks(true);
	footer->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	footer->setStyleSheet("");
	footer->setText("<small>Copyright (C) 2008-2013 The Quazaa Development Team</small>");

	QLineEdit lineEdit;
	lineEdit.setStyleSheet("QLineEdit { border: 1px solid transparent; }");
	slogan->setFixedHeight(lineEdit.sizeHint().height());
	footer->setFixedHeight(lineEdit.sizeHint().height());

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(header);
	layout->addWidget(slogan);
	layout->addWidget(createBody(this), 1);
	layout->addWidget(footer);
	setLayout(layout);
}

void HomePage::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	painter.setOpacity(0.4);
	painter.drawPixmap(width() - bg.width(), height() - footer->height() - bg.height(), bg);
}

void HomePage::showSettings()
{
	CDialogIrcSettings *dlgIrcSettings = new CDialogIrcSettings(this);

	dlgIrcSettings->exec();
}

QWidget* HomePage::createBody(QWidget* parent) const
{
	QWidget* body = new QWidget(parent);

	QCommandLinkButton* connectButton = new QCommandLinkButton(tr("Connect"), body);
	connectButton->setDescription(tr("New IRC connection"));
	QCommandLinkButton* settingsButton = new QCommandLinkButton(tr("Settings"), body);
	settingsButton->setDescription(tr("Configure %1").arg(SingleApplication::applicationName()));

	connect(connectButton, SIGNAL(clicked()), this, SIGNAL(connectRequested()));
	connect(settingsButton, SIGNAL(clicked()), this, SLOT(showSettings()));

	QGridLayout* layout = new QGridLayout(body);
	layout->setColumnStretch(0, 1);
	layout->setColumnStretch(3, 1);
	layout->setRowStretch(0, 1);
	layout->addWidget(connectButton, 1, 1);
	layout->addWidget(settingsButton, 1, 2);
	layout->setRowStretch(7, 1);
	body->setLayout(layout);

	return body;
}
