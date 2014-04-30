#ifndef DOWNLOADS_H
#define DOWNLOADS_H

#include <QObject>
#include <QMutex>

class QueryHit;
class Download;

class Downloads : public QObject
{
	Q_OBJECT
public:
	QMutex m_pSection;

	QList<Download*> m_lDownloads;
public:
	Downloads( QObject* parent = 0 );

	void start();
	void stop();

	void add( QueryHit* pHit );

	bool exists( Download* pDownload );
signals:
	void downloadAdded( Download* );
	void downloadRemoved();
public slots:
	void emitDownloads();
	void onTimer();
};

extern Downloads downloads;

#endif // DOWNLOADS_H
