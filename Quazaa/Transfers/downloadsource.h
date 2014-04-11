#ifndef DOWNLOADSOURCE_H
#define DOWNLOADSOURCE_H

#include "types.h"
#include "Hashes/hash.h"
#include "FileFragments.hpp"

class QueryHit;
class CDownload;
class CTransfer;

class CDownloadSource : public QObject
{
	Q_OBJECT
public:
	CEndPoint			m_oAddress;		// source address
	bool				m_bPush;		// is it a push source?
	QList<CEndPoint>	m_oPushProxies;	// push proxy, in case we can't connect directly
	TransferProtocol	m_nProtocol;	// protocol
	DiscoveryProtocol::Protocol	m_nNetwork;		// network
	QUuid				m_oGUID;		// source GUID (needed for pushing)
	QList<CHash>		m_lHashes;		// list of hashes
	time_t				m_tNextAccess;	// seconds since 1970
	quint32				m_nFailures;	// number of failures
	QString				m_sURL;			// URL

	CDownload*			m_pDownload;
	CTransfer*			m_pTransfer;

	Fragments::List		m_lAvailableFrags;
	Fragments::List		m_lDownloadedFrags;
public:
	CDownloadSource(CDownload* pDownload, QObject* parent = 0);
	CDownloadSource(CDownload* pDownload, QueryHit* pHit, QObject* parent = 0);
	virtual ~CDownloadSource();

public slots:
	CTransfer*  createTransfer();
	void		closeTransfer();

public:
	inline bool canAccess();
	inline bool hasTransfer();

signals:
	void transferCreated();
	void transferClosed();
	void bytesReceived(quint64, quint64); // offset, length
};

Q_DECLARE_METATYPE(CDownloadSource*)

bool CDownloadSource::canAccess()
{
	return time(0) > m_tNextAccess;
}
bool CDownloadSource::hasTransfer()
{
	return (m_pTransfer != 0);
}

QDataStream& operator<<(QDataStream& s, const CDownloadSource& rhs);
QDataStream& operator>>(QDataStream& s, CDownloadSource& rhs);

#endif // DOWNLOADSOURCE_H
