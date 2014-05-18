/*
** g2cachetablemodel.cpp
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

#include <QAbstractItemView>

#include "hostcachetablemodel.h"
#include "hostcachehost.h"
#include "debug_new.h"

using namespace HostManagement;

HostCacheTableModel::HostCacheTableModel( QObject* parent, QWidget* container ) :
	QAbstractTableModel( parent ),
	m_oContainer( container ),
	m_nSortOrder( Qt::AscendingOrder ),
	m_nSortColumn( -1 ),
	m_bNeedSorting( false ),
	m_nHostInfo( 0 ),
	m_vHosts( std::vector<HostData*>() )
{
	// register necessary meta types before using them
	hostCache.registerMetaTypes();

	connect( &hostCache, &HostCache::hostAdded, this,
			 &HostCacheTableModel::addHost, Qt::QueuedConnection );

	connect( &hostCache, &HostCache::hostRemoved, this,
			 &HostCacheTableModel::removeHost, Qt::QueuedConnection );

	connect( &hostCache, &HostCache::loadingFinished, this,
			 &HostCacheTableModel::updateAll, Qt::QueuedConnection );

	// speedup shutdown
	connect( &hostCache, &HostCache::clearTriggered, this,
			 &HostCacheTableModel::clear, Qt::DirectConnection );

	// If there were hosts in the cache, we would need
	// to have to do a complete refresh at this point.
	Q_ASSERT( !hostCache.size() );
	//completeRefresh();
}

HostCacheTableModel::~HostCacheTableModel()
{
	m_vHosts.clear();
}

int HostCacheTableModel::rowCount( const QModelIndex& parent ) const
{
	if ( parent.isValid() )
	{
		return 0;
	}
	else
	{
		return ( int )m_vHosts.size();
	}
}

int HostCacheTableModel::columnCount( const QModelIndex& parent ) const
{
	if ( parent.isValid() )
	{
		return 0;
	}
	else
	{
		return _NO_OF_COLUMNS;
	}
}

QVariant HostCacheTableModel::data( const QModelIndex& index, int nRole ) const
{
	if ( !index.isValid() || index.row() >= m_vHosts.size() || index.row() < 0 )
	{
		Q_ASSERT( false );
		return QVariant();
	}

	const HostData* const pData = m_vHosts[ index.row() ];

	switch ( nRole )
	{
	case Qt::DisplayRole:
		return pData->data( index.column() );

	case Qt::DecorationRole:
		if ( index.column() == COUNTRY )
		{
			return pData->m_iCountry;
		}
		break;

	// TODO: Reimplement formatting options in models.
	/*case Qt::ForegroundRole:
		switch ( nbr->nState )
		{
		case nsConnected:
			//return skinSettings.listsColorSpecial;
			break;
		case nsConnecting:
			//return skinSettings.listsColorActive;
			break;
		default:
			//return skinSettings.listsColorNormal;
			break;
		}
		return QVariant();*/

	/*case Qt::FontRole:
		QFont font = qApp->font( m_oContainer );
		switch ( nbr->nState )
		{
		case nsConnected:
			//font.setWeight(skinSettings.listsWeightSpecial);
			return font;
			break;
		case nsConnecting:
			//font.setWeight(skinSettings.listsWeightActive);
			return font;
			break;
		default:
			//font.setWeight(skinSettings.listsWeightNormal);
			return font;
			break;
		}*/

	default:
		break;
	}

	return QVariant();
}

QVariant HostCacheTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation != Qt::Horizontal )
	{
		return QVariant();
	}

	if ( role == Qt::DisplayRole )
	{
		switch ( section )
		{
		case ADDRESS:
			return tr( "IP Address" );

		case LASTCONNECT:
			return tr( "Last Connected" );

		case FAILURES:
			return tr( "Failure Count" );

		case COUNTRY:
			return tr( "Country" );
		}
	}
	else if ( role == Qt::ToolTipRole )
	{
		switch ( section )
		{
		case ADDRESS:
			return tr( "The Host IP Address" );

		case LASTCONNECT:
			return tr( "Last time this host has been connected" );

		case FAILURES:
			return tr( "The number of unsuccessful connections in a row" );

		case COUNTRY:
			return tr( "The host country" );
		}
	}

	return QVariant();
}

QModelIndex HostCacheTableModel::index( int row, int column, const QModelIndex& parent ) const
{
	if ( parent.isValid() || row < 0 || row >= m_vHosts.size() )
	{
		return QModelIndex();
	}
	else
	{
		return createIndex( row, column, m_vHosts[row] );
	}
}

class G2CacheTableModelCmp
{
public:
	G2CacheTableModelCmp( int nCol, Qt::SortOrder nOrder ) :
		m_nColumn( nCol ),
		m_nOrder( nOrder )
	{
	}

	/**
	 * @brief operator () Allows to compare a and b.
	 * @param a
	 * @param b
	 * @return true if a is smaller than b in the current sort order; false otherwise.
	 */
	bool operator()( const HostData* const a, const HostData* const b ) const
	{
		Q_ASSERT( a );
		Q_ASSERT( b );

		if ( m_nOrder == Qt::AscendingOrder )
		{
			return a->lessThan( m_nColumn, b );
		}
		else
		{
			return b->lessThan( m_nColumn, a );
		}
	}

	int m_nColumn;
	Qt::SortOrder m_nOrder;
};

void HostCacheTableModel::sort( int column, Qt::SortOrder order )
{
	m_nSortColumn = column;
	m_nSortOrder  = order;

	emit layoutAboutToBeChanged();

	// I hope this is the correct way to convince Qt...
	QModelIndexList oldIdx = persistentIndexList();
	QModelIndexList newIdx = oldIdx;

	qStableSort( m_vHosts.begin(), m_vHosts.end(), G2CacheTableModelCmp( column, order ) );

	for ( int i = 0; i < oldIdx.size(); ++i ) // For each persistent index
	{
		int oldRow = oldIdx.at( i ).row();

		// if oldRow is outside range
		if ( oldRow >= m_vHosts.size()
			 // or the index points to another item
			 || oldIdx.at( i ).internalPointer() != m_vHosts[oldRow] )
		{
			// find the correct item and update persistent index
			for ( int j = 0; j < m_vHosts.size(); ++j )
			{
				if ( oldIdx.at( i ).internalPointer() == m_vHosts[j] )
				{
					newIdx[i] = createIndex( j, oldIdx.at( i ).column(),
											 oldIdx.at( i ).internalPointer() );
					break;
				}
			}
		}
	}

	changePersistentIndexList( oldIdx, newIdx );
	emit layoutChanged();

	m_bNeedSorting = false;
}

/*int HostCacheTableModel::find( quint32 nRuleID )
{
	const int nSize = ( int )m_vHosts.size();

	for ( int nPos = 0; nPos < nSize; ++nPos )
	{
		if ( m_vHosts[nPos]->m_nID == nRuleID )
		{
			return nPos;
		}
	}

	return -1;
}*/

HostData* HostCacheTableModel::dataFromRow( int nRow ) const
{
	if ( nRow < m_vHosts.size() && nRow >= 0 )
	{
		return m_vHosts[nRow];
	}
	else
	{
		return NULL;
	}
}

/*Rule* SecurityTableModel::ruleFromIndex(const QModelIndex &index) const
{
	if ( index.isValid() && index.row() < m_lNodes.count() && index.row() >= 0 )
		return m_lNodes[ index.row() ]->m_pRule;
	else
		return NULL;
}*/

void HostCacheTableModel::completeRefresh()
{
	// Remove all Hosts.
	if ( m_vHosts.size() )
	{
		beginRemoveRows( QModelIndex(), 0, ( int )m_vHosts.size() - 1 );
		for ( uint i = 0; i < m_vHosts.size(); ++i )
		{
			delete m_vHosts[i];
		}
		m_vHosts.clear();
		endRemoveRows();
	}

	// Note that this slot is automatically disconnected once all Hosts have been recieved once.
	connect( &hostCache, SIGNAL( hostInfo( HostData* ) ), this,
			 SLOT( recieveHostInfo( HostData* ) ), Qt::QueuedConnection );

#ifdef _DEBUG
	hostCache.verifyIterators();
#endif

	// Request getting them back from the Host Cache.
	m_nHostInfo = hostCache.requestHostInfo();

	// avoid unnecessary reallocations in the future
	m_vHosts.reserve( 2 * m_nHostInfo );

	if ( !m_nHostInfo )
	{
		disconnect( &hostCache, SIGNAL( hostInfo( HostData* ) ),
					this, SLOT( recieveHostInfo( HostData* ) ) );
	}
}

/**
 * @brief SecurityTableModel::triggerRuleRemoval
 * @param nIndex
 */
void HostCacheTableModel::triggerHostRemoval( int nIndex )
{
	Q_ASSERT( nIndex >= 0 && nIndex < m_vHosts.size() );

	hostCache.remove( m_vHosts[nIndex]->m_oAddress );
}

void HostCacheTableModel::recieveHostInfo( HostData* pHostData )
{
	Q_ASSERT( pHostData->m_oAddress.isValid() );

	--m_nHostInfo;

	// This handles disconnecting the ruleInfo signal after a completeRefresh() has been finished.
	if ( !m_nHostInfo )
	{
		// Make sure we don't recieve any signals we don't want once we got all hosts once.
		disconnect( &hostCache, SIGNAL( hostInfo( HostData* ) ),
					this, SLOT( recieveHostInfo( HostData* ) ) );
	}

	addHost( pHostData );
}

/**
 * @brief addHost adds a rule to the GUI.
 * @param pHost : the host
 */
void HostCacheTableModel::addHost( HostData* pHostData )
{
	Q_ASSERT( pHostData->m_oAddress.isValid() );

	if ( pHostData->m_nType == DiscoveryProtocol::G2 )
	{
		insert( pHostData );
	}
}

/**
 * @brief removeHost removes a host from the table model.
 * This is to be triggered from the host cache AFTER the host has been removed.
 * @param pHost : the host
 */
void HostCacheTableModel::removeHost( SharedHostPtr pHost )
{
	for ( int i = 0; i < m_vHosts.size(); ++i )
	{
		if ( m_vHosts[i]->m_pHost == pHost.data() )
		{
			erase( i );

			// m_bNeedSorting needs not to be set to true here as sorting is not required on removal
			break;
		}
	}
}

/**
 * @brief updateHost updates the GUI for a specified host.
 * @param nHostID : the ID of the host
 */
/*void HostCacheTableModel::updateHost(quint32 nHostID)
{
	QModelIndexList uplist;

	const int nHostRowPos = find( nHostID );

	Q_ASSERT( nHostRowPos != -1 );

	hostCache.m_pSection.lock();
	if ( m_vHosts[nHostRowPos]->update( nHostRowPos, m_nSortColumn, uplist, this ) )
	{
		m_bNeedSorting = true;
	}
	hostCache.m_pSection.unlock();

	updateView( uplist );
}*/

/**
 * @brief updateAll updates all hosts in the GUI.
 */
void HostCacheTableModel::updateAll()
{
	if ( ( quint32 )m_vHosts.size() != hostCache.size() )
	{
		completeRefresh();
		return;
	}

	QModelIndexList uplist;

	hostCache.lock();
	for ( int i = 0, max = ( int )m_vHosts.size(); i < max; ++i )
	{
		if ( m_vHosts[i]->update( i, m_nSortColumn, uplist, this ) )
		{
			m_bNeedSorting = true;
		}
	}
	hostCache.unlock();

	updateView( uplist );
}

void HostCacheTableModel::clear()
{
	if ( m_vHosts.size() )
	{
		beginRemoveRows( QModelIndex(), 0, ( int )( m_vHosts.size() - 1 ) );
		m_vHosts.clear();
		m_bNeedSorting = false;
		endRemoveRows();
	}
}

void HostCacheTableModel::updateView( QModelIndexList uplist )
{
	// if necessary adjust container order (also updates view)
	if ( m_bNeedSorting )
	{
		sort( m_nSortColumn, m_nSortOrder );
	}
	// update view for all changed model indexes
	else if ( !uplist.isEmpty() )
	{
		QAbstractItemView* pView = qobject_cast< QAbstractItemView* >( m_oContainer );

		if ( pView )
		{
			foreach ( const QModelIndex & index, uplist )
			{
				pView->update( index );
			}
		}
	}
}

int HostCacheTableModel::findInsertPos( const HostData* const pData )
{
	if ( m_vHosts.empty() )
	{
		return 0;
	}

	const HostData* const * const pHosts = &m_vHosts[0];
	G2CacheTableModelCmp oComparator = G2CacheTableModelCmp( m_nSortColumn, m_nSortOrder );

	int nMiddle;
	int nPosStart = 0;
	int nPosEnd   = ( int )m_vHosts.size() - 1;

	// REMOVE for Quazaa 1.0
#ifdef _DEBUG
	{
	for ( int i = 0; i < nPosEnd - 1; ++i )
	{
		bool bSmaller   =  oComparator( pHosts[i], pHosts[i+1] );
		bool bNotBigger = !oComparator( pHosts[i+1], pHosts[i] );
		if ( !bSmaller && bNotBigger )
		{
			// may not be equal
			Q_ASSERT( bSmaller || bNotBigger );
		}
	}
	}
#endif // _DEBUG

	while ( nPosStart <= nPosEnd )
	{
		nMiddle = ( nPosStart + nPosEnd ) / 2;

		// if pData smaller than pHosts[nMiddle] in current sort order
		if ( oComparator( pData, pHosts[nMiddle] ) )
		{
			nPosEnd = nMiddle - 1;
		}
		else
		{
			nPosStart = nMiddle + 1;
		}
	}

	return nPosEnd + 1;
}

void HostCacheTableModel::insert( HostData* pData )
{
	if ( m_bNeedSorting )
	{
		beginInsertRows( QModelIndex(), ( int )m_vHosts.size(), ( int )m_vHosts.size() );

		m_vHosts.push_back( pData );

		endInsertRows();

		sort( m_nSortColumn, m_nSortOrder );
	}
	else
	{
		int nPos = findInsertPos( pData );

		beginInsertRows( QModelIndex(), nPos, nPos );
		insertAt( pData, nPos );
		endInsertRows();
	}
}

void HostCacheTableModel::insertAt( HostData* pData, const int nPos )
{
	const int nMax = ( int )m_vHosts.size();

#ifdef _DEBUG
	Q_ASSERT( pData );
	Q_ASSERT( nPos >= 0 && nPos <= nMax );
#endif // _DEBUG

	if ( nPos == nMax )
	{
		m_vHosts.push_back( pData );
	}
	else
	{
		m_vHosts.push_back( NULL );

		HostData** pHosts = &m_vHosts[0];
		memmove( pHosts + nPos + 1, pHosts + nPos, ( nMax - nPos ) * sizeof( HostData* ) );
		pHosts[nPos] = pData;
	}
}

void HostCacheTableModel::erase( const int nPos )
{
	beginRemoveRows( QModelIndex(), nPos, nPos );

	const int nMax = ( int )m_vHosts.size() - 1;
	HostData** pHosts = &m_vHosts[0];
	delete pHosts[nPos];

	// Move all items on positions after nPos one spot to the left.
	memmove( pHosts + nPos, pHosts + nPos + 1, ( nMax - nPos ) * sizeof( HostData* ) );

	m_vHosts.pop_back();
	endRemoveRows();
}
