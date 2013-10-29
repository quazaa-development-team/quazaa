/*
** g2packet.h
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

#ifndef G2PACKET_H
#define G2PACKET_H

#include "types.h"
#include <QtGlobal>
#include <QMutex>
#include <QList>

struct packet_error {};
struct packet_read_past_end {};

class CBuffer;

class G2Packet
{
	// Construction
protected:
	G2Packet();
	~G2Packet();

public:
	static G2Packet* New(const char* pszType = 0, bool bCompound = false);
	static G2Packet* New(char* pSource);

	// Attributes
public:
	G2Packet* 	m_pNext;
	quint32		m_nReference;

public:
	uchar*		m_pBuffer;
	quint32		m_nBuffer;
	quint32		m_nLength;
	quint32		m_nPosition;
	char		m_sType[9];
	bool		m_bCompound; // true: packet contains child packet(s); false: otherwise. (CF bit)

	enum { seekStart, seekEnd };

	// Operations
public:
	void	Reset();
	void	Seek(quint32 nPosition, int nRelative = seekStart);
	uchar* 	WriteGetPointer(quint32 nLength, quint32 nOffset = 0xFFFFFFFF);
public:
	char* 	GetType() const;

public:
	G2Packet* 	WritePacket(G2Packet* pPacket);
	G2Packet* 	WritePacket(const char* pszType, quint32 nLength, bool bCompound = false);
	G2Packet*	PrependPacket(G2Packet* pPacket, bool bRelease = true);
	bool	ReadPacket(char* pszType, quint32& nLength, bool* pbCompound = 0);
	bool	SkipCompound();
	bool	SkipCompound(quint32& nLength, quint32 nRemaining = 0);
	bool	GetTo(QUuid& pGUID);

public:
	static	G2Packet* ReadBuffer(CBuffer* pBuffer);
	void	ToBuffer(CBuffer* pBuffer) const;

	// Inline Packet Operations
	inline bool IsType(const char* sType);
	inline int GetRemaining();
	inline bool Ensure(quint32 nBytes);
	inline void Read(void* pData, int nLength);
	inline void Write(void* pData, int nLength);
	template <typename T>
	inline T ReadIntBE();
	template <typename T>
	inline T ReadIntLE();
	template <typename T>
	inline void WriteIntBE(T nValue);
	template <typename T>
	inline void WriteIntLE(T nValue);
	inline uchar ReadByte();
	inline void WriteByte(uchar nByte);
	inline void ReadHostAddress(CEndPoint* pDest, bool bIP4 = true);
	inline QUuid ReadGUID();
	inline void WriteHostAddress(const CEndPoint& src);
	inline void WriteGUID(QUuid& guid);

public:
	QString ReadString(quint32 nMaximum = 0xFFFFFFFF);
	void WriteString(QString sToWrite, bool bTerminate = false);
	G2Packet* AddOrReplaceChild(const char* pFind, G2Packet* pReplacement, bool bRelease = true, bool bPreserveExtensions = true);
	// Inline Allocation

public:
	inline void AddRef();
	inline void Release();
	inline void ReleaseChain();
	void Delete();

	QString ToHex() const;
	QString ToASCII() const;
	QString Dump() const;

protected:
	friend class G2PacketPool;
};

#define G2_FLAG_COMPOUND	0x04
#define G2_FLAG_BIG_ENDIAN	0x02

class G2PacketPool
{
	// Construction
public:
	G2PacketPool();
	~G2PacketPool();

	// Attributes
protected:
	G2Packet* 	m_pFree;
	quint32		m_nFree;
protected:
	QMutex				m_pSection;
	QList<G2Packet*>	m_pPools;

	// Operations
protected:
	void	Clear();
	void	NewPool();

	// Inlines
public:
	inline G2Packet* New();
	inline void Delete(G2Packet* pPacket);

};

// Inlines impl

// G2Packet
bool G2Packet::IsType(const char* sType)
{
	return strcmp(sType, m_sType) == 0;
}
int G2Packet::GetRemaining()
{
	return m_nLength - m_nPosition;
}
bool G2Packet::Ensure(quint32 nBytes)
{
	if(m_nLength + nBytes > m_nBuffer)
	{
		m_nBuffer += qMax(nBytes, 128u);
		m_pBuffer = (uchar*)realloc(m_pBuffer, m_nBuffer);

		if(!m_pBuffer)
		{
			return false;
		}
	}

	return true;
}
void G2Packet::Read(void* pData, int nLength)
{
	if(m_nPosition + nLength > m_nLength)
	{
		throw packet_read_past_end();
	}
	memcpy(pData, m_pBuffer + m_nPosition, nLength);
	m_nPosition += nLength;
}
void G2Packet::Write(void* pData, int nLength)
{
	Ensure(nLength);

	memcpy(m_pBuffer + m_nLength, pData, nLength);
	m_nLength += nLength;
}
template <typename T>
T G2Packet::ReadIntBE()
{
	if(m_nLength - m_nPosition < sizeof(T))
	{
		throw packet_read_past_end();
	}

	T nRet = qFromBigEndian(*(T*)(m_pBuffer + m_nPosition));
	m_nPosition += sizeof(T);
	return nRet;
}
template <typename T>
T G2Packet::ReadIntLE()
{
	if(m_nLength - m_nPosition < sizeof(T))
	{
		throw packet_read_past_end();
	}

	T nRet = qFromLittleEndian(*(T*)(m_pBuffer + m_nPosition));
	m_nPosition += sizeof(T);
	return nRet;
}
template <typename T>
void G2Packet::WriteIntBE(T nValue)
{
	nValue = qToBigEndian(nValue);
	Write(reinterpret_cast<void*>(&nValue), sizeof(T));
}
template <typename T>
void G2Packet::WriteIntLE(T nValue)
{
	nValue = qToLittleEndian(nValue);
	Write(reinterpret_cast<void*>(&nValue), sizeof(T));
}
uchar G2Packet::ReadByte()
{
	uchar nRet;
	Read(&nRet, 1);
	return nRet;
}
void G2Packet::WriteByte(uchar nByte)
{
	Write(&nByte, 1);
}
void G2Packet::ReadHostAddress(CEndPoint* pDest, bool bIP4)
{
	if(bIP4)
	{
		quint32 nIP;
		quint16 nPort;
		nIP = ReadIntBE<quint32>();
		nPort = ReadIntLE<quint16>();
		pDest->setAddress(nIP);
		pDest->setPort(nPort);
	}
	else
	{
		Q_IPV6ADDR ip6;
		quint16 nPort;
		Read(&ip6, 16);
		nPort = ReadIntLE<quint16>();
		pDest->setAddress(ip6);
		pDest->setPort(nPort);
	}
}

QUuid G2Packet::ReadGUID()
{
	QUuid ret;
	ret.data1 = ReadIntLE<uint>();
	ret.data2 = ReadIntLE<ushort>();
	ret.data3 = ReadIntLE<ushort>();
	Read(&ret.data4[0], 8);

	return ret;
}
void G2Packet::WriteHostAddress(const CEndPoint& src)
{
	if(src.protocol() == 0)   // IPv4
	{
		Ensure(6);
		WriteIntBE(src.toIPv4Address());
		WriteIntLE(src.port());
	}
	else
	{
		Ensure(18);
		Q_IPV6ADDR ip6 = src.toIPv6Address();
		Write(&ip6, 16);
		WriteIntLE(src.port());
	}
}
void G2Packet::WriteGUID(QUuid& guid)
{
	Ensure(16);
	WriteIntLE(guid.data1);
	WriteIntLE(guid.data2);
	WriteIntLE(guid.data3);
	Write(&guid.data4[0], 8);
}
void G2Packet::AddRef()
{
	m_nReference++;
}
void G2Packet::Release()
{
	if(this != NULL && ! --m_nReference)
	{
		Delete();
	}
}
void G2Packet::ReleaseChain()
{
	if(this == NULL)
	{
		return;
	}

	for(G2Packet* pPacket = this ; pPacket ;)
	{
		G2Packet* pNext = pPacket->m_pNext;
		pPacket->Release();
		pPacket = pNext;
	}
}


// G2PacketPool
G2Packet* G2PacketPool::New()
{
	m_pSection.lock();

	if(m_nFree == 0)
	{
		NewPool();
	}
	Q_ASSERT(m_nFree > 0);

	G2Packet* pPacket = m_pFree;
	m_pFree = m_pFree->m_pNext;
	m_nFree --;

	m_pSection.unlock();

	pPacket->Reset();
	pPacket->AddRef();

	return pPacket;
}
void G2PacketPool::Delete(G2Packet *pPacket)
{
	Q_ASSERT(pPacket != NULL);
	Q_ASSERT(pPacket->m_nReference == 0);

	m_pSection.lock();

	pPacket->m_pNext = m_pFree;
	m_pFree = pPacket;
	m_nFree ++;

	m_pSection.unlock();
}

extern G2PacketPool G2Packets;

#endif // G2PACKET_H
