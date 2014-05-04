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

#include "querykeys.h"
#include <limits>
#include <QHostAddress>

#include "debug_new.h"

QueryKeys queryKeys;

QueryKeys::QueryKeys()
	: m_pTable( 0 ), m_nTable( 0 )
{
}

QueryKeys::~QueryKeys()
{
	delete [] m_pTable;
}

void QueryKeys::prepare()
{
	m_nTable = 1u << 16;
	m_pTable = new quint32[m_nTable];

	for ( uint i = 0; i < m_nTable; i++ )
	{
		m_pTable[i] = ( ( qrand() % std::numeric_limits<quint32>::max() ) << 16 ) | qrand() %
					  std::numeric_limits<quint32>::max();
	}
}

quint32 QueryKeys::create( QHostAddress pAddr )
{
	if ( !m_pTable )
	{
		prepare();
	}

	int nHash = qHash( pAddr ) % m_nTable;

	return m_pTable[nHash];
}
bool QueryKeys::check( QHostAddress pAddr, quint32 nKey )
{
	return ( nKey == create( pAddr ) );
}

