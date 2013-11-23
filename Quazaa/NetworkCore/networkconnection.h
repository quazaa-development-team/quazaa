/*
** networkconection.h
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

#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include "types.h"
#include <QElapsedTimer>
#include <QObject>

class QByteArray;
class QTcpSocket;
class QThread;

#include "buffer.h"

class TCPBandwidthMeter
{
public:
	QElapsedTimer	m_tTime;

	quint32 m_pSlots[20]; // 4slots / 1sec
	quint32 m_nCurrentSlot;
	quint64 m_nTotal;

public:
	TCPBandwidthMeter();

	void	Add(quint32 nBytes);
	quint32	AvgUsage();
	quint32 Usage();

};

class CNetworkConnection : public QObject
{
	Q_OBJECT
public:
	QTcpSocket* m_pSocket;  // Socket ;)

	// Host Address
	CEndPoint   m_oAddress;

	// Buffer I/O
	CBuffer* m_pInput;
	CBuffer* m_pOutput;

	bool    m_bInitiated;
	bool    m_bConnected;
	qint32  m_tConnected;

public:
	CNetworkConnection(QObject* parent = 0);
	virtual ~CNetworkConnection();
	void moveToThread(QThread* thread);

public:
	virtual void connectTo(CEndPoint oAddress);
	virtual void attachTo(CNetworkConnection* pOther);
	virtual void acceptFrom(qintptr nHandle);
	virtual void close(bool bDelayed = false);

private:
	Q_INVOKABLE void closeImplementation(bool bDelayed);

public:
	void write(QByteArray& baData)
	{
		write(baData.constData(), baData.size());
	}

	inline void write(const char* szData, quint32 nLength)
	{
		writeData(szData, nLength);
	}

	inline qint64 read(char* pData, qint64 nMaxSize = 0)
	{
		return readData(pData, nMaxSize);
	}

	QByteArray read(qint64 nMaxSize = 0);
	QByteArray peek(qint64 nMaxLength = 0);

protected:
	virtual qint64 readFromNetwork(qint64 nBytes);
	virtual qint64 writeToNetwork(qint64 nBytes);

protected:
	virtual qint64 readData(char* data, qint64 maxlen);
	virtual qint64 writeData(const char* data, qint64 len);

	void initializeSocket();

public:

	qint64 bytesAvailable();
	qint64 bytesToWrite() const;
	qint64 networkBytesAvailable() const;
	bool isValid() const;
	void setReadBufferSize(qint64 nSize);

	inline CEndPoint address() const
	{
		return m_oAddress;
	}

	inline virtual bool hasData()
	{
		if(!m_pSocket)
		{
			return false;
		}

		if(m_pInput && !m_pInput->isEmpty())
		{
			return true;
		}
		if(m_pOutput && !m_pOutput->isEmpty())
		{
			return true;
		}
		if(networkBytesAvailable())
		{
			return true;
		}

		return false;
	}

	inline virtual CBuffer* getInputBuffer()
	{
		Q_ASSERT(m_pInput != 0);

		return m_pInput;
	}
	inline virtual CBuffer* getOutputBuffer()
	{
		Q_ASSERT(m_pOutput != 0);

		return m_pOutput;
	}
signals:
	void connected();
	void readyRead();
	void disconnected();
	void error(QAbstractSocket::SocketError);
	void bytesWritten(qint64);
	void stateChanged(QAbstractSocket::SocketState);
	void aboutToClose();
	void readyToTransfer();

public slots:
	void onDisconnectInt();
	void onErrorInt(QAbstractSocket::SocketError e);

public slots:
	virtual void onConnectNode() = 0;
	virtual void onDisconnectNode() = 0;
	virtual void onRead() = 0;
	virtual void onError(QAbstractSocket::SocketError e) = 0;
	virtual void onStateChange(QAbstractSocket::SocketState s);

public:
	TCPBandwidthMeter m_mInput;
	TCPBandwidthMeter m_mOutput;

	friend class CRateController;
};


#endif // NETWORKCONNECTION_H
