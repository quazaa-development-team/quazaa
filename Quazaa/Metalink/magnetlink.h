#ifndef MAGNETLINK_H
#define MAGNETLINK_H

#include <QList>
#include <QString>
#include <QUrl>

#include "download.h"

namespace URI
{

class Magnet
{
public:

	// internal storage structure
	struct MediaURL
	{
		QUrl    m_oURL;
		quint8  m_nPriority;		// 255: highest priority; 1: lowest priority; 0: temporary disabled
	};

	// internal storage structure
	struct MagnetFile
	{
		bool            m_bNull;
		quint64         m_nFileSize;
		QString         m_sFileName;
		HashSet      m_vHashes;      // Includes all hashes provided via <hash> tag.
		QList<MediaURL> m_lURLs;        // Includes http, https, ftp, ftps, etc.
		QList<QUrl>     m_lTrackers;    // BitTorrent Trackers for this file

		MagnetFile();   // Creates an empty MagnetFile. Note that m_bNull needs to be handled manually.
		~MagnetFile();
		bool isNull() const;
		bool isValid() const; // Returns true if file struct contains enough data to initialize a download.
	};

private:

	bool				m_bNull;
	QString				m_sMagnet;
	QList<MagnetFile>	m_lFiles;			// Contains all files sorted by ID.
	QList<QString>		m_lSearches;		// Contains all files sorted by ID.

public:
	Magnet();
	Magnet( QString sMagnet );

	/**
	  * Returns File struct for ID. Remember to verify their validity with isNull().
	  */
	MagnetFile operator[]( const quint16 nID ) const;

	/**
	  * Returns true if Magnet link could be parsed without errors.
	  */
	bool parseMagnet( QString sMagnet );

	/**
	  * Inserts the information for ID into the download passed as parameter. If NULL is passed, a
	  * new download is created.
	  */
	bool file( const quint16 nID, Download* pDownload ) const;

	/**
	  * Returns search string by ID. Returns empty strings for invalid IDs.
	  */
	QString search( const quint16 nID ) const;

	/**
	  * Returns true for magnets that have been created using the default constructor without having
	  * ever been given a Magnet link to parse. False otherwise.
	  */
	inline bool isNull() const;

	/**
	  * Returns true if magnet points to at least one file that is valid - e.g. which we can download.
	  */
	inline bool isValid() const;

	/**
	  * Returns the number of valid files within this Magnet.
	  */
	inline int fileCount() const;

	/**
	  * Returns the number of searches within this Magnet.
	  */
	inline int searchCount() const;

	/**
	  * Returns a string representation of this Magnet.
	  */
	inline QString toString() const;

private:
	void subsectionError( QString sParam, QString sSubsection );
};

bool Magnet::isNull() const
{
	return m_bNull;
}

bool Magnet::isValid() const
{
	return m_lFiles.size() || m_lSearches.size();
}

int Magnet::fileCount() const
{
	return m_lFiles.size();
}

int Magnet::searchCount() const
{
	return m_lSearches.size();
}

QString Magnet::toString() const
{
	return m_sMagnet;
}

} // namespace URI

#endif // MAGNETLINK_H
