#ifndef FILE_H
#define FILE_H

#include <QFile>
#include <QFileInfo>

#include "Hashes/hash.h"

class CFile : public QObject, public QFileInfo
{
	Q_OBJECT

protected:
	quint64			m_nDirectoryID;
	quint64			m_nFileID;
	QFile*			m_pFile;

	bool			m_bNull;

	QList< CHash >	m_Hashes; // SHA1 (Base32), ED2K (MD4, Base16), BitTorrent Info Hash (Base32), TigerTree Root Hash (Base32), MD5 (Base16)
	QSet< QString > m_Tags;

public:
	typedef enum { Magnet, eD2k } URIType;

public:
	explicit CFile(CFile& file);
	explicit CFile(QObject* parent = NULL);
	explicit CFile(const QString& file, QObject* parent = NULL);
	explicit CFile(const QFile& file, QObject* parent = NULL);
	explicit CFile(const QDir& dir, const QString& file, QObject* parent = NULL);
	explicit CFile(const QFileInfo& fileinfo, QObject* parent = NULL);

	inline virtual ~CFile();

	// Refreshes all information from the file on disk if existing.
	virtual void refresh();

	// Returns true if the file has been created using the default constructor CFile()
	// without any parameter and a file name has not been set using setFileName.
	// To verify whether a file exists physically on the HDD, use CFile::exists().
	inline bool isNull() const;

	// Returns a list of all hashes attributed to this file. Note that this does not
	// perform checking operations, so it is in theory possible to have 2 differnet
	// hashes of the same type returned within this list.
	inline QList< CHash > getHashes() const;

	// Sets a single hash for the file.
	inline void setHash(const CHash& oHash);

	// Sets a list of hashes for the file.
	inline void setHashes(const QList<CHash*>& lHashes);

	// Removes a hash from the set of hashes of a file. Returns false if the requested
	// hash could not be found; otherwise returns true.
	bool removeHash(const CHash& oHash);

	// Creates an URI of the files attributes.
	QString toURI(URIType type) const;

	// Allows to store tagging information. This can be used for example to store
	// information about whether a file is shared or not, to which sharing group it
	// belongs or whether it exists on the hard drive or not.
	inline void setTag(const QString& sTag);

	// Removes a specified tag. Returns true if the operation was successful.
	inline bool removeTag(const QString& sTag);

	// Checks whether a file has been marked with a given tag.
	bool isTagged(const QString& sTag) const;

	inline void setDirectoryID(const quint64& ID);
	inline quint64 getDirectoryID() const;

	inline void setFileID(const quint64& ID);
	inline quint64 getFileID() const;

	/* ------ reimplemented QFileInfo functions ------ */

	inline void setFile(const QString& file);
	inline void setFile(const QFile& file);
	inline void setFile(const QDir& dir, const QString& file);

	/* ------ QFile wrapper functions ------ */

	inline QFile* file(); // Returns a new QFile object that allows reading and writing separate from the current CFile obejct.

	inline bool copy(const QString& newName);
	inline QFile::FileError error() const;
	inline bool flush();
	inline int handle() const;
	inline bool link(const QString& linkName);
	inline uchar* map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags = QFile::NoOptions);
	inline bool open(FILE* fh, QIODevice::OpenMode mode);
	inline bool open(int fd, QIODevice::OpenMode mode);
	inline bool remove();
	inline bool rename(const QString& newName);
	inline bool resize(qint64 sz);
	inline void setFileName(const QString& name);
	inline bool setPermissions(QFile::Permissions permissions);
	inline bool unmap(uchar* address);
	inline void unsetError();

	inline virtual bool atEnd() const;
	inline virtual void close();
	inline virtual bool isSequential() const;
	inline virtual bool open(QIODevice::OpenMode mode);
	inline virtual qint64 pos() const;
	inline virtual bool seek(qint64 off);
	inline virtual qint64 size() const;

	/* ------ QIODevice wrapper functions ------ */

	inline QString errorString() const;
	inline bool getChar(char* c);
	inline bool isOpen() const;
	inline bool isReadable() const;
	inline bool isTextModeEnabled() const;
	inline bool isWritable() const;
	inline QIODevice::OpenMode openMode() const;
	inline qint64 peek(char* data, qint64 maxSize);
	inline QByteArray peek(qint64 maxSize);
	inline bool putChar(char c);
	inline qint64 read(char* data, qint64 maxSize);
	inline QByteArray read(qint64 maxSize);
	inline QByteArray readAll();
	inline qint64 readLine(char* data, qint64 maxSize);
	inline QByteArray readLine(qint64 maxSize = 0);
	inline virtual bool reset();
	inline void setTextModeEnabled(bool enabled);
	inline void ungetChar(char c);
	inline virtual bool waitForBytesWritten(int msecs);
	inline virtual bool waitForReadyRead(int msecs);
	inline qint64 write(const char* data, qint64 maxSize);
	inline qint64 write(const char* data);
	inline qint64 write(const QByteArray& byteArray);

private:

signals:

public slots:
};

CFile::~CFile()
{
	delete m_pFile;
}

bool CFile::isNull() const
{
	return m_bNull;
}

QList< CHash > CFile::getHashes() const
{
	return m_Hashes;
}

void CFile::setHash(const CHash& oHash)
{
	m_Hashes.push_back( oHash );
}

void CFile::setHashes(const QList<CHash*>& lHashes)
{
	for ( int i = 0, nSize = lHashes.size(); i < nSize; ++i )
	{
		m_Hashes.append( *lHashes[i] );
	}
}

void CFile::setTag(const QString& sTag)
{
	m_Tags.insert( sTag );
}

bool CFile::removeTag(const QString& sTag)
{
	return m_Tags.remove( sTag );
}

void CFile::setDirectoryID(const quint64& ID)
{
	m_nDirectoryID = ID;
}

quint64 CFile::getDirectoryID() const
{
	return m_nDirectoryID;
}

void CFile::setFileID(const quint64& ID)
{
	m_nFileID = ID;
}

quint64 CFile::getFileID() const
{
	return m_nFileID;
}

void CFile::setFile(const QString& file)
{
	if ( !file.isEmpty() )
		m_bNull = false;
	QFileInfo::setFile( file );
	refresh();
}

void CFile::setFile(const QFile& file)
{
	m_bNull = false;
	QFileInfo::setFile( file );
	refresh();
}

void CFile::setFile(const QDir& dir, const QString& file)
{
	if ( !file.isEmpty() )
		m_bNull = false;
	QFileInfo::setFile( dir, file );
	refresh();
}

QFile* CFile::file()
{
	return new QFile( absoluteFilePath() );
}

bool CFile::copy(const QString& newName)
{
	return m_pFile->copy( newName );
}

QFile::FileError CFile::error() const
{
	return m_pFile->error();
}

bool CFile::flush()
{
	return m_pFile->flush();
}

int CFile::handle() const
{
	return m_pFile->handle();
}

bool CFile::link(const QString& linkName)
{
	return m_pFile->link( linkName );
}

uchar* CFile::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
	return m_pFile->map( offset, size, flags );
}

bool CFile::open(FILE* fh, QIODevice::OpenMode mode)
{
	return m_pFile->open( fh, mode );
}

bool CFile::open(int fd, QIODevice::OpenMode mode)
{
	return m_pFile->open( fd, mode );
}

bool CFile::remove()
{
	bool bResult = m_pFile->remove();
	refresh();
	return bResult;
}

bool CFile::rename(const QString& newName)
{
	bool bResult = m_pFile->rename( newName );
	QFileInfo::setFile( m_pFile->fileName() );
	return bResult;
}

bool CFile::resize( qint64 sz )
{
	bool bResult = m_pFile->resize( sz );
	refresh();
	return bResult;
}

void CFile::setFileName(const QString& name)
{
	QFileInfo::setFile( name );
	refresh();
}

bool CFile::setPermissions(QFile::Permissions permissions)
{
	bool bResult = m_pFile->setPermissions( permissions );
	refresh();
	return bResult;
}

bool CFile::unmap(uchar* address)
{
	return m_pFile->unmap( address );
}

void CFile::unsetError()
{
	m_pFile->unsetError();
}

bool CFile::atEnd() const
{
	return m_pFile->atEnd();
}

void CFile::close()
{
	m_pFile->close();
}

bool CFile::isSequential() const
{
	return m_pFile->isSequential();
}

bool CFile::open(QIODevice::OpenMode mode)
{
	return m_pFile->open( mode );
}

qint64 CFile::pos() const
{
	return m_pFile->pos();
}

bool CFile::seek(qint64 off)
{
	return m_pFile->seek( off );
}

qint64 CFile::size() const
{
	return m_pFile->size();
}

QString CFile::errorString() const
{
	return m_pFile->errorString();
}

bool CFile::getChar(char* c)
{
	return m_pFile->getChar( c );
}

bool CFile::isOpen() const
{
	return m_pFile->isOpen();
}

bool CFile::isReadable() const
{
	return m_pFile->isReadable();
}

bool CFile::isWritable() const
{
	return m_pFile->isWritable();
}

QIODevice::OpenMode CFile::openMode() const
{
	return m_pFile->openMode();
}

qint64 CFile::peek(char* data, qint64 maxSize)
{
	return m_pFile->peek( data, maxSize );
}

QByteArray CFile::peek(qint64 maxSize)
{
	return m_pFile->peek( maxSize );
}

bool CFile::putChar(char c)
{
	return m_pFile->putChar( c );
}

qint64 CFile::read(char* data, qint64 maxSize)
{
	return m_pFile->read( data, maxSize );
}

QByteArray CFile::read(qint64 maxSize)
{
	return m_pFile->read( maxSize );
}

QByteArray CFile::readAll()
{
	return m_pFile->readAll();
}

qint64 CFile::readLine(char* data, qint64 maxSize)
{
	return m_pFile->readLine( data, maxSize );
}

QByteArray CFile::readLine(qint64 maxSize)
{
	return m_pFile->readLine( maxSize );
}

bool CFile::reset()
{
	return m_pFile->reset();
}

void CFile::setTextModeEnabled(bool enabled)
{
	m_pFile->setTextModeEnabled( enabled );
}

void CFile::ungetChar(char c)
{
	m_pFile->ungetChar( c );
}

bool CFile::waitForBytesWritten(int msecs)
{
	return m_pFile->waitForBytesWritten( msecs );
}

bool CFile::waitForReadyRead(int msecs)
{
	return m_pFile->waitForReadyRead( msecs );
}

qint64 CFile::write(const char* data, qint64 maxSize)
{
	return m_pFile->write( data, maxSize );
}

qint64 CFile::write(const char* data)
{
	return m_pFile->write( data );
}

qint64 CFile::write(const QByteArray& byteArray)
{
	return m_pFile->write( byteArray );
}

#endif // FILE_H
