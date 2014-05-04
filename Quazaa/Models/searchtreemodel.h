/*
** searchtreemodel.h
**
** Copyright © Quazaa Development Team, 2009-2014.
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

#ifndef SEARCHTREEMODEL_H
#define SEARCHTREEMODEL_H

#include <QObject>
#include <QIcon>
#include <QAbstractItemModel>

#include <unordered_map>

#include "searchfilter.h"

class Hash;
class QFileInfo;
class FileIconProvider;

namespace SearchHitData
{
struct sSearchHitData
{
	QIcon iNetwork;
	QIcon iCountry;
	QueryHitSharedPtr pQueryHit;
};
} // namespace SearchHitData

namespace SearchFilter
{
class Filter;
class FilterControl;
struct FilterControlData;
}

class SearchTreeItem : public QObject
{
	Q_OBJECT
public:
	enum Type { SearchTreeItemType = 0, TreeRootType = 1, SearchFileType = 2, SearchHitType = 3 };

	enum Column
	{
		FILE           = 0,
		EXTENSION      = 1,
		SIZE           = 2,
		RATING         = 3,
		STATUS         = 4,
		HOSTCOUNT      = 5,
		SPEED          = 6,
		CLIENT         = 7,
		COUNTRY        = 8,
		_NO_OF_COLUMNS = 9
	};

private:
	quint32                 m_nVisibleChildren;
	int                     m_nRow;

protected:
	Type                    m_eType;        // item type

	QList<SearchTreeItem*>  m_lChildItems;
	SearchTreeItem* const   m_pParentItem;  // must be set by constructor
	QVariant*               m_pItemData;    // = new QVariant[_NO_OF_COLUMNS]

	SearchFilter::Filter*   m_pFilter;

public:
	SearchHitData::sSearchHitData m_oHitData;

public:
	SearchTreeItem( SearchTreeItem* pParent );
	virtual ~SearchTreeItem();

	SearchTreeItem* parent() const;

	Type type() const;
	bool visible() const;
	int row() const;

	virtual void appendChild( SearchTreeItem* pItem );
	virtual void removeChild( int position );

	SearchTreeItem* child( int row ) const;
	virtual int childCount() const;

	int columnCount() const;
	QVariant data( int column ) const;

	const SearchFilter::Filter* const getFilter() const;

private:
	void addVisibleChild();
	void removeVisibleChild();
	quint32 visibleChildCount();

	friend class SearchFilter::FilterControl; // for access to m_oFilter
};

class SearchFile;
class SearchTreeModel;
class TreeRoot : public SearchTreeItem
{
	Q_OBJECT
private:
	SearchTreeModel* m_pModel; // The model the root node is part of.
	std::unordered_map< Hash, SearchFile* > m_mHashes;

public:
	TreeRoot( SearchTreeModel* pModel );
	~TreeRoot();

	void appendChild( SearchTreeItem* pItem );
	void removeChild( int position );

	void clearSearch();

	QueryHit* addQueryHit( QueryHit* pHit );
	int find( const Hash& rHash ) const; // find child number with given hash

	void registerHash( const Hash& rHash, SearchFile* pFileItem );
	void unregisterHash( const Hash& rHash );

private:
	void addToFilterControl( SearchTreeItem* pItem );
	void removeFromFilterControl( SearchTreeItem* pItem );
	friend class SearchFile;
};

class SearchFile : public SearchTreeItem
{
	Q_OBJECT
public:
	HashSet      m_vHashes;

public:
	SearchFile( SearchTreeItem* parent, const QueryHit* const pHit, const QFileInfo& fileInfo );
	~SearchFile();

	void appendChild( SearchTreeItem* pItem );
	void removeChild( int position );

	bool manages( const Hash& rHash ) const;
	void updateHitCount(); // change number of hits

private:
	void insertHashes( const HashSet& vHashes );
	bool duplicateHitCheck( QueryHit* pNewHit ) const;
	void addQueryHit( QueryHit* pHit, const QFileInfo& fileInfo );

	friend class TreeRoot;
};

class SearchHit : public SearchTreeItem
{
	Q_OBJECT

public:
	// Note: SearchHit takes ownership of Query hit on creation
	SearchHit( SearchTreeItem* parent, QueryHit* pHit, const QFileInfo& fileInfo );
	~SearchHit();

	void appendChild( SearchTreeItem* pItem );
	void removeChild( int position );

	int childCount() const;
	void updateFilterData();
};

// TODO: update file name to select a non-filtered hit as file name source
class SearchTreeModel : public QAbstractItemModel
{
	Q_OBJECT

private:
	FileIconProvider*  m_pIconProvider;
	TreeRoot*          m_pRootItem;

	int m_nFileCount;

	SearchFilter::FilterControl* m_pFilterControl;

public:
	SearchTreeModel();
	~SearchTreeModel();

	QModelIndex parent( const QModelIndex& index ) const;
	QVariant data( const QModelIndex& index, int role ) const;
	Qt::ItemFlags flags( const QModelIndex& index ) const;
	QVariant headerData( int section, Qt::Orientation orientation,
						 int role = Qt::DisplayRole ) const;
	QModelIndex index( int row, int column,
					   const QModelIndex& parent = QModelIndex() ) const;
	int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	int columnCount( const QModelIndex& parent = QModelIndex() ) const;
	int fileCount() const;

	bool fileVisible( int row ) const;

	const SearchTreeItem* topLevelItemFromIndex( QModelIndex index ) const;
	SearchTreeItem* itemFromIndex( QModelIndex index );

	SearchFilter::FilterControlData* getFilterControlDataCopy() const;
	void updateFilter( const SearchFilter::FilterControlData& rControlData );

signals:
	void updateStats();
	void sort();
	void filterVisibilityUpdated();

private:
	//void setupModelData(const QStringList& lines, SearchTreeItem* parent);

public slots:
	void clear();
	//bool isRoot(QModelIndex index);
	void addQueryHit( QueryHit* pHit );
	void removeQueryHit( int position, const QModelIndex& parent );

	friend class TreeRoot;
};

#endif // SEARCHTREEMODEL_H
