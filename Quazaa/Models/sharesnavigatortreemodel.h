#ifndef SHARESNAVIGATORTREEMODEL_H
#define SHARESNAVIGATORTREEMODEL_H

#include <QObject>
#include <QIcon>
#include <QAbstractItemModel>

namespace SharesFolderData
{
struct sSharesFolderData
{
	QString directoryName;
	QString fullPath;
	QString parentIndex;
	QString index;
};
};

class SharesNavigatorTreeItem : public QObject
{
	Q_OBJECT
public:
	SharesNavigatorTreeItem( const QList<QVariant>& data, SharesNavigatorTreeItem* parent = 0 );
	~SharesNavigatorTreeItem();

	SharesFolderData::sSharesFolderData FolderData;

	void appendChild( SharesNavigatorTreeItem* child );
	void clearChildren();

	SharesNavigatorTreeItem* child( int row );
	int childCount() const;
	int columnCount() const;
	int find( SharesNavigatorTreeItem* containerItem );
	void updateHitCount( int count );
	bool duplicateCheck( SharesNavigatorTreeItem* containerItem, QString ip );
	QVariant data( int column ) const;
	int row() const;
	SharesNavigatorTreeItem* parent();

private:
	QList<SharesNavigatorTreeItem*> childItems;
	QList<QVariant> itemData;
	SharesNavigatorTreeItem* parentItem;
};

class SharesNavigatorTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	SharesNavigatorTreeModel();
	~SharesNavigatorTreeModel();

	QVariant data( const QModelIndex& index, int role ) const;
	Qt::ItemFlags flags( const QModelIndex& index ) const;
	QVariant headerData( int section, Qt::Orientation orientation,
						 int role = Qt::DisplayRole ) const;
	QModelIndex index( int row, int column,
					   const QModelIndex& parent = QModelIndex() ) const;
	QModelIndex parent( const QModelIndex& index ) const;
	int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	int columnCount( const QModelIndex& parent = QModelIndex() ) const;
	int nFileCount;

signals:
	void updateStats();

private:
	void setupModelData( const QStringList& lines, SharesNavigatorTreeItem* parent );
	SharesNavigatorTreeItem* rootItem;

public slots:
	void clear();
	bool isRoot( QModelIndex index );

private slots:
	void addFolder( );
};

#endif // SHARESNAVIGATORTREEMODEL_H
