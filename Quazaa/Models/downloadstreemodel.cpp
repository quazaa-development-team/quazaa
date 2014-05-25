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


#include "downloadstreemodel.h"
#include "downloads.h"
#include "download.h"
#include "downloadsource.h"

#include "commonfunctions.h"
#include "geoiplist.h"
#include "fileiconprovider.h"
#include "networkiconprovider.h"

using namespace common;

#include "debug_new.h"

DownloadsTreeModel::DownloadsTreeModel( QObject* parent ) :
	QAbstractItemModel( parent ),
	m_pIconProvider( new FileIconProvider )
{
	rootItem = new DownloadsItemBase( this );
	connect( &downloads, SIGNAL( downloadAdded( Download* ) ), this, SLOT( onDownloadAdded( Download* ) ),
			 Qt::QueuedConnection );
	QMetaObject::invokeMethod( &downloads, "emitDownloads" );
}

DownloadsTreeModel::~DownloadsTreeModel()
{
	delete rootItem;
	delete m_pIconProvider;
}

QVariant DownloadsTreeModel::data( const QModelIndex& index, int role ) const
{
	if ( !index.isValid() )
	{
		return QVariant();
	}

	switch ( role )
	{
	case Qt::DisplayRole:
	{
		DownloadsItemBase* item = static_cast<DownloadsItemBase*>( index.internalPointer() );
		return item->data( index.column() );
	}
	case Qt::TextAlignmentRole:
	{
		switch ( index.column() )
		{
		case DownloadsTreeModel::BANDWIDTH:
		case DownloadsTreeModel::STATUS:
		case DownloadsTreeModel::COMPLETED:
			return Qt::AlignCenter;
		}
	}
	case Qt::DecorationRole:
	{
		if ( index.column() == DownloadsTreeModel::NAME )
		{
			if ( !index.parent().isValid() )
			{
				DownloadItem* item = static_cast<DownloadItem*>( index.internalPointer() );
				return m_pIconProvider->icon( item->data( DownloadsTreeModel::NAME ).toString() );
			}
			else
			{
				DownloadSourceItem* item = static_cast<DownloadSourceItem*>( index.internalPointer() );
				return NetworkIconProvider::icon( item->m_nProtocol );
			}
		}
		if ( index.column() == DownloadsTreeModel::COUNTRY )
		{
			DownloadSourceItem* item = static_cast<DownloadSourceItem*>( index.internalPointer() );
			if ( item->m_nProtocol != tpNull )
			{
				return NetworkIconProvider::icon( item->getCountryCode() );
			}
		}
	}
	}

	return QVariant();
}

Qt::ItemFlags DownloadsTreeModel::flags( const QModelIndex& index ) const
{
	if ( !index.isValid() )
	{
		return 0;
	}

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant DownloadsTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
	{
		switch ( section )
		{
		case DownloadsTreeModel::NAME:
			return tr( "Downloaded file" );
			break;
		case DownloadsTreeModel::SIZE:
			return tr( "Size" );
			break;
		case DownloadsTreeModel::PROGRESS:
			return tr( "Progress" );
			break;
		case DownloadsTreeModel::BANDWIDTH:
			return tr( "Bandwidth" );
			break;
		case DownloadsTreeModel::STATUS:
			return tr( "Status" );
			break;
		case DownloadsTreeModel::PRIORITY:
			return tr( "Priority" );
			break;
		case DownloadsTreeModel::CLIENT:
			return tr( "Client" );
			break;
		case DownloadsTreeModel::COMPLETED:
			return tr( "Completed" );
			break;
		case DownloadsTreeModel::COUNTRY:
			return tr( "Country" );
			break;
		default:
			return QVariant();
		}
	}

	return QVariant();
}

QModelIndex DownloadsTreeModel::index( int row, int column, const QModelIndex& parent ) const
{
	if ( !hasIndex( row, column, parent ) )
	{
		return QModelIndex();
	}

	DownloadsItemBase* parentItem;

	if ( !parent.isValid() )
	{
		parentItem = rootItem;
	}
	else
	{
		parentItem = static_cast<DownloadsItemBase*>( parent.internalPointer() );
	}

	DownloadsItemBase* childItem = parentItem->child( row );
	if ( childItem )
	{
		return createIndex( row, column, childItem );
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex DownloadsTreeModel::parent( const QModelIndex& index ) const
{
	if ( !index.isValid() )
	{
		return QModelIndex();
	}

	DownloadsItemBase* childItem = static_cast<DownloadsItemBase*>( index.internalPointer() );
	DownloadsItemBase* parentItem = childItem->parent();

	if ( parentItem == rootItem )
	{
		return QModelIndex();
	}

	return createIndex( parentItem->row(), 0, parentItem );
}

int DownloadsTreeModel::rowCount( const QModelIndex& parent ) const
{
	DownloadsItemBase* parentItem;
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
		parentItem = static_cast<DownloadsItemBase*>( parent.internalPointer() );
	}

	return parentItem->childCount();
}

int DownloadsTreeModel::columnCount( const QModelIndex& parent ) const
{
	Q_UNUSED( parent );
	return _NO_OF_COLUMNS;
}

void DownloadsTreeModel::onDownloadAdded( Download* pDownload )
{
	QMutexLocker l( &downloads.m_pSection );

	if ( !downloads.exists( pDownload ) )
	{
		return;
	}

	beginInsertRows( QModelIndex(), rootItem->childCount(), rootItem->childCount() );
	DownloadItem* pItem = new DownloadItem( pDownload, rootItem, this );
	rootItem->appendChild( pItem );
	endInsertRows();
}



DownloadsItemBase::DownloadsItemBase( QObject* parent )
	: QObject( parent ),
	  m_bChanged( false ),
	  parentItem( 0 )
{
	m_nProtocol = tpNull;
}

DownloadsItemBase::~DownloadsItemBase()
{
	qDeleteAll( childItems );
}

void DownloadsItemBase::appendChild( DownloadsItemBase* child )
{
	childItems.append( child );
}

DownloadsItemBase* DownloadsItemBase::child( int row )
{
	return childItems.value( row );
}

int DownloadsItemBase::childCount() const
{
	return childItems.count();
}

int DownloadsItemBase::columnCount() const
{
	return DownloadsTreeModel::_NO_OF_COLUMNS;
}

QVariant DownloadsItemBase::data( int column ) const
{
	Q_UNUSED( column );
	return QVariant();
}

int DownloadsItemBase::row() const
{
	if ( parentItem )
	{
		return parentItem->childItems.indexOf( const_cast<DownloadsItemBase*>( this ) );
	}
	return 0;
}

DownloadsItemBase* DownloadsItemBase::parent()
{
	return parentItem;
}

DownloadItem::DownloadItem( Download* download, DownloadsItemBase* parent, DownloadsTreeModel* model,
							  QObject* parentQObject )
	: DownloadsItemBase( parentQObject ),
	  m_pDownload( download ),
	  m_oCompletedFrags( 0 ),
	  m_oVerifiedFrags( 0 ),
	  m_pModel( model )
{
	parentItem = parent;

	connect( download, SIGNAL( sourceAdded( DownloadSource* ) ), this, SLOT( onSourceAdded( DownloadSource* ) ),
			 Qt::QueuedConnection );
	connect( download, SIGNAL( stateChanged( int ) ), this, SLOT( onStateChanged( int ) ), Qt::QueuedConnection );

	m_sName = download->m_sDisplayName;
	m_nSize = download->m_nSize;
	m_nProgress = 0;
	m_nBandwidth = 0;
	m_nStatus = download->m_nState;
	m_nPriority = download->m_nPriority; // 255: highest priority; 1: lowest priority; 0: temporary disabled
	m_nCompleted = download->m_nCompletedSize;
	m_nProtocol = tpNull;

	if ( m_nSize )
	{
		Fragments::List oCompleted( m_nSize );
		Fragments::List oVerified( m_nSize );

		for ( Fragments::List::iterator it = download->m_lCompleted.begin(); it != download->m_lCompleted.end(); ++it )
		{
			oCompleted.insert( *it );
		}
		m_oCompletedFrags.swap( oCompleted );

		for ( Fragments::List::iterator it = download->m_lVerified.begin(); it != download->m_lVerified.end(); ++it )
		{
			oVerified.insert( *it );
		}
		m_oVerifiedFrags.swap( oVerified );

	}

	QMetaObject::invokeMethod( download, "emitSources", Qt::QueuedConnection );
}

DownloadItem::~DownloadItem()
{
}

void DownloadItem::appendChild( DownloadsItemBase* child )
{
	Q_ASSERT_X( qobject_cast<DownloadSourceItem*>( child ) != 0, "CDownloadItem::appendChild()",
				"child can only be CDownloadSourceItem!" );

	DownloadsItemBase::appendChild( child );
}

QVariant DownloadItem::data( int column ) const
{
	switch ( column )
	{
	case DownloadsTreeModel::NAME:
		return m_sName;
	case DownloadsTreeModel::SIZE:
		return formatBytes( m_nSize );
	case DownloadsTreeModel::PROGRESS:
		return m_nProgress;
	case DownloadsTreeModel::BANDWIDTH:
		if ( m_nBandwidth == 0 )
		{
			return QVariant();
		}
		else
		{
			return formatBytes( m_nBandwidth ).append( "/s" );
		}
	case DownloadsTreeModel::STATUS:
		switch ( m_nStatus )
		{
		case Download::dsQueued:
			return tr( "Queued" );
		case Download::dsPaused:
			return tr( "Paused" );
		case Download::dsSearching:
			return tr( "Searching" );
		case Download::dsPending:
			return tr( "Pending" );
		case Download::dsDownloading:
			return tr( "Downloading" );
		case Download::dsVerifying:
			return tr( "Verifying" );
		case Download::dsMoving:
			return tr( "Moving" );
		case Download::dsFileError:
			return tr( "File Error" );
		case Download::dsCompleted:
			return tr( "Completed" );
		}
		return tr( "Unknown" ); // should not happen, but...
	case DownloadsTreeModel::PRIORITY:
		/*switch(m_nPriority)
		{
			case CDownload::LOW:
				return tr("Low");
			case CDownload::NORMAL:
				return tr("Normal");
			case CDownload::HIGH:
				return tr("High");
			default:
				if(m_nPriority < CDownload::HIGH)
					return tr("Highest");
				else if(m_nPriority > CDownload::LOW)
					return tr("Lowest");
				else
					return tr("Unknown"); // not possible, but...
		}*/
		return QString::number( m_nPriority );
	case DownloadsTreeModel::CLIENT:
		if ( childCount() )
		{
			return QString( tr( "(%n source(s))", "", childCount() ) );
		}
		return tr( "(No Sources)" );
	case DownloadsTreeModel::COMPLETED:
		return formatBytes( m_nCompleted );
	case DownloadsTreeModel::COUNTRY:
		return QVariant();
	default:
		return QVariant();

	}
}

void DownloadItem::onSourceAdded( DownloadSource* pSource )
{
	QMutexLocker l( &downloads.m_pSection );

	if ( m_pDownload->sourceExists( pSource ) )
	{
		m_pModel->beginInsertRows( m_pModel->index( row(), 0 ), childCount(), childCount() );
		DownloadSourceItem* pItem = new DownloadSourceItem( pSource, this );
		appendChild( pItem );
		m_pModel->endInsertRows();
	}
}

void DownloadItem::onStateChanged( int state )
{
	m_nStatus = state;
	m_bChanged = true;
}

void DownloadItem::onBytesReceived( quint64 offset, quint64 length, DownloadSourceItem* source )
{
	Q_UNUSED( source );
	//TODO: Make use of CDownloadSourceItem *source

	qDebug() << "CDownloadItem::onBytesReceived()" << offset << length;
	m_oCompletedFrags.insert( Fragments::Fragment( offset, offset + length ) );
	m_bChanged = true;

	emit progressChanged();
}

DownloadSourceItem::DownloadSourceItem( DownloadSource* downloadSource,
										  DownloadsItemBase* parent,
										  QObject* parentQObject ) :
	DownloadsItemBase( parentQObject ),
	m_pDownloadSource( downloadSource ),
	m_oDownloaded( static_cast<DownloadItem*>( parent )->m_nSize )
{
	parentItem = parent;

	// TODO: import values from downloadSource
	m_sAddress    = downloadSource->m_oAddress.toStringWithPort();
	m_nSize       = downloadSource->m_pDownload->m_nSize;
	m_nProgress   = 0;
	m_nBandwidth  = 0;
	m_nStatus     = 0;
	m_sClient     = "";
	m_nDownloaded = 0;

	m_sCountryCode = downloadSource->m_oAddress.country();
	m_sCountryName = geoIP.countryNameFromCode( m_sCountryCode );

	m_nProtocol = downloadSource->m_nProtocol;

	connect( downloadSource, SIGNAL( bytesReceived( quint64, quint64 ) ),
			 this, SLOT( onBytesReceived( quint64, quint64 ) ) );

	//static int test = (srand(QDateTime::currentMSecsSinceEpoch()), 1);
	onBytesReceived( common::getRandomNum<quint64>( 0, ( ( DownloadItem* )parent )->m_nSize ),
					 common::getRandomNum<quint64>( 100, 1024 * 1024 ) );
}

DownloadSourceItem::~DownloadSourceItem()
{
}

void DownloadSourceItem::appendChild( DownloadsItemBase* child )
{
	Q_ASSERT_X( false, "CDownloadSourceItem::appendChild()", "CDownloadSourceItem cannot have children!" );
	delete child; // good?
}

QVariant DownloadSourceItem::data( int column ) const
{
	switch ( column )
	{
	case DownloadsTreeModel::NAME:
		return m_sAddress;

	case DownloadsTreeModel::SIZE:
		return formatBytes( m_nSize );

	case DownloadsTreeModel::PROGRESS:
		return QVariant();

	case DownloadsTreeModel::BANDWIDTH:
		if ( m_nBandwidth )
		{
			return formatBytes( m_nBandwidth ).append( "/s" );
		}
		else
		{
			return QVariant();
		}

	case DownloadsTreeModel::STATUS:
		return m_nStatus;

	case DownloadsTreeModel::PRIORITY:
		return QVariant();

	case DownloadsTreeModel::CLIENT:
		return m_sClient;

	case DownloadsTreeModel::COMPLETED:
		return formatBytes( m_nDownloaded );

	case DownloadsTreeModel::COUNTRY:
		return m_sCountryName;

	default:
		return QVariant();
	}
}

QString DownloadSourceItem::getCountryCode()
{
	return m_sCountryCode;
}

QString DownloadSourceItem::getCountry()
{
	return m_sCountryName;
}

void DownloadSourceItem::onBytesReceived( quint64 offset, quint64 length )
{
	DownloadItem* pParent = static_cast<DownloadItem*>( parentItem );

	if ( pParent && m_nSize != -1 )
	{
		m_bChanged = true;
		m_oDownloaded.insert( Fragments::Fragment( offset, offset + length ) );
		pParent->onBytesReceived( offset, length, this );
	}
}
