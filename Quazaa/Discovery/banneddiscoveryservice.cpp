#include "banneddiscoveryservice.h"

using namespace Discovery;

BannedDiscoveryService::BannedDiscoveryService( const QUrl& oURL, const NetworkType&, quint8 ) :
	DiscoveryService( oURL, NetworkType( DiscoveryProtocol::None ), 0 )
{
	m_bBanned      = true;
	m_nServiceType = ServiceType::Banned;
}

BannedDiscoveryService::~BannedDiscoveryService()
{
}

void BannedDiscoveryService::doQuery() throw()
{
	Q_ASSERT( false );
}

void BannedDiscoveryService::doUpdate() throw()
{
	Q_ASSERT( false );
}


void BannedDiscoveryService::doCancelRequest() throw()
{
	Q_ASSERT( false );
}
