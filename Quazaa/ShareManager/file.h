#ifndef FILE_H
#define FILE_H

#include <QSet>
#include <QFile>
#include <QFileInfo>

#include "NetworkCore/Hashes/hashset.h"

class File : public QObject, public QFileInfo
{
	Q_OBJECT

protected:
	quint64			m_nDirectoryID;
	quint64			m_nFileID;
	QFile*			m_pFile;

	bool			m_bNull;

	// SHA1 (Base32), ED2K (MD4, Base16), BitTorrent Info Hash (Base32), TigerTree Root Hash (Base32), MD5 (Base16)
	HashSet m_vHashes;
	QSet< QString > m_Tags;

public:
	typedef enum { Magnet, eD2k } URIType;

public:
	explicit File( File& file );
	explicit File( QObject* parent = NULL );
	explicit File( const QString& file, QObject* parent = NULL );
	explicit File( const QFile& file, QObject* parent = NULL );
	explicit File( const QDir& dir, const QString& file, QObject* parent = NULL );
	explicit File( const QFileInfo& fileinfo, QObject* parent = NULL );

	inline virtual ~File();

	// Refreshes all information from the file on disk if existing.
	virtual void refresh();

	// Returns true if the file has been created using the default constructor CFile()
	// without any parameter and a file name has not been set using setFileName.
	// To verify whether a file exists physically on the HDD, use CFile::exists().
	inline bool isNull() const;

	// Returns a list of all hashes attributed to this file.
	inline const HashSet& getHashes() const;

	// Sets a single hash for the file. Returns false on hash conflict. In that case, the hash is
	// not inserted.
	inline bool setHash( const Hash& rHash );

	// Sets a single hash for the file and takes control of that hash. Use this only if you do not
	// use the hash anymore afterwards. Returns false on hash conflict. In that case, the hash is
	// not inserted and deleted. Else, the hash is deleted uppon the destruction of the file or once
	// it is removed explicitly from the file.
	inline bool setHash( Hash* pHash );

	// Sets a list of hashes for the file. Returns false on hash conflict. In that case, no hash is
	// inserted.
	inline bool setHashes(const HashSet& vHashes );

	// Removes a hash from the set of hashes of a file. Returns false if the requested
	// hash could not be found; otherwise returns true.
	bool removeHash( const Hash& rHash );

	// Creates an URI of the files attributes.
	QString toURI( URIType type ) const;

	// Allows to store tagging information. This can be used for example to store
	// information about whether a file is shared or not, to which sharing group it
	// belongs or whether it exists on the hard drive or not.
	inline void setTag( const QString& sTag );

	// Removes a specified tag. Returns true if the operation was successful.
	inline bool removeTag( const QString& sTag );

	// Checks whether a file has been marked with a given tag.
	bool isTagged( const QString& sTag ) const;

	inline void setDirectoryID( const quint64& ID );
	inline quint64 getDirectoryID() const;

	inline void setFileID( const quint64& ID );
	inline quint64 getFileID() const;

	/* ------ reimplemented QFileInfo functions ------ */

	inline void setFile( const QString& file );
	inline void setFile( const QFile& file );
	inline void setFile( const QDir& dir, const QString& file );

	/* ------ QFile wrapper functions ------ */

	inline QFile*
	file(); // Returns a new QFile object that allows reading and writing separate from the current CFile obejct.

	inline bool copy( const QString& newName );
	inline QFile::FileError error() const;
	inline bool flush();
	inline int handle() const;
	inline bool link( const QString& linkName );
	inline uchar* map( qint64 offset, qint64 size, QFile::MemoryMapFlags flags = QFile::NoOptions );
	inline bool open( FILE* fh, QIODevice::OpenMode mode );
	inline bool open( int fd, QIODevice::OpenMode mode );
	inline bool remove();
	inline bool rename( const QString& newName );
	inline bool resize( qint64 sz );
	inline void setFileName( const QString& name );
	inline bool setPermissions( QFile::Permissions permissions );
	inline bool unmap( uchar* address );
	inline void unsetError();

	inline virtual bool atEnd() const;
	inline virtual void close();
	inline virtual bool isSequential() const;
	inline virtual bool open( QIODevice::OpenMode mode );
	inline virtual qint64 pos() const;
	inline virtual bool seek( qint64 off );
	inline virtual qint64 size() const;

	/* ------ QIODevice wrapper functions ------ */

	inline QString errorString() const;
	inline bool getChar( char* c );
	inline bool isOpen() const;
	inline bool isReadable() const;
	inline bool isTextModeEnabled() const;
	inline bool isWritable() const;
	inline QIODevice::OpenMode openMode() const;
	inline qint64 peek( char* data, qint64 maxSize );
	inline QByteArray peek( qint64 maxSize );
	inline bool putChar( char c );
	inline qint64 read( char* data, qint64 maxSize );
	inline QByteArray read( qint64 maxSize );
	inline QByteArray readAll();
	inline qint64 readLine( char* data, qint64 maxSize );
	inline QByteArray readLine( qint64 maxSize = 0 );
	inline virtual bool reset();
	inline void setTextModeEnabled( bool enabled );
	inline void ungetChar( char c );
	inline virtual bool waitForBytesWritten( int msecs );
	inline virtual bool waitForReadyRead( int msecs );
	inline qint64 write( const char* data, qint64 maxSize );
	inline qint64 write( const char* data );
	inline qint64 write( const QByteArray& byteArray );

private:

signals:

public slots:
};

File::~File()
{
	delete m_pFile;
}

bool File::isNull() const
{
	return m_bNull;
}

const HashSet& File::getHashes() const
{
	return m_vHashes;
}

bool File::setHash( const Hash& rHash )
{
	return m_vHashes.insert( rHash );
}

bool File::setHash( Hash* pHash )
{
	return m_vHashes.insert( pHash );
}

bool File::setHashes( const HashSet& vHashes )
{
	return m_vHashes.insert( vHashes );
}

void File::setTag( const QString& sTag )
{
	m_Tags.insert( sTag );
}

bool File::removeTag( const QString& sTag )
{
	return m_Tags.remove( sTag );
}

void File::setDirectoryID( const quint64& ID )
{
	m_nDirectoryID = ID;
}

quint64 File::getDirectoryID() const
{
	return m_nDirectoryID;
}

void File::setFileID( const quint64& ID )
{
	m_nFileID = ID;
}

quint64 File::getFileID() const
{
	return m_nFileID;
}

void File::setFile( const QString& file )
{
	if ( !file.isEmpty() )
	{
		m_bNull = false;
	}
	QFileInfo::setFile( file );
	refresh();
}

void File::setFile( const QFile& file )
{
	m_bNull = false;
	QFileInfo::setFile( file );
	refresh();
}

void File::setFile( const QDir& dir, const QString& file )
{
	if ( !file.isEmpty() )
	{
		m_bNull = false;
	}
	QFileInfo::setFile( dir, file );
	refresh();
}

QFile* File::file()
{
	return new QFile( absoluteFilePath() );
}

bool File::copy( const QString& newName )
{
	return m_pFile->copy( newName );
}

QFile::FileError File::error() const
{
	return m_pFile->error();
}

bool File::flush()
{
	return m_pFile->flush();
}

int File::handle() const
{
	return m_pFile->handle();
}

bool File::link( const QString& linkName )
{
	return m_pFile->link( linkName );
}

uchar* File::map( qint64 offset, qint64 size, QFile::MemoryMapFlags flags )
{
	return m_pFile->map( offset, size, flags );
}

bool File::open( FILE* fh, QIODevice::OpenMode mode )
{
	return m_pFile->open( fh, mode );
}

bool File::open( int fd, QIODevice::OpenMode mode )
{
	return m_pFile->open( fd, mode );
}

bool File::remove()
{
	bool bResult = m_pFile->remove();
	refresh();
	return bResult;
}

bool File::rename( const QString& newName )
{
	bool bResult = m_pFile->rename( newName );
	QFileInfo::setFile( m_pFile->fileName() );
	return bResult;
}

bool File::resize( qint64 sz )
{
	bool bResult = m_pFile->resize( sz );
	refresh();
	return bResult;
}

void File::setFileName( const QString& name )
{
	QFileInfo::setFile( name );
	refresh();
}

bool File::setPermissions( QFile::Permissions permissions )
{
	bool bResult = m_pFile->setPermissions( permissions );
	refresh();
	return bResult;
}

bool File::unmap( uchar* address )
{
	return m_pFile->unmap( address );
}

void File::unsetError()
{
	m_pFile->unsetError();
}

bool File::atEnd() const
{
	return m_pFile->atEnd();
}

void File::close()
{
	m_pFile->close();
}

bool File::isSequential() const
{
	return m_pFile->isSequential();
}

bool File::open( QIODevice::OpenMode mode )
{
	return m_pFile->open( mode );
}

qint64 File::pos() const
{
	return m_pFile->pos();
}

bool File::seek( qint64 off )
{
	return m_pFile->seek( off );
}

qint64 File::size() const
{
	return m_pFile->size();
}

QString File::errorString() const
{
	return m_pFile->errorString();
}

bool File::getChar( char* c )
{
	return m_pFile->getChar( c );
}

bool File::isOpen() const
{
	return m_pFile->isOpen();
}

bool File::isReadable() const
{
	return m_pFile->isReadable();
}

bool File::isWritable() const
{
	return m_pFile->isWritable();
}

QIODevice::OpenMode File::openMode() const
{
	return m_pFile->openMode();
}

qint64 File::peek( char* data, qint64 maxSize )
{
	return m_pFile->peek( data, maxSize );
}

QByteArray File::peek( qint64 maxSize )
{
	return m_pFile->peek( maxSize );
}

bool File::putChar( char c )
{
	return m_pFile->putChar( c );
}

qint64 File::read( char* data, qint64 maxSize )
{
	return m_pFile->read( data, maxSize );
}

QByteArray File::read( qint64 maxSize )
{
	return m_pFile->read( maxSize );
}

QByteArray File::readAll()
{
	return m_pFile->readAll();
}

qint64 File::readLine( char* data, qint64 maxSize )
{
	return m_pFile->readLine( data, maxSize );
}

QByteArray File::readLine( qint64 maxSize )
{
	return m_pFile->readLine( maxSize );
}

bool File::reset()
{
	return m_pFile->reset();
}

void File::setTextModeEnabled( bool enabled )
{
	m_pFile->setTextModeEnabled( enabled );
}

void File::ungetChar( char c )
{
	m_pFile->ungetChar( c );
}

bool File::waitForBytesWritten( int msecs )
{
	return m_pFile->waitForBytesWritten( msecs );
}

bool File::waitForReadyRead( int msecs )
{
	return m_pFile->waitForReadyRead( msecs );
}

qint64 File::write( const char* data, qint64 maxSize )
{
	return m_pFile->write( data, maxSize );
}

qint64 File::write( const char* data )
{
	return m_pFile->write( data );
}

qint64 File::write( const QByteArray& byteArray )
{
	return m_pFile->write( byteArray );
}

#endif // FILE_H
