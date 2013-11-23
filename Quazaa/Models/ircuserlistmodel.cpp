/*
** $Id$
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

#include "ircuserlistmodel.h"
#include "ircusermodel_p.h"
#include "quazaasettings.h"

IrcUserListModel::IrcUserListModel(QObject *parent) :
IrcUserModel(parent)
{
}

IrcUserListModel::~IrcUserListModel()
{
}

QVariant IrcUserListModel::data(const QModelIndex& index, int role) const
{
    if (!channel() || !hasIndex(index.row(), index.column(), index.parent()))
        return QVariant();

    IrcUser* user = static_cast<IrcUser*>(index.internalPointer());
    Q_ASSERT(user);

    switch (role) {
    case Qt::DisplayRole:
        return user->name();
    case Qt::DecorationRole:
        return modeToIcon(user->mode().left(1));
    case Qt::ForegroundRole:
        if(user->isServOp()) {
            if(user->isAway())
                return QColor("red");
            else
                return QColor("#9A2A2A");
        } else {
            if(user->isAway())
                return QColor(quazaaSettings.Chat.Colors[IrcColorType::Inactive]);
            else
                return QColor(quazaaSettings.Chat.Colors[IrcColorType::Default]);
        }
    case Irc::UserRole:
        return QVariant::fromValue(user);
    case Irc::NameRole:
        return user->name();
    case Irc::PrefixRole:
        return user->prefix().left(1);
    case Irc::ModeRole:
        return user->mode().left(1);
    case Irc::TitleRole:
        return user->title();
    }

    return QVariant();
}

QIcon IrcUserListModel::modeToIcon(const QString &mode) const
{
    if(mode == "Y")
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/ImmuneDark.png");
        else
            return QIcon(":/Resource/Chat/Immune.png");
    }
    else if(mode == "q")
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/OwnerDark.png");
        else
            return QIcon(":/Resource/Chat/Owner.png");
    }
    else if(mode == "a")
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/AdminDark.png");
        else
            return QIcon(":/Resource/Chat/Admin.png");
    }
    else if(mode == "o")
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/OpDark.png");
        else
            return QIcon(":/Resource/Chat/Op.png");
    }
    else if(mode == "h")
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/HalfOpDark.png");
        else
            return QIcon(":/Resource/Chat/HalfOp.png");
    }
    else if(mode == "v")
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/VoiceDark.png");
        else
            return QIcon(":/Resource/Chat/Voice.png");
    }
    else
    {
        if (quazaaSettings.Chat.DarkTheme)
            return QIcon(":/Resource/Chat/NormalDark.png");
        else
            return QIcon(":/Resource/Chat/Normal.png");
    }
}
