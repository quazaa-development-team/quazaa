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

#include "systemlog.h"
#include <QMetaType>
#include <QtCore>

#include "debug_new.h"

SystemLog systemLog;

#ifndef QUAZAA_SETUP_UNIT_TESTS
SystemLog::SystemLog() :
	m_pSection( QMutex::Recursive )
{
	m_pComponents = new QString[Component::NoComponents];
	m_bProcessingMessage = false;

	qRegisterMetaType<LogSeverity>( "LogSeverity" );
	qRegisterMetaType<Component>( "Component" );
}

SystemLog::~SystemLog()
{
	delete[] m_pComponents;
}
#endif

void SystemLog::start()
{
	m_pComponents[ 0] = QString();
	m_pComponents[ 1] = tr( "[Chat] "       );
	m_pComponents[ 2] = tr( "[IRC] "        );
	m_pComponents[ 3] = tr( "[Discovery] "  );
	m_pComponents[ 4] = tr( "[Network] "    );
	m_pComponents[ 5] = tr( "[Ares] "       );
	m_pComponents[ 6] = tr( "[BitTorrent] " );
	m_pComponents[ 7] = tr( "[eD2k] "       );
	m_pComponents[ 8] = tr( "[G2] "         );
	m_pComponents[ 9] = tr( "[Security] "   );
	m_pComponents[10] = tr( "[Library] "    );
	m_pComponents[11] = tr( "[Downloads] "  );
	m_pComponents[12] = tr( "[Uploads] "    );
	m_pComponents[13] = tr( "[GUI] "        );
	m_pComponents[14] = tr( "[SignalQueue] " );
	m_pComponents[15] = tr( "[HostCache] "  );
}

QString SystemLog::msgFromComponent( Component nComponent )
{
	return m_pComponents[static_cast<quint8>( nComponent )];
}

void SystemLog::postLog( LogSeverity nSeverity, const QString& sMessage )
{
	postLog( nSeverity, Component::None, sMessage );
}

#ifndef QUAZAA_SETUP_UNIT_TESTS
void SystemLog::postLog( LogSeverity nSeverity, Component nComponent,
						 const QString& sMessage )
{
	QMutexLocker locker( &m_pSection );

	static LogSeverity lastSeverity = LogSeverity::Information;
	static Component lastComponent  = Component::None;
	static QString lastMessage;
	static int suppressed = 0;
	static bool bCheck = true;

	if ( bCheck )
	{
		if ( nSeverity == lastSeverity && nComponent == lastComponent && sMessage == lastMessage )
		{
			++suppressed;
			return;
		}
		else
		{
			if ( suppressed > 0 )
			{
				bCheck = false;
				postLog( lastSeverity, lastComponent,
						 tr( "Suppressed %n identical message(s).", 0, suppressed ) );
				bCheck = true;
			}
			lastMessage   = sMessage;
			lastSeverity  = nSeverity;
			lastComponent = nComponent;
			suppressed = 0;
		}
	}

	const QString sComponentMessage = msgFromComponent( nComponent ) + sMessage;

	/*switch ( nSeverity )
	{
		case LogSeverity::Debug:
		case LogSeverity::Warning:
		case LogSeverity::Critical:
		case LogSeverity::Error:
			qDebug() << qPrintable( sComponentMessage );
			break;
		default:
			break;
	}*/

	qDebug() << qPrintable( sComponentMessage );

	emit logPosted( sComponentMessage, nSeverity );
}

void SystemLog::postLog( LogSeverity nSeverity, Component nComponent,
						 const char* format, ... )
{
	va_list argList;
	va_start( argList, format );
	QString message = QString().vsprintf( format, argList );
	postLog( nSeverity, nComponent, message );
	va_end( argList );
}

#else
SystemLog::SystemLog()
{
	m_pComponents = new QString[Component::NoComponents];
}

SystemLog::~SystemLog()
{
	delete[] m_pComponents;
}

void SystemLog::postLog( LogSeverity, Component nComponent,
						 const QString& sMessage )
{
	qDebug() << ( msgFromComponent( nComponent ) + sMessage ).toLocal8Bit().data();
}

void SystemLog::postLog( LogSeverity, Component, const char*, ... )
{
}
#endif

