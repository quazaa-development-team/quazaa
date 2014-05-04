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

#include "compressedconnection.h"
#include "buffer.h"
#include "systemlog.h"

#include "debug_new.h"

CompressedConnection::CompressedConnection( QObject* parent ) :
	NetworkConnection( parent )
{
	m_bCompressedInput = false;
	m_bCompressedOutput = false;

	m_pZInput = 0;
	m_pZOutput = 0;

	m_nTotalInput = 0;
	m_nTotalOutput = 0;

	m_nTotalInputDec = 0;
	m_nTotalOutputCom = 0;

	m_nNextDeflateFlush = 4096;
	m_bOutputPending = false;

	memset( &m_sInput, 0, sizeof( z_stream ) );
	memset( &m_sOutput, 0, sizeof( z_stream ) );
}

CompressedConnection::~CompressedConnection()
{
	cleanupInputStream();
	cleanupOutputStream();
}

bool CompressedConnection::enableInputCompression( bool bEnable )
{
	if ( bEnable && !m_bCompressedInput )
	{
		bool bRet = setupInputStream();
		if ( bRet )
		{
			m_bCompressedInput = true;
		}
		return bRet;
	}
	else if ( !bEnable && m_bCompressedInput )
	{
		return false;
	}

	return true;
}

bool CompressedConnection::enableOutputCompression( bool bEnable )
{
	if ( bEnable && !m_bCompressedOutput )
	{
		bool bRet = setupOutputStream();
		if ( bRet )
		{
			m_bCompressedOutput = true;
		}
		return bRet;
	}
	else if ( !bEnable && m_bCompressedOutput )
	{
		return false;
	}

	return true;
}

bool CompressedConnection::setupInputStream()
{
	m_pZInput = new Buffer( 8192 );

	if ( m_pZInput == 0 )
	{
		return false;
	}

	if ( inflateInit( &m_sInput ) != Z_OK )
	{
		delete m_pZInput;
		m_pZInput = 0;
		return false;
	}
	return true;
}

bool CompressedConnection::setupOutputStream()
{
	m_pZOutput = new Buffer( 8192 );
	if ( m_pZOutput == 0 )
	{
		return false;
	}

	if ( deflateInit( &m_sOutput, Z_DEFAULT_COMPRESSION ) != Z_OK )
	{
		delete m_pZOutput;
		m_pZOutput = 0;
		return false;
	}
	m_nNextDeflateFlush = m_nTotalOutput + 4096;
	m_tDeflateFlush.start();

	return true;
}

void CompressedConnection::cleanupInputStream()
{
	if ( m_pZInput )
	{
		m_pInput->prepend( m_pZInput->data(), m_pZInput->size() );
		delete m_pZInput;
		m_pZInput = 0;
	}

	inflateEnd( &m_sInput );
}

void CompressedConnection::cleanupOutputStream()
{
	if ( m_pZOutput )
	{
		m_pOutput->prepend( m_pZOutput->data(), m_pZOutput->size() );
		delete m_pZOutput;
		m_pZOutput = 0;
	}
	deflateEnd( &m_sOutput );
}

qint64 CompressedConnection::readFromNetwork( qint64 nBytes )
{
	qint64 nRet = NetworkConnection::readFromNetwork( nBytes );

	if ( m_bCompressedInput )
	{
		inflateInput();
		if ( m_pZInput->size() )
		{
			emit readyRead();
		}
	}

	return nRet;
}

qint64 CompressedConnection::writeToNetwork( qint64 nBytes )
{
	if ( m_bCompressedOutput )
	{
		if ( m_pOutput->size() == 0 )
		{
			deflateOutput();
		}
	}

	return NetworkConnection::writeToNetwork( nBytes );
}

void CompressedConnection::inflateInput()
{
	if ( m_pInput->size() == 0 )
	{
		return;
	}

	qint32 nRet = Z_OK;

	do
	{
		m_sInput.next_in   = ( Bytef* )m_pInput->data();
		m_sInput.avail_in  = m_pInput->size();
		m_sInput.total_in  = 0u;

		quint32 oldSize = m_pZInput->size();

		if ( m_pZInput->capacity() - m_pZInput->size() < 2048 )
		{
			m_pZInput->ensure( 2048 );
		}

		m_sInput.next_out  = ( Bytef* )m_pZInput->data() + oldSize;
		m_sInput.avail_out = qMax( m_pZInput->capacity() - oldSize, 2048u );
		m_sInput.total_out = 0u;

		nRet = inflate( &m_sInput, Z_SYNC_FLUSH );

		switch ( nRet )
		{
		case Z_MEM_ERROR:
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
			break;
		default:
			m_pZInput->resize( oldSize + m_sInput.total_out );
			m_pInput->remove( m_sInput.total_in );

			m_nTotalInput += m_sInput.total_out;
			m_nTotalInputDec += m_sInput.total_in;

		}
	}
	while ( m_sInput.avail_out == 0u );

	if ( nRet == Z_BUF_ERROR )
	{
		nRet = Z_OK;
	}

	if ( nRet != Z_OK )
	{
		systemLog.postLog( LogSeverity::Debug, Component::Network, QString( "Error in decompressor! " ).arg( nRet ) );

		close();
	}
}

void CompressedConnection::deflateOutput()
{
	qint32 nFlushMode = Z_NO_FLUSH;

	if ( m_tDeflateFlush.elapsed() > 250ll || m_nTotalOutput > m_nNextDeflateFlush )
	{
		nFlushMode = Z_SYNC_FLUSH;
		m_nNextDeflateFlush += 4096;
		m_tDeflateFlush.start();
	}

	if ( m_pZOutput->size() == 0 && nFlushMode == Z_NO_FLUSH )
	{
		return;
	}

	do
	{
		m_sOutput.next_in = ( Bytef* )m_pZOutput->data();
		m_sOutput.avail_in = m_pZOutput->size();
		m_sOutput.total_in = 0u;

		if ( m_pOutput->capacity() - m_pOutput->size() < 2048 )
		{
			m_pOutput->ensure( 2048u );
		}

		qint32 nOldSize = m_pOutput->size();

		m_sOutput.next_out = ( Bytef* )m_pOutput->data() + nOldSize;
		m_sOutput.avail_out = m_pOutput->capacity() - nOldSize;
		m_sOutput.total_out = 0u;

		qint32 nRet = deflate( &m_sOutput, nFlushMode );

		if ( nRet == Z_OK || nRet == Z_BUF_ERROR )
		{
			m_pOutput->resize( nOldSize + m_sOutput.total_out );
			m_pZOutput->remove( m_sOutput.total_in );
			m_nTotalOutput += m_sOutput.total_out;
			m_nTotalOutputCom += m_sOutput.total_in;
		}
		else
		{
			systemLog.postLog( LogSeverity::Debug, Component::Network,
							   QString( "Error in compressor! %1" ).arg( nRet ) );
			close();
			break;
		}

	}
	while ( m_sOutput.avail_in != 0 );

	m_bOutputPending = ( nFlushMode == Z_NO_FLUSH );
}
