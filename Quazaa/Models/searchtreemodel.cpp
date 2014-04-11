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

#include "searchtreemodel.h"
#include <QFileInfo>
#include "systemlog.h"
#include "geoiplist.h"
#include "commonfunctions.h"
#include "Hashes/hash.h"
#include "fileiconprovider.h"
#include "networkiconprovider.h"

#include "debug_new.h"

using namespace common;

SearchTreeModel::SearchTreeModel() :
	m_pIconProvider( new CFileIconProvider ),
	m_pFilter( new SearchFilter )
{
	QList<QVariant> rootItemData;
	rootItemData << "File"
				 << "Extension"
				 << "Size"
				 << "Rating"
				 << "Status"
				 << "Host/Count"
				 << "Speed"
				 << "Client"
				 << "Country";
	rootItem = new SearchTreeItem( rootItemData );
	nFileCount = 0;
}

SearchTreeModel::~SearchTreeModel()
{
	clear();
	delete rootItem;
	delete m_pIconProvider;
}

bool SearchTreeModel::isRoot(QModelIndex index)
{
	SearchTreeItem* item;

	if ( !index.isValid() )
	{
		item = rootItem;
	}
	else
	{
		item = static_cast<SearchTreeItem*>( index.internalPointer() );
	}

	if ( item == rootItem )
	{
		return true;
	}

	return false;
}

void SearchTreeModel::removeQueryHit(int position, const QModelIndex &parent)
{
	SearchTreeItem *parentItem = rootItem;
	if (parent.isValid()) {
		parentItem = static_cast<SearchTreeItem*>(parent.internalPointer());
	}

	if(parentItem) {
		beginRemoveRows(parent, position, position);
		parentItem->removeChild(position);
		endRemoveRows();
	}
}

int SearchTreeModel::columnCount(const QModelIndex& parent) const
{
	if ( parent.isValid() )
	{
		return static_cast<SearchTreeItem*>( parent.internalPointer() )->columnCount();
	}
	else
	{
		return rootItem->columnCount();
	}
}

QVariant SearchTreeModel::data(const QModelIndex& index, int role) const
{
	if ( !index.isValid() )
	{
		return QVariant();
	}

	SearchTreeItem* item = static_cast<SearchTreeItem*>( index.internalPointer() );

	if ( role == Qt::DecorationRole )
	{
		if ( index.column() == 0 ) // Index CAN be a negative value.
		{
			if ( item->parent() == rootItem )
				return m_pIconProvider->icon( item->data( 1 ).toString().prepend( "." ) );
			else
				return item->HitData.iNetwork;
		}

		if ( index.column() == 8 )
		{
			return item->HitData.iCountry;
		}
	}

	if ( role == Qt::DisplayRole )
	{
		return item->data( index.column() );
	}

	return QVariant();
}

Qt::ItemFlags SearchTreeModel::flags(const QModelIndex& index) const
{
	if ( !index.isValid() )
	{
		return 0;
	}

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SearchTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
	{
		return rootItem->data( section );
	}

	return QVariant();
}

QModelIndex SearchTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if ( !hasIndex( row, column, parent ) )
	{
		return QModelIndex();
	}

	SearchTreeItem* parentItem;

	if ( !parent.isValid() )
	{
		parentItem = rootItem;
	}
	else
	{
		parentItem = static_cast<SearchTreeItem*>( parent.internalPointer() );
	}

	SearchTreeItem* childItem = parentItem->child( row );
	if ( childItem )
	{
		return createIndex( row, column, childItem );
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex SearchTreeModel::parent(const QModelIndex& index) const
{
	if ( !index.isValid() )
	{
		return QModelIndex();
	}

	SearchTreeItem* childItem = static_cast<SearchTreeItem*>( index.internalPointer() );
	SearchTreeItem* parentItem = childItem->parent();

	if ( parentItem == rootItem )
	{
		return QModelIndex();
	}

	return createIndex( parentItem->row(), 0, parentItem );
}

int SearchTreeModel::rowCount(const QModelIndex& parent) const
{
	SearchTreeItem* parentItem;
	if ( parent.column() > 0 )
	{
		return 0;
	}

	if ( !parent.isValid() )
	{
		parentItem = rootItem;
	}
	else
	{
		parentItem = static_cast<SearchTreeItem*>( parent.internalPointer() );
	}

	return parentItem->childCount();
}

void SearchTreeModel::setupModelData(const QStringList& lines, SearchTreeItem* parent)
{
	QList<SearchTreeItem*> parents;
	QList<int> indentations;
	parents << parent;
	indentations << 0;

	int number = 0;

	while ( number < lines.count() )
	{
		int position = 0;
		while ( position < lines[number].length() )
		{
			if ( lines[number].mid( position, 1 ) != " " )
			{
				break;
			}
			++position;
		}

		QString lineData = lines[number].mid( position ).trimmed();

		if ( !lineData.isEmpty() )
		{
			// Read the column data from the rest of the line.
			QStringList columnStrings = lineData.split( "\t", QString::SkipEmptyParts );
			QList<QVariant> columnData;
			for ( int column = 0; column < columnStrings.count(); ++column )
			{
				columnData << columnStrings[column];
			}

			if ( position > indentations.last() )
			{
				// The last child of the current parent is now the new parent
				// unless the current parent has no children.

				if ( parents.last()->childCount() > 0 )
				{
					parents << parents.last()->child( parents.last()->childCount() - 1 );
					indentations << position;
				}
			}
			else
			{
				while ( position < indentations.last() && parents.count() > 0 )
				{
					parents.pop_back();
					indentations.pop_back();
				}
			}

			// Append a new item to the current parent's list of children.
			parents.last()->appendChild( new SearchTreeItem( columnData, parents.last() ) );
		}

		++number;
	}
}

void SearchTreeModel::clear()
{
	beginRemoveRows( QModelIndex(), 0, rootItem->childCount() );
	//qDebug() << "clearSearch passing to rootItem";
	rootItem->clearChildren();
	endRemoveRows();

	QModelIndex idx1 = index( 0, 0, QModelIndex() );
	QModelIndex idx2 = index( rootItem->childCount(), 10, QModelIndex() );
	emit dataChanged( idx1, idx2 );
}

void SearchTreeModel::addQueryHit(QueryHitSharedPtr pHitPtr)
{
	QueryHit* pHit = pHitPtr.data();

	while ( pHit )
	{
		int existingFileEntry = -1;

		// Check for duplicate file.
		foreach ( CHash oHash, pHit->m_lHashes )
		{
			existingFileEntry = rootItem->find( oHash );
			if ( existingFileEntry != -1 )
				break;
		}

		// This hit is a new non duplicate file.
		if ( existingFileEntry == -1 )
		{
			QFileInfo fileInfo( pHit->m_sDescriptiveName );

			QString sCountry = geoIP.findCountryCode(pHit->m_pHitInfo.data()->m_oNodeAddress.toIPv4Address());

			// Create SearchTreeItem representing the new file
			QList<QVariant> lParentData;
			lParentData << fileInfo.completeBaseName()        // File name
						<< fileInfo.suffix()                  // Extension
						<< formatBytes( pHit->m_nObjectSize ) // Size
						<< ""                                 // Rating
						<< ""                                 // Status
						<< 1                                  // Host/Count
						<< ""                                 // Speed
						<< ""                                 // Client
						<< "";                                // Country
			SearchTreeItem* m_oFileItem = new SearchTreeItem( lParentData, rootItem );

			m_oFileItem->HitData.lHashes << pHit->m_lHashes;

			// Create SearchTreeItem representing hit
			QList<QVariant> lChildData;
			lChildData << fileInfo.completeBaseName()
					   << fileInfo.suffix()
					   << formatBytes( pHit->m_nObjectSize )
					   << ""
					   << ""
					   << pHit->m_pHitInfo.data()->m_oNodeAddress.toString()
					   << ""
					   << common::vendorCodeToName( pHit->m_pHitInfo.data()->m_sVendor )
					   << geoIP.countryNameFromCode( sCountry );
			SearchTreeItem* m_oHitItem = new SearchTreeItem(lChildData, m_oFileItem);

			m_oHitItem->HitData.lHashes << pHit->m_lHashes;
			m_oHitItem->HitData.iNetwork = CNetworkIconProvider::icon( DiscoveryProtocol::G2 );
			m_oHitItem->HitData.iCountry = QIcon( ":/Resource/Flags/" + sCountry.toLower() + ".png" );

			QueryHitSharedPtr pHitX( new QueryHit( pHit ) );
			m_oHitItem->HitData.pQueryHit = pHitX;

			// add both items to the model
			beginInsertRows( QModelIndex(), rootItem->childCount(), rootItem->childCount() );
			rootItem->appendChild( m_oFileItem );
			m_oFileItem->appendChild( m_oHitItem );
			endInsertRows();

			nFileCount = rootItem->childCount();
		}
		// We do already have a file for that hit. Check for duplicate IP address. If not duplicate, add item.
		else if ( !rootItem->child( existingFileEntry
									)->duplicateCheck( rootItem->child( existingFileEntry ),
													   pHit->m_pHitInfo.data()->
													   m_oNodeAddress.toString() ) )
		{
			QModelIndex idxParent = index( existingFileEntry, 0, QModelIndex() );
			QFileInfo fileInfo( pHit->m_sDescriptiveName );

			QString sCountry = geoIP.findCountryCode(pHit->m_pHitInfo.data()->m_oNodeAddress.toIPv4Address());

			QList<QVariant> lChildData;
			lChildData << fileInfo.completeBaseName()
					   << fileInfo.suffix()
					   << formatBytes( pHit->m_nObjectSize )
					   << ""
					   << ""
					   << pHit->m_pHitInfo.data()->m_oNodeAddress.toString()
					   << ""
					   << common::vendorCodeToName( pHit->m_pHitInfo.data()->m_sVendor )
					   << geoIP.countryNameFromCode( sCountry );
			SearchTreeItem* oHitItem = new SearchTreeItem( lChildData,
															 rootItem->child( existingFileEntry ) );

			oHitItem->HitData.lHashes << pHit->m_lHashes;
			oHitItem->HitData.iNetwork = CNetworkIconProvider::icon( DiscoveryProtocol::G2 );
			oHitItem->HitData.iCountry = QIcon( ":/Resource/Flags/" + sCountry.toLower() + ".png" );

			QueryHitSharedPtr pHitX( new QueryHit( pHit ) );
			oHitItem->HitData.pQueryHit = pHitX;

			beginInsertRows( idxParent, rootItem->child( existingFileEntry )->childCount(),
							 rootItem->child( existingFileEntry )->childCount() );
			rootItem->child( existingFileEntry )->appendChild( oHitItem );
			rootItem->child( existingFileEntry )->updateHitCount( rootItem->child( existingFileEntry
																				  )->childCount() );
			endInsertRows();
		}

		pHit = pHit->m_pNext;
	}

	emit updateStats();

	QModelIndex idx1 = index( 0, 0, QModelIndex() );
	QModelIndex idx2 = index( rootItem->childCount(), 10, QModelIndex() );
	emit dataChanged( idx1, idx2 );
	emit sort();
}

SearchTreeItem::SearchTreeItem(const QList<QVariant> &data, SearchTreeItem* parent)
{
	parentItem = parent;
	itemData = data;
}

SearchTreeItem::~SearchTreeItem()
{
	qDeleteAll(childItems);
}

void SearchTreeItem::appendChild(SearchTreeItem* item)
{
	item->parentItem = this;
	childItems.append(item);
}

void SearchTreeItem::clearChildren()
{
	qDeleteAll(childItems);
	childItems.clear();
}

SearchTreeItem* SearchTreeItem::child(int row) const
{
	return childItems.value( row );
}

int SearchTreeItem::childCount() const
{
	return childItems.count();
}

int SearchTreeItem::columnCount() const
{
	return itemData.count();
}

int SearchTreeItem::find(CHash& pHash) const
{
	for ( int i = 0; i < childItems.size(); ++i )
	{
		if ( child( i )->HitData.lHashes.contains( pHash ) )
		{
			return i;
		}
	}
	return -1;
}

QVariant SearchTreeItem::data(int column) const
{
	return itemData.value(column);
}

SearchTreeItem* SearchTreeItem::parent()
{
	return parentItem;
}

void SearchTreeItem::removeChild(int position)
{
	if (position < 0 || position  > childItems.size())
		return;

	delete childItems.takeAt(position);
}

int SearchTreeItem::row() const
{
	if(parentItem)
	{
		return parentItem->childItems.indexOf(const_cast<SearchTreeItem*>(this));
	}

	return 0;
}

void SearchTreeItem::updateHitCount(int count)
{
	itemData[5] = count;
}

bool SearchTreeItem::duplicateCheck(SearchTreeItem* containerItem, QString ip)
{
	for(int index = 0; index < containerItem->childItems.size(); ++index)
	{
		if(containerItem->child(index)->data(5).toString() == ip)
		{
			return true;
		}
	}
	return false;
}

SearchTreeItem * SearchTreeModel::topLevelItemFromIndex(QModelIndex index)
{
	Q_ASSERT(index.model() == this);

	if(index.isValid())
	{
		QModelIndex idxThis = index;

		while( parent(idxThis).isValid() )
		{
			idxThis = parent(idxThis);
		}

		SearchTreeItem* pThis = static_cast<SearchTreeItem*>(idxThis.internalPointer());

		Q_ASSERT(pThis != NULL);

		return pThis;
	}

	return NULL;
}

SearchTreeItem * SearchTreeModel::itemFromIndex(QModelIndex index)
{
	Q_ASSERT(index.model() == this);

	if(index.isValid())
	{
		QModelIndex idxThis = index;

		SearchTreeItem* pThis = static_cast<SearchTreeItem*>(idxThis.internalPointer());

		Q_ASSERT(pThis != NULL);

		return pThis;
	}

	return NULL;
}

SearchFilter::SearchFilter() :
	m_sMatchString( "" ),
	m_bRegExp( false ),
	m_nMinSize( 0 ),
	m_nMaxSize( 18446744073709551615 ), // max value of 64 bit int
	m_nMinSources( 0 ),
	m_bBusy( true ),
	m_bFirewalled( true ),
	m_bUnstable( true ),
	m_bDRM( true ),
	m_bSuspicious( true ),
	m_bNonMatching( true ),
	m_bExistsInLibrary( true ),
	m_bBogus( true ),
	m_bAdult( true )
{
}

bool SearchFilter::operator==(const SearchFilter& rOther)
{
	return m_sMatchString     == rOther.m_sMatchString     &&
		   m_bRegExp          == rOther.m_bRegExp          &&
		   m_nMinSize         == rOther.m_nMinSize         &&
		   m_nMaxSize         == rOther.m_nMaxSize         &&
		   m_nMinSources      == rOther.m_nMinSources      &&
		   m_bBusy            == rOther.m_bBusy            &&
		   m_bFirewalled      == rOther.m_bFirewalled      &&
		   m_bUnstable        == rOther.m_bUnstable        &&
		   m_bDRM             == rOther.m_bDRM             &&
		   m_bSuspicious      == rOther.m_bSuspicious      &&
		   m_bNonMatching     == rOther.m_bNonMatching     &&
		   m_bExistsInLibrary == rOther.m_bExistsInLibrary &&
		   m_bBogus           == rOther.m_bBogus           &&
		   m_bAdult           == rOther.m_bAdult;
}

bool SearchFilter::operator!=(const SearchFilter& rOther)
{
	return !operator==( rOther );
}

// smaller amount of files
bool SearchFilter::operator<(const SearchFilter& rOther)
{
	return m_nMinSize         >  rOther.m_nMinSize         ||
		   m_nMaxSize         <  rOther.m_nMaxSize         ||
		   m_nMinSources      >  rOther.m_nMinSources      ||
		  !m_bBusy            && rOther.m_bBusy            ||
		  !m_bFirewalled      && rOther.m_bFirewalled      ||
		  !m_bUnstable        && rOther.m_bUnstable        ||
		  !m_bDRM             && rOther.m_bDRM             ||
		  !m_bSuspicious      && rOther.m_bSuspicious      ||
		  !m_bNonMatching     && rOther.m_bNonMatching     ||
		  !m_bExistsInLibrary && rOther.m_bExistsInLibrary ||
		  !m_bBogus           && rOther.m_bBogus           ||
		  !m_bAdult           && rOther.m_bAdult;
}

// bigger amount of files
bool SearchFilter::operator>(const SearchFilter& rOther)
{
	return m_nMinSize         <   rOther.m_nMinSize         ||
		   m_nMaxSize         >   rOther.m_nMaxSize         ||
		   m_nMinSources      <   rOther.m_nMinSources      ||
		   m_bBusy            && !rOther.m_bBusy            ||
		   m_bFirewalled      && !rOther.m_bFirewalled      ||
		   m_bUnstable        && !rOther.m_bUnstable        ||
		   m_bDRM             && !rOther.m_bDRM             ||
		   m_bSuspicious      && !rOther.m_bSuspicious      ||
		   m_bNonMatching     && !rOther.m_bNonMatching     ||
		   m_bExistsInLibrary && !rOther.m_bExistsInLibrary ||
		   m_bBogus           && !rOther.m_bBogus           ||
		   m_bAdult           && !rOther.m_bAdult;
}

