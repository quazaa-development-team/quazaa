#ifndef TRANSFERS_H
#define TRANSFERS_H

#include "types.h"
#include <QObject>
#include <QMutex>
#include <QMultiHash>
#include <QTimer>

#include "thread.h"

class CTransfer;
class RateController;

class CTransfers : public QObject
{
	Q_OBJECT
public:
	QMutex m_pSection;
	bool   m_bActive;
	RateController* m_pController;
	QTimer m_oTimer;

protected:
	QMultiHash<void*, CTransfer*> m_lTransfers;

public:
	CTransfers(QObject* parent = 0);
	~CTransfers();
	void start();
	void stop();

	void add(CTransfer* pTransfer);
	void remove(CTransfer* pTransfer);

	QList<CTransfer*> getByOwner(void* pOwner);
signals:

public slots:
	void onTimer();
};

extern CTransfers Transfers;
extern CThread TransfersThread;
#endif // TRANSFERS_H
