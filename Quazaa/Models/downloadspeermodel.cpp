/*
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

#include "downloadspeermodel.h"

#include "downloadstreemodel.h"

CDownloadsPeerModel::CDownloadsPeerModel( QObject* parent ) :
	QSortFilterProxyModel( parent )
{
}

QVariant CDownloadsPeerModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
	{
		switch ( section )
		{
		case CDownloadsTreeModel::NAME:
			return tr( "IP" );
			break;
		case CDownloadsTreeModel::SIZE:
			return tr( "Size" );
			break;
		case CDownloadsTreeModel::PROGRESS:
			return tr( "Progress" );
			break;
		case CDownloadsTreeModel::BANDWIDTH:
			return tr( "Bandwidth" );
			break;
		case CDownloadsTreeModel::STATUS:
			return tr( "Status" );
			break;
		case CDownloadsTreeModel::PRIORITY:
			return tr( "Priority" );
			break;
		case CDownloadsTreeModel::CLIENT:
			return tr( "Client" );
			break;
		case CDownloadsTreeModel::COMPLETED:
			return tr( "Completed" );
			break;
		case CDownloadsTreeModel::COUNTRY:
			return tr( "Country" );
			break;
		default:
			return QVariant();
		}
	}

	return QVariant();
}

void CDownloadsPeerModel::setCurrentRoot( const QModelIndex& index )
{
	Q_ASSERT( index.model() == sourceModel() );

	m_oCurrentRoot = index;
	invalidate();
}

bool CDownloadsPeerModel::filterAcceptsRow( int sourceRow, const QModelIndex& sourceParent ) const
{
	QModelIndex index1 = sourceModel()->index( sourceRow, 0, sourceParent );

	return ( m_oCurrentRoot.isValid() && ( index1 == m_oCurrentRoot || index1.parent() == m_oCurrentRoot ) );
}
