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

#include "widgetneighbours.h"
#include "ui_widgetneighbours.h"
#include "dialogsettings.h"
#include "dialogconnectto.h"
#include "dialogneighbourinfo.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include "commonfunctions.h"
#include "network.h"
#include "neighbours.h"
#include "neighbour.h"
#include "datagrams.h"
#include "neighbourstablemodel.h"

#include "securitymanager.h"

#include "chatsessiong2.h"

#include "debug_new.h"

#include <QMenu>
#include <QInputDialog>

CWidgetNeighbours::CWidgetNeighbours(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::CWidgetNeighbours)
{
	ui->setupUi(this);
	neighboursMenu = new QMenu(ui->tableViewNeighbours);
	neighboursMenu->addAction(ui->actionNeighbourDisconnect);
	neighboursMenu->addAction(ui->actionNetworkBan);
	neighboursMenu->addSeparator();
	neighboursMenu->addAction(ui->actionNetworkChatWith);
	neighboursMenu->addAction(ui->actionViewProfile);
	neighboursMenu->addAction(ui->actionNetworkBrowse);
	labelG2StatsIcon = new QLabel();
	QPixmap const pixmapG2Stats = QPixmap(":/Resource/Networks/Gnutella2.png");
	labelG2StatsIcon->setPixmap(pixmapG2Stats);
	labelG2StatsIcon->setScaledContents(true);
	labelG2StatsIcon->setMaximumSize(16, 16);
	labelG2Stats = new QLabel();
	ui->toolBarG2->addWidget(labelG2StatsIcon);
	ui->toolBarG2->addWidget(labelG2Stats);
	restoreState(quazaaSettings.WinMain.NeighboursToolbars);
	ui->tableViewNeighbours->horizontalHeader()->restoreState(quazaaSettings.WinMain.NeighboursHeader);

	neighboursList = new CNeighboursTableModel(this, treeView());
	setModel(neighboursList);
	neighboursList->sort(ui->tableViewNeighbours->horizontalHeader()->sortIndicatorSection(), ui->tableViewNeighbours->horizontalHeader()->sortIndicatorOrder());
	setSkin();
}

CWidgetNeighbours::~CWidgetNeighbours()
{
	quazaaSettings.WinMain.NeighboursHeader = ui->tableViewNeighbours->horizontalHeader()->saveState();
	delete ui;
}

void CWidgetNeighbours::changeEvent(QEvent* e)
{
	QMainWindow::changeEvent(e);
	switch(e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void CWidgetNeighbours::setModel(QAbstractItemModel* model)
{
	ui->tableViewNeighbours->setModel(model);
}

QWidget* CWidgetNeighbours::treeView()
{
	return ui->tableViewNeighbours;
}

void CWidgetNeighbours::saveWidget()
{
	quazaaSettings.WinMain.NeighboursToolbars = saveState();
	quazaaSettings.WinMain.NeighboursHeader = ui->tableViewNeighbours->horizontalHeader()->saveState();
}

void CWidgetNeighbours::on_actionSettings_triggered()
{
	CDialogSettings* dlgSettings = new CDialogSettings(this, SettingsPage::Protocols);
	dlgSettings->show();
}

void CWidgetNeighbours::updateG2()
{
	quint32 nHubsConnected = 0;
	quint32 nLeavesConnected = 0;
	quint32 nTCPInSpeed = 0;
	quint32 nTCPOutSpeed = 0;
	quint32 nUDPInSpeed = 0;
	quint32 nUDPOutSpeed = 0;

	if(Neighbours.m_pSection.tryLock(50))
	{
		nHubsConnected = Neighbours.m_nHubsConnectedG2;
		nLeavesConnected = Neighbours.m_nLeavesConnectedG2;

		nTCPInSpeed = Neighbours.downloadSpeed();
		nTCPOutSpeed = Neighbours.uploadSpeed();

		if(Network.m_pSection.tryLock(50))
		{
			nUDPInSpeed = Datagrams.downloadSpeed();
			nUDPOutSpeed = Datagrams.uploadSpeed();

			Network.m_pSection.unlock();
		}

		Neighbours.m_pSection.unlock();
	}

	labelG2Stats->setText(tr(" %1 Hubs, %2 Leaves, %3/s In:%4/s Out").arg(nHubsConnected).arg(nLeavesConnected).arg(common::formatBytes(nTCPInSpeed + nUDPInSpeed)).arg(common::formatBytes(nTCPOutSpeed + nUDPOutSpeed)));
}

void CWidgetNeighbours::updateAres()
{

}

void CWidgetNeighbours::updateEDonkey()
{

}

void CWidgetNeighbours::onTimer()
{
	neighboursList->updateAll();
	updateG2();
}

void CWidgetNeighbours::on_actionNeighbourConnectTo_triggered()
{
	CDialogConnectTo* dlgConnectTo = new CDialogConnectTo(this);
	bool accepted = dlgConnectTo->exec();

	if (accepted)
	{
		CEndPoint ip(dlgConnectTo->getAddressAndPort());

		switch (dlgConnectTo->getConnectNetwork())
		{
		case CDialogConnectTo::G2:
			Neighbours.m_pSection.lock();
			Neighbours.connectTo(ip, dpG2, false);
			Neighbours.m_pSection.unlock();
			break;
		case CDialogConnectTo::eDonkey:
			break;
		case CDialogConnectTo::Ares:
			break;
		default:
			break;
		}
	}
}

void CWidgetNeighbours::on_actionNeighbourDisconnect_triggered()
{
	QModelIndex idx = ui->tableViewNeighbours->currentIndex();
	CNeighbour* pNode = neighboursList->nodeFromIndex(idx);

	if( pNode == 0 )
		return;

	Neighbours.m_pSection.lock();
	if ( Neighbours.neighbourExists(pNode) )
	{
		systemLog.postLog( LogSeverity::Information, Components::Network,
						   qPrintable( tr( "Closing connection to neighbour %s" ) ),
						   qPrintable( pNode->m_oAddress.toStringWithPort() ) );
		pNode->close();
	}
	Neighbours.m_pSection.unlock();
}

void CWidgetNeighbours::on_tableViewNeighbours_customContextMenuRequested(QPoint pos)
{
	QModelIndex currIndex = ui->tableViewNeighbours->indexAt(pos);
	if( currIndex.isValid() )
	{
		neighboursMenu->exec(QCursor::pos());
	}
}

void CWidgetNeighbours::on_actionNetworkChatWith_triggered()
{
	QMutexLocker l(&Neighbours.m_pSection);

	if (ui->tableViewNeighbours->currentIndex().isValid())
	{
		QModelIndex idx = ui->tableViewNeighbours->currentIndex();
		CNeighbour* pNode = neighboursList->nodeFromIndex(idx);

		if( pNode == 0 )
			return;

		switch( pNode->m_nProtocol)
		{
		case dpG2:
		{
			CChatSessionG2* pSession = new CChatSessionG2(pNode->m_oAddress);
			pSession->connectNode();
			break;
		}
		default:
			break;

		}
	}
}

void CWidgetNeighbours::setSkin()
{
	ui->tableViewNeighbours->setStyleSheet(skinSettings.listViews);
}

void CWidgetNeighbours::on_tableViewNeighbours_doubleClicked(const QModelIndex &index)
{
	CNeighboursTableModel::Neighbour* pNbr = static_cast<CNeighboursTableModel::Neighbour*>(index.internalPointer());
	CDialogNeighbourInfo* dlgNeighbourInfo = new CDialogNeighbourInfo(pNbr, this);
	dlgNeighbourInfo->exec();
}

void CWidgetNeighbours::on_actionNetworkBan_triggered()
{
	CNeighboursTableModel::Neighbour* pNbr = static_cast<CNeighboursTableModel::Neighbour*>(ui->tableViewNeighbours->currentIndex().internalPointer());
	bool ok;
	QString reason = QInputDialog::getText( this, tr("Ban Reason"), tr("Please enter a ban reason."), QLineEdit::Normal, "", &ok );
	if ( ok && !reason.isEmpty() )
	{
		securityManager.ban( pNbr->oAddress, Security::RuleTime::SixMonths, false, reason, false );
		on_actionNeighbourDisconnect_triggered();
	}
}
