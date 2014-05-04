#ifndef BANNEDDISCOVERYSERVICE_H
#define BANNEDDISCOVERYSERVICE_H

#include "discoveryservice.h"

namespace Discovery
{

class BannedDiscoveryService : public DiscoveryService
{
	Q_OBJECT

public:
	BannedDiscoveryService( const QUrl& oURL, const NetworkType& oNType, quint8 nRating );

	~BannedDiscoveryService();

	inline QString type() const
	{
		return QString( "Banned" );
	}

private:
	void doQuery()  throw();
	void doUpdate() throw();
	void doCancelRequest() throw();
};

}

#endif // BANNEDDISCOVERYSERVICE_H
