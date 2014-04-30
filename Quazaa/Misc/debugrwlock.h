#ifndef DEBUGRWLOCK_H
#define DEBUGRWLOCK_H

#include <QReadWriteLock>

/**
 * @brief The CDebugRWLock class is designed to replace the QReadWriteLock class during lock debugging.
 */
class CDebugRWLock : public QReadWriteLock
{
private:
	static quint32 nLockIDCounter;
	quint32 nLockID;

public:
	CDebugRWLock( RecursionMode recursionMode = NonRecursive );
	~CDebugRWLock();

	void	lockForRead( const char* caller = NULL );
	void	lockForWrite( const char* caller = NULL );
	bool	tryLockForRead( const char* caller = NULL );
	bool	tryLockForRead( int timeout );
	bool	tryLockForWrite( const char* caller = NULL );
	bool	tryLockForWrite( int timeout );
	void	unlock();
};

#endif // DEBUGRWLOCK_H
