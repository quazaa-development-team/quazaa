/*
** types.h
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

#ifndef TYPES_H
#define TYPES_H

#include <time.h>

#ifdef __cplusplus

#include <QObject>
#include <QApplication>
#include <QString>
#include <QStringList>
#include <QHostAddress>
#include <QtGlobal>
#include <QtEndian>
#include <QUuid>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QDateTime>

// STL
#include <set>
#include <utility>
#include <functional>
#include <algorithm>
#include <limits>
#include <iterator>
#include <exception>

#include "endpoint.h"
#include "systemlog.h"
#include "commonfunctions.h"
#include "Discovery/networktype.h"

uint qHash( const QUuid& key );

enum TransferProtocol
{
	tpNull,
	tpHTTP,
	tpBitTorrent
};

enum NodeState { nsClosed,
				 nsConnecting,
				 nsHandshaking,
				 nsConnected,
				 nsClosing,
				 nsError };

enum G2NodeType {G2_UNKNOWN = 0, G2_LEAF = 1, G2_HUB = 2};

const quint64 SIZE_UNKNOWN = ~0ull;

typedef unsigned char BYTE;


#ifdef _DEBUG

#define ASSUME_LOCK(mutex) \
	if( (mutex).tryLock() ) { (mutex).unlock(); Q_ASSERT_X(false, Q_FUNC_INFO, "This code must be protected by " #mutex "!"); }

#else

#define ASSUME_LOCK(mutex) ((void)0)

#endif



#endif // __cplusplus


#endif // TYPES_H
