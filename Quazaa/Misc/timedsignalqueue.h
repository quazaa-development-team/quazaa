/*
** $Id: timedsignalqueue.h $
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

#ifndef TIMEDSIGNALQUEUE_H
#define TIMEDSIGNALQUEUE_H

#include <QElapsedTimer>
#include <QBasicTimer>
#include <QTimerEvent>
#include <QDateTime>
#include <QMultiMap>
#include <QMutex>
#include <QUuid>

#ifdef _DEBUG
#include <QDebug>
#endif

#include "commonfunctions.h"

#define ENABLE_SIGNAL_QUEUE_DEBUGGING 0

class TimedSignalQueue;

/* ---------------------------------------------------------------------------------------------- */
/* ---------------------------------------- CTimerObject ---------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */
/**
 * @brief The CTimerObject class is a helper class that allows to store signals and related relevant
 * information, as well as to emit the signals at a given time.
 */
class TimerObject
{
private:
	struct
	{
		QObject* obj;
		const char* sName;
		QGenericArgument val0;
		QGenericArgument val1;
		QGenericArgument val2;
		QGenericArgument val3;
		QGenericArgument val4;
		QGenericArgument val5;
		QGenericArgument val6;
		QGenericArgument val7;
		QGenericArgument val8;
		QGenericArgument val9;
	} m_sSignal; // String based Qt4 signal

	QUuid   m_oUUID;
	quint64 m_tTime;      // relative time in ms as provided by getRelativeTimeInMs()
	quint64 m_tInterval;  // repetition interval in ms
	bool    m_bMultiShot; // repeat after m_tInterval yes/no

private:
	TimerObject( QObject* obj, const char* member, quint64 tIntervalMs, bool bMultiShot,
				 QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
				 QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument(),
				 QGenericArgument val4 = QGenericArgument(), QGenericArgument val5 = QGenericArgument(),
				 QGenericArgument val6 = QGenericArgument(), QGenericArgument val7 = QGenericArgument(),
				 QGenericArgument val8 = QGenericArgument(), QGenericArgument val9 = QGenericArgument() );

	TimerObject( QObject* obj, const char* member, quint32 tDelaySec,
				 QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
				 QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument(),
				 QGenericArgument val4 = QGenericArgument(), QGenericArgument val5 = QGenericArgument(),
				 QGenericArgument val6 = QGenericArgument(), QGenericArgument val7 = QGenericArgument(),
				 QGenericArgument val8 = QGenericArgument(), QGenericArgument val9 = QGenericArgument() );

	TimerObject( const TimerObject* const pTimerObject );

	~TimerObject();

	void resetTime();
	bool emitSignal() const;

	friend class TimedSignalQueue;
};

/* ---------------------------------------------------------------------------------------------- */
/* -------------------------------------- CTimedSignalQueue ------------------------------------- */
/* ---------------------------------------------------------------------------------------------- */
class TimedSignalQueue : public QObject
{
	Q_OBJECT

private:
	// Key: The relative time as provided by getRelativeTimeInMs()
	typedef QMultiMap<quint64, TimerObject*>           TSignalQueue;
	typedef QMultiMap<quint64, TimerObject*>::iterator SignalQueueIterator;

	static QElapsedTimer m_oElapsedTime;         // the relative time since the timer was started
#ifdef _DEBUG
	static quint64       m_tTimerStartUTCInMSec; // ms since 1970-01-01T00:00:00 UTC (timer start)
#endif

	QBasicTimer			m_oTimer;                // for timer events
	QMutex				m_pSection;
	quint64				m_nPrecision;            // the interval in ms between two timer events
	TSignalQueue		m_QueuedSignals;         // the queued signals

	// Before deleting the timed signal queue object, this lock is aquired. If you have other global
	// objects depending on this component, you can aquire this lock to prevent deletion until
	// you're finished with whatever you're doing.
	QMutex				m_pShutdownLock;

public:
	explicit TimedSignalQueue( QObject* parent = NULL );
	~TimedSignalQueue();

	// Hold this lock to delay the deletion of the queue on application shutdown.
	void shutdownLock();
	void shutdownUnlock();

	// (Re)initializes internal structures. Note that this does NOT clear the queue.
	void setup();

	void stop();

	// Clears the queue and removes all scheduled items.
	void clear();

	// Sets the interval used by the queue to check for new signals to be scheduled. This defaults to 1000ms.
	void setPrecision( quint64 tInterval = 1000 );

protected:
	void timerEvent( QTimerEvent* event );

private:
	inline static quint64 getRelativeTimeInMs()
	{
		Q_ASSERT( m_oElapsedTime.isValid() );

#ifdef _DEBUG
#if ENABLE_SIGNAL_QUEUE_DEBUGGING
		quint64 tElapsed = m_oElapsedTime.elapsed();
		quint64 tStart   = m_tTimerStartUTCInMSec;
		qint64 tNowTheo  = tElapsed + tStart;
		qint64 tNow64    = common::getTNowUTC();
		tNow64 *= 1000;

		qDebug() << "tNowTheo - tNowReal64 = " << QString::number( tNowTheo - tNow64 );

		// max diff 1000ms
		//Q_ASSERT( tNowTheo - tNow64 <  1000 );
		//Q_ASSERT( tNowTheo - tNow64 > -1000 );
#endif
#endif

		return m_oElapsedTime.elapsed();
	}
	QUuid push( TimerObject* pTimedSignal );

public slots:
	// Allows to manually check for new scheduled items in the queue.
	void checkSchedule();

	// This schedules a signal or slot to be invoqued after an interval of tInterval milliseconds.
	// If multiShot is set to true, the slot will be invoqued in the given interval until the signal queue
	// recieves a pop() request for the signal or the given parent turns invalid.
	QUuid push( QObject* parent, const char* signal, quint64 tIntervalMs, bool multiShot,
				QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
				QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument(),
				QGenericArgument val4 = QGenericArgument(), QGenericArgument val5 = QGenericArgument(),
				QGenericArgument val6 = QGenericArgument(), QGenericArgument val7 = QGenericArgument(),
				QGenericArgument val8 = QGenericArgument(), QGenericArgument val9 = QGenericArgument() );

	// This schedules a signal to be invoqued once in tDelaySec seconds.
	QUuid push( QObject* parent, const char* signal, quint32 tDelaySec,
				QGenericArgument val0 = QGenericArgument(), QGenericArgument val1 = QGenericArgument(),
				QGenericArgument val2 = QGenericArgument(), QGenericArgument val3 = QGenericArgument(),
				QGenericArgument val4 = QGenericArgument(), QGenericArgument val5 = QGenericArgument(),
				QGenericArgument val6 = QGenericArgument(), QGenericArgument val7 = QGenericArgument(),
				QGenericArgument val8 = QGenericArgument(), QGenericArgument val9 = QGenericArgument() );

	// Removes all scheduled combinations of a given parent and signal/slot from the queue.
	// If no signal/slot is specified, all entries for the given parent are removed.
	bool pop( const QObject* parent, const char* signal = NULL );

	// Removes a scheduled parent + signal/slot combination by its UUID.
	bool pop( QUuid oTimer_ID );

	// Sets the interval time of a given parent + signal/slot combination to tInterval. After this call,
	// the next schedule time of that combination is in tInterval milliseconds, no matter the timing state
	// of the previously scheduled combination.
	bool setInterval( QUuid oTimer_ID, quint64 tInterval );

private slots:
	void restartTimer();

private:
	TimerObject* popInternal( QUuid oTimer_ID );

	friend class TimerObject;
};

extern TimedSignalQueue signalQueue;

#endif // TIMEDSIGNALQUEUE_H
