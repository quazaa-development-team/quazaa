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

#include "queryhashgroup.h"
#include "queryhashmaster.h"
#include "quazaasettings.h"

#include "debug_new.h"

QueryHashGroup::QueryHashGroup( quint32 nHash )
{
	m_nHash = nHash ? nHash : 1u << quazaaSettings.Library.QueryRouteSize;
	m_pHash = new uchar[ m_nHash ];
	memset( m_pHash, 0, m_nHash );
	m_nCount = 0;
}

QueryHashGroup::~QueryHashGroup()
{
#ifdef _DEBUG
	uchar* pTest = m_pHash;

	for ( quint32 nHash = m_nHash ; nHash ; --nHash )
	{
		Q_ASSERT( *pTest++ == 0 );
	}
#endif

	delete [] m_pHash;
}

void QueryHashGroup::add( QueryHashTable* pTable )
{
	Q_ASSERT( pTable != 0 );
	Q_ASSERT( pTable->m_pGroup == 0 );
	Q_ASSERT( m_pTables.indexOf( pTable ) == -1 );

	pTable->m_pGroup = this;
	m_pTables.append( pTable );

	operate( pTable, true );
	queryHashMaster.invalidate();
}

void QueryHashGroup::remove( QueryHashTable* pTable )
{
	Q_ASSERT( pTable != 0 );
	Q_ASSERT( pTable->m_pGroup == this );

	int pos = m_pTables.indexOf( pTable );
	Q_ASSERT( pos != -1 );

	m_pTables.removeAt( pos );
	pTable->m_pGroup = 0;

	operate( pTable, false );
	queryHashMaster.invalidate();
}

void QueryHashGroup::operate( QueryHashTable* pTable, bool bAdd )
{
	Q_ASSERT( m_pHash != 0 );
	Q_ASSERT( pTable->m_nHash == m_nHash );

	uchar* pSource = pTable->m_pHash;
	uchar* pTarget = m_pHash;

	if ( bAdd )
	{
		for ( quint32 nHash = m_nHash >> 3 ; nHash ; --nHash )
		{
			register uchar nSource = *pSource++;

			if ( ( nSource & 0x01 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x02 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x04 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x08 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x10 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x20 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x40 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x80 ) == 0 )
			{
				( *pTarget++ ) ++;
			}
			else
			{
				pTarget++;
			}
		}
	}
	else
	{
		for ( quint32 nHash = m_nHash >> 3 ; nHash ; --nHash )
		{
			register uchar nSource = *pSource++;

			if ( ( nSource & 0x01 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x02 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x04 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x08 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x10 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x20 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x40 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
			if ( ( nSource & 0x80 ) == 0 )
			{
				( *pTarget++ ) --;
			}
			else
			{
				pTarget++;
			}
		}
	}
}

