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

CDownloadsTreeModel::CDownloadsTreeModel(QObject *parent) :
	QAbstractItemModel(parent),
	m_pIconProvider(new FileIconProvider)
{
	rootItem = new CDownloadsItemBase(this);
	connect(&downloads, SIGNAL(downloadAdded(Download*)), this, SLOT(onDownloadAdded(Download*)), Qt::QueuedConnection);
	QMetaObject::invokeMethod(&downloads, "emitDownloads");
}

CDownloadsTreeModel::~CDownloadsTreeModel()
{
	delete rootItem;
	delete m_pIconProvider;
}

QVariant CDownloadsTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	switch(role)
	{
		case Qt::DisplayRole:
		{
			CDownloadsItemBase *item = static_cast<CDownloadsItemBase*>(index.internalPointer());
			return item->data(index.column());
		}
		case Qt::TextAlignmentRole:
		{
			switch(index.column())
			{
				case CDownloadsTreeModel::BANDWIDTH:
				case CDownloadsTreeModel::STATUS:
				case CDownloadsTreeModel::COMPLETED:
					return Qt::AlignCenter;
			}
		}
	case Qt::DecorationRole:
		{
			if(index.column() == CDownloadsTreeModel::NAME)
			{
				if( !index.parent().isValid() )
				{
					CDownloadItem *item = static_cast<CDownloadItem*>(index.internalPointer());
					return m_pIconProvider->icon(item->data(CDownloadsTreeModel::NAME).toString());
				}
				else
				{
					CDownloadSourceItem *item = static_cast<CDownloadSourceItem*>(index.internalPointer());
					return CNetworkIconProvider::icon(item->m_nProtocol);
				}
			}
			if(index.column() == CDownloadsTreeModel::COUNTRY)
			{
				CDownloadSourceItem *item = static_cast<CDownloadSourceItem*>(index.internalPointer());
				if(item->m_nProtocol != tpNull)
				{
					return QIcon(":/Resource/Flags/" + item->getCountryCode().toLower() + ".png");
				}
			}
		}
	}

	return QVariant();
}

Qt::ItemFlags CDownloadsTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant CDownloadsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
		case CDownloadsTreeModel::NAME:
				return tr("Downloaded file");
			break;
		case CDownloadsTreeModel::SIZE:
				return tr("Size");
			break;
		case CDownloadsTreeModel::PROGRESS:
				return tr("Progress");
			break;
		case CDownloadsTreeModel::BANDWIDTH:
				return tr("Bandwidth");
			break;
		case CDownloadsTreeModel::STATUS:
				return tr("Status");
			break;
		case CDownloadsTreeModel::PRIORITY:
				return tr("Priority");
			break;
		case CDownloadsTreeModel::CLIENT:
				return tr("Client");
			break;
		case CDownloadsTreeModel::COMPLETED:
				return tr("Completed");
			break;
		case CDownloadsTreeModel::COUNTRY:
				return tr("Country");
			break;
		default:
				return QVariant();
		}
	}

	return QVariant();
}

QModelIndex CDownloadsTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	CDownloadsItemBase *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<CDownloadsItemBase*>(parent.internalPointer());

	CDownloadsItemBase *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex CDownloadsTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	CDownloadsItemBase *childItem = static_cast<CDownloadsItemBase*>(index.internalPointer());
	CDownloadsItemBase *parentItem = childItem->parent();

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int CDownloadsTreeModel::rowCount(const QModelIndex &parent) const
{
	CDownloadsItemBase *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<CDownloadsItemBase*>(parent.internalPointer());

	return parentItem->childCount();
}

int CDownloadsTreeModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _NO_OF_COLUMNS;
}

void CDownloadsTreeModel::onDownloadAdded(Download *pDownload)
{
	QMutexLocker l(&downloads.m_pSection);

	if(!downloads.exists(pDownload))
		return;

	beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
	CDownloadItem* pItem = new CDownloadItem(pDownload, rootItem, this);
	rootItem->appendChild(pItem);
	endInsertRows();
}



CDownloadsItemBase::CDownloadsItemBase(QObject *parent)
	: QObject(parent),
	  m_bChanged(false),
	  parentItem(0)
{
	m_nProtocol = tpNull;
}

CDownloadsItemBase::~CDownloadsItemBase()
{
	qDeleteAll(childItems);
}

void CDownloadsItemBase::appendChild(CDownloadsItemBase *child)
{
	childItems.append(child);
}

CDownloadsItemBase *CDownloadsItemBase::child(int row)
{
	return childItems.value(row);
}

int CDownloadsItemBase::childCount() const
{
	return childItems.count();
}

int CDownloadsItemBase::columnCount() const
{
	return CDownloadsTreeModel::_NO_OF_COLUMNS;
}

QVariant CDownloadsItemBase::data(int column) const
{
	Q_UNUSED(column);
	return QVariant();
}

int CDownloadsItemBase::row() const
{
	if( parentItem )
		return parentItem->childItems.indexOf(const_cast<CDownloadsItemBase*>(this));
	return 0;
}

CDownloadsItemBase *CDownloadsItemBase::parent()
{
	return parentItem;
}

CDownloadItem::CDownloadItem(Download *download, CDownloadsItemBase *parent, CDownloadsTreeModel* model, QObject *parentQObject)
	: CDownloadsItemBase(parentQObject),
	  m_pDownload(download),
	  m_oCompletedFrags(0),
	  m_oVerifiedFrags(0),
	  m_pModel(model)
{
	parentItem = parent;

	connect(download, SIGNAL(sourceAdded(CDownloadSource*)), this, SLOT(onSourceAdded(CDownloadSource*)), Qt::QueuedConnection);
	connect(download, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)), Qt::QueuedConnection);

	m_sName = download->m_sDisplayName;
	m_nSize = download->m_nSize;
	m_nProgress = 0;
	m_nBandwidth = 0;
	m_nStatus = download->m_nState;
	m_nPriority = download->m_nPriority; // 255: highest priority; 1: lowest priority; 0: temporary disabled
	m_nCompleted = download->m_nCompletedSize;
	m_nProtocol = tpNull;

	if( m_nSize )
	{
		Fragments::List oCompleted(m_nSize);
		Fragments::List oVerified(m_nSize);

		for( Fragments::List::iterator it = download->m_lCompleted.begin(); it != download->m_lCompleted.end(); ++it )
		{
			oCompleted.insert(*it);
		}
		m_oCompletedFrags.swap(oCompleted);

		for( Fragments::List::iterator it = download->m_lVerified.begin(); it != download->m_lVerified.end(); ++it )
		{
			oVerified.insert(*it);
		}
		m_oVerifiedFrags.swap(oVerified);

	}

	QMetaObject::invokeMethod(download, "emitSources", Qt::QueuedConnection);
}

CDownloadItem::~CDownloadItem()
{
}

void CDownloadItem::appendChild(CDownloadsItemBase *child)
{
	Q_ASSERT_X(qobject_cast<CDownloadSourceItem*>(child) != 0, "CDownloadItem::appendChild()", "child can only be CDownloadSourceItem!");

	CDownloadsItemBase::appendChild(child);
}

QVariant CDownloadItem::data(int column) const
{
	switch(column)
	{
		case CDownloadsTreeModel::NAME:
			return m_sName;
		case CDownloadsTreeModel::SIZE:
			return formatBytes(m_nSize);
		case CDownloadsTreeModel::PROGRESS:
			return m_nProgress;
		case CDownloadsTreeModel::BANDWIDTH:
			if(m_nBandwidth == 0)
				return QVariant();
			else
				return formatBytes(m_nBandwidth).append("/s");
		case CDownloadsTreeModel::STATUS:
			switch( m_nStatus )
			{
				case Download::dsQueued:
					return tr("Queued");
				case Download::dsPaused:
					return tr("Paused");
				case Download::dsSearching:
					return tr("Searching");
				case Download::dsPending:
					return tr("Pending");
				case Download::dsDownloading:
					return tr("Downloading");
				case Download::dsVerifying:
					return tr("Verifying");
				case Download::dsMoving:
					return tr("Moving");
				case Download::dsFileError:
					return tr("File Error");
				case Download::dsCompleted:
					return tr("Completed");
			}
			return tr("Unknown"); // should not happen, but...
		case CDownloadsTreeModel::PRIORITY:
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
		case CDownloadsTreeModel::CLIENT:
			if( childCount() )
			{
				return QString(tr("(%n source(s))", "", childCount()));
			}
			return tr("(No Sources)");
		case CDownloadsTreeModel::COMPLETED:
			return formatBytes(m_nCompleted);
		case CDownloadsTreeModel::COUNTRY:
			return QVariant();
		default:
			return QVariant();

	}
}

void CDownloadItem::onSourceAdded(CDownloadSource *pSource)
{
	QMutexLocker l(&downloads.m_pSection);

	if(m_pDownload->sourceExists(pSource))
	{
		m_pModel->beginInsertRows(m_pModel->index(row(), 0), childCount(), childCount());
		CDownloadSourceItem* pItem = new CDownloadSourceItem(pSource, this);
		appendChild(pItem);
		m_pModel->endInsertRows();
	}
}

void CDownloadItem::onStateChanged(int state)
{
	m_nStatus = state;
	m_bChanged = true;
}

void CDownloadItem::onBytesReceived(quint64 offset, quint64 length, CDownloadSourceItem *source)
{
	Q_UNUSED(source);
	//TODO: Make use of CDownloadSourceItem *source

	qDebug() << "CDownloadItem::onBytesReceived()" << offset << length;
	m_oCompletedFrags.insert(Fragments::Fragment(offset, offset + length));
	m_bChanged = true;

	emit progressChanged();
}

CDownloadSourceItem::CDownloadSourceItem(CDownloadSource *downloadSource,
										 CDownloadsItemBase *parent,
										 QObject *parentQObject) :
	CDownloadsItemBase( parentQObject ),
	m_pDownloadSource( downloadSource ),
	m_oDownloaded( static_cast<CDownloadItem*>(parent)->m_nSize )
{
	parentItem = parent;

	// TODO: import values from downloadSource
	m_sAddress = downloadSource->m_oAddress.toStringWithPort();
	m_nSize = 0;
	m_nProgress = 0;
	m_nBandwidth = 0;
	m_nStatus = 0;
	m_sClient = "";
	m_nDownloaded = 0;

	m_sCountryCode = downloadSource->m_oAddress.country();
	m_sCountryName = geoIP.countryNameFromCode( m_sCountryCode );

	m_nProtocol = downloadSource->m_nProtocol;

	connect( downloadSource, SIGNAL( bytesReceived(quint64,quint64) ),
			 this, SLOT( onBytesReceived(quint64,quint64) ) );

	//static int test = (srand(QDateTime::currentMSecsSinceEpoch()), 1);
	onBytesReceived( common::getRandomNum<quint64>( 0, ((CDownloadItem*)parent)->m_nSize ),
					 common::getRandomNum<quint64>( 100, 1024 * 1024 ) );
}

CDownloadSourceItem::~CDownloadSourceItem()
{
}

void CDownloadSourceItem::appendChild(CDownloadsItemBase *child)
{
	Q_ASSERT_X(false, "CDownloadSourceItem::appendChild()", "CDownloadSourceItem cannot have children!");
	delete child; // good?
}

QVariant CDownloadSourceItem::data(int column) const
{
	switch( column )
	{
	case CDownloadsTreeModel::NAME:
			return m_sAddress;

	case CDownloadsTreeModel::SIZE:
			return formatBytes( m_nSize );

	case CDownloadsTreeModel::PROGRESS:
			return QVariant();

	case CDownloadsTreeModel::BANDWIDTH:
			if ( m_nBandwidth )
				return formatBytes( m_nBandwidth ).append( "/s" );
			else
				return QVariant();

	case CDownloadsTreeModel::STATUS:
			return m_nStatus;

	case CDownloadsTreeModel::PRIORITY:
			return QVariant();

	case CDownloadsTreeModel::CLIENT:
			return m_sClient;

	case CDownloadsTreeModel::COMPLETED:
			return formatBytes( m_nDownloaded );

	case CDownloadsTreeModel::COUNTRY:
			return m_sCountryName;

	default:
			return QVariant();
	}
}

QString CDownloadSourceItem::getCountryCode()
{
	return m_sCountryCode;
}

QString CDownloadSourceItem::getCountry()
{
	return m_sCountryName;
}

void CDownloadSourceItem::onBytesReceived(quint64 offset, quint64 length)
{
	CDownloadItem* pParent = static_cast<CDownloadItem*>(parentItem);

	if( pParent && m_nSize != -1 )
	{
		m_bChanged = true;
		m_oDownloaded.insert(Fragments::Fragment(offset, offset + length));
		pParent->onBytesReceived(offset, length, this);
	}
}
