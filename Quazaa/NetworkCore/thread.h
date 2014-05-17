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

// REMOVE most of the try lock asserts for alpha1
class CThread: public QThread
{
	Q_OBJECT

	QMutex*         m_pMutex;
	QWaitCondition  m_oWaitCond;
	QObject*        m_pTargetObject;
	QString			m_sThreadName;
public:
	CThread( QObject* parent = 0 );

	/**
	 * @brief start      Starts this thread.
	 * @param sName      The name to be used for this thread.
	 * @param pMutex     The thread mutex. Note that this is expected to be locked.
	 * @param pTargetObj A QQbject* that has a setupThread() and cleanupThread() slot.
	 * @param p          The thread execution priority.
	 */
	void start( QString sName, QMutex* pMutex, QObject* pTargetObj = NULL,
				Priority p = InheritPriority )
	{
		Q_ASSERT( !pMutex->tryLock() );

		m_sThreadName = sName;
		QThread::setObjectName( sName );

		systemLog.postLog( LogSeverity::Debug, QString( "%1 Thread::start" ).arg( sName ) );

		m_pMutex = pMutex;
		m_pTargetObject = pTargetObj;
		if ( pTargetObj )
		{
			pTargetObj->moveToThread( this );
		}

		systemLog.postLog( LogSeverity::Debug, QString( "%1 Starting..." ).arg( sName ) );

		QThread::start( p );
		systemLog.postLog( LogSeverity::Debug, QString( "%1 Waiting for thread to start..."
													  ).arg( sName ) );

		if ( !isRunning() )
		{
			// m_pMutex MUST be locked when calling m_oWaitCond.wait( m_pMutex );
			Q_ASSERT( !m_pMutex->tryLock() );
			m_oWaitCond.wait( m_pMutex );
		}
		systemLog.postLog( LogSeverity::Debug, QString( "%1 Thread started" ).arg( sName ) );
	}

	/**
	 * @brief exit      Exits the thread.
	 * @param retcode   The return value for exec().
	 * @param bNeedLock If this is true, the thread mutex is locked (and released) within this
	 *                  method; else the mutex is expected to be locked on entering this method.
	 */
	void exit( int retcode, bool bNeedLock = false )
	{
		// Note: we cannot use the system log here as the thread in question might finish after the
		// systemLog object has already been destroyed.

		qDebug() << m_sThreadName << "exit(): Exiting thread";

		QThread::exit( retcode );

		qDebug() << m_sThreadName << "exit(): Waiting for thread to finish...";

		if ( bNeedLock )
		{
			m_pMutex->lock();
		}
		ASSUME_LOCK( *m_pMutex );

		if ( isRunning() )
		{
			// m_pMutex MUST be locked when calling m_oWaitCond.wait( m_pMutex );
			Q_ASSERT( !m_pMutex->tryLock() );
			m_oWaitCond.wait( m_pMutex );
		}

		if ( bNeedLock )
		{
			m_pMutex->unlock();
		}

		qDebug() << m_sThreadName << "exit(): Thread finished.";
	}

protected:
	void run()
	{
		m_pMutex->lock();
		if ( m_pTargetObject )
		{
			if ( !QMetaObject::invokeMethod( m_pTargetObject, "setupThread",
											 Qt::DirectConnection ) )
			{
				systemLog.postLog( LogSeverity::Warning,
								   "Failed to call target's setupThread method" );
				//qWarning() << "Failed to call target's setupThread method";
			}
		}

		Q_ASSERT( !m_pMutex->tryLock() );
		m_oWaitCond.wakeAll();
		Q_ASSERT( !m_pMutex->tryLock() );

		m_pMutex->unlock();

		qDebug() << m_sThreadName << ": thread started: " << this << currentThreadId();

		exec();

		// Note: we cannot use the system log after this point as the thread in question might
		// finish after the systemLog object has already been destroyed.

		qDebug() << m_sThreadName << "run(): Starting cleanup.";

		m_pMutex->lock();
		if ( m_pTargetObject )
		{
			qDebug() << m_sThreadName << "run(): Calling cleanupThread().";
			if ( !QMetaObject::invokeMethod( m_pTargetObject, "cleanupThread",
											 Qt::DirectConnection ) )
			{
				qWarning() << "Failed to call target's cleanupThread method";
			}
		}

		qDebug() << m_sThreadName << "run(): Finished cleanup";

		Q_ASSERT( !m_pMutex->tryLock() );
		m_oWaitCond.wakeAll();
		Q_ASSERT( !m_pMutex->tryLock() );

		m_pMutex->unlock();
	}
};


#endif // THREAD_H
