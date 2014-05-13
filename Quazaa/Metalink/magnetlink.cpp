#include <QMultiMap>
#include <QObject>

#include "magnetlink.h"
#include "systemlog.h"

#include "NetworkCore/Hashes/hash.h"

using namespace URI;

Magnet::MagnetFile::MagnetFile() :
	m_bNull( true ),
	m_nFileSize( 0 )
{}

Magnet::MagnetFile::~MagnetFile()
{
}

bool Magnet::MagnetFile::isValid() const
{
	return !m_vHashes.empty() || m_lURLs.size();
}

Magnet::Magnet() :
	m_bNull( true )
{}

Magnet::Magnet( QString sMagnet ) :
	m_bNull( false )
{
	parseMagnet( sMagnet );
}

Magnet::MagnetFile Magnet::operator[]( const quint16 nID ) const
{
	return m_lFiles[nID];
}

bool Magnet::parseMagnet( QString sMagnet )
{
	if ( !sMagnet.startsWith( "magnet:?" ) )
	{
		return false;
	}

	m_sMagnet = sMagnet;
	m_lFiles.clear();
	m_lSearches.clear();

	sMagnet.remove( 0, 8 );

	int pos, pos2, nFileNo;
	QString sSubsection, sParam;

	QMap<quint16, MagnetFile> mFiles;
	QMap<quint16, QString> mSearches;

	while ( sMagnet.length() )
	{
		pos = sMagnet.indexOf( "&" );
		pos = ( pos == -1 ) ? sMagnet.length() : pos;	// determine the end of the current subsection

		sSubsection = sMagnet.left( pos );	// isolate current subsection
		sMagnet.remove( 0, ++pos );		// remove current subsection from string

		pos2 = sSubsection.indexOf( "=" );	// get parameter type string length

		if ( pos2 == -1 )
		{
			QString sub = QObject::tr( "Subsection: %1" ).arg( sSubsection );
			systemLog.postLog( LogSeverity::Error, QObject::tr( "Error while parsing subsection of Magnet Link:" ) );
			systemLog.postLog( LogSeverity::Error, m_sMagnet );
			systemLog.postLog( LogSeverity::Error, sub );
			continue;
		}

		sParam = sSubsection.left( pos2 );            // get parameter type string
		sSubsection.remove( 0, sParam.length() + 1 ); // isolate current subsection content

		if ( sParam.length() == 2 || ( sParam.length() > 3 &&
			 ( sParam[2] == '.' ||     // covers multi file magnets
			   sParam[1] == '.' ) ) )  // covers experimental sections starting with "x."
		{
			// Determine ID the current file.
			if ( sParam.length() == 2 )
			{
				nFileNo = 65535; // = 2^16 - 1;
			}
			else if ( sParam.length() > 3 && sParam[2] == '.' )
			{
				bool bOK;
				nFileNo = sParam.mid( 3 ).toInt( &bOK );

				if ( !bOK || nFileNo < 0 )
				{
					subsectionError( sParam, sSubsection );
					continue;
				}
			}
			else if ( sParam.startsWith( "x." ) ) // Experimental stuff
			{
				QString sub = QObject::tr( "Experimental Subsection: %1=%2" ).arg( sParam, sSubsection );
				systemLog.postLog( LogSeverity::Error, QObject::tr( "Detected unsupported experimental subsection in Magnet Link:" ) );
				systemLog.postLog( LogSeverity::Error, m_sMagnet );
				systemLog.postLog( LogSeverity::Error, sub );
			}
			else
			{
				subsectionError( sParam, sSubsection );
				continue;
			}

			if ( sParam.startsWith( "dn" ) )		// Display Name
			{
				mFiles[nFileNo].m_sFileName = sSubsection;
			}
			else if ( sParam.startsWith( "xl" ) )	// EXact Length
			{
				bool bOK;
				qulonglong nFileSize = sSubsection.toULongLong( &bOK );

				if ( bOK )
				{
					mFiles[nFileNo].m_nFileSize = nFileSize;
				}
				else
				{
					subsectionError( sParam, sSubsection );
					continue;
				}
			}
			else if ( sParam.startsWith( "xt" ) )	// EXect Topic
			{
				Hash* pHash = Hash::fromURN( sSubsection );

				if ( pHash )
				{
					if ( !mFiles[nFileNo].m_vHashes.insert( pHash ) )
					{
						systemLog.postLog( LogSeverity::Error, QObject::tr( "Detected and ignored conflicting hash within Magnet Link:" ) );
						systemLog.postLog( LogSeverity::Error, m_sMagnet );
					}
					else
					{
						m_bNull = false;
					}
				}
				else
				{
					QString sub = QObject::tr( "Hash URN: %1" ).arg( sSubsection );
					systemLog.postLog( LogSeverity::Error, QObject::tr( "Detected and ignored unknown hash type within Magnet Link:" ) );
					systemLog.postLog( LogSeverity::Error, m_sMagnet );
					systemLog.postLog( LogSeverity::Error, sub );

					continue;
				}
			}
			else if ( sParam.startsWith( "as" ) || sParam.startsWith( "xs" ) )
			{
				MediaURL url;
				url.m_oURL		= QUrl( sSubsection );

				if ( url.m_oURL.isValid() )
				{
					url.m_nPriority = sParam.startsWith( "xs" ) ? 128 : 0;
					mFiles[nFileNo].m_lURLs.append( url );
					m_bNull = false;
				}
				else
				{
					subsectionError( sParam, sSubsection );
					continue;
				}
			}
			else if ( sParam.startsWith( "kt" ) )
			{
				mSearches[nFileNo] = sSubsection;
				m_bNull = false;
			}
			else if ( sParam.startsWith( "mt" ) )
			{
				QString sub = QObject::tr( "Manifest Topic: %1=%2" ).arg( sParam, sSubsection );
				systemLog.postLog( LogSeverity::Error,
								   QObject::tr( "Detected unsupported \"Manifest Topic\" section in Magnet Link:" ) );
				systemLog.postLog( LogSeverity::Error, m_sMagnet );
				systemLog.postLog( LogSeverity::Error, sub );
			}
			else if ( sParam.startsWith( "tr" ) )
			{
				QUrl url = QUrl( sSubsection );

				if ( url.isValid() )
				{
					mFiles[nFileNo].m_lTrackers.append( url );
				}
				else
				{
					subsectionError( sParam, sSubsection );
					continue;
				}
			}
			else
			{
				QString sub = QObject::tr( "Subsection: %1=%2" ).arg( sParam, sSubsection );
				systemLog.postLog( LogSeverity::Error, QObject::tr( "Detected unknown type of subsection within Magnet Link:" ) );
				systemLog.postLog( LogSeverity::Error, m_sMagnet );
				systemLog.postLog( LogSeverity::Error, sub );
			}
		}
		else
		{
			subsectionError( sParam, sSubsection );
		}
	}

	if ( m_bNull ) // We got neither hashes, nor direct web sources, nor keywords to search for.
	{
		m_sMagnet.clear();

		m_lFiles.clear();
		m_lSearches.clear();
		return false;
	}

	for ( int i = 0, nSize = mFiles.size(); i < nSize; ++i )
	{
		if ( mFiles[i].isValid() )
		{
			mFiles[i].m_bNull = false;
			m_lFiles.push_back( mFiles[i] );
		}
	}

	foreach ( const QString& sSearch, mSearches )
	{
		if ( !sSearch.trimmed().isEmpty() )
		{
			m_lSearches.push_back( sSearch.trimmed() );
		}
	}

	return m_lFiles.size() + m_lSearches.size();
}

bool Magnet::file( const quint16 nID, Download* pDownload ) const
{
	if ( nID >= m_lFiles.size() )
	{
		return false;
	}

	if ( !pDownload )
	{
		pDownload = new Download();
	}

	MagnetFile file = operator[]( nID );



	// TODO: implement rest of this once Downloads have been finished.



	return true;
}

QString Magnet::search( const quint16 nID ) const
{
	if ( nID >= m_lSearches.size() )
	{
		return QString();
	}

	QList<QString>::const_iterator i = m_lSearches.begin();
	i += nID;

	return *i;
}


void Magnet::subsectionError( QString sParam, QString sSubsection )
{
	QString sub = QObject::tr( "Subsection: %1=%2" ).arg( sParam, sSubsection );
	systemLog.postLog( LogSeverity::Error, QObject::tr( "Error while parsing subsection of Magnet Link:" ) );
	systemLog.postLog( LogSeverity::Error, m_sMagnet );
	systemLog.postLog( LogSeverity::Error, sub );
}
