#ifndef TRANSFER_H
#define TRANSFER_H

#include "networkconnection.h"

class Transfer : public NetworkConnection
{
	Q_OBJECT

public:
	void* m_pOwner;
public:
	Transfer( void* pOwner, QObject* parent = 0 );
	virtual ~Transfer();

	virtual void onTimer( quint32 tNow = 0 );
signals:

public slots:

};

#endif // TRANSFER_H
