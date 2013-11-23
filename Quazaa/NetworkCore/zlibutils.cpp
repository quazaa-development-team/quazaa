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

#include "zlibutils.h"
#include "zlib.h"

#include "debug_new.h"

QMutex ZLibUtils::m_oMutex;
CBuffer ZLibUtils::m_oCompressBuffer(262144); // 256KB

bool ZLibUtils::compressBuffer(CBuffer& pSrc, bool bIfSmaller)
{
	QMutexLocker l(&ZLibUtils::m_oMutex);

	if(bIfSmaller && pSrc.size() < 64)
	{
		return false;
	}

	ulong nCompress = qMax(ZLibUtils::m_oCompressBuffer.size(), pSrc.size() * 2);

	ZLibUtils::m_oCompressBuffer.resize(nCompress);

	int nRet = ::compress((Bytef*)ZLibUtils::m_oCompressBuffer.data(), &nCompress, (const Bytef*)pSrc.data(), pSrc.size());

	if(nRet != Z_OK)
	{
		Q_ASSERT(nRet != Z_BUF_ERROR);
		return false;
	}

	ZLibUtils::m_oCompressBuffer.resize(nCompress);

	if(bIfSmaller && ZLibUtils::m_oCompressBuffer.size() > pSrc.size())
	{
		return false;
	}

	pSrc.clear();
	pSrc.append(ZLibUtils::m_oCompressBuffer);

	return true;
}

bool ZLibUtils::uncompressBuffer(CBuffer& pSrc)
{
	QMutexLocker l(&ZLibUtils::m_oMutex);

	for(ulong nSize = qMax(pSrc.size() * 6u, 1024u); ; nSize *= 2)
	{
		ZLibUtils::m_oCompressBuffer.resize(nSize);

		ulong nCompress = ZLibUtils::m_oCompressBuffer.size();

		int nRet = ::uncompress((Bytef*)ZLibUtils::m_oCompressBuffer.data(), &nCompress, (Bytef*)pSrc.data(), pSrc.size());

		if(nRet == Z_OK)
		{
			ZLibUtils::m_oCompressBuffer.resize(nCompress);
			pSrc.clear();
			pSrc.append(ZLibUtils::m_oCompressBuffer);

			break;
		}

		if(nRet != Z_BUF_ERROR)
		{
			return false;
		}
	}

	return true;

}

