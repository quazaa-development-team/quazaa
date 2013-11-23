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
class CFileIconProvider;

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
	SearchTreeItem(const QList<QVariant> &data, SearchTreeItem* parent = 0);
	~SearchTreeItem();

	SearchHitData::sSearchHitData HitData;

	void appendChild(SearchTreeItem* child);
	void clearChildren();

	SearchTreeItem* child(int row) const;
	int childCount() const;
	int columnCount() const;
	int find(CHash& pHash) const;
	void updateHitCount(int count);
	bool duplicateCheck(SearchTreeItem* containerItem, QString ip);
	QVariant data(int column) const;
	int row() const;
	SearchTreeItem* parent();
	void removeChild(int position);

private:
	QList<SearchTreeItem*> childItems;
	QList<QVariant> itemData;
	SearchTreeItem* parentItem;
};

class SearchTreeModel : public QAbstractItemModel
{
	Q_OBJECT

private:
	SearchFilter*      m_pFilter;
	CFileIconProvider* m_pIconProvider;

	SearchTreeItem*    rootItem;

public:
	SearchTreeModel();
	~SearchTreeModel();

	QVariant data(const QModelIndex& index, int role) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	SearchTreeItem* topLevelItemFromIndex(QModelIndex index);
	SearchTreeItem* itemFromIndex(QModelIndex index);
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	int nFileCount;

signals:
	void updateStats();
	void sort();

private:
	void setupModelData(const QStringList& lines, SearchTreeItem* parent);

public slots:
	void clear();
	bool isRoot(QModelIndex index);
	void removeQueryHit(int position, const QModelIndex &parent);

private slots:
	void addQueryHit(QueryHitSharedPtr pHit);
};

#endif // SEARCHTREEMODEL_H
