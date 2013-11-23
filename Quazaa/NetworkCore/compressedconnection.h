/*
** compressedconnection.h
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

#ifndef COMPRESSEDCONNECTION_H
#define COMPRESSEDCONNECTION_H

#include "networkconnection.h"
#include <QElapsedTimer>
#include "zlib.h"


class CBuffer;

class CCompressedConnection : public CNetworkConnection
{
	Q_OBJECT

public:
	z_stream    m_sInput;               // zlib compressed input stream
	z_stream    m_sOutput;              // zlib compressed output stream
	bool        m_bCompressedInput;     // Compress input streams?
	bool        m_bCompressedOutput;    // Compress output streams?
	CBuffer* 	m_pZInput;              // Local input buffer
	CBuffer* 	m_pZOutput;             // Local output buffer
	quint64     m_nTotalInput;          // Total input in bytes
	quint64		m_nTotalInputDec;       // Total decompressed input in bytes.
	quint64     m_nTotalOutput;         // Total output in bytes
	quint64		m_nTotalOutputCom;      // Total decompressed output in bytes.
	quint64     m_nNextDeflateFlush;    // Amount of bytes until a deflate buffer flush is triggered.
	bool        m_bOutputPending;       // Do we have data to send on the compressed output stream?
	QElapsedTimer m_tDeflateFlush;      // Amount of time until a deflate buffer flush is triggered.
public:
	CCompressedConnection(QObject* parent = 0);
	virtual ~CCompressedConnection();

	bool enableInputCompression(bool bEnable = true);
	bool enableOutputCompression(bool bEnable = true);

	virtual qint64 readFromNetwork(qint64 nBytes);
	virtual qint64 writeToNetwork(qint64 nBytes);

protected:
	bool setupInputStream();
	bool setupOutputStream();
	void cleanupInputStream();
	void cleanupOutputStream();

	void inflateInput();
	void deflateOutput();

public:
	inline CBuffer* getInputBuffer()
	{
		return (m_bCompressedInput ? m_pZInput : m_pInput);
	}
	inline CBuffer* getOutputBuffer()
	{
		return (m_bCompressedOutput ? m_pZOutput : m_pOutput);
	}
	inline virtual bool hasData()
	{
		if(m_bOutputPending)
		{
			return true;
		}

		if(m_pZInput && !m_pZInput->isEmpty())
		{
			return true;
		}
		if(m_pZOutput && !m_pZOutput->isEmpty())
		{
			return true;
		}

		return CNetworkConnection::hasData();
	}

	float getTotalInDecompressed()
	{
		if(m_nTotalInput == 0)
		{
			return 0;
		}

		float ret = 1.0f - (float)m_nTotalInputDec / (float)m_nTotalInput;
		return ret;
	}
	float getTotalOutCompressed()
	{
		if(m_nTotalOutputCom == 0)
		{
			return 0;
		}

		float ret = 1.0f - (float)m_nTotalOutput / (float)m_nTotalOutputCom;
		return ret;
	}

signals:

public slots:

};

#endif // COMPRESSEDCONNECTION_H
