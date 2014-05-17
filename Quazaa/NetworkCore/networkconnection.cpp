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

TCPBandwidthMeter::TCPBandwidthMeter()
{
	memset( &m_pSlots[0], 0, sizeof( m_pSlots ) );
	m_nCurrentSlot = m_nTotal = 0;
	m_tTime.start();
}

void TCPBandwidthMeter::add( quint32 nBytes )
{
	if ( m_tTime.hasExpired( 250 ) )
	{
		int nSlotsToClear = qMin( 20ll, m_tTime.elapsed() / 250ll );

		for ( ; nSlotsToClear; --nSlotsToClear )
		{
			m_nCurrentSlot = ( m_nCurrentSlot + 1 ) % 20;
			m_pSlots[m_nCurrentSlot] = 0;
		}

		m_tTime.start();
	}

	m_pSlots[m_nCurrentSlot] += nBytes;
	m_nTotal += nBytes;
}

quint32 TCPBandwidthMeter::avgUsage()
{
	quint64 nTotal = 0;

	if ( m_tTime.hasExpired( 250 ) )
	{
		int nSlotsToClear = qMin( 20ll, m_tTime.elapsed() / 250ll );

		for ( ; nSlotsToClear; --nSlotsToClear )
		{
			m_nCurrentSlot = ( m_nCurrentSlot + 1 ) % 20;
			m_pSlots[m_nCurrentSlot] = 0;
		}

		m_tTime.start();
	}

	for ( int i = 0; i < 20; ++i )
	{
		nTotal += m_pSlots[i];
	}

	return nTotal / 5;
}

quint32 TCPBandwidthMeter::usage()
{
	if ( m_tTime.hasExpired( 5000 ) )
	{
		return 0;
	}

	int nSlot = m_nCurrentSlot;
	quint32 nUsage = 0;

	for ( int i = 0; i < 4; ++i )
	{
		nUsage += m_pSlots[nSlot];
		--nSlot;
		if ( nSlot < 0 )
		{
			nSlot = 19;
		}
	}

	return nUsage;
}

NetworkConnection::NetworkConnection( QObject* parent ) :
	QObject( parent )
{
	static int foo =
			qRegisterMetaType< QAbstractSocket::SocketError >( "QAbstractSocket::SocketError" );
	static int bar =
			qRegisterMetaType< QAbstractSocket::SocketState >( "QAbstractSocket::SocketState" );
	Q_UNUSED( foo );
	Q_UNUSED( bar );

	// qDebug() << "NetworkConnection constructor";

	m_pSocket = NULL;
	m_pInput  = NULL;
	m_pOutput = NULL;

	m_bInitiated = false;
	m_bConnected = false;
	m_tConnected = 0;
}
NetworkConnection::~NetworkConnection()
{
	if ( m_pInput )
	{
		delete m_pInput;
	}

	if ( m_pOutput )
	{
		delete m_pOutput;
	}

	if ( m_pSocket )
	{
		delete m_pSocket;
	}
}

void NetworkConnection::connectTo( EndPoint oAddress )
{
	m_oAddress = oAddress;

	m_bInitiated = true;
	m_bConnected = false;
	m_tConnected = common::getTNowUTC();

	Q_ASSERT( !m_pInput );
	Q_ASSERT( !m_pOutput );
	m_pInput  = new Buffer( 8192 );
	m_pOutput = new Buffer( 8192 );

	Q_ASSERT( !m_pSocket );
	m_pSocket = new QTcpSocket();
	initializeSocket();
	m_pSocket->connectToHost( oAddress, oAddress.port() );
}

void NetworkConnection::attachTo( NetworkConnection* pOther )
{
	Q_ASSERT( !m_pSocket );
	m_pSocket = pOther->m_pSocket;
	pOther->m_pSocket = NULL;

	m_bConnected = pOther->m_bConnected;
	m_tConnected = pOther->m_tConnected;
	m_bInitiated = pOther->m_bInitiated;

	Q_ASSERT( !m_pInput );
	Q_ASSERT( !m_pOutput );
	m_pInput  = pOther->m_pInput;
	m_pOutput = pOther->m_pOutput;
	pOther->m_pInput = NULL;
	pOther->m_pOutput = NULL;

	m_oAddress.setAddress( m_pSocket->peerAddress() );
	m_oAddress.setPort( m_pSocket->peerPort() );

	initializeSocket();

	emit readyRead();
}

void NetworkConnection::acceptFrom( qintptr nHandle )
{
	Q_ASSERT( !m_pSocket );
	m_pSocket = new QTcpSocket();

	initializeSocket();

	m_bInitiated = false;
	m_bConnected = true;
	m_tConnected = common::getTNowUTC();

	Q_ASSERT( m_pInput == 0 );
	Q_ASSERT( m_pOutput == 0 );
	m_pInput = new Buffer( 8192 );
	m_pOutput = new Buffer( 8192 );

	m_pSocket->setSocketDescriptor( nHandle );

	m_oAddress.setAddress( m_pSocket->peerAddress() );
	m_oAddress.setPort( m_pSocket->peerPort() );

	emit readyToTransfer();

}

void NetworkConnection::close( bool bDelayed )
{
	systemLog.postLog( LogSeverity::Information, Component::Network,
					   "Closing connection to %s.", qPrintable( m_oAddress.toStringWithPort() ) );

	QMetaObject::invokeMethod( this, "closeImplementation", Q_ARG( bool, bDelayed ) );
}

void NetworkConnection::closeImplementation( bool bDelayed )
{
	if ( bDelayed )
	{
		if ( !getOutputBuffer()->isEmpty() || !m_pOutput->isEmpty() )
		{
			writeToNetwork( m_pOutput->size() + getOutputBuffer()->size() );
			m_pSocket->disconnectFromHost();
			return;
		}
	}

	m_pSocket->abort();
	emit disconnected();
}
void NetworkConnection::moveToThread( QThread* thread )
{
	if ( m_pSocket )
	{
		m_pSocket->moveToThread( thread );
	}
	QObject::moveToThread( thread );
}

void NetworkConnection::initializeSocket()
{
	m_pSocket->disconnect();

	void ( QTcpSocket::*qTcpSocketErrorPtr )( QAbstractSocket::SocketError ) = &QTcpSocket::error;

	connect( m_pSocket, &QTcpSocket::connected,    this, &NetworkConnection::connected       );
	connect( m_pSocket, &QTcpSocket::connected,    this, &NetworkConnection::readyToTransfer );
	connect( m_pSocket, &QTcpSocket::readyRead,    this, &NetworkConnection::readyToTransfer );
	connect( m_pSocket, &QTcpSocket::disconnected, this, &NetworkConnection::onDisconnectInt );
	connect( m_pSocket, qTcpSocketErrorPtr,        this, &NetworkConnection::onErrorInt      );
	connect( m_pSocket, &QTcpSocket::bytesWritten, this, &NetworkConnection::bytesWritten    );
	connect( m_pSocket, &QTcpSocket::stateChanged, this, &NetworkConnection::stateChanged    );
	connect( m_pSocket, &QTcpSocket::aboutToClose, this, &NetworkConnection::aboutToClose    );

	// avoid double connections
	disconnect( this, 0, this, 0 );

	connect( this, &NetworkConnection::connected,
			 this, &NetworkConnection::onConnectNode,    Qt::QueuedConnection );
	connect( this, &NetworkConnection::disconnected,
			 this, &NetworkConnection::onDisconnectNode, Qt::QueuedConnection );
	connect( this, &NetworkConnection::readyRead,
			 this, &NetworkConnection::onRead,           Qt::QueuedConnection );
	connect( this, &NetworkConnection::error,
			 this, &NetworkConnection::onError,          Qt::QueuedConnection );
	connect( this, &NetworkConnection::stateChanged,
			 this, &NetworkConnection::onStateChange,    Qt::QueuedConnection );
}

qint64 NetworkConnection::readFromNetwork( qint64 nBytes )
{
	Q_ASSERT( m_pInput );
	Q_ASSERT( nBytes >= 0 );

	int nOldSize = m_pInput->size();
	qint64 nBytesRead = 0;

	if ( m_pSocket->state() != QTcpSocket::ConnectedState )
	{
		m_pInput->append( m_pSocket->readAll() );
		nBytesRead += m_pInput->size() - nOldSize;
	}
	else
	{
		m_pInput->resize( nOldSize + nBytes );
		nBytesRead = m_pSocket->read( m_pInput->data() + nOldSize, nBytes );
		m_pInput->resize( ( nBytesRead <= 0 ? nOldSize : nOldSize + nBytesRead ) );
	}

	if ( nBytesRead > 0 )
	{
		m_mInput.add( nBytesRead );
		emit readyRead();
	}

	return nBytesRead;
}
qint64 NetworkConnection::writeToNetwork( qint64 nBytes )
{
	Q_ASSERT( m_pOutput );
	Q_ASSERT( nBytes >= 0 );

	qint64 nBytesWritten = m_pSocket->write( m_pOutput->data(), qMin( ( qint64 )m_pOutput->size(), nBytes ) );

	if ( nBytesWritten <= 0 )
	{
		return nBytesWritten;
	}

	m_pOutput->remove( nBytesWritten );
	m_mOutput.add( nBytesWritten );

	return nBytesWritten;
}
qint64 NetworkConnection::readData( char* data, qint64 maxlen )
{
	Q_ASSERT( m_pInput );

	int nBytesRead = qMin<qint64>( maxlen, m_pInput->size() );
	memcpy( data, m_pInput->data(), nBytesRead );
	m_pInput->remove( nBytesRead );

	if ( m_pSocket->state() != QTcpSocket::ConnectedState )
	{
		m_pInput->append( m_pSocket->readAll() );
	}

	return ( qint64 )nBytesRead;
}

qint64 NetworkConnection::writeData( const char* data, qint64 len )
{
	Q_ASSERT( m_pOutput );

	int nOldSize = m_pOutput->size();
	m_pOutput->resize( nOldSize + len );
	memcpy( m_pOutput->data() + nOldSize, data, len );
	emit readyToTransfer();
	return len;
}

qint64 NetworkConnection::bytesAvailable()
{
	Q_ASSERT( m_pInput );

	if ( m_pSocket->state() != QTcpSocket::ConnectedState )
	{
		m_pInput->append( m_pSocket->readAll() );
	}

	return m_pInput->size();
}

qint64 NetworkConnection::bytesToWrite() const
{
	if ( !m_pSocket )
	{
		return 0;
	}

	return m_pSocket->bytesToWrite();
}
qint64 NetworkConnection::networkBytesAvailable() const
{
	if ( !isValid() )
	{
		return 0;
	}

	if ( m_pSocket->state() != QTcpSocket::ConnectedState )
	{
		return m_pInput->size();
	}

	return m_pSocket->bytesAvailable();
}
bool NetworkConnection::isValid() const
{
	if ( !m_pSocket || !m_pInput || !m_pOutput )
	{
		return false;
	}

	return m_pSocket->isValid();
}
void NetworkConnection::setReadBufferSize( qint64 nSize )
{
	Q_ASSERT( m_pSocket );

	m_pSocket->setReadBufferSize( nSize );
}

QString NetworkConnection::errorString() const
{
	return m_pSocket ? m_pSocket->errorString() : QString();
}

QByteArray NetworkConnection::read( qint64 nMaxSize )
{
	QByteArray baRet;

	baRet.resize( qMin<qint64>( nMaxSize, getInputBuffer()->size() ) );
	readData( baRet.data(), baRet.size() );
	return baRet;
}

QByteArray NetworkConnection::peek( qint64 nMaxLength )
{
	Buffer* pBuffer = getInputBuffer();

	return QByteArray::fromRawData( pBuffer->data(),
									qMin<qint64>( nMaxLength > 0 ? nMaxLength : pBuffer->size(),
												  pBuffer->size() ) );
}

void NetworkConnection::onDisconnectInt()
{
	readFromNetwork( m_pSocket->bytesAvailable() );

	// notify objects
	emit disconnected();
}

void NetworkConnection::onErrorInt( QAbstractSocket::SocketError e )
{
	//qDebug() << m_pSocket << e;
	emit error( e );
}

void NetworkConnection::onStateChange( QAbstractSocket::SocketState s )
{
	Q_UNUSED( s )
}

