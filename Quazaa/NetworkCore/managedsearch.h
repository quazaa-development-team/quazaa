/*
** managedsearch.h
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

#ifndef MANAGEDSEARCH_H
#define MANAGEDSEARCH_H

#include "types.h"
#include <QHash>
#include "queryhit.h" // needed for signals

#include "g2hostcache.h"
#include "network.h"

class Query;

class ManagedSearch : public QObject
{
	Q_OBJECT
public:
	QDateTime   m_tStarted;
	bool        m_bActive;
	bool        m_bPaused;
	Query*     m_pQuery;

	bool        m_bCanRequestKey;

	quint32     m_nHubs;
	quint32     m_nLeaves;
	quint32     m_nHits;
	quint32     m_nQueryCount;
	quint32     m_nQueryHitLimit;

	QUuid       m_oGUID;

	QueryHit*   m_pCachedHit;
	quint32     m_nCachedHits;

	quint32     m_nCookie;

	QDateTime   m_tCleanHostsNext;

	QHash<QHostAddress, QDateTime> m_lSearchedNodes;

public:
	ManagedSearch( Query* pQuery, QObject* parent = NULL );
	~ManagedSearch();

	void start();
	void stop();
	void pause();

	void execute( const QDateTime& tNowDT, quint32* pnMaxPackets );
	void searchG2( const QDateTime& tNowDT, quint32* pnMaxPackets );
	void searchNeighbours( const QDateTime& tNowDT );

	void sendG2Query( EndPoint pReceiver, SharedG2HostPtr pHost,
					  quint32* pnMaxPackets, const QDateTime& tNowDT );
	void requestG2QueryKey( SharedG2HostPtr pHost );
	G2Node* findBestHubForRoutingG2( const G2Node* const pLastNeighbour );

	void onHostAcknowledge( QHostAddress nHost, const QDateTime& tNow );
	void onQueryHit( QueryHit* pHits );
	void sendHits();

signals:
	void onHit( QueryHit* );
	void statsUpdated();
	void stateChanged();

public slots:

	friend class CSearchManager;
};

#endif // MANAGEDSEARCH_H
