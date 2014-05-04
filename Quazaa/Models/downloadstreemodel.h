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


#ifndef DOWNLOADSTREEMODEL_H
#define DOWNLOADSTREEMODEL_H

#include <QAbstractItemModel>
#include "types.h"
#include "FileFragments.hpp"

#include <QItemDelegate>
#include <QPalette>

class Download;
class DownloadSource;
class DownloadsTreeModel;
class DownloadSourceItem;
class FileIconProvider;

class DownloadsItemBase : public QObject
{
	Q_OBJECT
public:
	DownloadsItemBase( QObject* parent = 0 );
	virtual ~DownloadsItemBase();

	virtual void appendChild( DownloadsItemBase* child );
	virtual DownloadsItemBase* child( int row );
	virtual int childCount() const;
	virtual int columnCount() const;
	virtual QVariant data( int column ) const;
	virtual int row() const;
	virtual DownloadsItemBase* parent();

	bool m_bChanged;
	TransferProtocol
	m_nProtocol;	// the network used. this should always be tpNull for CDownloadsItemBase but child sources should always contain a value

protected:
	DownloadsItemBase* parentItem;
	QList<DownloadsItemBase*> childItems;
	// item data here

signals:
	void progressChanged();

	friend class DownloadItem;
	friend class DownloadSourceItem;
};

class DownloadItem : public DownloadsItemBase
{
	Q_OBJECT
public:
	DownloadItem( Download* download, DownloadsItemBase* parent, DownloadsTreeModel* model, QObject* parentQObject = 0 );
	virtual ~DownloadItem();

	void appendChild( DownloadsItemBase* child );
	QVariant data( int column ) const;
protected:
	Download* m_pDownload; // pointer to corresponding CDownload object

	QString m_sName;		// download name
	quint64 m_nSize;		// size
	int		m_nProgress;	// progress in % TODO: change it to something different when nice progress bar will be ready
	qint64	m_nBandwidth;	// total bandwidth, B/s
	int		m_nStatus;		// download status
	quint8	m_nPriority;	//
	quint64 m_nCompleted;	// bytes completed

	Fragments::List	m_oCompletedFrags;
	Fragments::List m_oVerifiedFrags;

	DownloadsTreeModel* m_pModel;
public slots:
	void onSourceAdded( DownloadSource* pSource );
	void onStateChanged( int state );
	void onBytesReceived( quint64 offset, quint64 length, DownloadSourceItem* source );

	friend class DownloadSourceItem;
	friend class DownloadsItemDelegate;
};

class DownloadSourceItem : public DownloadsItemBase
{
	Q_OBJECT
public:
	DownloadSourceItem( DownloadSource* downloadSource, DownloadsItemBase* parent, QObject* parentQObject = 0 );
	virtual ~DownloadSourceItem();

	void appendChild( DownloadsItemBase* child );
	QVariant data( int column ) const;
protected:
	DownloadSource* m_pDownloadSource; // pointer to corresponding CDownloadSource object

	QString m_sAddress;		// remote address
	quint64 m_nSize;		// available bytes on remote server/client
	int		m_nProgress;	// unused for now
	qint64	m_nBandwidth;	// bandwidth, B/s
	int		m_nStatus;		// dl source status
	QString m_sClient;		// remote client
	quint64 m_nDownloaded;  // downloaded bytes from this source
	QString m_sCountryName; // The full country name.
	QString m_sCountryCode; // The country code for this node. Used to get the correct icon for the country.

	Fragments::List	m_oDownloaded; // downloaded frags

public slots:
	QString getCountryCode();
	QString getCountry();
	void onBytesReceived( quint64 offset, quint64 length );

	friend class DownloadItem;
	friend class DownloadsItemDelegate;
};

class DownloadsTreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum Column
	{
		NAME = 0,	// file name / dl source ip
		SIZE,		// file size / available size
		PROGRESS,	// progress / progress from this user
		BANDWIDTH,	// total bandwidth / bandwidth provided by user
		STATUS,		// download status / source status
		PRIORITY,	// download priority / -
		CLIENT,		// - / remote client
		COMPLETED,	// MB completed / MB downloaded
		COUNTRY,	// - / country
		_NO_OF_COLUMNS
	};

public:
	DownloadsTreeModel( QObject* parent = 0 );
	~DownloadsTreeModel();

	QVariant data( const QModelIndex& index, int role ) const;
	Qt::ItemFlags flags( const QModelIndex& index ) const;
	QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
	QModelIndex parent( const QModelIndex& index ) const;
	int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	int columnCount( const QModelIndex& parent = QModelIndex() ) const;

private:
	DownloadsItemBase* rootItem;
	FileIconProvider* m_pIconProvider;
signals:

public slots:
	void onDownloadAdded( Download* pDownload );

	friend class DownloadItem;
};

#include <QPainter>

class DownloadsItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	inline DownloadsItemDelegate( QObject* parent )
		: QItemDelegate( parent )
	{}

	// what: 0 - normal fragment, 1 - verified chunks, 2 - failed chunks
	inline void paintFrags( const Fragments::List& frags, const QColor& color, const QStyleOptionViewItem& option,
							QPainter* painter, int what = 0 ) const
	{
		int		width = option.rect.width();
		double  fileSize = frags.limit();

		QBrush progressBrush( color, Qt::SolidPattern );
		QPen   framePen( QBrush(), 2, Qt::SolidLine );
		framePen.setColor( color.lighter() );

		painter->setPen( Qt::NoPen );

		Fragments::List::const_iterator pItr = frags.begin();
		const Fragments::List::const_iterator pEnd = frags.end();
		for ( ; pItr != pEnd; ++pItr )
		{
			double nLeft, nRight;
			QRectF r;

			painter->setBrush( progressBrush );

			switch ( what )
			{
			case 1:
			case 2:
				nLeft = option.rect.left() + ( width + 1 ) * pItr->begin() / fileSize;
				nRight = option.rect.left() + ( width + 1 ) * pItr->end() / fileSize;
				r.setRect( nLeft, option.rect.bottom() - 5, nRight - nLeft, 5 );
				break;
			default:
				nLeft = option.rect.left() + ( width + 1 ) * pItr->begin() / fileSize;
				nRight = option.rect.left() + ( width + 1 ) * pItr->end() / fileSize;
				r.setRect( nLeft, option.rect.top() + 2, nRight - nLeft, option.rect.height() - 2 );
			}

			painter->drawRect( r );

			QPointF topLeft = r.topLeft();
			QPointF topRight = r.topRight();
			QPointF bottomLeft = r.bottomLeft();

			topLeft += QPointF( 1, 0 );
			topRight += QPointF( -1, 0 );
			bottomLeft += QPointF( 1, -1 );
			painter->setPen( framePen );
			framePen.setWidth( 1 );
			painter->drawLine( topLeft, topRight );
			framePen.setWidth( 1 );
			painter->drawLine( topLeft, bottomLeft );
		}
	}

	inline void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
	{
		if ( index.column() != DownloadsTreeModel::PROGRESS )
		{
			QItemDelegate::paint( painter, option, index );
			return;
		}

		if ( !index.parent().isValid() )
		{
			// top-level
			DownloadItem* item = static_cast<DownloadItem*>( index.internalPointer() );

			Q_ASSERT( item );

			if ( item->m_nSize != -1 )
			{
				Fragments::List completedFrags( item->m_nSize );

				for ( Fragments::List::iterator it = item->m_oCompletedFrags.begin(); it != item->m_oCompletedFrags.end(); ++it )
				{
					completedFrags.insert( *it );
				}

				for ( int i = 0; i < item->childCount(); ++i )
				{
					DownloadSourceItem* sourceItem = static_cast<DownloadSourceItem*>( item->child( i ) );

					completedFrags.erase( sourceItem->m_oDownloaded.begin(), sourceItem->m_oDownloaded.end() );

					QColor thisColor = QColor::fromHsl( qHash( sourceItem->m_sAddress ) % 359, 255, 64 );

					paintFrags( sourceItem->m_oDownloaded, thisColor, option, painter );
				}

				QColor defaultColor = QColor::fromRgb( 0, 0, 255 );
				QColor verifiedColor = QColor( 48, 94, 137, 240 );
				//QColor failedColor = QColor(255, 0, 0, 240);

				paintFrags( completedFrags, defaultColor, option, painter );
				paintFrags( item->m_oCompletedFrags, verifiedColor, option, painter, 1 );
				//paintFrags(item->m_oVerifiedFrags, verifiedColor, option, painter, 1);
				//paintFrags(item->m_oCompletedFrags, failedColor, option, painter, 2);

			}

		}
		else
		{

		}

		QPen pen( Qt::transparent );
		pen.setWidth( 4 );
		painter->setPen( pen );
		painter->setBrush( Qt::NoBrush );
		painter->drawRect( option.rect );

		pen.setWidth( 1 );
		pen.setColor( qApp->palette().color( QPalette::Mid ) );
		painter->setPen( pen );
		QRect borderRect( option.rect.left(), option.rect.top() + 1, option.rect.width(), option.rect.height() - 1 );
		painter->drawLine( borderRect.topLeft(), borderRect.topRight() );
		painter->drawLine( borderRect.topLeft(), borderRect.bottomLeft() );
		painter->drawLine( borderRect.bottomLeft(), borderRect.bottomRight() );
		painter->drawLine( borderRect.topRight(), borderRect.bottomRight() );

		/*
		// Set up a QStyleOptionProgressBar to precisely mimic the
		// environment of a progress bar.
		QStyleOptionProgressBar progressBarOption;
		progressBarOption.state = QStyle::State_Enabled;
		progressBarOption.direction = QApplication::layoutDirection();
		progressBarOption.rect = option.rect;
		progressBarOption.fontMetrics = QApplication::fontMetrics();
		progressBarOption.minimum = 0;
		progressBarOption.maximum = 100;
		progressBarOption.textAlignment = Qt::AlignCenter;
		progressBarOption.textVisible = true;

		// Set the progress and text values of the style option.
		int progress = index.data().toInt();
		progressBarOption.progress = progress < 0 ? 0 : progress;
		progressBarOption.text = QString().sprintf("%d%%", progressBarOption.progress);

		// Draw the progress bar onto the view.
		QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);*/
	}
};



#endif // DOWNLOADSTREEMODEL_H
