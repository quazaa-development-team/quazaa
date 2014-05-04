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

#include "messageview.h"
#include "singleapplication.h"
#include "quazaasettings.h"
#include "syntaxhighlighter.h"
#include "ircchannelstackview.h"
#include "messageformatter.h"
#include "ignoremanager.h"
#include "commandparser.h"
#include "connection.h"
#include "chatcompleter.h"
#include "quazaasysinfo.h"
#include "quazaaglobals.h"
#include "chatconverter.h"
#include <QRegularExpression>
#include <QAbstractTextDocumentLayout>
#include <QDesktopServices>
#include <QTextBlock>
#include <QShortcut>
#include <QKeyEvent>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QUrl>
#include <QTimer>
#include <IrcUser>
#include <irctextformat.h>
#include <ircuserlistmodel.h>
#include <ircpalette.h>
#include <ircmessage.h>
#include <irccommand.h>
#include <ircchannel.h>
#include <ircbuffer.h>
#include <irc.h>
#include <widgetchatinput.h>

Q_GLOBAL_STATIC(IrcTextFormat, irc_text_format)

MessageView::MessageView(ViewInfo::Type type, IrcConnection* connection, IrcChannelStackView* stackView) :
    QWidget(stackView)
{
    d.setupUi(this);
    d.viewType = type;
    d.sentId = 1;
#if QT_VERSION >= 0x040700
    d.awayReply.invalidate();
#else
    d.awayReply = QTime();
#endif
    d.playback = false;
    d.parser = stackView->parser();
    d.firstNames = true;
    d.messageTimer = new QTimer(this);
    connect(d.messageTimer, SIGNAL(timeout()), this, SLOT(sendMessageLine()));
    d.whoTimer = new QTimer(this);
    connect(d.whoTimer, SIGNAL(timeout()), this, SLOT(sendWho()));

    d.joined = 0;
    d.parted = 0;
    d.connected = 0;
    d.disconnected = 0;

    d.chatInput = new CWidgetChatInput(this, true);
    d.verticalLayoutInputWidget->addWidget(d.chatInput);
    d.widgetInput->setFixedHeight(130);
    d.chatInput->setFixedHeight(130);

    connect(d.splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(onSplitterMoved()));

    setFocusProxy(d.chatInput->textEdit());
    d.textBrowser->setBuddy(d.chatInput->textEdit());
    d.textBrowser->viewport()->installEventFilter(this);
    connect(d.textBrowser, SIGNAL(anchorClicked(QUrl)), SLOT(onAnchorClicked(QUrl)));

    d.highlighter = new SyntaxHighlighter(d.textBrowser->document());

    d.connection = connection;
    connect(d.connection, SIGNAL(statusChanged(IrcConnection::Status)), this, SLOT(onConnectionStatusChanged()));

    if (type == ViewInfo::Server)
        connect(d.connection, SIGNAL(socketError(QAbstractSocket::SocketError)), this, SLOT(onSocketError()));

    d.topicLabel->setVisible(type == ViewInfo::Channel);
    d.listView->setVisible(type == ViewInfo::Channel);
    if (type == ViewInfo::Channel) {
        d.listView->setConnection(connection);
        d.listView->setIconSize(QSize(16,16));
        connect(d.listView, SIGNAL(queried(QString)), this, SIGNAL(queried(QString)));
        connect(d.listView, SIGNAL(doubleClicked(QString)), this, SIGNAL(queried(QString)));
        connect(d.listView, SIGNAL(commandRequested(IrcCommand*)), d.connection, SLOT(sendCommand(IrcCommand*)));
        connect(d.topicLabel, SIGNAL(edited(QString)), this, SLOT(onTopicEdited(QString)));
    } else if (type == ViewInfo::Server) {
        connect(d.connection, SIGNAL(connected()), this, SLOT(onConnected()));
        connect(d.connection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    }

    static bool init = false;
    if (!init) {
        init = true;
        IrcTextFormat* format = irc_text_format();
        IrcPalette* palette = format->palette();
        palette->setColorName(Irc::Black, "black");
        palette->setColorName(Irc::White, "white");
        palette->setColorName(Irc::Gray, "#606060");
        palette->setColorName(Irc::LightGray, "#808080");

        // http://ethanschoonover.com/solarized
        palette->setColorName(Irc::Blue, "#268bd2");
        palette->setColorName(Irc::Green, "#859900");
        palette->setColorName(Irc::Red, "#dc322f");
        palette->setColorName(Irc::Brown, "#cb4b16");
        palette->setColorName(Irc::Purple, "#6c71c4");
        palette->setColorName(Irc::Orange, "#cb4b16");
        palette->setColorName(Irc::Yellow, "#b58900");
        palette->setColorName(Irc::LightGreen, "#859900");
        palette->setColorName(Irc::Cyan, "#2aa198");
        palette->setColorName(Irc::LightCyan, "#2aa198");
        palette->setColorName(Irc::LightBlue, "#268bd2");
        palette->setColorName(Irc::Pink, "#6c71c4");
    }

    QTextDocument* doc = d.topicLabel->findChild<QTextDocument*>();
    if (doc)
        doc->setDefaultStyleSheet("a { color: #4040ff }");

    d.chatInput->textEdit()->completer()->setCommandModel(stackView->commandModel());
    connect(d.chatInput->textEdit()->completer(), SIGNAL(commandCompletion(QString)), this, SLOT(completeCommand(QString)));

    connect(d.chatInput, SIGNAL(messageSent(QString)), this, SLOT(sendMessage(QString)));
    connect(d.chatInput, SIGNAL(messageSent(QTextDocument*)), this, SLOT(sendMessage(QTextDocument*)));
    connect(d.chatInput->textEdit(), SIGNAL(textChanged(QString)), this, SLOT(showHelp(QString)));

    connect(d.chatInput->textEdit(), SIGNAL(scrollToTop()), d.textBrowser, SLOT(scrollToTop()));
    connect(d.chatInput->textEdit(), SIGNAL(scrollToBottom()), d.textBrowser, SLOT(scrollToBottom()));
    connect(d.chatInput->textEdit(), SIGNAL(scrollToNextPage()), d.textBrowser, SLOT(scrollToNextPage()));
    connect(d.chatInput->textEdit(), SIGNAL(scrollToPreviousPage()), d.textBrowser, SLOT(scrollToPreviousPage()));

    d.chatInput->helpLabel()->hide();
    d.searchEditor->setTextEdit(d.textBrowser);

    QShortcut* shortcut = new QShortcut(Qt::Key_Escape, this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(onEscPressed()));

    applySettings();
    connect(&quazaaSettings, SIGNAL(chatSettingsChanged()), this, SLOT(applySettings()));
}

MessageView::~MessageView()
{
    if (d.buffer) {
        if (d.buffer->isChannel() && d.buffer->isActive())
            connect(d.buffer, SIGNAL(activeChanged(bool)), d.buffer, SLOT(deleteLater()));
        else
            d.buffer->deleteLater();
    }
}

bool MessageView::isActive() const
{
    if (d.buffer)
        return d.buffer->isActive();
    return d.connection && d.connection->isConnected();
}

ViewInfo::Type MessageView::viewType() const
{
    return d.viewType;
}

IrcConnection* MessageView::connection() const
{
    return d.connection;
}

ChatCompleter* MessageView::completer() const
{
    return d.chatInput->textEdit()->completer();
}

QTextBrowser* MessageView::textBrowser() const
{
    return d.textBrowser;
}

QString MessageView::receiver() const
{
    return d.receiver;
}

void MessageView::setReceiver(const QString& receiver)
{
    if (d.receiver != receiver) {
        d.receiver = receiver;
        emit receiverChanged(receiver);
    }
}

IrcBuffer* MessageView::buffer() const
{
    return d.buffer;
}

void MessageView::setBuffer(IrcBuffer* buffer)
{
    if (d.buffer != buffer) {
        if (IrcChannel* channel = qobject_cast<IrcChannel*>(buffer)) {
            d.listView->setChannel(channel);
            IrcUserListModel* activityModel = new IrcUserListModel(channel);
            activityModel->setSortMethod(Irc::SortByActivity);
            d.chatInput->textEdit()->completer()->setUserModel(activityModel);
        }
        d.buffer = buffer;
        connect(buffer, SIGNAL(activeChanged(bool)), this, SIGNAL(activeChanged()));
        connect(buffer, SIGNAL(messageReceived(IrcMessage*)), this, SLOT(receiveMessage(IrcMessage*)));
        connect(buffer, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChanged(QString)));
    }
}

bool MessageView::playbackMode() const
{
    return d.playback;
}

void MessageView::setPlaybackMode(bool enabled)
{
    d.playback = enabled;
}

QByteArray MessageView::saveSplitter() const
{
    if (d.viewType != ViewInfo::Server)
        return d.splitter->saveState();
    return QByteArray();
}

void MessageView::restoreSplitter(const QByteArray& state)
{
    d.splitter->restoreState(state);
}

void MessageView::showHelp(const QString& text, bool error)
{
    QString syntax;
    if (text == "/") {
        QStringList commands = d.parser->availableCommands();
        syntax = commands.join(" ");
    } else if (text.startsWith('/')) {
        QStringList words = text.mid(1).split(' ');
        QString command = words.value(0);
        QStringList suggestions = d.parser->suggestedCommands(command, words.mid(1));
        if (suggestions.count() == 1)
            syntax = d.parser->visualSyntax(suggestions.first());
        else
            syntax = suggestions.join(" ");

        if (syntax.isEmpty())
            syntax = tr("SERVER COMMAND: %1 %2").arg(command.toUpper(), QStringList(words.mid(1)).join(" "));
    }

    d.chatInput->helpLabel()->setVisible(!syntax.isEmpty());
    QPalette pal;
    if (error)
        pal.setColor(QPalette::WindowText, "#ff4040");
    d.chatInput->helpLabel()->setPalette(pal);
    d.chatInput->helpLabel()->setText(syntax);
}

void MessageView::sendMessage(const QString& line)
{
    if (d.messageTimer->isActive()) {
        d.messages.append(line.split(QRegularExpression("[\\r\\n]"), QString::KeepEmptyParts));
    } else {
        d.messages = line.split(QRegularExpression("[\\r\\n]"), QString::KeepEmptyParts);
        sendMessageLine();
        d.messageTimer->start(1000);
    }
}

void MessageView::sendMessage(QTextDocument *message)
{
    ChatConverter *converter = new ChatConverter(message);
    sendMessage(converter->toIrc());
}

void MessageView::sendMessageLine() {
    if(d.messages.isEmpty()) {
        d.messageTimer->stop();
        return;
    }

    QString message = d.messages.takeFirst();

    if ((viewType() == ViewInfo::Server) && !message.startsWith("/"))
    {
        QString modify = message;
        modify.prepend(tr("/quote "));
        sendMessage(modify);
    } else {
        d.parser->setTarget(d.receiver);
        IrcCommand* cmd = d.parser->parseCommand(message);
        if (cmd) {
            if (cmd->type() == IrcCommand::Quote)
            {
                if(viewType() == ViewInfo::Server)
                    d.textBrowser->append( tr("[RAW] %1").arg(cmd->parameters().join(" ") ));
                else
                    emit appendRawMessage( tr("[RAW] %1").arg(cmd->parameters().join(" ") ));
            }
            if (cmd->type() == IrcCommand::Join) {
                QStringList params = cmd->parameters();

                if (params.count() == 1 || params.count() == 2)
                {
                    QString channel = params.value(0);
                    QString password = params.value(1);

                    if(!d.connection->network()->channelTypes().contains(channel.at(0)))
                    {
                        channel.prepend("#");
                    }
                    if(!d.connection->network()->channelTypes().contains(channel.at(0)))
                    {
                        channel = channel.remove(0,1);
                        channel.prepend(d.connection->network()->channelTypes().at(0));
                    }
                    if(d.connection->network()->channelTypes().contains(channel.at(0)))
                        cmd = d.parser->parse(QString("/JOIN %1 %2").arg(channel).arg(password));
                }
            }
            if (cmd->type() == IrcCommand::Custom) {
                QString command = cmd->parameters().value(0);
                QStringList params = cmd->parameters().mid(1);
                if (command == "CLEAR") {
                    d.textBrowser->clear();
                } else if (command == "CLOSE") {
                    QMetaObject::invokeMethod(window(), "closeView");
                } else if (command == "MSG") {
                    // support "/msg foo /cmd" without executing /cmd
                    QString msg = QStringList(params.mid(1)).join(" ");
                    if (msg.startsWith('/') && !msg.startsWith("//") && !msg.startsWith("/ "))
                        msg.prepend('/');
                    emit messaged(params.value(0), msg);
                } else if (command == "QUERY") {
                    emit queried(params.value(0));
                } else if (command == "IGNORE") {
                    MessageFormatter::Options options;
                    options.timeStampFormat = d.timeStampFormat;
                    options.timeStamp = QDateTime::currentDateTime();
                    QString ignore  = params.value(0);
                    if (ignore.isEmpty()) {
                        const QStringList ignores = IgnoreManager::instance()->ignores();
                        if (!ignores.isEmpty()) {
                            foreach (const QString& ignore, ignores)
                                d.textBrowser->append(MessageFormatter::formatLine("! " + ignore, options));
                        } else {
                            d.textBrowser->append(MessageFormatter::formatLine("! no ignores", options));
                        }
                    } else {
                        QString mask = IgnoreManager::instance()->addIgnore(ignore);
                        d.textBrowser->append(MessageFormatter::formatLine("! ignored: " + mask));
                    }
                } else if (command == "UNIGNORE") {
                    QString mask = IgnoreManager::instance()->removeIgnore(params.value(0));
                    d.textBrowser->append(MessageFormatter::formatLine("! unignored: " + mask));
                }
                else if (command == "SYSINFO")
                {
                    QuazaaSysInfo *sysInfo = new QuazaaSysInfo();
                    sendMessage(tr("Application:%1 %2 OS:%3 Qt Version:%4").arg(QApplication::applicationName(), QuazaaGlobals::APPLICATION_VERSION_STRING(), sysInfo->osVersionToString(), qVersion()));
                    //onSend(tr("CPU:%1 Cores:%2 Memory:%3").arg(QApplication::applicationName(), QuazaaGlobals::APPLICATION_VERSION_STRING()));
                }
                else if (command == "CTCP")
                {
                    QString args = params.at(1).toUpper().append(" ");
                    QStringList command = params.mid(2);
                    args.append(command.join(" "));
                    cmd = IrcCommand::createCtcpRequest(params.at(0), args);
                }
                delete cmd;
            } else if (cmd->type() == IrcCommand::Message || cmd->type() == IrcCommand::CtcpAction || cmd->type() == IrcCommand::Notice) {
                if (Connection* s = qobject_cast<Connection*>(d.connection)) // TODO
                    s->sendUiCommand(cmd, QString("_communi_msg_%1_%2").arg(d.receiver).arg(++d.sentId));

                IrcMessage* msg = IrcMessage::fromData(":" + d.connection->nickName().toUtf8() + " " + cmd->toString().toUtf8(), d.connection);
                receiveMessage(msg);
                delete msg;

                // highlight as gray until acked
                QTextBlock block = d.textBrowser->document()->lastBlock();
                block.setUserState(d.sentId);
                d.highlighter->rehighlightBlock(block);
            } else {
                if (Connection* s = qobject_cast<Connection*>(d.connection)) // TODO
                    s->sendUiCommand(cmd);
            }
            d.chatInput->helpLabel()->hide();
        } else {
            showHelp(message, true);
        }
    }
    if(d.messages.isEmpty())
        d.messageTimer->stop();
}

void MessageView::sendWho()
{
    sendMessage(QString("/who %1").arg( receiver() ));
}

void MessageView::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    onEscPressed();
}

bool MessageView::eventFilter(QObject* object, QEvent* event)
{
    if (object == d.textBrowser->viewport() && event->type() == QEvent::ContextMenu) {
        QContextMenuEvent* menuEvent = static_cast<QContextMenuEvent*>(event);
        QMenu* menu = d.textBrowser->createStandardContextMenu(menuEvent->pos());

        QAction* query = 0;
        QAction* whois = 0;
        QAction* users = 0;
        QAction* views = 0;

        QUrl link(d.textBrowser->anchorAt(menuEvent->pos()));
        if (link.scheme() == "nick") {
            QAction* separator = menu->insertSeparator(menu->actions().value(0));

            query = new QAction(tr("Query"), menu);
            menu->insertAction(separator, query);

            whois = new QAction(tr("Whois"), menu);
            menu->insertAction(query, whois);
        }

        QAction* separator = 0;
        if (d.viewType == ViewInfo::Channel && d.splitter->sizes().value(1) == 0) {
            separator = menu->addSeparator();
            users = menu->addAction(tr("Restore users"));
        }

        QSplitter* splitter = window()->findChild<QSplitter*>();
        if (splitter && splitter->sizes().value(0) == 0) {
            if (!separator)
                menu->addSeparator();
            views = menu->addAction(tr("Restore views"));
        }

        QAction* action = menu->exec(menuEvent->globalPos());
        if (action) {
            const QString user = link.toString(QUrl::RemoveScheme);
            if (action == whois) {
                IrcCommand* command = IrcCommand::createWhois(user);
                d.connection->sendCommand(command);
            } else if (action == query) {
                emit queried(user);
            } else if (action == users) {
                d.splitter->setSizes(QList<int>() << d.textBrowser->sizeHint().width() << d.listView->sizeHint().width());
                onSplitterMoved();
            } else if (action == views) {
                splitter->setSizes(QList<int>() << splitter->widget(0)->sizeHint().width() << splitter->widget(1)->sizeHint().width());
            }
        }

        menu->deleteLater();
        return true;
    }
    return QWidget::eventFilter(object, event);
}

void MessageView::onConnected()
{
    ++d.connected;

    int blocks = d.textBrowser->document()->blockCount();
    if (blocks > 10) // TODO
        d.textBrowser->addMarker(blocks);
}

void MessageView::onDisconnected()
{
    ++d.disconnected;
}

void MessageView::onEscPressed()
{
    d.chatInput->helpLabel()->hide();
    d.searchEditor->hide();
    setFocus(Qt::OtherFocusReason);
}

void MessageView::onTitleChanged(const QString& title)
{
    const QString old = d.receiver;
    setReceiver(title);
    emit renamed(old, title);
}

void MessageView::onSplitterMoved()
{
    emit splitterChanged(saveSplitter());
}

void MessageView::onAnchorClicked(const QUrl& link)
{
    if (link.scheme() == "nick") {
        emit queried(link.toString(QUrl::RemoveScheme));
    } else {
        QDesktopServices::openUrl(link);
        // avoid focus rectangle around the link
        d.chatInput->textEdit()->setFocus();
    }
}

void MessageView::completeCommand(const QString& command)
{
    if (command == "TOPIC")
        d.chatInput->textEdit()->insertPlainText(d.topic);
}

void MessageView::onTopicEdited(const QString& topic)
{
    d.connection->sendCommand(IrcCommand::createTopic(d.receiver, topic));
}

void MessageView::onConnectionStatusChanged()
{
    d.chatInput->textEdit()->setFocusPolicy(d.connection->isActive() ? Qt::StrongFocus : Qt::NoFocus);
    d.textBrowser->setFocusPolicy(d.connection->isActive() ? Qt::StrongFocus : Qt::NoFocus);
}

void MessageView::onSocketError()
{
    QString msg = tr("[ERROR] %1").arg(d.connection->socket()->errorString());
    d.textBrowser->append(MessageFormatter::formatLine(msg));
}

void MessageView::applySettings()
{
    bool dark = quazaaSettings.Chat.DarkTheme;

    d.textBrowser->setFont(quazaaSettings.Chat.Font);
    d.textBrowser->document()->setMaximumBlockCount(quazaaSettings.Chat.Scrollback);

    d.highlighter->setUnsentColor(QColor(quazaaSettings.Chat.Colors[IrcColorType::Inactive]));

    if (dark) {
        d.textBrowser->setShadowColor(Qt::black);
        d.textBrowser->setMarkerColor(QColor(quazaaSettings.Chat.Colors[IrcColorType::Highlight]));
        d.textBrowser->setHighlightColor(QColor(quazaaSettings.Chat.Colors[IrcColorType::Highlight]).darker(300));

        d.searchEditor->setButtonPixmap(SearchEditor::Left, QPixmap(":/Resource/Chat/Buttons/prev-white.png"));
        d.searchEditor->setButtonPixmap(SearchEditor::Right, QPixmap(":/Resource/Chat/Buttons/next-white.png"));
    } else {
        d.textBrowser->setShadowColor(Qt::gray);
        d.textBrowser->setMarkerColor(QColor(quazaaSettings.Chat.Colors[IrcColorType::Highlight]));
        d.textBrowser->setHighlightColor(QColor(quazaaSettings.Chat.Colors[IrcColorType::Highlight]).darker(300));

        d.searchEditor->setButtonPixmap(SearchEditor::Left, QPixmap(":/Resource/Chat/Buttons/prev-black.png"));
        d.searchEditor->setButtonPixmap(SearchEditor::Right, QPixmap(":/Resource/Chat/Buttons/next-black.png"));
    }

    IrcTextFormat* format = irc_text_format();
    IrcPalette* palette = format->palette();
    palette->setColorName(Irc::Black, quazaaSettings.Chat.Colors[IrcColorType::Black]);
    palette->setColorName(Irc::White, quazaaSettings.Chat.Colors[IrcColorType::White]);
    palette->setColorName(Irc::Gray, quazaaSettings.Chat.Colors[IrcColorType::Gray]);
    palette->setColorName(Irc::LightGray, quazaaSettings.Chat.Colors[IrcColorType::LightGray]);
    palette->setColorName(Irc::Blue, quazaaSettings.Chat.Colors[IrcColorType::Blue]);
    palette->setColorName(Irc::Green, quazaaSettings.Chat.Colors[IrcColorType::Green]);
    palette->setColorName(Irc::Red, quazaaSettings.Chat.Colors[IrcColorType::Red]);
    palette->setColorName(Irc::Brown, quazaaSettings.Chat.Colors[IrcColorType::Brown]);
    palette->setColorName(Irc::Purple, quazaaSettings.Chat.Colors[IrcColorType::Purple]);
    palette->setColorName(Irc::Orange, quazaaSettings.Chat.Colors[IrcColorType::Orange]);
    palette->setColorName(Irc::Yellow, quazaaSettings.Chat.Colors[IrcColorType::Yellow]);
    palette->setColorName(Irc::LightGreen, quazaaSettings.Chat.Colors[IrcColorType::LightGreen]);
    palette->setColorName(Irc::Cyan, quazaaSettings.Chat.Colors[IrcColorType::Cyan]);
    palette->setColorName(Irc::LightCyan, quazaaSettings.Chat.Colors[IrcColorType::LightCyan]);
    palette->setColorName(Irc::LightBlue, quazaaSettings.Chat.Colors[IrcColorType::LightBlue]);
    palette->setColorName(Irc::Pink, quazaaSettings.Chat.Colors[IrcColorType::Pink]);

    QColor highlightText(quazaaSettings.Chat.Colors[IrcColorType::Highlight]);
    d.textBrowser->document()->setDefaultStyleSheet( QString(
            ".highlight { color: %1; }"
            ".notice    { color: %2; }"
            ".action    { color: %3; }"
            ".event     { color: %4; }"
            ".timestamp { color: %5; font-size: small; }"
            "a { color: %6; }")
    .arg(highlightText.lighter(175).name())
    .arg(quazaaSettings.Chat.Colors[IrcColorType::Notice])
    .arg(quazaaSettings.Chat.Colors[IrcColorType::Action])
    .arg(quazaaSettings.Chat.Colors[IrcColorType::Event])
    .arg(quazaaSettings.Chat.Colors[IrcColorType::TimeStamp])
    .arg(quazaaSettings.Chat.Colors[IrcColorType::Link]));

    d.showJoins = quazaaSettings.Chat.Messages[IrcMessageType::Joins];
    d.showParts = quazaaSettings.Chat.Messages[IrcMessageType::Parts];
    d.showQuits = quazaaSettings.Chat.Messages[IrcMessageType::Quits];
    d.stripNicks = quazaaSettings.Chat.StripNicks;
    d.timeStampFormat = quazaaSettings.Chat.TimestampFormat;
}

void MessageView::receiveMessage(IrcMessage* message)
{
    bool ignore = false;
    MessageFormatter::Options options;

    switch (message->type()) {
        case IrcMessage::Private: {
            if (message->nick() == QLatin1String("***")) {
                QString content = static_cast<IrcPrivateMessage*>(message)->content();
                if (content == QLatin1String("Buffer Playback..."))
                    ignore = true;
                else if (content == QLatin1String("Playback Complete."))
                    ignore = true;
            }
            if (static_cast<IrcPrivateMessage*>(message)->content().contains(d.connection->nickName()))
                options.highlight = true;
            break;
        }
        case IrcMessage::Notice:
            if (static_cast<IrcNoticeMessage*>(message)->content().contains(d.connection->nickName()))
                options.highlight = true;
            break;
        case IrcMessage::Topic:
        {
            d.topic = static_cast<IrcTopicMessage*>(message)->topic();
            IrcTextFormat* format = irc_text_format();
            d.topicLabel->setText(format->toHtml(d.topic));
            if (d.topicLabel->text().isEmpty())
                d.topicLabel->setText(tr("-"));
            break;
        }
        case IrcMessage::Join:
            if (!d.playback && message->flags() & IrcMessage::Own) {
                ++d.joined;
                d.firstNames = true;
                int blocks = d.textBrowser->document()->blockCount();
                if (blocks > 1)
                    d.textBrowser->addMarker(blocks);
            }
            if (!d.showJoins)
                return;
            break;
        case IrcMessage::Quit:
            if (!d.playback && message->flags() & IrcMessage::Own)
                ++d.parted;
            if (!d.showQuits)
                return;
            break;
        case IrcMessage::Part:
            if (!d.playback && message->flags() & IrcMessage::Own)
                ++d.parted;
            if (!d.showParts)
                return;
            break;
        case IrcMessage::Kick:
            if (!d.playback && !static_cast<IrcKickMessage*>(message)->user().compare(d.connection->nickName(), Qt::CaseInsensitive))
                ++d.parted;
            break;
        case IrcMessage::Pong: {
            QString arg = static_cast<IrcPongMessage*>(message)->argument();
            if (arg.startsWith("_communi_msg_")) {
                int id = arg.mid(arg.lastIndexOf("_") + 1).toInt();
                if (id > 0) {
                    QTextBlock block = d.textBrowser->document()->lastBlock();
                    while (block.isValid()) {
                        if (block.userState() == id) {
                            block.setUserState(-1);
                            d.highlighter->rehighlightBlock(block);
                            break;
                        }
                        block = block.previous();
                    }
                }
            }
            break;
        }
        case IrcMessage::WhoReply:
            return;
        case IrcMessage::Numeric:
            switch (static_cast<IrcNumericMessage*>(message)->code()) {
                case Irc::RPL_TOPICWHOTIME: {
                    QDateTime dateTime = QDateTime::fromTime_t(message->parameters().value(3).toInt());
                    d.topicLabel->setToolTip(tr("Set %1 by %2").arg(dateTime.toString(), message->parameters().value(2)));
                    break;
                }
                case Irc::RPL_AWAY: // TODO: configurable
                    if (d.awayReply.isValid() && d.awayReply.elapsed() < 1800000
                            && message->parameters().last() == d.awayMessage) {
                        return;
                    }
                    d.awayReply.restart();
                    d.awayMessage = message->parameters().last();
                    break;
                case Irc::RPL_ENDOFWHO:
                {
                    if( viewType() == ViewInfo::Channel ) {
                        d.listView->userModel()->sort();
                        d.whoTimer->start(300000);
                    }

                    return;
                }
                default:
                    break;
            }
            break;
        default:
            break;
    }

    options.nickName = d.connection->nickName();
    if (IrcUserListModel* model = d.listView->userModel())
        options.users = model->names();
    options.stripNicks = d.stripNicks;
    options.timeStampFormat = d.timeStampFormat;
    options.textFormat = irc_text_format();
    if (d.viewType == ViewInfo::Channel) {
        options.repeat = (d.joined > 1 && d.joined > d.parted);
        if (message->type() == IrcMessage::Names) {
            // - short version on first join
            // - ignore names on consecutive joins
            // - long version as reply to /names
            bool wasFirst = d.firstNames;
            options.repeat = !d.firstNames;
            d.firstNames = false;
            if (d.joined > 1 && wasFirst)
                return;
        }
    } else if (d.viewType == ViewInfo::Server) {
        options.repeat = (d.connected > 1);
    }

    QString formatted = MessageFormatter::formatMessage(message, options);
    if (formatted.length()) {
        if (!ignore && (!isVisible() || !isActiveWindow())) {
            IrcMessage::Type type = message->type();
            if (options.highlight || ((type == IrcMessage::Notice || type == IrcMessage::Private) && d.viewType != ViewInfo::Channel))
                emit highlighted(message);
            if ((type == IrcMessage::Notice || type == IrcMessage::Private) && !playbackMode()) // TODO: || (!d.receivedCodes.contains(Irc::RPL_ENDOFMOTD) && d.viewType == ViewInfo::Server))
                emit missed(message);
            if(type == IrcMessage::Notice || type == IrcMessage::Private || type == IrcMessage::Invite || type == IrcMessage::Kick)
                emit messageAlert();
        }

        d.textBrowser->append(formatted, options.highlight);
    }
}

bool MessageView::hasUser(const QString& user) const
{
    return (!d.connection->nickName().compare(user, Qt::CaseInsensitive) && d.viewType != ViewInfo::Query) ||
           (d.viewType == ViewInfo::Query && !d.receiver.compare(user, Qt::CaseInsensitive)) ||
           (d.viewType == ViewInfo::Channel && d.listView->hasUser(user));
}

void MessageView::updateLag(qint64 lag)
{
    d.chatInput->setLag(lag);
}
