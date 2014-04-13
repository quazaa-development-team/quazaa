/*
** searchtreemodel.h
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

#ifndef SEARCHTREEMODEL_H
#define SEARCHTREEMODEL_H

#include <QObject>
#include <QIcon>
#include <QAbstractItemModel>
#include "NetworkCore/queryhit.h"

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
}

class SearchFilter
{
public:
	QString m_sMatchString;
	bool m_bRegExp;

	quint64 m_nMinSize;
	quint64 m_nMaxSize;
	quint16 m_nMinSources;

	// bools: state allowed
	bool m_bBusy;
	bool m_bFirewalled;
	bool m_bUnstable;
	bool m_bDRM;
	bool m_bSuspicious;
	bool m_bNonMatching;
	bool m_bExistsInLibrary;
	bool m_bBogus;
	bool m_bAdult;

public:
	SearchFilter();
	bool operator==(const SearchFilter& rOther);
	bool operator!=(const SearchFilter& rOther);
	bool operator<(const SearchFilter& rOther);
	bool operator>(const SearchFilter& rOther);
};

class SearchTreeItem : public QObject
{
	Q_OBJECT
public:
	enum Type { SearchTreeItemType, TreeRootType, SearchFileType, SearchHitType };

protected:
	QList<SearchTreeItem*>  m_lChildItems;
	QList<QVariant>         m_lItemData;
	SearchTreeItem*         m_pParentItem;

	Type                    m_eType;

public:
	SearchHitData::sSearchHitData m_oHitData;

public:
	SearchTreeItem(const QList<QVariant> &data, SearchTreeItem* parent);
	virtual ~SearchTreeItem();

	Type type() const;

	void appendChild(SearchTreeItem* child);
	void clearChildren();

	int row() const;
	void removeChild(int position);

	inline SearchTreeItem* child(int row) const;
	inline virtual int childCount() const;

	inline int columnCount() const;
	inline QVariant data(int column) const;
	inline SearchTreeItem* parent() const;
};

SearchTreeItem* SearchTreeItem::child(int row) const
{
	return m_lChildItems.value( row );
}

int SearchTreeItem::childCount() const
{
	return m_lChildItems.count();
}

int SearchTreeItem::columnCount() const
{
	return m_lItemData.count();
}

QVariant SearchTreeItem::data(int column) const
{
	return m_lItemData.value(column);
}

SearchTreeItem* SearchTreeItem::parent() const
{
	return m_pParentItem;
}

class SearchTreeModel;
class TreeRoot : public SearchTreeItem
{
	Q_OBJECT
private:
	SearchTreeModel* m_pModel; // The model the root node is part of.
public:
	TreeRoot(const QList<QVariant> &data, SearchTreeModel* pModel);
	~TreeRoot();

	void addQueryHit(QueryHit* pHit);
	int find(CHash& pHash) const; // find child number with given hash
private:
};

class SearchFile : public SearchTreeItem
{
	Q_OBJECT
public:
	HashVector m_lHashes;

public:
	SearchFile(const QList<QVariant> &data, SearchTreeItem* parent);
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
	SearchHit(const QList<QVariant> &data, SearchTreeItem* parent);
	~SearchHit();

	int childCount() const;

private:
};

// TODO: replace with forward_list (C++11)
typedef std::list<SearchTreeItem*> SearchList;

class SearchTreeModel : public QAbstractItemModel
{
	Q_OBJECT

private:
	SearchFilter*      m_pFilter;
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
