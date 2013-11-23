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

#include "chatcore.h"
#include "ratecontroller.h"
#include "chatsession.h"

#ifdef _DEBUG
#include "debug_new.h"
#endif

CChatCore ChatCore;
CThread ChatThread;

CChatCore::CChatCore(QObject *parent) :
	QObject(parent), m_bActive(false)
{
}
CChatCore::~CChatCore()
{
	if( m_bActive )
		Stop();
}

void CChatCore::Add(CChatSession *pSession)
{
	QMutexLocker l(&m_pSection);

	if( !m_lSessions.contains(pSession) )
		m_lSessions.append(pSession);

	Start();

	m_pController->addSocket(pSession);
}
void CChatCore::Remove(CChatSession *pSession)
{
	QMutexLocker l(&m_pSection);

	if( int nSession = m_lSessions.indexOf(pSession) != -1 )
	{
		m_lSessions.removeAt(nSession);
		m_pController->removeSocket(pSession);
	}
}

void CChatCore::Start()
{
	if( m_bActive )
		return;

	ChatThread.start("ChatCore", &m_pSection);
	m_bActive = true;

	m_pController = new CRateController(&m_pSection);
	m_pController->setDownloadLimit(8192);
	m_pController->setUploadLimit(8192);
	m_pController->moveToThread(&ChatThread);
}
void CChatCore::Stop()
{
	ChatThread.exit(0);
	m_bActive = false;

	qDeleteAll(m_lSessions);
	m_lSessions.clear();
	delete m_pController;
}

void CChatCore::OnTimer()
{

}

