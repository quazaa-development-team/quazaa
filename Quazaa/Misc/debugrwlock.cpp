#include <QDebug>
#include <QString>

#include "debugrwlock.h"

CDebugRWLock::CDebugRWLock( RecursionMode recursionMode ) :
	QReadWriteLock( recursionMode ),
	nLockID( nLockIDCounter++ )
{
	qDebug() << "Constructed new RWLock. ID #" << QString::number( nLockID ).toLocal8Bit().data();
}

CDebugRWLock::~CDebugRWLock()
{
}

void	CDebugRWLock::lockForRead( const char* caller )
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": read lock requested" <<
			 ( caller ? "    - caller: " : "" ) << ( caller ? caller : "" );
	QReadWriteLock::lockForRead();
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": read lock obtained " <<
			 ( caller ? "    - caller: " : "" ) << ( caller ? caller : "" );
}

void	CDebugRWLock::lockForWrite( const char* caller )
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": write lock requested" <<
			 ( caller ? "   - caller: " : "" ) << ( caller ? caller : "" );
	QReadWriteLock::lockForWrite();
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": write lock obtained " <<
			 ( caller ? "   - caller: " : "" ) << ( caller ? caller : "" );
}

bool	CDebugRWLock::tryLockForRead( const char* caller )
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": try read lock       " <<
			 ( caller ? "   - caller: " : "" ) << ( caller ? caller : "" );
	bool	bOK = QReadWriteLock::tryLockForRead();
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": read lock obtained  " <<
			 ( caller ? "   - caller: " : "" ) << ( caller ? caller : "" );

	return bOK;
}

bool	CDebugRWLock::tryLockForRead( int timeout )
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": try read lock - timeout: " <<
			 QString::number( timeout ).toLocal8Bit().data();
	bool	bOK = QReadWriteLock::tryLockForRead( timeout );
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": read lock obtained";

	return bOK;
}

bool	CDebugRWLock::tryLockForWrite( const char* caller )
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": try write lock      " <<
			 ( caller ? "   - caller: " : "" ) << ( caller ? caller : "" );
	bool	bOK = QReadWriteLock::tryLockForWrite();
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": write lock obtained " <<
			 ( caller ? "   - caller: " : "" ) << ( caller ? caller : "" );

	return bOK;
}

bool	CDebugRWLock::tryLockForWrite( int timeout )
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": try read lock - timeout: " <<
			 QString::number( timeout ).toLocal8Bit().data();
	bool	bOK = QReadWriteLock::tryLockForWrite( timeout );
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": read lock obtained";

	return bOK;
}

void	CDebugRWLock::unlock()
{
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": unlocking requested";
	QReadWriteLock::unlock();
	qDebug() << "RWLock ID #" << QString::number( nLockID ).toLocal8Bit().data() << ": *** unlocking done ***";
}

quint32 CDebugRWLock::nLockIDCounter = 0;
