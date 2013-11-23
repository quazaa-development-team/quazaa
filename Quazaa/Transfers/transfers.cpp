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

#include "transfers.h"
#include "ratecontroller.h"
#include "transfer.h"
#include "downloads.h"

#include <QMutexLocker>

#include "debug_new.h"

CTransfers Transfers;
CThread TransfersThread;

CTransfers::CTransfers(QObject* parent)
	: QObject(parent),
	  m_bActive(false)
{
	m_pController = new CRateController(&m_pSection);
}

CTransfers::~CTransfers()
{
	delete m_pController;
}

void CTransfers::start()
{
	if( m_bActive )
		return;

	systemLog.postLog(LogSeverity::Notice, qPrintable(tr("Starting transfers...")));

	m_bActive = true;
	TransfersThread.start("Transfers", &m_pSection);
	m_pController->moveToThread(&TransfersThread);
	Downloads.start();
	Downloads.moveToThread(&TransfersThread);

	connect(&m_oTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	connect(&m_oTimer, SIGNAL(timeout()), &Downloads, SLOT(onTimer()));
	m_oTimer.start(1000);
}

void CTransfers::stop()
{
	if( !m_bActive )
		return;

	systemLog.postLog(LogSeverity::Notice, qPrintable(tr("Stopping transfers...")));

	m_bActive = false;

	TransfersThread.exit(0);
	Downloads.stop();
}

void CTransfers::add(CTransfer *pTransfer)
{
	QMutexLocker l(&m_pSection);

	Q_ASSERT_X(m_bActive, "CTransfers::add()", "Adding transfer while thread is inactive");

	if( m_lTransfers.contains(pTransfer->m_pOwner, pTransfer) )
	{
		// TODO: Add new sources if transfer already exists
		systemLog.postLog(LogSeverity::Debug, "CTransfers::add(): transfer already added!");
		return;
	}

	m_lTransfers.insert(pTransfer->m_pOwner, pTransfer);
	m_pController->addSocket(pTransfer);
	// start
}

void CTransfers::remove(CTransfer *pTransfer)
{
	QMutexLocker l(&m_pSection);

	if(!m_lTransfers.contains(pTransfer->m_pOwner, pTransfer))
	{
		systemLog.postLog(LogSeverity::Debug, "CTransfers::remove(): removing non-existing transfer!");
		return;
	}

	m_pController->removeSocket(pTransfer);
	m_lTransfers.remove(pTransfer->m_pOwner, pTransfer);
}

QList<CTransfer *> CTransfers::getByOwner(void *pOwner)
{
	return m_lTransfers.values(pOwner);
}

void CTransfers::onTimer()
{
	if(!m_bActive || m_lTransfers.isEmpty())
		return;

	QMutexLocker l(&m_pSection);

	foreach(CTransfer* pTransfer, m_lTransfers)
	{
		pTransfer->onTimer();
	}
}

