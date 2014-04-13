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
class FileIconProvider;

namespace SearchHitData
{
	struct sSearchHitData
	{
		QList<CHash> lHashes;
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
	SearchTreeItem(const QList<QVariant> &data, SearchTreeItem* parent = 0);
	~SearchTreeItem();

	Type type() const;

	virtual void appendChild(SearchTreeItem* child);
	virtual void clearChildren();

	virtual SearchTreeItem* child(int row) const;
	virtual int childCount() const;

	int columnCount() const;
	void updateHitCount(int count); // change number of hits
	bool duplicateCheck(SearchTreeItem* containerItem, QString ip);
	QVariant data(int column) const;
	int row() const;

	SearchTreeItem* parent();
	void removeChild(int position);
};

class TreeRoot : public SearchTreeItem
{
	Q_OBJECT
public:
	TreeRoot(const QList<QVariant> &data, SearchTreeItem* parent = 0);
	int find(CHash& pHash) const; // find child number with given hash
private:
};

class SearchFile : public SearchTreeItem
{
	Q_OBJECT
public:
	SearchFile(const QList<QVariant> &data, SearchTreeItem* parent = 0);
private:
};

class SearchHit : public SearchTreeItem
{
	Q_OBJECT
public:
	SearchHit(const QList<QVariant> &data, SearchTreeItem* parent = 0);
	int childCount() const;

private:
};

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
};

#endif // SEARCHTREEMODEL_H
