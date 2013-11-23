/*
** datagramfrags.h
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

#ifndef DATAGRAMFRAGS_H
#define DATAGRAMFRAGS_H

#include "types.h"

class CBuffer;
class G2Packet;

class DatagramIn
{
protected:
	CEndPoint  m_oAddress;

	quint16 m_nSequence;
	quint8  m_nCount;
	quint8  m_nLeft;
	bool    m_bCompressed;
	quint32 m_tStarted;
	quint32 m_nBuffer;
	bool*   m_bLocked;

	CBuffer** m_pBuffer;
public:
	DatagramIn();
	~DatagramIn();

	void create(CEndPoint pHost, quint8 nFlags, quint16 nSequence, quint8 nCount);
	bool add(quint8 nPart, const void* pData, qint32 nLength);
	G2Packet* toG2Packet();


	friend class CDatagrams;
};

class DatagramWatcher;

class DatagramOut
{
protected:
	CEndPoint   m_oAddress;

	quint16     m_nSequence;
	bool        m_bCompressed;
	quint32     m_nPacket;
	quint8      m_nCount;
	quint8      m_nAcked;
	quint32*    m_pLocked;
	quint8      m_nLocked;
	quint32     m_tSent;
	bool        m_bAck;

	DatagramWatcher*    m_pWatcher;
	void*               m_pParam;
	CBuffer* m_pBuffer;

public:
	DatagramOut();
	~DatagramOut();

	void create(CEndPoint oAddr, G2Packet* pPacket, quint16 nSequence, CBuffer* pBuffer, bool bAck = false);
	bool getPacket(quint32 tNow, char** ppPacket, quint32* pnPacket, bool bResend = false);
	bool acknowledge(quint8 nPart);

	friend class CDatagrams;

};

#endif // DATAGRAMFRAGS_H
