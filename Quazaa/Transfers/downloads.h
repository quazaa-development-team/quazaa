#ifndef DOWNLOADS_H
#define DOWNLOADS_H

#include <QObject>
#include <QMutex>

class QueryHit;
class CDownload;

class CDownloads : public QObject
{
	Q_OBJECT
public:
	QMutex m_pSection;

	QList<CDownload*> m_lDownloads;
public:
	CDownloads(QObject *parent = 0);

	void start();
	void stop();

	void add(QueryHit* pHit);

	bool exists(CDownload* pDownload);
signals:
	void downloadAdded(CDownload*);
	void downloadRemoved();
public slots:
	void emitDownloads();
	void onTimer();
};

extern CDownloads Downloads;

#endif // DOWNLOADS_H
