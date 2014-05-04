#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "types.h"
#include "FileFragments.hpp"
#include "Hashes/hash.h"

class DownloadSource;
class QueryHit;
class Transfer;

#define DOWNLOAD_CODE_FILE_VERSION 1

class Download : public QObject
{
	Q_OBJECT

public:
	struct FileListItem
	{
		QString sFileName;
		QString sPath;          // for multifile downloads (like torrents)
		QString sTempName;
		quint64 nStartOffset;
		quint64 nEndOffset;
		HashSet vHashes;
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
	QList<DownloadSource*> m_lSources;
	bool					m_bMultifile;
	QList<FileListItem>		m_lFiles;	// for multifile downloads
	Fragments::List			m_lCompleted;
	Fragments::List			m_lVerified;
	Fragments::List			m_lActive;
	HashSet                 m_vHashes; // hashes for whole download

	bool					m_bSignalSources;
	quint8					m_nPriority; // 255: highest priority; 1: lowest priority; 0: temporary disabled
	bool					m_bModified;
	int						m_nTransfers;
	QDateTime				m_tStarted;

public:
	Download() :
		m_lCompleted( 0 ),
		m_lVerified( 0 ),
		m_lActive( 0 ),
		m_bSignalSources( false ),
		m_bModified( false ),
		m_nTransfers( 0 )
	{}

	Download( QueryHit* pHit, QObject* parent = NULL );
	~Download();

	void start();
	void pause();
	void cancelDownload();
	bool addSource( DownloadSource* pSource );
	int  addSource( QueryHit* pHit );
	void removeSource( DownloadSource* pSource );
	int  startTransfers( int nMaxTransfers = -1 );
	void stopTransfers();
	bool sourceExists( DownloadSource* pSource );

	QList<Transfer*> getTransfers();

	Fragments::List getPossibleFragments( const Fragments::List& oAvailable, Fragments::Fragment& oLargest );
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
	void setState( Download::DownloadState state );
signals:
	void sourceAdded( DownloadSource* );
	void stateChanged( int );
public slots:
	void emitSources();
};

Q_DECLARE_METATYPE( Download* )
Q_DECLARE_METATYPE( Download::DownloadState )

QDataStream& operator<<( QDataStream& s, const Download& rhs );
QDataStream& operator>>( QDataStream& s, Download& rhs );

bool Download::isModified()
{
	return m_bModified;
}
bool Download::isCompleted()
{
	return ( m_nState == dsCompleted );
}
bool Download::isDownloading()
{
	return ( m_nState == dsDownloading );
}

int Download::sourceCount()
{
	return m_lSources.size();
}
int Download::transfersCount()
{
	return m_nTransfers;
}
bool Download::canDownload()
{
	return ( m_nState != dsPaused && m_nState != dsCompleted
			 && m_nState != dsMoving && m_nState != dsVerifying
			 && m_nState != dsFileError && m_nState != dsQueued );
}

#endif // DOWNLOAD_H
