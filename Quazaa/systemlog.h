/*
** systemlog.h
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

#ifndef SYSTEMLOG_H
#define SYSTEMLOG_H

#ifndef QUAZAA_SETUP_UNIT_TESTS
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#endif

#include <QObject>
#include <QString>

enum class LogSeverity : quint8
{
	Information = 1, // Inform the user about something
	Security    = 2, // security related
	Notice      = 3, // Inform the user about something less important
	Debug       = 4, // Debugging output
	Warning     = 5,
	Error       = 6,
	Critical    = 7
};

enum class Component : quint8
{
	None         =  0,
	Chat         =  1, // P2P chat
	IRC          =  2,
	Discovery    =  3,
	Network      =  4, // global network stuff
	Ares         =  5,
	BitTorrent   =  6,
	eD2k         =  7,
	G2           =  8,
	Security     =  9,
	Library      = 10, // Share manager
	Downloads    = 11,
	Uploads      = 12,
	GUI          = 13,
	SignalQueue  = 14,
	HostCache    = 15,
	NoComponents = 16
};

class SystemLog : public QObject
{
	Q_OBJECT

#ifndef QUAZAA_SETUP_UNIT_TESTS
private:
	QMutex m_pSection;
	bool m_bProcessingMessage;
#endif

	QString* m_pComponents;

public:
	SystemLog();
	~SystemLog();

	void start();

	QString msgFromComponent( Component nComponent );

#ifndef QUAZAA_SETUP_UNIT_TESTS
signals:
	void logPosted( QString sMessage, LogSeverity nSeverity );

public slots:
	void postLog( LogSeverity nSeverity, const QString& sMessage );

public:
#else
	void postLog( LogSeverity nSeverity, const QString& sMessage );
#endif
	void postLog( LogSeverity nSeverity, Component nComponent, const QString& sMessage );
	void postLog( LogSeverity nSeverity, Component nComponent, const char* format, ... );
};

extern SystemLog systemLog;

#endif // SYSTEMLOG_H
