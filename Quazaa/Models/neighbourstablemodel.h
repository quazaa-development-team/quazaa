/*
** neighbourstablemodel.h
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

#ifndef NEIGHBOURSTABLEMODELL_H
#define NEIGHBOURSTABLEMODELL_H

#include <QAbstractTableModel>
#include "types.h"
#include <QVector>
#include <QTime>
#include <QMutex>
#include <QIcon>

class CNeighbour;

class CNeighboursTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum Column
	{
		ADDRESS = 0,
		TIME = 1,
		BANDWIDTH = 2,
		BYTES = 3,
		PACKETS = 4,
		MODE = 5,
		LEAVES = 6,
		PING = 7,
		USER_AGENT = 8,
		COUNTRY = 9,
		_NO_OF_COLUMNS
	};

	struct Neighbour
	{
		CNeighbour*   pNode;


		CEndPoint     oAddress;
		G2NodeType    nType;
		int           nState;
		QString       sHandshake;
		int           nDiscoveryProtocol;
		quint32       tConnected;
		quint32       nPacketsIn;
		quint32       nPacketsOut;
		quint32       nBandwidthIn;
		quint32       nBandwidthOut;
		quint64       nBytesSent;
		quint64       nBytesReceived;
		float         nCompressionIn;
		float         nCompressionOut;
		quint32       nLeafCount;
		quint32       nLeafMax;
		qint64        nRTT;
		QString       sUserAgent;
		QString		  sCountryCode;
		QString       sCountry;
		QIcon		  iNetwork;
		QIcon		  iCountry;

		Neighbour(CNeighbour* pNeighbour);
		bool update(int row, int col, QModelIndexList& to_update, CNeighboursTableModel* model);
		QVariant data(int col) const;
		bool lessThan(int col, CNeighboursTableModel::Neighbour* pOther) const;

		QString stateToString(int s) const;
		QString typeToString(G2NodeType t) const;

	};

protected:
	QVector<Neighbour*>   m_lNodes;
public:
	explicit CNeighboursTableModel(QObject* parent = 0, QWidget* container = 0);
	~CNeighboursTableModel();

	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

	void sort(int column, Qt::SortOrder order);

	CNeighbour* nodeFromIndex(const QModelIndex& index);
protected:

private:
	QWidget*		m_oContainer;
	int				m_nSortColumn;
	Qt::SortOrder	m_nSortOrder;
	bool			m_bNeedSorting;

public slots:
	void addNode(CNeighbour* pNode);
	void removeNode(CNeighbour* pNode);
	void updateAll();
};

#endif // NEIGHBOURSTABLEMODELL_H
