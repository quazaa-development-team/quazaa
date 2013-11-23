/*
* Copyright (C) 2008-2013 The Communi Project
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESSAGEFORMATTER_H
#define MESSAGEFORMATTER_H

#include <IrcMessage>
#include <IrcTextFormat>

class MessageFormatter : public QObject
{
    Q_OBJECT

public:
    struct Options
    {
        Options() : repeat(false), highlight(false), stripNicks(false), textFormat(0) { }
        bool repeat;
        bool highlight;
        bool stripNicks;
        QString nickName;
        QStringList users;
        QDateTime timeStamp;
        QString timeStampFormat;
        IrcTextFormat* textFormat;
    };

    Q_INVOKABLE static QString formatMessage(IrcMessage* message, const Options& options = Options());
    static QString formatLine(const QString& message, const Options& options = Options());
    static QString formatHtml(const QString& message, const Options& options = Options());

protected:
    static QString formatInviteMessage(IrcInviteMessage* message, const Options& options);
    static QString formatJoinMessage(IrcJoinMessage* message, const Options& options);
    static QString formatKickMessage(IrcKickMessage* message, const Options& options);
    static QString formatModeMessage(IrcModeMessage* message, const Options& options);
    static QString formatNamesMessage(IrcNamesMessage* message, const Options& options);
    static QString formatNickMessage(IrcNickMessage* message, const Options& options);
    static QString formatNoticeMessage(IrcNoticeMessage* message, const Options& options);
    static QString formatNumericMessage(IrcNumericMessage* message, const Options& options);
    static QString formatPartMessage(IrcPartMessage* message, const Options& options);
    static QString formatPongMessage(IrcPongMessage* message, const Options& options);
    static QString formatPrivateMessage(IrcPrivateMessage* message, const Options& options);
    static QString formatQuitMessage(IrcQuitMessage* message, const Options& options);
    static QString formatTopicMessage(IrcTopicMessage* message, const Options& options);
    static QString formatUnknownMessage(IrcMessage* message, const Options& options);

    static QString formatPingReply(const QString& nick, const QString& arg);

    static QString formatNick(const QString& nick, bool own = false);
    static QString formatPrefix(const QString& prefix, bool strip = true, bool own = false);

    static QString formatIdleTime(int secs);

    static QString formatNames(const QStringList& names, int columns = 6);
};

#endif // MESSAGEFORMATTER_H
