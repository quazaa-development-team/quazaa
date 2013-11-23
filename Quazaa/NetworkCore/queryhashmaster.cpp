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

#include "queryhashmaster.h"
#include "queryhashgroup.h"
#include "sharemanager.h"
#include <QDateTime>

#include "debug_new.h"

CQueryHashMaster QueryHashMaster;

CQueryHashMaster::CQueryHashMaster()
{
	m_nPerGroup = 0;
}

CQueryHashMaster::~CQueryHashMaster()
{
	Q_ASSERT(getCount() == 0);
}

void CQueryHashMaster::create()
{
	CQueryHashTable::create();

	m_nPerGroup			= 100;
	m_bValid			= false;
	m_bLive				= false;
	m_nCookie			= 0;
}

void CQueryHashMaster::add(CQueryHashTable* pTable)
{
	Q_ASSERT(m_nPerGroup > 0);
	Q_ASSERT(pTable != 0);
	Q_ASSERT(pTable->m_nHash > 0);
	Q_ASSERT(pTable->m_pGroup == 0);

	for(QList<CQueryHashGroup*>::iterator itGroup = m_pGroups.begin(); itGroup != m_pGroups.end(); itGroup++)
	{
		CQueryHashGroup* pGroup = *itGroup;

		if(pGroup->m_nHash == pTable->m_nHash &&
				pGroup->getCount() < m_nPerGroup)
		{
			pGroup->add(pTable);
			m_bValid = false;
			return;
		}
	}

	CQueryHashGroup* pGroup = new CQueryHashGroup(pTable->m_nHash);
	m_pGroups.append(pGroup);
	pGroup->add(pTable);
	m_bValid = false;
}

void CQueryHashMaster::remove(CQueryHashTable* pTable)
{
	Q_ASSERT(pTable != 0);
	if(pTable->m_pGroup == 0)
	{
		return;
	}

	CQueryHashGroup* pGroup = pTable->m_pGroup;
	pGroup->remove(pTable);

	if(pGroup->getCount() == 0)
	{
		int pos = m_pGroups.indexOf(pGroup);
		Q_ASSERT(pos >= 0);
		m_pGroups.removeAt(pos);
		delete pGroup;
	}

	m_bValid = false;
}

void CQueryHashMaster::build()
{
	quint32 tNow = time(0);

	if(m_bValid)
	{
		if(tNow - m_nCookie < 600)
		{
			return;
		}
	}
	else
	{
		if(tNow - m_nCookie < 20)
		{
			return;
		}
	}

	ShareManager.m_oSection.lock();

	const CQueryHashTable* pLocalTable = ShareManager.getHashTable();

	if(!pLocalTable)
	{
		ShareManager.m_oSection.unlock();
		return;
	}

	clear();
	merge(pLocalTable);

	ShareManager.m_oSection.unlock();

	for(QList<CQueryHashGroup*>::iterator itGroup = m_pGroups.begin(); itGroup != m_pGroups.end(); itGroup++)
	{
		CQueryHashGroup* pGroup = *itGroup;
		merge(pGroup);
	}

	m_bValid	= true;
	m_bLive		= true;
	m_nCookie	= tNow;
}

