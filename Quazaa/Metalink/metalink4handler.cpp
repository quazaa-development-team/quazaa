/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2011.
** This file is part of QUAZAA (quazaa.sourceforge.net)
**
** Quazaa is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Quazaa is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with Quazaa; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <QXmlStreamReader>

#include "metalink4handler.h"
#include "debug_new.h"

using namespace URI;

CMetalink4Handler::CMetalink4Handler(QFile& oFile) :
	CMetalinkHandler( oFile )
{
/*

 <?xml version="1.0" encoding="UTF-8"?>
 <metalink xmlns="urn:ietf:params:xml:ns:metalink">
   <published>2009-05-15T12:23:23Z</published>
   <file name="example.ext">
	 <size>14471447</size>
	 <identity>Example</identity>
	 <version>1.0</version>
	 <language>en</language>
	 <description>
	 A description of the example file for download.
	 </description>
	 <hash type="sha-256">3d6fece8033d146d8611eab4f032df738c8c1283620fd02a1f2bfec6e27d590d</hash>
	 <url location="de" priority="1">ftp://ftp.example.com/example.ext</url>
	 <url location="fr" priority="1">http://example.com/example.ext</url>
	 <metaurl mediatype="torrent" priority="2">http://example.com/example.ext.torrent</metaurl>
   </file>
 </metalink>

*/

	quint16 fileID = 0;
	QList<MetaFile> lFiles;

	while ( !m_oMetaLink.atEnd() )
	{
		if ( m_oMetaLink.readNextStartElement() && !m_oMetaLink.name().compare( "file" ) )
		{
			if ( parseFile( lFiles, fileID ) )
			{
				++fileID;
			}
		}

		m_nParsingState = m_oMetaLink.error();
		if ( m_nParsingState != QXmlStreamReader::NoError )
		{
			postParsingError( m_oMetaLink.lineNumber(),
							  tr( "Malformatted XML or broken file detected." ) );
		}
	}

	// reserve space for migrating entries to vector
	m_vFiles.reserve( lFiles.size() );

	fileID = 0;
	foreach ( MetaFile oFile, lFiles )
	{
		Q_ASSERT ( oFile.m_nID == fileID ); // verify sort order
		oFile.m_nID = fileID;
		m_vFiles[fileID++] = oFile;
	}

	m_bValid = fileID;

	//TODO: Parse more data within the metalink
}

CDownload* CMetalink4Handler::file(const unsigned int& ID) const
{
	MetaFile oFile = m_vFiles[ ID ];


	// TODO: Implement.


	return new CDownload();
}

bool CMetalink4Handler::parseFile(QList<MetaFile> &lFiles, quint16 ID)
{
	MetaFile oCurrentFile( ID );

	while ( m_oMetaLink.readNextStartElement() )
	{
		QStringRef sComp = m_oMetaLink.name();
		QXmlStreamAttributes vAttributes = m_oMetaLink.attributes();

		if ( !sComp.compare( "hash" ) )
		{
			if ( vAttributes.hasAttribute( "type" ) )
			{
				QString urn = "urn:" + vAttributes.value( "type" ).toString().trimmed() + ":" +
								  m_oMetaLink.readElementText();
				CHash* pHash = CHash::fromURN( urn );

				if ( pHash )
				{
					oCurrentFile.m_lHashes.append( pHash );
				}
				else
				{
					postParsingInfo( m_oMetaLink.lineNumber(),
									 tr( "Found unsupported hash of type \"%1\" within <hash> tag." ) );
					continue;
				}
			}
			else
			{
				postParsingError( m_oMetaLink.lineNumber(),
								  tr( "<hash> tag is missing type specification." ) );
				continue;
			}
		}
		else if ( !sComp.compare( "url" ) )
		{
			MediaURI uri;
			uri.m_nType = uriURL;
			uri.m_pURL = new QUrl( m_oMetaLink.readElementText() );

			if ( uri.m_pURL->isValid() )
			{
				if ( vAttributes.hasAttribute( "location" ) )
				{
					uri.m_sLocation = vAttributes.value( "location" ).toString().trimmed();
				}

				if ( vAttributes.hasAttribute( "priority" ) )
				{
					bool bOK;
					quint16 nPriority = vAttributes.value( "priority" ).toString().toUShort( &bOK );

					if ( bOK && nPriority < 256 )
					{
						uri.m_nPriority = nPriority;
					}
					else
					{
						postParsingError( m_oMetaLink.lineNumber(),
										  tr( "\"priority\" attribute within <url> tag could not be parsed." ) );
					}
				}

				oCurrentFile.m_lURIs.append( uri );
			}
			else
			{
				postParsingError( m_oMetaLink.lineNumber(),
								  tr( "Could not parse invalid URL: %1" ).arg( uri.m_pURL->toString() ) );
				delete uri.m_pURL;
				continue;
			}
		}
		else if ( !sComp.compare( "size" ) )
		{
			bool bOK;
			quint64 nFileSize = m_oMetaLink.readElementText().toULongLong( &bOK );

			if ( bOK )
			{
				oCurrentFile.m_nFileSize = nFileSize;
			}
			else
			{
				postParsingError( m_oMetaLink.lineNumber(),
								  tr( "<size> tag could not be parsed." ) );
				continue;
			}
		}
		else if ( !sComp.compare( "identity" ) )
		{
			oCurrentFile.m_sIdentity = m_oMetaLink.readElementText();
		}
		else if ( !sComp.compare( "description" ) )
		{
			oCurrentFile.m_sDescription = m_oMetaLink.readElementText();
		}
		else if ( !sComp.compare( "language" ) )
		{
			oCurrentFile.m_sLanguage = m_oMetaLink.readElementText();
		}
		else if ( !sComp.compare( "version" ) )
		{
			oCurrentFile.m_sVersion = m_oMetaLink.readElementText();
		}
		else if ( !sComp.compare( "metaurl" ) )
		{
			if ( vAttributes.hasAttribute( "mediatype" ) )
			{
				MediaURI uri;
				QString sMediaType = vAttributes.value( "mediatype" ).toString().trimmed();

				if ( !sMediaType.compare( "torrent" ) )
				{

					uri.m_nType = uriTorrent;
					uri.m_pTorrent = new QUrl( m_oMetaLink.readElementText() );

					if ( !uri.m_pTorrent->isValid() )
					{
						postParsingError( m_oMetaLink.lineNumber(),
										  tr( "Could not parse invalid torrent URL: %1" ).arg( uri.m_pTorrent->toString() ) );
						delete uri.m_pTorrent;
						continue;
					}
				}
				else if ( !sMediaType.compare( "magnet" ) )
				{
					uri.m_nType = uriMagnet;
					uri.m_pMagnet = new CMagnet( m_oMetaLink.readElementText() );

					if ( !uri.m_pMagnet->isValid() )
					{
						postParsingError( m_oMetaLink.lineNumber(),
										  tr( "Could not parse magnet link: %1" ).arg( uri.m_pMagnet->toString() ) );
						delete uri.m_pMagnet;
						continue;
					}
				}
				else
				{
					postParsingError( m_oMetaLink.lineNumber(),
									  tr( "Found unknown media type \"%1\" within <metaurl> tag." ).arg( sMediaType ) );
					continue;
				}

				if ( vAttributes.hasAttribute( "priority" ) )
				{
					bool bOK;
					quint16 nPriority = vAttributes.value( "priority" ).toString().toUShort( &bOK );

					if ( bOK && nPriority < 256 )
					{
						uri.m_nPriority = nPriority;
					}
					else
					{
						postParsingError( m_oMetaLink.lineNumber(),
										  tr( "\"priority\" attribute within <metaurl> tag could not be parsed." ) );
					}
				}

				oCurrentFile.m_lURIs.append( uri );
			}
			else
			{
				postParsingError( m_oMetaLink.lineNumber(),
								  tr( "<metaurl> tag could not be parsed." ) );
				continue;
			}
		}
		else
		{
			QString e = tr( "Caught unexpected XML tag \"<%1>\" while parsing <file> in Metalink XML." );
			postParsingError( m_oMetaLink.lineNumber(), e.arg( sComp.toString() ) );
			continue;
		}
	}

	if ( oCurrentFile.isValid() )
	{
		lFiles.push_back( oCurrentFile );

		return true;
	}
	else
	{
		return false;
	}

}
