#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "types.h"
#include "FileFragments.hpp"
#include "Hashes/hash.h"

class CDownloadSource;
class CQueryHit;
class CTransfer;

class CDownload : public QObject
{
	Q_OBJECT

public:
	struct FileListItem
	{
		QString sFileName;
		QString sPath; // for multifile downloads (like torrents)
		QString sTempName;
		quint64 nStartOffset;
		quint64 nEndOffset;
		QList<CHash> lHashes;
	};
	enum DownloadState
	{
		dsQueued,
		dsPaused,
		dsSearching,
		dsPending,
		dsDownloading,
		dsVerifying,
		dsMoving,
		dsFileError,
		dsCompleted
	};

	QString					m_sDisplayName;
	QString					m_sTempName;
	quint64					m_nSize;
	quint64					m_nCompletedSize;
	DownloadState			m_nState;
	QList<CDownloadSource*> m_lSources;
	bool					m_bMultifile;
	QList<FileListItem>		m_lFiles;	// for multifile downloads
	Fragments::List			m_lCompleted;
	Fragments::List			m_lVerified;
	Fragments::List			m_lActive;
	HashVector				m_lHashes; // hashes for whole download

	bool					m_bSignalSources;
	quint8					m_nPriority; // 255: highest priority; 1: lowest priority; 0: temporary disabled
	bool					m_bModified;
	int						m_nTransfers;
	QDateTime				m_tStarted;
public:
	CDownload()
		: m_lCompleted(0),
		  m_lVerified(0),
		  m_lActive(0),
		  m_bSignalSources(false), m_bModified(false),m_nTransfers(0)
	{}
	CDownload(CQueryHit* pHit, QObject *parent = 0);
	~CDownload();

	void start();
	void pause();
	void cancelDownload();
	bool addSource(CDownloadSource* pSource);
	int  addSource(CQueryHit* pHit);
	void removeSource(CDownloadSource* pSource);
	int  startTransfers(int nMaxTransfers = -1);
	void stopTransfers();
	bool sourceExists(CDownloadSource* pSource);

	QList<CTransfer*> getTransfers();

	Fragments::List getPossibleFragments(const Fragments::List& oAvailable, Fragments::Fragment& oLargest);
	Fragments::List getWantedFragments();

	void saveState();
public:
	inline bool isModified();
	inline bool isCompleted();
	inline bool isDownloading();
	inline int  sourceCount();
	inline int  transfersCount();
	inline bool canDownload();
protected:
	void setState(CDownload::DownloadState state);
signals:
	void sourceAdded(CDownloadSource*);
	void stateChanged(int);
public slots:
	void emitSources();
};

Q_DECLARE_METATYPE(CDownload*);
Q_DECLARE_METATYPE(CDownload::DownloadState);

QDataStream& operator<<(QDataStream& s, const CDownload& rhs);
QDataStream& operator>>(QDataStream& s, CDownload& rhs);

bool CDownload::isModified()
{
	return m_bModified;
}
bool CDownload::isCompleted()
{
	return (m_nState == dsCompleted);
}
bool CDownload::isDownloading()
{
	return (m_nState == dsDownloading);
}

int CDownload::sourceCount()
{
	return m_lSources.size();
}
int CDownload::transfersCount()
{
	return m_nTransfers;
}
bool CDownload::canDownload()
{
	return (m_nState != dsPaused && m_nState != dsCompleted
			&& m_nState != dsMoving && m_nState != dsVerifying
			&& m_nState != dsFileError && m_nState != dsQueued);
}

#endif // DOWNLOAD_H
