#ifndef DOWNLOADTRANSFER_H
#define DOWNLOADTRANSFER_H

#include "transfer.h"
#include "FileFragments.hpp"

class Download;
class DownloadSource;

class DownloadTransfer : public Transfer
{
	Q_OBJECT
public:
	enum DownloadTransferState
	{
		dtsNull,
		dtsConnecting,
		dtsRequesting,
		dtsResponse,
		dtsDownloading,
		dtsEnqueue,
		dtsQueued,
		dtsBusy
	};

public:
	Download*			m_pOwner;
	DownloadSource*	m_pSource;

	DownloadTransferState m_nState;
	quint32				m_tLastResponse;

	quint32				m_nQueuePos;
	quint32				m_nQueueLength;
	QString				m_sQueueName;

	Fragments::Queue	m_lRequested;
public:
	DownloadTransfer( Download* pOwner, DownloadSource* pSource, QObject* parent = 0 );
	virtual ~DownloadTransfer();

	virtual void onTimer( quint32 tNow = 0 );
	virtual void requestBlock( Fragments::Fragment oFragment );
	virtual void subtractRequested( Fragments::List& oFragments );
public:
	inline DownloadSource* source() const;
signals:

public slots:

};

DownloadSource* DownloadTransfer::source() const
{
	return m_pSource;
}

#endif // DOWNLOADTRANSFER_H
