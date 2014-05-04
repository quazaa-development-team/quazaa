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


#ifndef NEIGHBOURSBASE_H
#define NEIGHBOURSBASE_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QHash>
#include <QSet>
#include "types.h"

class Neighbour;

class NeighboursBase : public QObject
{
	Q_OBJECT

public:
	QMutex	m_pSection;
	bool	m_bActive;
protected:
	QList<Neighbour*>				 m_lNodes;
	QHash<QHostAddress, Neighbour*> m_lNodesByAddr;  // lookups by ip address
	QSet<Neighbour*>				 m_lNodesByPtr;	// lookups by pointer
public:
	NeighboursBase( QObject* parent = 0 );
	virtual ~NeighboursBase();

	virtual void connectNode();
	virtual void disconnectNode();

	virtual void addNode( Neighbour* pNode );
	virtual void removeNode( Neighbour* pNode );

	Neighbour* find( const QHostAddress& oAddress,
					 DiscoveryProtocol::Protocol nProtocol = DiscoveryProtocol::None );
	bool neighbourExists( const Neighbour* pNode );

	virtual quint32 downloadSpeed()
	{
		return 0;
	}

	virtual quint32 uploadSpeed()
	{
		return 0;
	}

public:
	inline QList<Neighbour*>::iterator begin()
	{
		return m_lNodes.begin();
	}
	inline QList<Neighbour*>::iterator end()
	{
		return m_lNodes.end();
	}

	inline int getCount()
	{
		return m_lNodes.size();
	}
	inline Neighbour* getAt( int nIndex )
	{
		Q_ASSERT( nIndex >= 0 && nIndex < m_lNodes.size() );

		return m_lNodes.at( nIndex );
	}
signals:
	void sanityCheckPerformed();
	void neighbourAdded( Neighbour* );
	void neighbourRemoved( Neighbour* );

public slots:
	virtual void maintain();

	/**
	 * @brief sanityCheck removes nodes newly banned by the Security Manager.
	 */
	void sanityCheck();

	void localAddressChanged();
};

#endif // NEIGHBOURSBASE_H
