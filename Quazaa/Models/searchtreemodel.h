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

#include "searchfilter.h"

class CHash;
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

class SearchTreeItem : public QObject
{
	Q_OBJECT
public:
	enum Type { SearchTreeItemType, TreeRootType, SearchFileType, SearchHitType };

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

protected:
	Type                    m_eType;        // item type

	QList<SearchTreeItem*>  m_lChildItems;
	SearchTreeItem* const   m_pParentItem;  // must be set by constructor
	QVariant*               m_pItemData;    // = new QVariant[_NO_OF_COLUMNS]

public:
	SearchHitData::sSearchHitData m_oHitData;

public:
	SearchTreeItem(SearchTreeItem* parent);
	virtual ~SearchTreeItem();

	SearchTreeItem* parent() const;

	Type type() const;
	bool visible() const;
	int row() const;

	void appendChild(SearchTreeItem* pItem);
	void removeChild(int position);
	void clearChildren();

	SearchTreeItem* child(int row) const;
	virtual int childCount() const;

	int columnCount() const;
	QVariant data(int column) const;

};

class SearchTreeModel;
class TreeRoot : public SearchTreeItem
{
	Q_OBJECT
private:
	SearchTreeModel* m_pModel; // The model the root node is part of.

public:
	TreeRoot(SearchTreeModel* pModel);
	~TreeRoot();

	void addQueryHit(QueryHit* pHit);
	int find(CHash& pHash) const; // find child number with given hash
};

class SearchFile : public SearchTreeItem
{
	Q_OBJECT
public:
	HashVector      m_lHashes;

public:
	SearchFile(SearchTreeItem* parent, QueryHit* pHit, const QFileInfo& fileInfo);
	~SearchFile();

	bool manages(CHash hash) const;
	void updateHitCount(); // change number of hits

private:
	void insertHashes(const HashVector& hashes);
	bool duplicateHitCheck(QueryHit* pNewHit) const;
	void addQueryHit(QueryHit* pHit, const QFileInfo& fileInfo);

	friend class TreeRoot;
};

class SearchHit : public SearchTreeItem
{
	Q_OBJECT

public:
	SearchHit(SearchTreeItem* parent, QueryHit* pHit, const QFileInfo& fileInfo);
	~SearchHit();

	int childCount() const;
	void updateFilterData();
};

// TODO: replace with forward_list (C++11)
typedef std::list<SearchTreeItem*> SearchList;

class SearchTreeModel : public QAbstractItemModel
{
	Q_OBJECT

private:
	FileIconProvider*  m_pIconProvider;
	TreeRoot*          m_pRootItem;

	int m_nFileCount;

	// lists used to keep track of items for filtering purposes
	SearchList m_lVisibleHits;          // contains currently visible hits
	SearchList m_lFilteredHits;         // contains currently hidden hits
	SearchList m_lNewlyVisibleHits;     // contains hits moved from hidden list on filter change
	SearchList m_lNewlyFilteredHits;    // contains hits moved from visible list on filter change

public:
	SearchTreeModel();
	~SearchTreeModel();

	QModelIndex parent(const QModelIndex& index) const;
	QVariant data(const QModelIndex& index, int role) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex& parent = QModelIndex()) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	int fileCount() const;

	SearchTreeItem* topLevelItemFromIndex(QModelIndex index);
	SearchTreeItem* itemFromIndex(QModelIndex index);

signals:
	void updateStats();
	void sort();
	void filter();

private:
	//void setupModelData(const QStringList& lines, SearchTreeItem* parent);

private slots:
	void addQueryHit(QueryHitSharedPtr pHit);

public slots:
	void clear();
	//bool isRoot(QModelIndex index);
	void removeQueryHit(int position, const QModelIndex &parent);

	friend class TreeRoot;
};

#endif // SEARCHTREEMODEL_H
