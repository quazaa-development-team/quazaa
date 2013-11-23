/*
** discoverytablemodel.cpp
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

#include <QAbstractItemView>

#include "discoverytablemodel.h"
#include "Discovery/gwc.h"

#include "debug_new.h"

CDiscoveryTableModel::Service::Service(TConstServicePtr pService, CDiscoveryTableModel* model) :
	m_pNode       ( pService                 ),
	m_nID         ( pService->id()           ),
	m_nType       ( pService->serviceType()  ),
	m_sType       ( pService->type()         ),
	m_bBanned     ( pService->isBanned()     ),
	m_sURL        ( pService->url()          ),
	m_tAccessed   ( pService->lastAccessed() ),
	m_nLastHosts  ( pService->lastHosts()    ),
	m_nTotalHosts ( pService->totalHosts()   ),
	m_nAltServices( pService->altServices()  ),
	m_nFailures   ( pService->failures()     ),
#if ENABLE_DISCOVERY_DEBUGGING
	m_nRating       ( pService->rating()     ),
	m_nMultipilcator( pService->probaMult()  ),
#endif
	m_sPong       ( pService->pong()         )

{
	refreshServiceIcon( model );

	connect( m_pNode.data(), &Discovery::CDiscoveryService::updated, model,
			 &CDiscoveryTableModel::update, Qt::QueuedConnection );
}

CDiscoveryTableModel::Service::~Service()
{
}

bool CDiscoveryTableModel::Service::update(int row, int sortCol, QModelIndexList &to_update,
										   CDiscoveryTableModel* model)
{
	bool bReturn = false;

	m_pNode->lockForRead();

	if ( m_nType != m_pNode->serviceType() || m_bBanned != m_pNode->isBanned() )
	{
		to_update.append( model->index( row, TYPE ) );

		m_nType   = m_pNode->serviceType();
		m_sType   = m_pNode->type();
		m_bBanned = m_pNode->isBanned();

		refreshServiceIcon( model );

		if ( sortCol == TYPE )
			bReturn = true;
	}

	if ( m_sURL != m_pNode->url() )
	{
		to_update.append( model->index( row, URL ) );
		m_sURL = m_pNode->url();

		if ( sortCol == URL )
			bReturn = true;
	}

	if ( m_tAccessed != m_pNode->lastAccessed() )
	{
		to_update.append( model->index( row, ACCESSED ) );
		m_tAccessed = m_pNode->lastAccessed();

		if ( sortCol == ACCESSED )
			bReturn = true;
	}

	if ( m_nLastHosts != m_pNode->lastHosts() )
	{
		to_update.append( model->index( row, HOSTS ) );
		m_nLastHosts = m_pNode->lastHosts();

		if ( sortCol == HOSTS )
			bReturn = true;
	}

	if ( m_nTotalHosts != m_pNode->totalHosts() )
	{
		to_update.append( model->index( row, TOTAL_HOSTS ) );
		m_nTotalHosts = m_pNode->totalHosts();

		if ( sortCol == TOTAL_HOSTS )
			bReturn = true;
	}

	if ( m_nAltServices != m_pNode->altServices() )
	{
		to_update.append( model->index( row, ALTERNATE_SERVICES ) );
		m_nAltServices = m_pNode->altServices();

		if ( sortCol == ALTERNATE_SERVICES )
			bReturn = true;
	}

	if ( m_nFailures != m_pNode->failures() )
	{
		to_update.append( model->index( row, FAILURES ) );
		m_nFailures = m_pNode->failures();

		if ( sortCol == FAILURES )
			bReturn = true;
	}

#if ENABLE_DISCOVERY_DEBUGGING
	if ( m_nRating != m_pNode->rating() )
	{
		to_update.append( model->index( row, RATING ) );
		m_nRating = m_pNode->rating();

		if ( sortCol == RATING )
			bReturn = true;
	}

	if ( m_nMultipilcator != m_pNode->probaMult() )
	{
		to_update.append( model->index( row, MULTIPLICATOR ) );
		m_nMultipilcator = m_pNode->probaMult();

		if ( sortCol == MULTIPLICATOR )
			bReturn = true;
	}
#endif

	if ( m_sPong != m_pNode->pong() )
	{
		to_update.append( model->index( row, PONG ) );
		m_sPong = m_pNode->pong();

		if ( sortCol == PONG )
			bReturn = true;
	}

	m_pNode->unlock();

	return bReturn;
}

QVariant CDiscoveryTableModel::Service::data(int col) const
{
	switch ( col )
	{
	case TYPE:
		return m_sType;

	case URL:
		return m_sURL;

	case ACCESSED:
		return m_tAccessed ? QDateTime::fromTime_t( m_tAccessed ).toString() : tr( "N/A" );

	case HOSTS:
		return QString::number( m_nLastHosts );

	case TOTAL_HOSTS:
		return QString::number( m_nTotalHosts );

	case ALTERNATE_SERVICES:
		return QString::number( m_nAltServices );

	case FAILURES:
		return QString::number( m_nFailures );

#if ENABLE_DISCOVERY_DEBUGGING
	case RATING:
		return QString::number( m_nRating );

	case MULTIPLICATOR:
		return QString::number( m_nMultipilcator );
#endif

	case PONG:
		return m_sPong;
	}

	return QVariant();
}

bool CDiscoveryTableModel::Service::lessThan(int col, const CDiscoveryTableModel::Service*
											 const pOther) const
{
	if ( !pOther )
		return false;

	switch ( col )
	{
	case TYPE:
		return (         m_bBanned ?         m_nType * -1 :         m_nType ) <
			   ( pOther->m_bBanned ? pOther->m_nType * -1 : pOther->m_nType );

	case URL:
		return m_sURL  < pOther->m_sURL;

	case ACCESSED:
		return m_tAccessed < pOther->m_tAccessed;

	case HOSTS:
		return m_nLastHosts < pOther->m_nLastHosts;

	case TOTAL_HOSTS:
		return m_nTotalHosts < pOther->m_nTotalHosts;

	case ALTERNATE_SERVICES:
		return m_nAltServices < pOther->m_nAltServices;

	case FAILURES:
		return m_nFailures < pOther->m_nFailures;

#if ENABLE_DISCOVERY_DEBUGGING
	case RATING:
		return m_nRating < pOther->m_nRating;

	case MULTIPLICATOR:
		return m_nMultipilcator < pOther->m_nMultipilcator;
#endif

	case PONG:
		return m_sPong < pOther->m_sPong;

	default:
		return false;
	}
}

void CDiscoveryTableModel::Service::refreshServiceIcon(CDiscoveryTableModel* model)
{
	switch( m_nType )
	{
	case Discovery::stNull:
		Q_ASSERT( false ); // Should not happen.
		break;

	case Discovery::stBanned:
		m_piType = model->m_pIcons[BANNED];
		break;

	case Discovery::stGWC:
	{
		Discovery::CGWC* pGWC = (Discovery::CGWC*)m_pNode.data();

		if ( pGWC->isG2() )
		{
			if ( pGWC->isGnutella() )
			{
				m_piType = model->m_pIcons[GWC_GREEN];
			}
			else
			{
				m_piType = model->m_pIcons[GWC_BLUE];
			}
		}
		else
		{
			m_piType = model->m_pIcons[GWC_GRAY];
		}
	}
		break;

	default:
		Q_ASSERT( false );
	}

	if ( /*m_nType == Discovery::stBanned ||*/ m_bBanned )
	{
		m_piType = model->m_pIcons[BANNED];
	}
}

CDiscoveryTableModel::CDiscoveryTableModel(QObject *parent, QWidget* container) :
	QAbstractTableModel( parent ),
	m_oContainer( container ),
	m_nSortColumn( -1 ),
	m_bNeedSorting( false )
{
	m_pIcons            = new const QIcon*[_NO_OF_ICONS];
	m_pIcons[BANNED]    = new QIcon( ":/Resource/Discovery/Banned.ico"            );
	m_pIcons[BOOTSTRAP] = new QIcon( ":/Resource/Discovery/BootstrapServer.ico"   );
	m_pIcons[DISCOVERY] = new QIcon( ":/Resource/Discovery/Discovery.ico"         );
	m_pIcons[GWC_GREEN] = new QIcon( ":/Resource/Discovery/DiscoveryGWCGreen.ico" );
	m_pIcons[GWC_BLUE]  = new QIcon( ":/Resource/Discovery/DiscoveryGWCBlue.ico"  );
	m_pIcons[GWC_GRAY]  = new QIcon( ":/Resource/Discovery/DiscoveryGWCGray.ico"  );

	connect( &discoveryManager, &Discovery::CDiscovery::serviceAdded, this,
			 &CDiscoveryTableModel::addService, Qt::QueuedConnection );
	connect( &discoveryManager, &Discovery::CDiscovery::serviceRemoved, this,
			 &CDiscoveryTableModel::removeService, Qt::QueuedConnection );

	// This needs to be called to make sure that all rules added to the discoveryManager before this
	// part of the GUI is loaded are properly added to the model.
	completeRefresh();
}

CDiscoveryTableModel::~CDiscoveryTableModel()
{
	qDeleteAll( m_lNodes );
	m_lNodes.clear();

	for ( quint8 i = 0; i < _NO_OF_ICONS; ++i )
	{
		delete m_pIcons[i];
	}
	delete[] m_pIcons;
}

int CDiscoveryTableModel::rowCount(const QModelIndex& parent) const
{
	if ( parent.isValid() )
	{
		return 0;
	}
	else
	{
		return m_lNodes.count();
	}
}

int CDiscoveryTableModel::columnCount(const QModelIndex& parent) const
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

QVariant CDiscoveryTableModel::data(const QModelIndex& index, int role) const
{
	if ( !index.isValid() || index.row() > m_lNodes.size() || index.row() < 0 )
	{
		Q_ASSERT( false );
		return QVariant();
	}

	const Service* pService = m_lNodes.at( index.row() );

	if ( role == Qt::DisplayRole )
	{
		return pService->data( index.column() );
	}
	else if ( role == Qt::DecorationRole )
	{
		if ( index.column() == TYPE )
		{
			return *pService->m_piType;
		}
	}
	// TODO: Add formatting options back to models
	/*else if ( role == Qt::ForegroundRole )
	{
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
	}*/
	/*else if ( role == Qt::FontRole )
	{
		QFont font = qApp->font(m_oContainer);
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
		}
	}*/

	return QVariant();
}

QVariant CDiscoveryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ( orientation != Qt::Horizontal )
		return QVariant();

	if ( role == Qt::DisplayRole )
	{
		switch ( section )
		{
		case TYPE:
			return tr( "Type" );

		case URL:
			return tr( "URL" );

		case ACCESSED:
			return tr( "Accessed" );

		case HOSTS:
			return tr( "Hosts" );

		case TOTAL_HOSTS:
			return tr( "Total Hosts" );

		case ALTERNATE_SERVICES:
			return tr( "Alt. Services" );

		case FAILURES:
			return tr( "Failures" );

#if ENABLE_DISCOVERY_DEBUGGING
		case RATING:
			return tr( "Rating" );

		case MULTIPLICATOR:
			return tr( "Mult." );
#endif

		case PONG:
			return tr( "Pong" );
		}
	}
	else if ( role == Qt::ToolTipRole )
	{
		switch( section )
		{
		case TYPE:
			return tr( "The type of the Discovery Service" );

		case URL:
			return tr( "The URL of the Discovery Service" );

		case ACCESSED:
			return tr( "The time the Discovery Service has last been accessed" );

		case HOSTS:
			return tr( "The number of hosts returned the last time this service was queried" );

		case TOTAL_HOSTS:
			return tr( "The total number of hosts returned from this service" );

		case ALTERNATE_SERVICES:
			return tr( "The number of alternate discovery services " ) +
				   tr( "we have been provided with by this service when last we queried" );

		case FAILURES:
			return tr( "The number of failed accesses to the service" );

#if ENABLE_DISCOVERY_DEBUGGING
		case RATING:
			return tr( "Rating" );

		case MULTIPLICATOR:
			return tr( "Mult." );
#endif

		case PONG:
			return tr( "The reply " );
		}
	}

	return QVariant();
}

QModelIndex CDiscoveryTableModel::index(int row, int column, const QModelIndex &parent) const
{
	if ( parent.isValid() || row < 0 || row >= m_lNodes.count() )
		return QModelIndex();
	else
		return createIndex( row, column, m_lNodes[row] );
}

class CDiscoveryTableModelCmp
{
public:
	CDiscoveryTableModelCmp( int col, Qt::SortOrder o ) :
		column( col ),
		order( o )
	{
	}

	bool operator()( CDiscoveryTableModel::Service* a, CDiscoveryTableModel::Service* b )
	{
		if ( order == Qt::AscendingOrder )
		{
			return a->lessThan( column, b );
		}
		else
		{
			return b->lessThan( column, a );
		}
	}

	int column;
	Qt::SortOrder order;
};

void CDiscoveryTableModel::sort(int column, Qt::SortOrder order)
{
	m_nSortColumn = column;
	m_nSortOrder = order;

	emit layoutAboutToBeChanged();
	qStableSort( m_lNodes.begin(), m_lNodes.end(), CDiscoveryTableModelCmp( column, order ) );
	emit layoutChanged();

	m_bNeedSorting = false;
}

CDiscoveryTableModel::TConstServicePtr CDiscoveryTableModel::nodeFromRow(quint32 row) const
{
	if ( row < (quint32)m_lNodes.count() )
	{
		return m_lNodes[ row ]->m_pNode;
	}
	else
	{
		return TConstServicePtr();
	}
}

CDiscoveryTableModel::TConstServicePtr
CDiscoveryTableModel::nodeFromIndex(const QModelIndex &index) const
{
	if ( !index.isValid() || !( index.row() < m_lNodes.count() ) || index.row() < 0 )
		return TConstServicePtr();
	else
		return m_lNodes[ index.row() ]->m_pNode;
}


bool CDiscoveryTableModel::isIndexBanned(const QModelIndex& index) const
{
	// All service with type stBanned have m_bBanned set to true.
	return ( m_lNodes[ index.row() ]->m_bBanned /*||
			 m_lNodes[ index.row() ]->m_nType == Discovery::stBanned*/ );
}

void CDiscoveryTableModel::completeRefresh()
{
	// Remove all rules.
	if ( m_lNodes.size() )
	{
		beginRemoveRows( QModelIndex(), 0, m_lNodes.size() - 1 );
		for ( int i = 0; i < m_lNodes.size(); ++i )
		{
			delete m_lNodes[i];
		}
		m_lNodes.clear();
		endRemoveRows();
	}

	// Note that this slot is automatically disconnected once all rules have been recieved once.
	connect( &discoveryManager, SIGNAL( serviceInfo( TConstServicePtr ) ), this,
			 SLOT( addService( TConstServicePtr ) ), Qt::QueuedConnection );

	// Request getting them back from the Security Manager.
	discoveryManager.requestServiceList();
}

void CDiscoveryTableModel::addService(TConstServicePtr pService)
{
	if ( discoveryManager.check( pService ) )
	{
		beginInsertRows( QModelIndex(), m_lNodes.size(), m_lNodes.size() );

		pService->lockForRead();
		m_lNodes.append( new Service( pService, this ) );
		pService->unlock();

		endInsertRows();

		m_bNeedSorting = true;
	}

	// if we're not currently doing a complete refresh/if the complete refresh has just been finished
	if ( m_lNodes.size() == (int)discoveryManager.count() )
	{
		// Make sure we don't recieve any signals we don't want once we got all rules once.
		disconnect( &discoveryManager, SIGNAL( serviceInfo( TConstServicePtr ) ),
					this, SLOT( addService( TConstServicePtr ) ) );

		// Make sure the view stays sorted.
		updateView();
	}
}

void CDiscoveryTableModel::removeService(TServiceID nID)
{
	for ( int i = 0; i < m_lNodes.size(); ++i )
	{
		if ( m_lNodes[i]->m_pNode->id() == nID )
		{
			beginRemoveRows( QModelIndex(), i, i );
			delete m_lNodes[i];
			m_lNodes.remove( i, 1 );
			endRemoveRows();
			break;
		}
	}
}

void CDiscoveryTableModel::update(TServiceID nID)
{
	QModelIndexList uplist;
	for ( quint32 i = 0, max = m_lNodes.count(); i < max; ++i )
	{
		if ( m_lNodes[i]->m_nID == nID )
		{
			if ( m_lNodes[i]->update( i, m_nSortColumn, uplist, this ) )
				m_bNeedSorting = true;
			break;
		}
	}

	updateView( uplist );
}

void CDiscoveryTableModel::updateAll()
{
	if ( (quint32)m_lNodes.size() != discoveryManager.count() )
	{
#ifdef _DEBUG
		// This is something that should not have happened.
		Q_ASSERT( false );
#endif // _DEBUG

		completeRefresh();
		return;
	}

	QModelIndexList uplist;
	for ( quint32 i = 0, max = m_lNodes.count(); i < max; ++i )
	{
		if ( m_lNodes[i]->update( i, m_nSortColumn, uplist, this ) )
			m_bNeedSorting = true;
	}

	updateView( uplist );
}

void CDiscoveryTableModel::updateView(QModelIndexList uplist)
{
	if ( m_bNeedSorting )
	{
		sort( m_nSortColumn, m_nSortOrder );
	}
	else if ( !uplist.isEmpty() )
	{
		QAbstractItemView* pView = qobject_cast< QAbstractItemView* >( m_oContainer );

		if ( pView )
		{
			foreach ( QModelIndex index, uplist )
			{
				pView->update( index );
			}
		}
	}
}

