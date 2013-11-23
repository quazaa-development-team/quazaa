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

#include "networkconnection.h"

#include <QTcpSocket>
#include <QMetaType>

#include "buffer.h"

#include "debug_new.h"

CNetworkConnection::CNetworkConnection(QObject* parent)
	: QObject(parent)
{
	static int dummy = qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
	static int dummy2 = qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
	Q_UNUSED(dummy);
	Q_UNUSED(dummy2);

	//qDebug() << "CNetworkConnection constructor";

	m_pSocket = 0;

	m_pInput = 0;
	m_pOutput = 0;

	m_bInitiated = false;
	m_bConnected = false;
	m_tConnected = 0;
}
CNetworkConnection::~CNetworkConnection()
{
	if(m_pInput)
	{
		delete m_pInput;
	}
	if(m_pOutput)
	{
		delete m_pOutput;
	}

	if(m_pSocket)
	{
		delete m_pSocket;
	}
}

void CNetworkConnection::connectTo(CEndPoint oAddress)
{
	m_oAddress = oAddress;

	m_bInitiated = true;
	m_bConnected = false;
	m_tConnected = time(0);

	Q_ASSERT(m_pInput == 0);
	Q_ASSERT(m_pOutput == 0);
	m_pInput = new CBuffer(8192);
	m_pOutput = new CBuffer(8192);
	Q_ASSERT(m_pSocket == 0);

	m_pSocket = new QTcpSocket();
	initializeSocket();
	m_pSocket->connectToHost(oAddress, oAddress.port());

}

void CNetworkConnection::attachTo(CNetworkConnection* pOther)
{
	Q_ASSERT(m_pSocket == 0);
	m_pSocket = pOther->m_pSocket;
	pOther->m_pSocket = 0;

	m_bConnected = pOther->m_bConnected;
	m_tConnected = pOther->m_tConnected;
	m_bInitiated = pOther->m_bInitiated;

	Q_ASSERT(m_pInput == 0);
	Q_ASSERT(m_pOutput == 0);
	m_pInput = pOther->m_pInput;
	m_pOutput = pOther->m_pOutput;
	pOther->m_pInput = pOther->m_pOutput = 0;

	if(m_pSocket->peerAddress().protocol() == 0)
	{
		m_oAddress.setAddress(m_pSocket->peerAddress().toIPv4Address());
	}
	else
	{
		m_oAddress.setAddress(m_pSocket->peerAddress().toIPv6Address());
	}
	m_oAddress.setPort(m_pSocket->peerPort());

	initializeSocket();

	emit readyRead();
}

void CNetworkConnection::acceptFrom(qintptr nHandle)
{
	Q_ASSERT(m_pSocket == 0);
	m_pSocket = new QTcpSocket();

	initializeSocket();

	m_bInitiated = false;
	m_bConnected = true;
	m_tConnected = time(0);

	Q_ASSERT(m_pInput == 0);
	Q_ASSERT(m_pOutput == 0);
	m_pInput = new CBuffer(8192);
	m_pOutput = new CBuffer(8192);

	m_pSocket->setSocketDescriptor(nHandle);
	if(m_pSocket->peerAddress().protocol() == 0)
	{
		m_oAddress.setAddress(m_pSocket->peerAddress().toIPv4Address());
	}
	else
	{
		m_oAddress.setAddress(m_pSocket->peerAddress().toIPv6Address());
	}
	m_oAddress.setPort(m_pSocket->peerPort());

	emit readyToTransfer();

}

void CNetworkConnection::close(bool bDelayed)
{
	systemLog.postLog( LogSeverity::Information, Components::Network,
					   "Closing connection to %s.", qPrintable( m_oAddress.toStringWithPort() ) );

	QMetaObject::invokeMethod(this, "closeImplementation", Q_ARG(bool, bDelayed));
}

void CNetworkConnection::closeImplementation(bool bDelayed)
{
	if( bDelayed )
	{
		if(!getOutputBuffer()->isEmpty() || !m_pOutput->isEmpty())
		{
			writeToNetwork(m_pOutput->size() + getOutputBuffer()->size());
			m_pSocket->disconnectFromHost();
			return;
		}
	}

	m_pSocket->abort();
	emit disconnected();
}
void CNetworkConnection::moveToThread(QThread* thread)
{
	if(m_pSocket)
	{
		m_pSocket->moveToThread(thread);
	}
	QObject::moveToThread(thread);
}

void CNetworkConnection::initializeSocket()
{
	m_pSocket->disconnect();

	connect(m_pSocket, SIGNAL(connected()),
			this, SIGNAL(connected()));
	connect(m_pSocket, SIGNAL(connected()),
			this, SIGNAL(readyToTransfer()));
	connect(m_pSocket, SIGNAL(readyRead()),
			this, SIGNAL(readyToTransfer()));
	connect(m_pSocket, SIGNAL(disconnected()),
			this, SLOT(onDisconnectInt()));
	connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(onErrorInt(QAbstractSocket::SocketError)));
	connect(m_pSocket, SIGNAL(bytesWritten(qint64)),
			this, SIGNAL(bytesWritten(qint64)));
	connect(m_pSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			this, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
	connect(m_pSocket, SIGNAL(aboutToClose()),
			this, SIGNAL(aboutToClose()));

	connect(this, SIGNAL(connected()), this, SLOT(onConnectNode()), Qt::QueuedConnection);
	connect(this, SIGNAL(disconnected()), this, SLOT(onDisconnectNode()), Qt::QueuedConnection);
	connect(this, SIGNAL(readyRead()), this, SLOT(onRead()), Qt::QueuedConnection);
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
	connect(this, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChange(QAbstractSocket::SocketState)), Qt::QueuedConnection);

}

qint64 CNetworkConnection::readFromNetwork(qint64 nBytes)
{
	Q_ASSERT(m_pInput != 0);
	Q_ASSERT(nBytes >= 0);

	int nOldSize = m_pInput->size();
	qint64 nBytesRead = 0;

	if(m_pSocket->state() != QTcpSocket::ConnectedState)
	{
		m_pInput->append(m_pSocket->readAll());
		nBytesRead += m_pInput->size() - nOldSize;
	}
	else
	{
		m_pInput->resize(nOldSize + nBytes);
		nBytesRead = m_pSocket->read(m_pInput->data() + nOldSize, nBytes);
		m_pInput->resize((nBytesRead <= 0 ? nOldSize : nOldSize + nBytesRead));
	}

	if(nBytesRead > 0)
	{
		m_mInput.Add(nBytesRead);
		emit readyRead();
	}

	return nBytesRead;
}
qint64 CNetworkConnection::writeToNetwork(qint64 nBytes)
{
	Q_ASSERT(m_pOutput != 0);
	Q_ASSERT(nBytes >= 0);

	qint64 nBytesWritten = m_pSocket->write(m_pOutput->data(), qMin((qint64)m_pOutput->size(), nBytes));

	if(nBytesWritten <= 0)
	{
		return nBytesWritten;
	}

	m_pOutput->remove(0, nBytesWritten);
	m_mOutput.Add(nBytesWritten);

	return nBytesWritten;
}
qint64 CNetworkConnection::readData(char* data, qint64 maxlen)
{
	Q_ASSERT(m_pInput != 0);

	int nBytesRead = qMin<qint64>(maxlen, m_pInput->size());
	memcpy(data, m_pInput->data(), nBytesRead);
	m_pInput->remove(0, nBytesRead);

	if(m_pSocket->state() != QTcpSocket::ConnectedState)
	{
		m_pInput->append(m_pSocket->readAll());
	}

	return (qint64)nBytesRead;
}

qint64 CNetworkConnection::writeData(const char* data, qint64 len)
{
	Q_ASSERT(m_pOutput != 0);

	int nOldSize = m_pOutput->size();
	m_pOutput->resize(nOldSize + len);
	memcpy(m_pOutput->data() + nOldSize, data, len);
	emit readyToTransfer();
	return len;
}

qint64 CNetworkConnection::bytesAvailable()
{
	Q_ASSERT(m_pInput != 0);

	if(m_pSocket->state() != QTcpSocket::ConnectedState)
	{
		m_pInput->append(m_pSocket->readAll());
	}

	return m_pInput->size();
}

qint64 CNetworkConnection::bytesToWrite() const
{
	if(!m_pSocket)
	{
		return 0;
	}

	return m_pSocket->bytesToWrite();
}
qint64 CNetworkConnection::networkBytesAvailable() const
{
	if(!isValid())
	{
		return 0;
	}

	if(m_pSocket->state() != QTcpSocket::ConnectedState)
	{
		return m_pInput->size();
	}

	return m_pSocket->bytesAvailable();
}
bool CNetworkConnection::isValid() const
{
	if(m_pSocket == 0 || m_pInput == 0 || m_pOutput == 0)
	{
		return false;
	}

	return m_pSocket->isValid();
}
void CNetworkConnection::setReadBufferSize(qint64 nSize)
{
	Q_ASSERT(m_pSocket != 0);

	m_pSocket->setReadBufferSize(nSize);
}

QByteArray CNetworkConnection::read(qint64 nMaxSize)
{
	QByteArray baRet;

	baRet.resize(qMin<qint64>(nMaxSize, getInputBuffer()->size()));
	readData(baRet.data(), baRet.size());
	return baRet;
}

QByteArray CNetworkConnection::peek(qint64 nMaxLength)
{
	CBuffer* pBuffer = getInputBuffer();

	return QByteArray::fromRawData(pBuffer->data(), qMin<qint64>(nMaxLength > 0 ? nMaxLength : pBuffer->size(), pBuffer->size()));
}

TCPBandwidthMeter::TCPBandwidthMeter()
{
	memset(&m_pSlots[0], 0, sizeof(m_pSlots));
	m_nCurrentSlot = m_nTotal = 0;
	m_tTime.start();
}

void TCPBandwidthMeter::Add(quint32 nBytes)
{
	if(m_tTime.hasExpired(250))
	{
		int nSlotsToClear = qMin(20ll, m_tTime.elapsed() / 250ll);

		for(; nSlotsToClear; nSlotsToClear--)
		{
			m_nCurrentSlot = (m_nCurrentSlot + 1) % 20;
			m_pSlots[m_nCurrentSlot] = 0;
		}

		m_tTime.start();
	}

	m_pSlots[m_nCurrentSlot] += nBytes;
	m_nTotal += nBytes;
}
quint32 TCPBandwidthMeter::AvgUsage()
{
	quint64 nTotal = 0;

	if(m_tTime.hasExpired(250))
	{
		int nSlotsToClear = qMin(20ll, m_tTime.elapsed() / 250ll);

		for(; nSlotsToClear; nSlotsToClear--)
		{
			m_nCurrentSlot = (m_nCurrentSlot + 1) % 20;
			m_pSlots[m_nCurrentSlot] = 0;
		}

		m_tTime.start();
	}

	for(int i = 0; i < 20; i++)
	{
		nTotal += m_pSlots[i];
	}

	return nTotal / 5;
}
quint32 TCPBandwidthMeter::Usage()
{
	if(m_tTime.hasExpired(5000))
	{
		return 0;
	}

	int nSlot = m_nCurrentSlot;
	quint32 nUsage = 0;

	for(int i = 0; i < 4; i++)
	{
		nUsage += m_pSlots[nSlot];
		nSlot--;
		if(nSlot < 0)
		{
			nSlot = 19;
		}
	}

	return nUsage;
}

void CNetworkConnection::onDisconnectInt()
{
	readFromNetwork(m_pSocket->bytesAvailable());

	// notify objects
	emit disconnected();
}

void CNetworkConnection::onErrorInt(QAbstractSocket::SocketError e)
{
	//qDebug() << m_pSocket << e;
	emit error(e);
}

void CNetworkConnection::onStateChange(QAbstractSocket::SocketState s)
{
	Q_UNUSED(s)
}

