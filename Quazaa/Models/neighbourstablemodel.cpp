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

#include "neighbourstablemodel.h"

#include <QColor>
#include <QSize>
#include <QIcon>
#include <QMutexLocker>
#include <QFont>
#include "neighbour.h"
#include "g2node.h"
#include "network.h"
#include "neighbours.h"
#include "geoiplist.h"
#include <qabstractitemview.h>
#include "networkiconprovider.h"

#include "debug_new.h"

NeighboursTableModel::NeighbourData::NeighbourData( Neighbour* pNeighbour ) : pNode( pNeighbour )
{
	quint32 tNow = time( 0 );

	sHandshake      = pNode->m_sHandshake;
	oAddress        = pNode->address();
	tConnected      = tNow - pNode->connectTime();
	nBandwidthIn    = pNode->m_mInput.usage();
	nBandwidthOut   = pNode->m_mOutput.usage();
	nBytesReceived  = pNode->m_mInput.m_nTotal;
	nBytesSent      = pNode->m_mOutput.m_nTotal;
	nCompressionIn  = pNode->getTotalInDecompressed();
	nCompressionOut = pNode->getTotalOutCompressed();
	nLeafCount      = 0;
	nLeafMax        = 0;
	nPacketsIn      = pNode->m_nPacketsIn;
	nPacketsOut     = pNode->m_nPacketsOut;
	nRTT            = pNode->m_tRTT;
	nState          = pNode->m_nState;
	nType           = G2_UNKNOWN;
	sUserAgent      = pNode->m_sUserAgent;
	sCountryCode    = geoIP.findCountryCode( oAddress );
	sCountry        = geoIP.countryNameFromCode( sCountryCode );
	iCountry        = NetworkIconProvider::icon( sCountryCode );

	switch ( pNode->m_nProtocol )
	{
	case DiscoveryProtocol::G2:
		nDiscoveryProtocol = DiscoveryProtocol::G2;
		nLeafCount = ( ( G2Node* )pNode )->m_nLeafCount;
		nLeafMax   = ( ( G2Node* )pNode )->m_nLeafMax;
		nType      = ( ( G2Node* )pNode )->m_nType;
		break;

	default:
		break;
	}

	iNetwork = NetworkIconProvider::icon( pNode->m_nProtocol );
}

bool NeighboursTableModel::NeighbourData::update( int row, int col, QModelIndexList& to_update,
												  NeighboursTableModel* model )
{
	if ( !neighbours.neighbourExists( pNode ) )
	{
		return false;
	}

	bool bRet = false;

	quint32 tNow = time( 0 );

	sHandshake = pNode->m_sHandshake;

	if ( oAddress != pNode->address() )
	{
		to_update.append( model->index( row, ADDRESS ) );
		oAddress = pNode->address();

		if ( col == ADDRESS )
		{
			bRet = true;
		}
	}

	if ( nState != pNode->m_nState && nState != nsConnected )
	{
		nState = pNode->m_nState;
		to_update.append( model->index( row, TIME ) );

		if ( col == TIME )
		{
			bRet = true;
		}
	}
	else if ( nState == nsConnected )
	{
		tConnected = tNow - pNode->connectTime();
		to_update.append( model->index( row, TIME ) );

		if ( col == TIME )
		{
			bRet = true;
		}
	}

	nBandwidthIn  = pNode->m_mInput.usage();
	nBandwidthOut = pNode->m_mOutput.usage();
	to_update.append( model->index( row, BANDWIDTH ) );

	if ( col == BANDWIDTH )
	{
		bRet = true;
	}

	nBytesReceived  = pNode->m_mInput.m_nTotal;
	nBytesSent      = pNode->m_mOutput.m_nTotal;
	nCompressionIn  = pNode->getTotalInDecompressed();
	nCompressionOut = pNode->getTotalOutCompressed();
	to_update.append( model->index( row, BYTES ) );

	if ( col == BYTES )
	{
		bRet = true;
	}

	if ( nPacketsIn != pNode->m_nPacketsIn || nPacketsOut != pNode->m_nPacketsOut )
	{
		nPacketsIn  = pNode->m_nPacketsIn;
		nPacketsOut = pNode->m_nPacketsOut;
		to_update.append( model->index( row, PACKETS ) );

		if ( col == PACKETS )
		{
			bRet = true;
		}
	}

	if ( nRTT != pNode->m_tRTT )
	{
		nRTT = pNode->m_tRTT;
		to_update.append( model->index( row, PING ) );
		if ( col == PING )
		{
			bRet = true;
		}
	}

	if ( sUserAgent != pNode->m_sUserAgent )
	{
		sUserAgent = pNode->m_sUserAgent;
		to_update.append( model->index( row, USER_AGENT ) );

		if ( col == USER_AGENT )
		{
			bRet = true;
		}
	}

	switch ( pNode->m_nProtocol )
	{
	case DiscoveryProtocol::G2:
		if ( nLeafCount != ( ( G2Node* )pNode )->m_nLeafCount ||
			 nLeafMax   != ( ( G2Node* )pNode )->m_nLeafMax )
		{
			nLeafCount = ( ( G2Node* )pNode )->m_nLeafCount;
			nLeafMax   = ( ( G2Node* )pNode )->m_nLeafMax;
			to_update.append( model->index( row, LEAVES ) );

			if ( col == LEAVES )
			{
				bRet = true;
			}
		}

		if ( nType != ( ( G2Node* )pNode )->m_nType )
		{
			nType = ( ( G2Node* )pNode )->m_nType;
			to_update.append( model->index( row, MODE ) );

			if ( col == MODE )
			{
				bRet = true;
			}
		}
		break;

	default:
		break;
	}

	return bRet;
}

QVariant NeighboursTableModel::NeighbourData::data( int col ) const
{
	switch ( col )
	{
	case ADDRESS:
		return const_cast<EndPoint*>( &oAddress )->toStringWithPort();
	case TIME:
		if ( nState == nsConnected )
			return QString().sprintf( "%.2u:%.2u:%.2u", tConnected / 3600,
									  tConnected % 3600 / 60, ( tConnected % 3600 ) % 60 );
		else
		{
			return stateToString( nState );
		}

	case BANDWIDTH:
		return QString().sprintf( "%1.3f / %1.3f", nBandwidthIn / 1024.0f,
								  nBandwidthOut / 1024.0f );

	case BYTES:
		return QString().sprintf( "%1.3f / %1.3f KB (%1.2f / %1.2f %%)",
								  nBytesReceived / 1024.0f,
								  nBytesSent / 1024.0f, 100.0f * nCompressionIn,
								  100.0f * nCompressionOut );

	case PACKETS:
		return QString().sprintf( "%u - %u", nPacketsIn, nPacketsOut );

	case MODE:
		return typeToString( nType );

	case LEAVES:
		if ( nType == G2_HUB )
		{
			return QString().sprintf( "%u / %u", nLeafCount, nLeafMax );
		}
		else
		{
			return QString();
		}

	case PING:
		if ( nRTT > 0 )
		{
			return QString( "%1 ms" ).arg( nRTT );
		}
		else
		{
			return QVariant();
		}

	case USER_AGENT:
		return sUserAgent;

	case COUNTRY:
		return sCountry;
	}

	return QVariant();
}

bool NeighboursTableModel::NeighbourData::lessThan( int col, NeighbourData* pOther ) const
{
	switch ( col )
	{
	case ADDRESS:
		if ( oAddress.protocol() == QAbstractSocket::IPv4Protocol )
		{
			return oAddress < pOther->oAddress;
		}
		else
		{
			return oAddress < pOther->oAddress;
		}

	case TIME:
		if ( nState != nsConnected )
		{
			return true;
		}
		return tConnected < pOther->tConnected;

	case BANDWIDTH:
		return ( nBandwidthIn + nBandwidthOut ) <
			   ( pOther->nBandwidthIn + pOther->nBandwidthOut );

	case BYTES:
		return ( nBytesReceived + nBytesSent ) <
			   ( pOther->nBytesReceived + pOther->nBytesSent );

	case PACKETS:
		return ( nPacketsIn + nPacketsOut ) < ( pOther->nPacketsIn + pOther->nPacketsOut );

	case MODE:
		return typeToString( nType ) < typeToString( pOther->nType );

	case LEAVES:
		if ( nLeafCount == pOther->nLeafCount )
		{
			return nLeafMax < pOther->nLeafMax;
		}

		return nLeafCount < pOther->nLeafCount;

	case PING:
		if ( nState < nsConnected )
		{
			return false;
		}

		if ( pOther->nState < nsConnected )
		{
			return true;
		}
		return nRTT < pOther->nRTT;

	case USER_AGENT:
		return sUserAgent < pOther->sUserAgent;

	case COUNTRY:
		return geoIP.countryNameFromCode( sCountry ) <
			   geoIP.countryNameFromCode( pOther->sCountry );
	default:
		return false;
	}

}
QString NeighboursTableModel::NeighbourData::stateToString( int s ) const
{
	switch ( s )
	{
	case nsClosed:
		return "closed";
	case nsConnecting:
		return "connecting";
	case nsHandshaking:
		return "handshaking";
	case nsConnected:
		return "connected";
	case nsClosing:
		return "closing";
	case nsError:
		return "error";
	default:
		return QString();
	}
}
QString NeighboursTableModel::NeighbourData::typeToString( G2NodeType t ) const
{
	if ( t == G2_HUB )
	{
		return QString( "G2 Hub" );
	}
	else if ( t == G2_LEAF )
	{
		return QString( "G2 Leaf" );
	}
	else
	{
		return QString();
	}
}

NeighboursTableModel::NeighboursTableModel( QObject* parent, QWidget* container ) :
	QAbstractTableModel( parent )
{
	m_oContainer   = container;
	m_nSortColumn  = -1;
	m_bNeedSorting = false;

	connect( &neighbours, SIGNAL( neighbourAdded( Neighbour* ) ),
			 this, SLOT( addNode( Neighbour* ) ), Qt::QueuedConnection );
	connect( &neighbours, SIGNAL( neighbourRemoved( Neighbour* ) ),
			 this, SLOT( removeNode( Neighbour* ) ), Qt::QueuedConnection );
}

NeighboursTableModel::~NeighboursTableModel()
{
	qDeleteAll( m_lNodes );
	m_lNodes.clear();
}

int NeighboursTableModel::rowCount( const QModelIndex& parent ) const
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

int NeighboursTableModel::columnCount( const QModelIndex& parent ) const
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

QVariant NeighboursTableModel::data( const QModelIndex& index, int role ) const
{
	if ( !index.isValid() )
	{
		return QVariant();
	}

	if ( index.row() > m_lNodes.size() || index.row() < 0 )
	{
		return QVariant();
	}

	const NeighbourData* nbr = m_lNodes.at( index.row() );

	if ( role == Qt::DisplayRole )
	{
		return nbr->data( index.column() );
	}
	else if ( role == Qt::DecorationRole )
	{
		if ( index.column() == ADDRESS )
		{
			return nbr->iNetwork;
		}
		else if ( index.column() == COUNTRY )
		{
			return nbr->iCountry;
		}
	}
	// TODO: Reimplement formatting options and use them for models.
	else if ( role == Qt::ForegroundRole )
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
	}
	else if ( role == Qt::FontRole )
	{
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
		}
	}

	return QVariant();
}

QVariant NeighboursTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
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
			return tr( "Address" );
		case TIME:
			return tr( "Time" );
		case BANDWIDTH:
			return tr( "Bandwidth" );
		case BYTES:
			return tr( "Bytes" );
		case PACKETS:
			return tr( "Packets" );
		case MODE:
			return tr( "Mode" );
		case LEAVES:
			return tr( "Leaves" );
		case PING:
			return tr( "Ping" );
		case USER_AGENT:
			return tr( "User Agent" );
		case COUNTRY:
			return tr( "Country" );
		}
	}
	else if ( role == Qt::ToolTipRole )
	{
		switch ( section )
		{
		case ADDRESS:
			return tr( "The address of remote host" );
		case TIME:
			return tr( "When this host was connected" );
		case BANDWIDTH:
			return tr( "Current download/upload bandwidth (and dl/ul compression)" );
		case BYTES:
			return tr( "How much data we downloaded/uploaded" );
		case PACKETS:
			return tr( "Number of received/sent packets" );
		case MODE:
			return tr( "Type of connection" );
		case LEAVES:
			return tr( "Number of nodes connected to the host" );
		case PING:
			return tr( "How fast remote host can reply" );
		case USER_AGENT:
			return tr( "Software operating on remote host" );
		case COUNTRY:
			return tr( "Country" );
		}
	}

	return QVariant();
}
QModelIndex NeighboursTableModel::index( int row, int column, const QModelIndex& parent ) const
{
	if ( parent.isValid() )
	{
		return QModelIndex();
	}

	if ( row < 0 || row >= m_lNodes.count() )
	{
		return QModelIndex();
	}
	else
	{
		return createIndex( row, column, m_lNodes[row] );
	}
}

class NeighboursTableModelCmp
{
public:
	int           column;
	Qt::SortOrder order;

	NeighboursTableModelCmp( int col, Qt::SortOrder o ) :
		column( col ),
		order( o )
	{}

	bool operator()( NeighboursTableModel::NeighbourData* a, NeighboursTableModel::NeighbourData* b )
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
};

void NeighboursTableModel::sort( int column, Qt::SortOrder order )
{
	m_nSortColumn = column;
	m_nSortOrder  = order;

	emit layoutAboutToBeChanged();

	// I hope this is the correct way to convince Qt...
	QModelIndexList oldIdx = persistentIndexList();
	QModelIndexList newIdx = oldIdx;

	qStableSort( m_lNodes.begin(), m_lNodes.end(), NeighboursTableModelCmp( column, order ) );

	for ( int i = 0; i < oldIdx.size(); ++i ) // For each persistent index
	{
		int oldRow = oldIdx.at( i ).row();

		// if oldRow is outside range
		if ( oldRow > m_lNodes.size()
			 // or the index points to another item
			 || oldIdx.at( i ).internalPointer() != m_lNodes.at( oldRow ) )
		{
			// find the correct item and update persistent index
			for ( int j = 0; j < m_lNodes.size(); ++j )
			{
				if ( oldIdx.at( i ).internalPointer() == m_lNodes.at( j ) )
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
}

Neighbour* NeighboursTableModel::nodeFromIndex( const QModelIndex& index )
{
	if ( index.isValid() && index.row() < m_lNodes.count() && index.row() >= 0 )
	{
		return m_lNodes[index.row()]->pNode;
	}
	else
	{
		return 0;
	}
}

void NeighboursTableModel::addNode( Neighbour* pNode )
{
	neighbours.m_pSection.lock();
	if ( neighbours.neighbourExists( pNode ) )
	{
		beginInsertRows( QModelIndex(), m_lNodes.size(), m_lNodes.size() );
		m_lNodes.append( new NeighbourData( pNode ) );
		endInsertRows();
		m_bNeedSorting = true;
	}
	neighbours.m_pSection.unlock();
}

void NeighboursTableModel::removeNode( Neighbour* pNode )
{
	for ( int i = 0, nMax = m_lNodes.size(); i < nMax; ++i )
	{
		if ( m_lNodes[i]->pNode == pNode )
		{
			// update selection
			QModelIndexList currIdx = persistentIndexList();
			QModelIndexList newIdx = currIdx;
			bool bChanged = false;

			for ( int j = 0; j < currIdx.size(); ++j )
			{
				if ( currIdx.at( j ).internalPointer() == m_lNodes.at( i ) )
				{
					newIdx[j] = QModelIndex();
					bChanged = true;
				}
			}
			if ( bChanged )
			{
				changePersistentIndexList( currIdx, newIdx );
			}

			// remove the item
			beginRemoveRows( QModelIndex(), i, i );
			delete m_lNodes[i];
			m_lNodes.remove( i, 1 );
			endRemoveRows();
			m_bNeedSorting = true;

			break;
		}
	}
}

void NeighboursTableModel::updateAll()
{
	QModelIndexList uplist;
	bool bSort = m_bNeedSorting;

	if ( neighbours.m_pSection.tryLock() )
	{
		for ( int i = 0, max = m_lNodes.count(); i < max; ++i )
		{
			if ( m_lNodes[i]->update( i, m_nSortColumn, uplist, this ) )
			{
				bSort = true;
			}
		}

		neighbours.m_pSection.unlock();
	}

	if ( bSort )
	{
		sort( m_nSortColumn, m_nSortOrder );
		m_bNeedSorting = false;
	}
	else
	{
		if ( !uplist.isEmpty() )
		{
			QAbstractItemView* pView = qobject_cast<QAbstractItemView*>( m_oContainer );

			if ( pView )
			{
				foreach ( const QModelIndex & index, uplist )
				{
					pView->update( index );
				}
			}
		}
	}
}

