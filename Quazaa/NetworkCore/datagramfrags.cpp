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

#include "datagramfrags.h"
#include "buffer.h"
#include "g2packet.h"
#include "zlibutils.h"
#include "datagrams.h"

#include "quazaasettings.h"

#include "debug_new.h"

#include <stdexcept>

DatagramIn::DatagramIn()
{
	m_pBuffer = 0;
	m_bLocked = 0;
	m_nBuffer = 0;
}

DatagramIn::~DatagramIn()
{
	if(m_pBuffer)
	{
		delete[] m_pBuffer;
	}
	if(m_bLocked)
	{
		delete[] m_bLocked;
	}
}

void DatagramIn::create(CEndPoint pHost, quint8 nFlags, quint16 nSequence, quint8 nCount)
{
	m_oAddress = pHost;

	m_nSequence = nSequence;
	m_bCompressed = (nFlags & 0x01) ? true : false;
	m_nCount = nCount;
	m_nLeft = nCount;

	m_tStarted = time(0);

	if(m_nBuffer < m_nCount)
	{
		if(m_pBuffer)
		{
			delete[] m_pBuffer;
		}
		if(m_bLocked)
		{
			delete[] m_bLocked;
		}

		m_nBuffer = nCount;
		m_pBuffer = new CBuffer*[nCount];
		m_bLocked = new bool[nCount];
	}

	memset(m_bLocked, 0x00, sizeof(bool) * m_nBuffer);
	memset(m_pBuffer, 0x00, sizeof(CBuffer*) * m_nBuffer);
}

bool DatagramIn::add(quint8 nPart, const void* pData, qint32 nLength)
{
	if(nPart < 1 || nPart > m_nCount)
	{
		return false;
	}

	if(m_nLeft == 0)
	{
		return false;
	}

	if(m_bLocked[nPart - 1] == false)
	{
		m_bLocked[nPart - 1] = true;
		m_pBuffer[nPart - 1]->append((char*)pData, nLength);

		if(--m_nLeft == 0)
		{
			return true;
		}
	}

	return false;
}

G2Packet* DatagramIn::toG2Packet()
{
	if(m_nCount > 1)
		for(qint32 i = 1; i < m_nCount; i++)
			m_pBuffer[0]->append(m_pBuffer[i]);


	if(m_bCompressed && !ZLibUtils::uncompressBuffer(*m_pBuffer[0]))
	{
		throw std::logic_error("Unable to uncompress compressed packet.");
	}

	return G2Packet::readBuffer(m_pBuffer[0]);
}

DatagramOut::DatagramOut()
{
	m_pLocked = 0;
	m_pBuffer = 0;
	m_nLocked = 0;
	m_bAck = false;
}

DatagramOut::~DatagramOut()
{
	if(m_pLocked)
	{
		delete[] m_pLocked;
	}
}

void DatagramOut::create(CEndPoint oAddr, G2Packet* pPacket, quint16 nSequence, CBuffer* pBuffer, bool bAck)
{
	Q_ASSERT(m_pBuffer == 0);

	m_oAddress = oAddr;
	m_nSequence = nSequence;
	m_pBuffer = pBuffer;

	pPacket->toBuffer(m_pBuffer);

	m_bCompressed = ZLibUtils::compressBuffer(*m_pBuffer, true);

	m_nPacket = quazaaSettings.Gnutella2.UdpMTU;
	m_nCount = quint8((m_pBuffer->size() + m_nPacket - 1) / m_nPacket);
	m_nAcked = m_nCount;

	GND_HEADER pHeader;
	strncpy(&pHeader.szTag[0], "GND", 3);
	pHeader.nFlags = (m_bCompressed ? 0x01 : 0);
	m_bAck = bAck;
	if(m_bAck)
	{
		pHeader.nFlags |= 0x02;
	}
	pHeader.nSequence = m_nSequence;
	pHeader.nCount = m_nCount;

	quint32 nOffset = 0;
	quint32 nPacket = m_nPacket + sizeof(GND_HEADER);

	for(quint8 nPart = 0; nPart < m_nCount; nPart++, nOffset += nPacket)
	{
		pHeader.nPart = nPart + 1;
		m_pBuffer->insert(nOffset, (char*)&pHeader, sizeof(pHeader));
	}

	if(m_nLocked < m_nCount)
	{
		if(m_pLocked)
		{
			delete[] m_pLocked;
		}
		m_nLocked = m_nCount;
		m_pLocked = new quint32[m_nLocked];
	}

	memset(m_pLocked, 0x00, sizeof(quint32) * m_nLocked);

	m_tSent = time(0);
}

bool DatagramOut::getPacket(quint32 tNow, char** ppPacket, quint32* pnPacket, bool bResend)
{
	Q_ASSERT(m_pBuffer != 0);

	int nPart = 0;
	for(; nPart < m_nCount; nPart++)
	{
		if(m_pLocked[nPart] < 0xFFFFFFFF)
		{
			if(bResend)
			{
				if(tNow - m_pLocked[nPart] >= quazaaSettings.Gnutella2.UdpOutResend)
				{
					break;
				}
			}
			else
			{
				if(m_pLocked[nPart] == 0)
				{
					break;
				}
			}
		}
	}

	if(nPart >= m_nCount)
	{
		return false;
	}

	m_pLocked[nPart] = bResend ? tNow : 0xFFFFFFFF;

	quint32 nPacket = m_nPacket + sizeof(GND_HEADER);
	*ppPacket = m_pBuffer->data() + (nPart * nPacket);
	*pnPacket = qMin(nPacket, m_pBuffer->size() - (nPart * nPacket));

	return true;
}

bool DatagramOut::acknowledge(quint8 nPart)
{
	if(nPart > 0 && nPart <= m_nCount && m_nAcked > 0)
	{
		if(m_pLocked[ nPart - 1 ] < 0xFFFFFFFF)
		{
			m_pLocked[ nPart - 1 ] = 0xFFFFFFFF;

			if(--m_nAcked == 0)
			{
				return true;
			}
		}
	}

	return false;
}
