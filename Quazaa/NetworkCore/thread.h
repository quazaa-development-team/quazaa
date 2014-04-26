/*
** thread.h
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

#ifndef THREAD_H
#define THREAD_H

#include "types.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

class CThread: public QThread
{
	Q_OBJECT

	QMutex*         m_pMutex;
	QWaitCondition  m_oStartCond;
	QObject*        m_pTargetObject;
	QString			m_sThreadName;
public:
	CThread(QObject* parent = 0);

	void start(QString strName, QMutex* pMutex, QObject* pTargetObj = 0,
			   Priority p = InheritPriority)
	{
		m_sThreadName = strName;

		systemLog.postLog( LogSeverity::Debug, QString( "%1 Thread::start" ).arg( strName ) );
		//qDebug() << strName << "Thread::start";
		//QMutexLocker l(pMutex);
		m_pMutex = pMutex;
		m_pTargetObject = pTargetObj;
		if ( pTargetObj )
		{
			pTargetObj->moveToThread( this );
		}
		systemLog.postLog(LogSeverity::Debug, QString("%1 Starting...").arg(strName));
		//qDebug() << strName << "Starting...";
		QThread::start(p);
		systemLog.postLog(LogSeverity::Debug, QString("%1 Waiting for thread to start...").arg(strName));
		//qDebug() << strName << "Waiting for thread to start...";
		if(!isRunning())
		{
			m_oStartCond.wait(m_pMutex);
		}
		systemLog.postLog(LogSeverity::Debug, QString("%1 Thread started").arg(strName));
		//qDebug() << strName << "Thread started";
	}

	void exit(int retcode)
	{
		//QMutexLocker l(m_pMutex);
		systemLog.postLog(LogSeverity::Debug, QString("%1 Exiting thread").arg(m_sThreadName));
		//qDebug() << m_sThreadName << "Exiting thread";
		QThread::exit(retcode);
		systemLog.postLog(LogSeverity::Debug, QString("%1 Waiting for thread to finish...").arg(m_sThreadName));
		//qDebug() << m_sThreadName << "Waiting for thread to finish...";

		if(isRunning())
		{
			m_oStartCond.wait(m_pMutex);
		}
		//wait();
		systemLog.postLog(LogSeverity::Debug, QString("%1 Thread Finished").arg(m_sThreadName));
	}

protected:
	void run()
	{
		QMutexLocker l(m_pMutex);
		msleep(50);

		if(m_pTargetObject)
		{
			if(!QMetaObject::invokeMethod(m_pTargetObject, "setupThread", Qt::DirectConnection))
			{
				systemLog.postLog(LogSeverity::Warning, "Failed to call target's setupThread method");
				//qWarning() << "Failed to call target's setupThread method";
			}
		}

		m_oStartCond.wakeAll();

		l.unlock();

		qDebug() << m_sThreadName << ": thread started: " << this << currentThreadId();

		exec();

		qDebug() << m_sThreadName << "Starting cleanup.";
		l.relock();

		if(m_pTargetObject)
		{
			if(!QMetaObject::invokeMethod(m_pTargetObject, "cleanupThread", Qt::DirectConnection))
			{
				systemLog.postLog(LogSeverity::Warning, "Failed to call target's cleanupThread method");
				//qWarning() << "Failed to call target's cleanupThread method";
			}
		}

		//msleep(50);
		m_oStartCond.wakeAll();
	}
};


#endif // THREAD_H
