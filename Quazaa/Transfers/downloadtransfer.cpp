#include "downloadtransfer.h"
#include "downloadsource.h"

#include "quazaasettings.h"

DownloadTransfer::DownloadTransfer( Download* pOwner, DownloadSource* pSource, QObject* parent ) :
	Transfer( pOwner, parent ),
	m_pSource( pSource ),
	m_nState( dtsNull ),
	m_tLastResponse( 0 ),
	m_nQueuePos( 0 ),
	m_nQueueLength( 0 )
{
}

DownloadTransfer::~DownloadTransfer()
{
}

void DownloadTransfer::onTimer( quint32 tNow )
{
	if ( tNow == 0 )
	{
		tNow = time( 0 );
	}

	switch ( m_nState )
	{
	case DownloadTransfer::dtsConnecting:
	case DownloadTransfer::dtsRequesting:
	case DownloadTransfer::dtsResponse:
		if ( tNow - m_tConnected > quazaaSettings.Connection.TimeoutConnect )
		{
			systemLog.postLog( LogSeverity::Error,
							   QString( tr( "Timed out connecting to download host %1." ) ).arg( m_pSource->m_oAddress.toStringWithPort() ) );
			close();
		}
		break;
	case DownloadTransfer::dtsDownloading:
		if ( tNow - m_tConnected > quazaaSettings.Connection.TimeoutTraffic )
		{
			systemLog.postLog( LogSeverity::Error,
							   QString( tr( "Closing download connection to %1 due to lack of traffic." ) ).arg(
								   m_pSource->m_oAddress.toStringWithPort() ) );
			close();
		}
		break;
	default:
		break;
	}
}

void DownloadTransfer::requestBlock( Fragments::Fragment oFragment )
{
	m_lRequested.push_back( oFragment );
}

void DownloadTransfer::subtractRequested( Fragments::List& oFragments )
{
	if ( m_lRequested.empty() )
	{
		return;
	}

	oFragments.erase( m_lRequested.begin(), m_lRequested.end() );
}
