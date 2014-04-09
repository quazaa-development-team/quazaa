/*
** hostcachetablemodeldata.h
**
** Copyright © Quazaa Development Team, 2014-2014.
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

#ifndef HOSTCACHETABLEMODELDATA_H
#define HOSTCACHETABLEMODELDATA_H

#include "HostCache/g2hostcache.h"

class HostCacheTableModel;

class HostData
{
public:
	SharedHostPtr   m_pHost;

	const CEndPoint m_oAddress;
	const QString   m_sAddress;
	const QString   m_sCountryCode;
	const QString   m_sCountry;
	const QIcon     m_iCountry;
	const quint32   m_nID;
	quint32         m_tLastConnect;
	QString         m_sLastConnect;
	quint8          m_nFailures;
	QString         m_sFailures;

	//QString         m_sUserAgent;
	//QIcon           m_iNetwork;

	HostData(SharedHostPtr pHost);
	bool update(int row, int col, QModelIndexList& to_update, HostCacheTableModel* model);
	QVariant data(int col) const;
	bool lessThan(int col, HostData* pOther) const;
};

#endif // HOSTCACHETABLEMODELDATA_H
