/*
** discoverytablemodel.h
**
** Copyright © Quazaa Development Team, 2009-2012.
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

#ifndef DISCOVERYTABLEMODEL_H
#define DISCOVERYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

#include "Discovery/discoveryservice.h"

class DiscoveryTableModel : public QAbstractTableModel
{
	Q_OBJECT

private:
	typedef Discovery::ConstServicePtr      ConstServicePtr;
	typedef Discovery::ServiceType::Type    Type;
	typedef Discovery::ServiceID            ServiceID;

	QWidget*        m_oContainer;
	int             m_nSortColumn;
	Qt::SortOrder   m_nSortOrder;
	bool            m_bNeedSorting;

public:
	enum Column
	{
		TYPE = 0,
		URL,
		ACCESSED,
		HOSTS,
		TOTAL_HOSTS,
		ALTERNATE_SERVICES,
		FAILURES,
#if ENABLE_DISCOVERY_DEBUGGING
		RATING,
		MULTIPLICATOR,
#endif
		PONG,
		_NO_OF_COLUMNS
	};

	enum Icon
	{
		BANNED = 0,
		BOOTSTRAP,
		DISCOVERY,
		GWC_GREEN,
		GWC_BLUE,
		GWC_GRAY,
		_NO_OF_ICONS
	};

	// icons used for the different services
	const QIcon** m_pIcons;

	struct Service
	{
		// Object directly managed by discovery manager.
		ConstServicePtr m_pNode;
		const ServiceID m_nID;

		Type             m_nType;
		QString          m_sType;
		const QIcon*     m_piType;
		bool             m_bBanned;
		QString          m_sURL;
		quint32          m_tAccessed;
		quint32          m_nLastHosts;
		quint32          m_nTotalHosts;
		quint16          m_nAltServices;
		quint8           m_nFailures;
#if ENABLE_DISCOVERY_DEBUGGING
		quint8           m_nRating;
		quint8           m_nMultipilcator;
#endif
		QString          m_sPong;

		/**
		 * @brief Service constructs a new service.
		 * @param pService - Needs to be locked for read before calling this.
		 * @param model
		 */
		Service( ConstServicePtr pService, DiscoveryTableModel* model );
		~Service();
		bool update( int row, int sortCol, QModelIndexList& to_update, DiscoveryTableModel* model );
		QVariant data( int col ) const;
		bool lessThan( int col, const DiscoveryTableModel::Service* const pOther ) const;
		void refreshServiceIcon( DiscoveryTableModel* model );
	};

protected:
	QVector<Service*>   m_lNodes;

public:
	explicit DiscoveryTableModel( QObject* parent = 0, QWidget* container = 0 );
	~DiscoveryTableModel();

	int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	int columnCount( const QModelIndex& parent ) const;
	QVariant data( const QModelIndex& index, int role ) const;
	QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
	QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;

	void sort( int column, Qt::SortOrder order );

	ConstServicePtr nodeFromRow( quint32 row ) const;
	ConstServicePtr nodeFromIndex( const QModelIndex& index ) const;

	bool isIndexBanned( const QModelIndex& index ) const;

	void completeRefresh();

public slots:
	void addService( ConstServicePtr pService );
	void removeService( ServiceID nID );
	void update( ServiceID nID );
	void updateAll();

private:
	void updateView( QModelIndexList uplist = QModelIndexList() );
};

#endif // DISCOVERYTABLEMODEL_H
