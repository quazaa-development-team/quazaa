/*
** hostcachetablemodel.h
**
** Copyright © Quazaa Development Team, 2013-2014.
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

#ifndef G2CACHETABLEMODEL_H
#define G2CACHETABLEMODEL_H

#include <QAbstractTableModel>

#include "g2hostcache.h"

class HostData;

class HostCacheTableModel : public QAbstractTableModel
{
	Q_OBJECT

private:
	typedef QSharedPointer<HostManagement::HostCacheHost> SharedHost;

	QWidget*        m_oContainer;
	Qt::SortOrder   m_nSortOrder;
	int             m_nSortColumn;
	bool            m_bNeedSorting;

	quint32         m_nHostInfo;

protected:
	std::vector<HostData*> m_vHosts;

public:
	enum Column
	{
		ADDRESS        = 0,
		LASTCONNECT    = 1,
		FAILURES       = 2,
		COUNTRY        = 3,
		_NO_OF_COLUMNS = 4
	};

public:
	explicit HostCacheTableModel( QObject* parent = NULL, QWidget* container = NULL );
	~HostCacheTableModel();

	int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	int columnCount( const QModelIndex& parent ) const;
	QVariant data( const QModelIndex& index, int nRole ) const;
	QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
	QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;

	void sort( int column, Qt::SortOrder order );

//	int find( quint32 nHostID );

	HostData* dataFromRow( int nRow ) const;
//	Rule* ruleFromIndex(const QModelIndex& index) const;

	void completeRefresh();

	void triggerHostRemoval( int nIndex );

public slots:
	void recieveHostInfo( HostData* pHostData );

	/**
	 * @brief addHost adds a rule to the GUI.
	 * @param pHost : the host
	 */
	void addHost( HostData* pHostData );

	/**
	 * @brief removeHost removes a host from the table model.
	 * This is to be triggered from the host cache AFTER the host has been removed.
	 * @param pHost : the host
	 */
	void removeHost( SharedHost pHost );

	/**
	 * @brief updateHost updates the GUI for a specified host.
	 * @param nHostID : the ID of the host
	 */
//	void updateHost(quint32 nHostID);

	/**
	 * @brief updateAll updates all hosts in the GUI.
	 */
	void updateAll();

	/**
	 * @brief clear removes all hosts from the GUI.
	 */
	void clear();

private:
	void updateView( QModelIndexList uplist = QModelIndexList() );
	int findInsertPos( const HostData* const pData );
	void insert( HostData* pData );
	void insertAt( HostData* pData, const int nPos );
	void erase( const int nPos );
};

#endif // G2CACHETABLEMODEL_H
