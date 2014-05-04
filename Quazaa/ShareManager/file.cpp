/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2013.
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

#include <QDateTime>
#include <QFileInfo>

#include "file.h"
#include "debug_new.h"

File::File( File& file ) :
	QObject( file.parent() ),
	QFileInfo( file.absoluteFilePath() ),
	m_nDirectoryID( file.m_nDirectoryID ),
	m_nFileID( file.m_nFileID ),
	m_bNull( file.m_bNull ),
	m_vHashes( HashSet( file.m_vHashes ) ),
	m_Tags( file.m_Tags )
{
	m_pFile = file.m_pFile ? new QFile( file.absoluteFilePath() ) : NULL;
}

File::File( QObject* parent ) :
	QObject( parent ),
	QFileInfo(),
	m_nDirectoryID( 0 ),
	m_nFileID( 0 ),
	m_pFile( NULL ),
	m_bNull( !parent )
{
}

File::File( const QString& file, QObject* parent ) :
	QObject( parent ),
	QFileInfo( file ),
	m_nDirectoryID( 0 ),
	m_nFileID( 0 ),
	m_pFile( NULL ),
	m_bNull( false )
{
	refresh();
}

File::File( const QFile& file, QObject* parent ) :
	QObject( parent ),
	QFileInfo( file ),
	m_nDirectoryID( 0 ),
	m_nFileID( 0 ),
	m_pFile( NULL ),
	m_bNull( false )
{
	refresh();
}

File::File( const QDir& dir, const QString& file, QObject* parent ) :
	QObject( parent ),
	QFileInfo( dir, file ),
	m_nDirectoryID( 0 ),
	m_nFileID( 0 ),
	m_pFile( NULL ),
	m_bNull( false )
{
	refresh();
}

File::File( const QFileInfo& fileinfo, QObject* parent ) :
	QObject( parent ),
	QFileInfo( fileinfo ),
	m_nDirectoryID( 0 ),
	m_nFileID( 0 ),
	m_pFile( NULL ),
	m_bNull( false )
{
	refresh();
}

void File::refresh()
{
	if ( m_pFile )
	{
		delete m_pFile;
	}
	m_pFile = NULL;

	if ( exists() )
	{
		setTag( "physical" ); // Tag the file as being physically existant on HDD.
		m_pFile = new QFile( filePath() );
	}
}

bool File::removeHash(const Hash& rHash )
{
	return m_vHashes.remove( rHash );
}

// todo: implement this
QString File::toURI( URIType /*type*/ ) const
{
	return QString();
}

bool File::isTagged( const QString& sTag ) const
{
	QSet< QString >::ConstIterator i = m_Tags.find( sTag );

	if ( i != m_Tags.end() )
	{
		return true;
	}

	return false;
}

