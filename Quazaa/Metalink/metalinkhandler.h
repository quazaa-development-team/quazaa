#ifndef METALINKHANDLER_H
#define METALINKHANDLER_H

#include <QObject>

#include <QList>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>

#include "download.h"
#include "magnetlink.h"

namespace URI
{

typedef enum
{
	uriMagnet = 1, uriURL = 2, uriTorrent = 3
} URIType;

struct MediaURI
{
	URIType m_nType;
	union
	{
		Magnet*    m_pMagnet;  // Magnet link
		QUrl*       m_pURL;     // http, https, ftp, ftps, etc.
		QUrl*       m_pTorrent; // link to .torrent file
	};
	QString m_sLocation;
	quint8 m_nPriority;      // 255: highest priority; 1: lowest priority; 0: temporary disabled
};

struct MetaFile
{
	quint16         m_nID;           // We assume there are no more than 2^16 files in a metalink XML.
	quint64         m_nFileSize;
	QString         m_sFileName;
	QString         m_sIdentity;
	QString         m_sVersion;
	QString         m_sLanguage;
	QString         m_sDescription;
	HashSet      m_vHashes; // Includes all hashes provided via <hash> tag.
	QList<MediaURI> m_lURIs; // Includes web links, links to .torrent files, as well as Magnets.

	MetaFile();
	MetaFile( quint16 ID );
	bool isValid() const;    // Returns true if file struct contains enough data to initialize a download.
};

class MetalinkHandler : public QObject
{
protected:
	static QFile&			m_oEmptyQFile; // MinGW workaround to enable references to be default parameters

	bool                    m_bNull;  // true if created by default constructor or if the passed file couldn't be opened
	bool                    m_bValid; // true if valid
	quint32                 m_nSize;  // number of files in metalink
	QXmlStreamReader        m_oMetaLink;
	QXmlStreamReader::Error m_nParsingState;
	QVector<MetaFile>       m_vFiles; // Files are inerted in the order of their ID and always stay sorted.

public:
	explicit MetalinkHandler( QFile& oFile = m_oEmptyQFile );
	virtual ~MetalinkHandler();

	virtual Download* file( const quint16& ID ) const = 0;
	QList<Download*> files() const;

	inline int size() const;
	inline bool isNull() const;
	inline bool isValid() const;

protected:
	void postParsingError( const int line, const QString sError ) const;
	void postParsingInfo( const int line, const QString sInfo ) const;

private:
	static QFile& getEmptyStaticFile(); // part of the MinGW workaround
};

int MetalinkHandler::size() const
{
	return m_vFiles.size();
}

bool MetalinkHandler::isNull() const
{
	return m_bNull;
}

bool MetalinkHandler::isValid() const
{
	return m_bValid;
}

} // namespace URI

#endif // METALINKHANDLER_H
