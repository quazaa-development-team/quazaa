#ifndef TRANSFERS_H
#define TRANSFERS_H

#include "types.h"
#include <QObject>
#include <QMutex>
#include <QMultiHash>
#include <QTimer>

#include "thread.h"

class Transfer;
class RateController;

class Transfers : public QObject
{
	Q_OBJECT
public:
	QMutex m_pSection;
	bool   m_bActive;
	RateController* m_pController;
	QTimer m_oTimer;

protected:
	QMultiHash<void*, Transfer*> m_lTransfers;

public:
	Transfers( QObject* parent = 0 );
	~Transfers();
	void start();
	void stop();

	void add( Transfer* pTransfer );
	void remove( Transfer* pTransfer );

	QList<Transfer*> getByOwner( void* pOwner );
signals:

public slots:
	void onTimer();
};

extern Transfers transfers;
extern CThread transfersThread;
#endif // TRANSFERS_H
