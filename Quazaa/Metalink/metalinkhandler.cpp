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

#include "metalinkhandler.h"
#include "systemlog.h"
#include "debug_new.h"

using namespace URI;

MetaFile::MetaFile() :
	m_nID( 0 ),
	m_nFileSize( 0 )
{}

MetaFile::MetaFile( quint16 ID ) :
	m_nID( ID ),
	m_nFileSize( 0 )
{}

bool MetaFile::isValid() const
{
	return !m_vHashes.empty() || m_lURIs.size();
}

QFile& MetalinkHandler::m_oEmptyQFile = getEmptyStaticFile();

MetalinkHandler::MetalinkHandler( QFile& oFile ) :
	m_bNull( true ),
	m_bValid( false ),
	m_nSize( 0 )
{
	if ( oFile.open( QIODevice::ReadOnly ) )
	{
		m_oMetaLink.setDevice( &oFile );
		m_bNull = false;
	}

	m_nParsingState = m_oMetaLink.error();
}

MetalinkHandler::~MetalinkHandler()
{}

QList<Download*> MetalinkHandler::files() const
{
	QList<Download*> result;

	for ( unsigned int i = 0; i < m_nSize; ++i )
	{
		Download* pFile = file( i );
		if ( pFile )
		{
			result.append( pFile );
		}
	}

	return result;
}

void MetalinkHandler::postParsingError( const int line, const QString sError ) const
{
	QString error = tr( "Metalink: " );
	error += tr( "Error while parsing XML (line %1): " ).arg( line );
	error += sError;

	systemLog.postLog( LogSeverity::Error, error );
}

void MetalinkHandler::postParsingInfo( const int line, const QString sInfo ) const
{
	QString info = tr( "Metalink: " );
	info += tr( "Line %1: " ).arg( line );
	info += sInfo;

	systemLog.postLog( LogSeverity::Debug, info );
}

QFile& MetalinkHandler::getEmptyStaticFile()
{
	static QFile s_file( NULL );
	return s_file;
}

