#include "downloadtransfer.h"
#include "downloadsource.h"

#include "quazaasettings.h"

CDownloadTransfer::CDownloadTransfer(CDownload *pOwner, CDownloadSource *pSource, QObject *parent) :
	CTransfer(pOwner, parent),
	m_pSource(pSource),
	m_nState(dtsNull),
	m_tLastResponse(0),
	m_nQueuePos(0),
	m_nQueueLength(0)
{
}

CDownloadTransfer::~CDownloadTransfer()
{
}

void CDownloadTransfer::onTimer(quint32 tNow)
{
	if( tNow == 0 )
		tNow = time(0);

	switch(m_nState)
	{
		case CDownloadTransfer::dtsConnecting:
		case CDownloadTransfer::dtsRequesting:
		case CDownloadTransfer::dtsResponse:
			if( tNow - m_tConnected > quazaaSettings.Connection.TimeoutConnect )
			{
				systemLog.postLog(LogSeverity::Error, QString(tr("Timed out connecting to download host %1.")).arg(m_pSource->m_oAddress.toStringWithPort()));
				close();
			}
			break;
		case CDownloadTransfer::dtsDownloading:
			if( tNow - m_tConnected > quazaaSettings.Connection.TimeoutTraffic )
			{
				systemLog.postLog(LogSeverity::Error, QString(tr("Closing download connection to %1 due to lack of traffic.")).arg(m_pSource->m_oAddress.toStringWithPort()));
				close();
			}
			break;
		default:
			break;
	}
}

void CDownloadTransfer::requestBlock(Fragments::Fragment oFragment)
{
	m_lRequested.push_back(oFragment);
}

void CDownloadTransfer::subtractRequested(Fragments::List &oFragments)
{
	if( m_lRequested.empty() )
		return;

	oFragments.erase(m_lRequested.begin(), m_lRequested.end());
}
