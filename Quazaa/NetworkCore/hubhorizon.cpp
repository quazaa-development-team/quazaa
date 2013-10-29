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

#include "hubhorizon.h"
#include "g2packet.h"
#include "quazaasettings.h"

#include "debug_new.h"

// Portions of this code are borrowed from Shareaza

CHubHorizonPool HubHorizonPool;

CHubHorizonPool::CHubHorizonPool()
{
	m_pBuffer	= 0;
	m_nBuffer	= 0;
	m_pFree		= 0;
	m_pActive	= 0;
	m_nActive	= 0;
}

CHubHorizonPool::~CHubHorizonPool()
{
	if(m_pBuffer != 0)
	{
		delete [] m_pBuffer;
	}
}

void CHubHorizonPool::Setup()
{
	if(m_pBuffer != 0)
	{
		delete [] m_pBuffer;
	}

	m_nBuffer	= quazaaSettings.Gnutella2.HubHorizonSize;
	m_pBuffer	= new CHubHorizonHub[ m_nBuffer ];
	m_pActive	= 0;
	m_nActive	= 0;
	m_pFree		= m_pBuffer;

	for(quint32 nItem = 0 ; nItem < m_nBuffer ; nItem++)
	{
		m_pBuffer[ nItem ].m_pNext	= (nItem < m_nBuffer - 1)
									  ? &m_pBuffer[ nItem + 1 ] : 0;
	}
}

void CHubHorizonPool::Clear()
{
	m_pActive	= 0;
	m_nActive	= 0;
	m_pFree		= m_pBuffer;

	for(quint32 nItem = 0 ; nItem < m_nBuffer ; nItem++)
	{
		m_pBuffer[ nItem ].m_pNext	= (nItem < m_nBuffer - 1)
									  ? &m_pBuffer[ nItem + 1 ] : 0;
	}
}

CHubHorizonHub* CHubHorizonPool::Add(CEndPoint oAddress)
{
	CHubHorizonHub* pHub = m_pActive;
	for(; pHub ; pHub = pHub->m_pNext)
	{
		if(pHub->m_oAddress == oAddress)
		{
			pHub->m_nReference ++;
			return pHub;
		}
	}

	if(m_nActive == m_nBuffer || m_pFree == 0)
	{
		return 0;
	}

	pHub = m_pFree;
	m_pFree = m_pFree->m_pNext;

	pHub->m_pNext = m_pActive;
	m_pActive = pHub;
	m_nActive ++;

	pHub->m_oAddress	= oAddress;
	pHub->m_nReference	= 1;

	return pHub;
}

void CHubHorizonPool::Remove(CHubHorizonHub* pHub)
{
	CHubHorizonHub** ppPrev = &m_pActive;

	for(CHubHorizonHub* pSeek = *ppPrev ; pSeek ; pSeek = pSeek->m_pNext)
	{
		if(pHub == pSeek)
		{
			*ppPrev = pHub->m_pNext;
			pHub->m_pNext = m_pFree;
			m_pFree = pHub;
			m_nActive --;
			break;
		}

		ppPrev = &pSeek->m_pNext;
	}
}

CHubHorizonHub* CHubHorizonPool::Find(CEndPoint oAddress)
{
	for(CHubHorizonHub* pHub = m_pActive ; pHub ; pHub = pHub->m_pNext)
	{
		if(pHub->m_oAddress == oAddress)
		{
			return pHub;
		}
	}

	return 0;
}

int CHubHorizonPool::AddHorizonHubs(G2Packet* pPacket)
{
	int nCount = 0;

	for(CHubHorizonHub* pHub = m_pActive ; pHub ; pHub = pHub->m_pNext)
	{
		pPacket->WritePacket("S", (pHub->m_oAddress.protocol() == QAbstractSocket::IPv4Protocol ? 6 : 18));
		pPacket->WriteHostAddress(pHub->m_oAddress);

		nCount++;
	}

	return nCount;
}

CHubHorizonGroup::CHubHorizonGroup()
{
	m_pList		= 0;
	m_nCount	= 0;
	m_nBuffer	= 0;
}

CHubHorizonGroup::~CHubHorizonGroup()
{
	Clear();
	if(m_pList != 0)
	{
		delete [] m_pList;
	}
}

void CHubHorizonGroup::Add(CEndPoint oAddress)
{
	CHubHorizonHub** ppHub = m_pList;

	for(quint32 nCount = m_nCount ; nCount ; nCount--, ppHub++)
	{
		if((*ppHub)->m_oAddress == oAddress)
		{
			return;
		}
	}

	CHubHorizonHub* pHub = HubHorizonPool.Add(oAddress);
	if(pHub == 0)
	{
		return;
	}

	if(m_nCount == m_nBuffer)
	{
		m_nBuffer += 8;
		CHubHorizonHub** pList = new CHubHorizonHub*[ m_nBuffer ];
		if(m_nCount)
		{
			memcpy(pList, m_pList, sizeof(CHubHorizonHub*) * m_nCount);
		}

		if(m_pList)
		{
			delete [] m_pList;
		}

		m_pList = pList;
	}

	m_pList[ m_nCount++ ] = pHub;
}

void CHubHorizonGroup::Clear()
{
	CHubHorizonHub** ppHub = m_pList;

	for(quint32 nCount = m_nCount ; nCount ; nCount--, ppHub++)
	{
		if(-- ((*ppHub)->m_nReference) == 0)
		{
			HubHorizonPool.Remove(*ppHub);
		}
	}

	m_nCount = 0;
}

