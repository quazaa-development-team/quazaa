/*
** $Id: timedsignalqueue.cpp 868 2012-01-16 14:39:44Z brov $
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

#include <QDebug>

#include "types.h"
#include "Misc/timedsignalqueue.h"

#include "debug_new.h"

TimedSignalQueue signalQueue;

/* ---------------------------------------------------------------------------------------------- */
/* ---------------------------------------- CTimerObject ---------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */

TimerObject::TimerObject( QObject* obj, const char* member, quint64 tIntervalMs, bool bMultiShot,
						  QGenericArgument val0, QGenericArgument val1,
						  QGenericArgument val2, QGenericArgument val3,
						  QGenericArgument val4, QGenericArgument val5,
						  QGenericArgument val6, QGenericArgument val7,
						  QGenericArgument val8, QGenericArgument val9 ) :
	m_tInterval( tIntervalMs ),
	m_bMultiShot( bMultiShot )
{
	resetTime();

	m_sSignal.obj = obj;

	// Copy the c-string we have been given.
	const size_t nLength = strlen( member );
	char* sTmp = new char[nLength + 1];	// We need 1 extra char for the terminal \0.

	// This is supposed to be safe as eventual changes to pTimerObject->
	// m_sSignal.sName should originate from the same thread.
#ifdef _MSC_VER
	strcpy_s( sTmp, nLength + 1, member );
#else
	strcpy( sTmp, member );
#endif

	m_sSignal.sName = sTmp;

	m_sSignal.val0 = val0;
	m_sSignal.val1 = val1;
	m_sSignal.val2 = val2;
	m_sSignal.val3 = val3;
	m_sSignal.val4 = val4;
	m_sSignal.val5 = val5;
	m_sSignal.val6 = val6;
	m_sSignal.val7 = val7;
	m_sSignal.val8 = val8;
	m_sSignal.val9 = val9;

	m_oUUID = QUuid::createUuid();
}

TimerObject::TimerObject( QObject* obj, const char* member, quint32 tDelaySec,
						  QGenericArgument val0, QGenericArgument val1,
						  QGenericArgument val2, QGenericArgument val3,
						  QGenericArgument val4, QGenericArgument val5,
						  QGenericArgument val6, QGenericArgument val7,
						  QGenericArgument val8, QGenericArgument val9 ) :
	m_tInterval( 0 ),
	m_bMultiShot( false )
{
	quint64 tDelay64 = tDelaySec;
	tDelay64 *= 1000; // transform to Milliseconds

	m_tTime = tDelay64 + signalQueue.getRelativeTimeInMs();

#ifdef _DEBUG
#if ENABLE_SIGNAL_QUEUE_DEBUGGING
	qint64 tTest = ( m_tTime + signalQueue.m_tTimerStartUTCInMSec ) / 1000 - common::getTNowUTC();
	systemLog.postLog( LogSeverity::Debug, Component::SignalQueue,
					   QString( "Added event with %1s delay to signal queue."
							  ).arg( QString::number( tTest ) ) );
#endif // ENABLE_SIGNAL_QUEUE_DEBUGGING
#endif // _DEBUG

	m_sSignal.obj = obj;

	// Copy the c-string we have been given.
	const size_t nLength = strlen( member );
	char* sTmp = new char[nLength + 1];	// We need 1 extra char for the terminal \0.

	// This is supposed to be safe as eventual changes to pTimerObject->
	// m_sSignal.sName should originate from the same thread.
#ifdef _MSC_VER
	strcpy_s( sTmp, nLength + 1, member );
#else
	strcpy( sTmp, member );
#endif

	m_sSignal.sName = sTmp;

	m_sSignal.val0 = val0;
	m_sSignal.val1 = val1;
	m_sSignal.val2 = val2;
	m_sSignal.val3 = val3;
	m_sSignal.val4 = val4;
	m_sSignal.val5 = val5;
	m_sSignal.val6 = val6;
	m_sSignal.val7 = val7;
	m_sSignal.val8 = val8;
	m_sSignal.val9 = val9;

	m_oUUID = QUuid::createUuid();
}

TimerObject::TimerObject( const TimerObject* const pTimerObject )
{
	m_tTime			= pTimerObject->m_tTime;
	m_tInterval		= pTimerObject->m_tInterval;
	m_bMultiShot	= pTimerObject->m_bMultiShot;

	m_oUUID			= pTimerObject->m_oUUID;

	m_sSignal.obj	= pTimerObject->m_sSignal.obj;

	// Copy the c-string from pTimerObject
	const size_t nLength = strlen( pTimerObject->m_sSignal.sName );
	char* sTmp = new char[nLength + 1];				// We need 1 extra char for the terminal 0.

	// This is supposed to be safe as eventual changes to pTimerObject->
	// m_sSignal.sName should originate from the same thread.
#ifdef _MSC_VER
	strcpy_s( sTmp, nLength + 1, pTimerObject->m_sSignal.sName );
#else
	strcpy( sTmp, pTimerObject->m_sSignal.sName );
#endif

	m_sSignal.sName = sTmp;

	m_sSignal.val0	= pTimerObject->m_sSignal.val0;
	m_sSignal.val1	= pTimerObject->m_sSignal.val1;
	m_sSignal.val2	= pTimerObject->m_sSignal.val2;
	m_sSignal.val3	= pTimerObject->m_sSignal.val3;
	m_sSignal.val4	= pTimerObject->m_sSignal.val4;
	m_sSignal.val5	= pTimerObject->m_sSignal.val5;
	m_sSignal.val6	= pTimerObject->m_sSignal.val6;
	m_sSignal.val7	= pTimerObject->m_sSignal.val7;
	m_sSignal.val8	= pTimerObject->m_sSignal.val8;
	m_sSignal.val9	= pTimerObject->m_sSignal.val9;
}

TimerObject::~TimerObject()
{
	delete[] m_sSignal.sName;
}

void TimerObject::resetTime()
{
	m_tTime = TimedSignalQueue::getRelativeTimeInMs() + m_tInterval;
}

bool TimerObject::emitSignal() const
{
#if ENABLE_SIGNAL_QUEUE_DEBUGGING
	systemLog.postLog( LogSeverity::Debug, Component::SignalQueue,
					   QString( "Invoking method:" ) + m_sSignal.sName );
#endif

	return QMetaObject::invokeMethod( m_sSignal.obj, m_sSignal.sName, Qt::QueuedConnection,
									  m_sSignal.val0, m_sSignal.val1, m_sSignal.val2,
									  m_sSignal.val3, m_sSignal.val4, m_sSignal.val5,
									  m_sSignal.val6, m_sSignal.val7, m_sSignal.val8,
									  m_sSignal.val9 );
}


/* ---------------------------------------------------------------------------------------------- */
/* ------------------------------------- CTimedSignalQueue -------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */

QElapsedTimer TimedSignalQueue::m_oElapsedTime;
#ifdef _DEBUG
quint64       TimedSignalQueue::m_tTimerStartUTCInMSec = 0;
#endif

TimedSignalQueue::TimedSignalQueue( QObject* parent ) :
	QObject( parent ),
	m_nPrecision( 1000 )
{
	// Make sure the timer is invalid the first time getRelativeTimeInMs() is called.
	m_oElapsedTime.invalidate();
}

TimedSignalQueue::~TimedSignalQueue()
{
	// If the deletion needs to be delayed for some reason, hold m_pShutdownLock.
	m_pShutdownLock.lock();
	stop(); // Prevent timer events from occurring during cleanup process.
	clear();
	m_pShutdownLock.unlock();
}

void TimedSignalQueue::shutdownLock()
{
	m_pShutdownLock.lock();
}

void TimedSignalQueue::shutdownUnlock()
{
	m_pShutdownLock.unlock();
}

void TimedSignalQueue::setup()
{
	QMutexLocker l( &m_pSection );

	Q_ASSERT( !m_oElapsedTime.isValid() );

	// start timer for timer events
	m_oTimer.start( m_nPrecision, this );
	m_oElapsedTime.start();

#ifdef _DEBUG
	m_tTimerStartUTCInMSec = ( quint64 )( common::getTNowUTC() ) * 1000;
#endif

	// 32 bit value with possible overflow
	if ( m_oElapsedTime.clockType() == QElapsedTimer::TickCounter )
	{
		// use the signal queue's own mechinism to reset the timer each time 2^31 is surpassed
		push( this, "restartTimer", 2147483650, true );
	}
}

void TimedSignalQueue::stop()
{
	QMutexLocker l( &m_pSection );
	m_oTimer.stop();
	m_oElapsedTime.invalidate();
}

void TimedSignalQueue::clear()
{
	QMutexLocker l( &m_pSection );

	for ( SignalQueueIterator i = m_QueuedSignals.begin(); i != m_QueuedSignals.end(); )
	{
		delete i.value();
		i = m_QueuedSignals.erase( i );
	}
}

void TimedSignalQueue::setPrecision( quint64 tInterval )
{
	if ( tInterval )
	{
		QMutexLocker l( &m_pSection );

		m_nPrecision = tInterval;
		m_oTimer.start( m_nPrecision, this );
	}
}

void TimedSignalQueue::timerEvent( QTimerEvent* event )
{
	if ( event->timerId() == m_oTimer.timerId() )
	{
		checkSchedule();
	}
	else
	{
		QObject::timerEvent( event );
	}
}

void TimedSignalQueue::checkSchedule()
{
	// don't block too long waiting for a lock...
	if ( m_pSection.tryLock( m_nPrecision / 2 ) )
	{
		const quint64 tRelativeTimeMs = getRelativeTimeInMs();

		for ( SignalQueueIterator i = m_QueuedSignals.begin(); i != m_QueuedSignals.end(); )
		{
			if ( i.key() > tRelativeTimeMs )
			{
				break;
			}

			TimerObject* pObj = i.value();
			i = m_QueuedSignals.erase( i );

			bool bSuccess = pObj->emitSignal();

#if ENABLE_SIGNAL_QUEUE_DEBUGGING
			systemLog.postLog( LogSeverity::Debug, Component::SignalQueue,
							   QString( "Success: " ) + QString( bSuccess ? "true" : "false" ) );
#endif //ENABLE_SIGNAL_QUEUE_DEBUGGING

			if ( bSuccess )
			{
				if ( pObj->m_bMultiShot )
				{
					pObj->resetTime();
					m_QueuedSignals.insert( pObj->m_tTime, pObj );
				}
			}
			else
			{
#if ENABLE_SIGNAL_QUEUE_DEBUGGING
				qDebug() << "Error in CTimedSignalQueue::checkSchedule(): Unable invoke method! "
						 << pObj->m_sSignal.sName;
#endif //ENABLE_SIGNAL_QUEUE_DEBUGGING
				delete pObj;
			}
		}

		m_pSection.unlock();
	}
	else
	{
		qDebug() << "CTimedSignalQueue::checkSchedule(): Unable to aquire a lock. Skipping check.";
	}
}

QUuid TimedSignalQueue::push( QObject* parent, const char* signal,
							  quint64 tIntervalMs, bool bMultiShot,
							  QGenericArgument val0, QGenericArgument val1,
							  QGenericArgument val2, QGenericArgument val3,
							  QGenericArgument val4, QGenericArgument val5,
							  QGenericArgument val6, QGenericArgument val7,
							  QGenericArgument val8, QGenericArgument val9 )
{
	return push( new TimerObject( parent, signal, tIntervalMs, bMultiShot,
								  val0, val1, val2, val3, val4, val5, val6, val7, val8, val9 ) );
}

QUuid TimedSignalQueue::push( QObject* parent, const char* signal, quint32 tDelaySec,
							  QGenericArgument val0, QGenericArgument val1,
							  QGenericArgument val2, QGenericArgument val3,
							  QGenericArgument val4, QGenericArgument val5,
							  QGenericArgument val6, QGenericArgument val7,
							  QGenericArgument val8, QGenericArgument val9 )
{
	return push( new TimerObject( parent, signal, tDelaySec,
								  val0, val1, val2, val3, val4, val5, val6, val7, val8, val9 ) );
}

QUuid TimedSignalQueue::push( TimerObject* pTimedSignal )
{
	QMutexLocker l( &m_pSection );

	m_QueuedSignals.insert( pTimedSignal->m_tTime, pTimedSignal );

	return pTimedSignal->m_oUUID;
}

bool TimedSignalQueue::pop( const QObject* parent, const char* signal )
{
	if ( !parent )
	{
		return false;
	}

	bool bFound = false;
	QString sSignalName = "";

	if ( signal )
	{
		sSignalName = signal;
	}

	QMutexLocker l( &m_pSection );

	for ( SignalQueueIterator iQueue = m_QueuedSignals.begin(); iQueue != m_QueuedSignals.end(); )
	{
		if ( iQueue.value()->m_sSignal.obj == parent &&
			 ( !signal || sSignalName == iQueue.value()->m_sSignal.sName ) )
		{
			delete iQueue.value();
			iQueue = m_QueuedSignals.erase( iQueue );
			bFound = true;
		}
		else
		{
			++iQueue;
		}
	}

	return bFound;
}

bool TimedSignalQueue::pop( QUuid oTimer_ID )
{
	QMutexLocker l( &m_pSection );

	TimerObject* pTimer = popInternal( oTimer_ID );

	if ( pTimer )
	{
		delete pTimer;
		return true;
	}

	return false;
}

bool TimedSignalQueue::setInterval( QUuid oTimer_ID, quint64 tInterval )
{
	m_pSection.lock();

	TimerObject* pTimer = popInternal( oTimer_ID );

	if ( pTimer )
	{
		pTimer->m_tInterval = tInterval;
		pTimer->resetTime();

		m_pSection.unlock();

		push( pTimer );
		return true;
	}

	m_pSection.unlock();
	return false;
}

void TimedSignalQueue::restartTimer()
{
	TimerObject* pTimer;
	quint64 tRelative = m_oElapsedTime.restart();

	for ( SignalQueueIterator iQueue = m_QueuedSignals.begin(); iQueue != m_QueuedSignals.end();
		  ++iQueue )
	{
		pTimer = iQueue.value();
		pTimer->m_tTime -= tRelative;
	}
}

TimerObject* TimedSignalQueue::popInternal( QUuid oTimer_ID )
{
	TimerObject* pTimer;

	for ( SignalQueueIterator iQueue = m_QueuedSignals.begin(); iQueue != m_QueuedSignals.end(); )
	{
		pTimer = iQueue.value();
		if ( pTimer->m_oUUID == oTimer_ID )
		{
			m_QueuedSignals.erase( iQueue );
			return pTimer;
		}
		else
		{
			++iQueue;
		}
	}

	return NULL;
}
