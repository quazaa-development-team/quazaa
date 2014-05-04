#ifndef DOWNLOADSOURCE_H
#define DOWNLOADSOURCE_H

#include "types.h"
#include "Hashes/hash.h"
#include "FileFragments.hpp"

class QueryHit;
class Download;
class Transfer;

class DownloadSource : public QObject
{
	Q_OBJECT
public:
	EndPoint			m_oAddress;		// source address
	bool				m_bPush;		// is it a push source?
	QList<EndPoint>     m_oPushProxies;	// push proxy, in case we can't connect directly
	TransferProtocol    m_nProtocol;	// protocol
	DiscoveryProtocol::Protocol	m_nNetwork;		// network
	QUuid				m_oGUID;		// source GUID (needed for pushing)

	HashSet          m_vHashes;		// list of hashes
	quint32				m_tNextAccess;	// seconds since 1970
	quint32				m_nFailures;	// number of failures
	QString				m_sURL;			// URL

	Download*			m_pDownload;
	Transfer*			m_pTransfer;

	Fragments::List		m_lAvailableFrags;
	Fragments::List		m_lDownloadedFrags;
public:
	DownloadSource( Download* pDownload, QObject* parent = 0 );
	DownloadSource( Download* pDownload, QueryHit* pHit, QObject* parent = 0 );
	virtual ~DownloadSource();

public slots:
	Transfer*  createTransfer();
	void		closeTransfer();

public:
	inline bool canAccess();
	inline bool hasTransfer();

signals:
	void transferCreated();
	void transferClosed();
	void bytesReceived( quint64, quint64 ); // offset, length
};

Q_DECLARE_METATYPE( DownloadSource* )

bool DownloadSource::canAccess()
{
	return time( 0 ) > m_tNextAccess;
}
bool DownloadSource::hasTransfer()
{
	return ( m_pTransfer != 0 );
}

QDataStream& operator<<( QDataStream& s, const DownloadSource& rhs );
QDataStream& operator>>( QDataStream& s, DownloadSource& rhs );

#endif // DOWNLOADSOURCE_H
