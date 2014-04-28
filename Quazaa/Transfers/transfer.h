#ifndef TRANSFER_H
#define TRANSFER_H

#include "networkconnection.h"

class CTransfer : public NetworkConnection
{
	Q_OBJECT

public:
	void* m_pOwner;
public:
	CTransfer(void* pOwner, QObject *parent = 0);
	virtual ~CTransfer();

	virtual void onTimer(quint32 tNow = 0);
signals:

public slots:

};

#endif // TRANSFER_H
