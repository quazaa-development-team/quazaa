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

#include "ircchannelstackview.h"
#include "quazaasettings.h"
#include "singleapplication.h"
#include "zncmanager.h"
#include "connection.h"
#include "chatcompleter.h"
#include <ircbuffer.h>
#include <irccommand.h>
#include <irclagtimer.h>
#include <ircbuffermodel.h>

IrcChannelStackView::IrcChannelStackView(IrcConnection* connection, QWidget* parent) : QStackedWidget(parent)
{
	d.connection = connection;

	d.bufferModel = new IrcBufferModel(connection);
	connect(d.bufferModel, SIGNAL(added(IrcBuffer*)), this, SLOT(setBuffer(IrcBuffer*)));
	connect(d.bufferModel, SIGNAL(messageIgnored(IrcMessage*)), &d.handler, SLOT(handleMessage(IrcMessage*)));
	connect(d.bufferModel, SIGNAL(channelsChanged(QStringList)), &d.parser, SLOT(setChannels(QStringList)));

	connection->installMessageFilter(qobject_cast<Connection*>(connection)); // TODO
	d.handler.znc()->setModel(d.bufferModel);

	connect(this, SIGNAL(currentChanged(int)), this, SLOT(activateView(int)));

	connect(&d.handler, SIGNAL(viewToBeAdded(QString)), this, SLOT(addView(QString)));
	connect(&d.handler, SIGNAL(viewToBeRemoved(QString)), this, SLOT(removeView(QString)));
	connect(&d.handler, SIGNAL(viewToBeRenamed(QString, QString)), this, SLOT(renameView(QString, QString)));

	d.parser.setTolerant(true);
	d.parser.setTriggers(QStringList() << "/");
	d.lagTimer = new IrcLagTimer(d.connection);

	MessageView* view = addView(connection->host());
	d.handler.setDefaultView(view);
	d.handler.setCurrentView(view);
	setCurrentWidget(view);

	applySettings();
}

IrcConnection* IrcChannelStackView::connection() const
{
	return d.connection;
}

CommandParser* IrcChannelStackView::parser() const
{
	return &const_cast<IrcChannelStackView*>(this)->d.parser;
}

QStringListModel* IrcChannelStackView::commandModel() const
{
	return &const_cast<IrcChannelStackView*>(this)->d.commandModel;
}

MessageView* IrcChannelStackView::currentView() const
{
	return qobject_cast<MessageView*>(currentWidget());
}

MessageView* IrcChannelStackView::viewAt(int index) const
{
	return qobject_cast<MessageView*>(widget(index));
}

MessageView* IrcChannelStackView::addView(const QString& receiver)
{
	MessageView* view = d.views.value(receiver.toLower());
	bool channel = !receiver.isEmpty() && d.connection->network()->channelTypes().contains(receiver.at(0));
	if (!view) {
		ViewInfo::Type type = ViewInfo::Server;
		if (!d.views.isEmpty())
			type = channel ? ViewInfo::Channel : ViewInfo::Query;
		view = createView(type, receiver);
	}
	if (channel && !view->isActive())
		openView(receiver);
	qApp->processEvents();
	return view;
}

void IrcChannelStackView::restoreView(const ViewInfo& view)
{
	createView(static_cast<ViewInfo::Type>(view.type), view.name);
}

MessageView* IrcChannelStackView::createView(ViewInfo::Type type, const QString& receiver)
{
	MessageView* view = new MessageView(type, static_cast<Connection*>(d.connection), this); // TODO
	const IrcNetwork* network = d.connection->network();
	view->completer()->setChannelPrefixes(network->channelTypes().join(""));
	view->completer()->setChannelModel(&d.viewModel);
	view->setReceiver(receiver);
	connect(view, SIGNAL(queried(QString)), this, SLOT(addView(QString)));
	connect(view, SIGNAL(queried(QString)), this, SLOT(openView(QString)));
	connect(view, SIGNAL(messaged(QString,QString)), this, SLOT(sendMessage(QString,QString)));
	connect(view, SIGNAL(renamed(QString,QString)), this, SLOT(renameView(QString,QString)));

	if(view->viewType() == ViewInfo::Server)
	{
		//TODO: Use this
	} else if (view->viewType() == ViewInfo::Channel) {
		connect(view, SIGNAL(appendRawMessage(QString)), d.handler.defaultView()->textBrowser(), SLOT(append(QString)));
		connect(view, SIGNAL(appendRawMessage(QString)), this, SLOT(switchToServerTab()));
	} else {
		connect(view, SIGNAL(appendRawMessage(QString)), d.handler.defaultView()->textBrowser(), SLOT(append(QString)));
		connect(view, SIGNAL(appendRawMessage(QString)), this, SLOT(switchToServerTab()));
	}

	connect(d.lagTimer, SIGNAL(lagChanged(qint64)), view, SLOT(updateLag(qint64)));
	view->updateLag(d.lagTimer->lag());

	d.handler.addView(receiver, view);
	d.views.insert(receiver.toLower(), view);
	view->setBuffer(d.bufferModel->add(receiver));
	view->buffer()->setPersistent(true);
	addWidget(view);
	d.viewModel.setStringList(d.viewModel.stringList() << receiver);
	emit viewAdded(view);
	return view;
}

void IrcChannelStackView::openView(const QString& receiver)
{
	MessageView* view = d.views.value(receiver.toLower());
	if (view)
		setCurrentWidget(view);
}

void IrcChannelStackView::removeView(const QString& receiver)
{
	MessageView* view = d.views.take(receiver.toLower());
	if (view) {
		view->deleteLater();
		QStringList views = d.viewModel.stringList();
		if (views.removeOne(receiver))
			d.viewModel.setStringList(views);
		emit viewRemoved(view);
		d.handler.removeView(view->receiver());
	}
}

void IrcChannelStackView::closeView(int index)
{
	MessageView* view = viewAt(index);
	if (view) {
		if (view->isActive()) {
			if (indexOf(view) == 0)
				static_cast<Connection*>(connection())->quit(); // TODO
			else if (view->viewType() == ViewInfo::Channel)
				d.connection->sendCommand(IrcCommand::createPart(view->receiver()));
		}
		d.handler.removeView(view->receiver());
	}
}

void IrcChannelStackView::renameView(const QString& from, const QString& to)
{
	if (!d.views.contains(to.toLower())) {
		MessageView* view = d.views.take(from.toLower());
		if (view) {
			view->setReceiver(to);
			d.views.insert(to.toLower(), view);
			emit viewRenamed(view);
		}
	} else if (currentView() == d.views.value(from.toLower())) {
		setCurrentWidget(d.views.value(to.toLower()));
	}
}

void IrcChannelStackView::sendMessage(const QString& receiver, const QString& message)
{
	MessageView* view = addView(receiver);
	if (view) {
		setCurrentWidget(view);
		view->sendMessage(message);
	}
}

void IrcChannelStackView::switchToServerTab()
{
	setCurrentIndex(0);
}

void IrcChannelStackView::applySettings()
{
	d.handler.znc()->setTimeStampFormat(quazaaSettings.Chat.TimestampFormat);

	d.parser.setAliases(quazaaSettings.Chat.Aliases);

	QStringList commands;
	foreach (const QString& command, d.parser.availableCommands()) {
		foreach(const QString& trigger, d.parser.triggers())
			commands += trigger + command;
	}
	d.commandModel.setStringList(commands);
}

void IrcChannelStackView::activateView(int index)
{
	MessageView* view = viewAt(index);
	if (view && isVisible()) {
		d.handler.setCurrentView(view);
		view->setFocus();
		emit viewActivated(view);
	}
}

void IrcChannelStackView::setBuffer(IrcBuffer* buffer)
{
	MessageView* view = addView(buffer->title());
	if (view) {
		view->setBuffer(buffer);
		buffer->setPersistent(true);
	}
}
