/*
** geoiplist.h
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

#ifndef GEOIPLIST_H
#define GEOIPLIST_H

#include "types.h"

#include <QObject>
#include <QList>
#include <QPair>

typedef QPair<quint32, QPair<quint32, QString> > GeoIPEntry;

class GeoIPList
{
protected:
	bool	m_bListLoaded;
public:
	struct sGeoID
	{
		QString countryCode;
		QString countryName;
	};
	sGeoID GeoID;

	QList<GeoIPEntry> m_lDatabase;

	GeoIPList();
	void loadGeoIP();

	/**
	 * @brief findCountryCode allows to find out the country code for a given IP.
	 * @param sIP : the IP
	 * @return the country code; "ZZ" if code could not be found or geo IP list not yet initialized.
	 */
	QString findCountryCode( const QString& sIP ) const;

	/**
	 * @brief findCountryCode allows to find out the country code for a given IP.
	 * @param ip : the IP
	 * @return the country code; "ZZ" if code could not be found or geo IP list not yet initialized.
	 */
	QString findCountryCode( const QHostAddress& ip ) const;

	/**
	 * @brief findCountryCode allows to find out the country code for a given IP.
	 * @param nIP : the IP
	 * @return the country code; "ZZ" if code could not be found or geo IP list not yet initialized.
	 */
	QString findCountryCode( const quint32 nIp ) const;

	QString countryNameFromCode( const QString& code ) const;
};

extern GeoIPList geoIP;

#endif // GEOIPLIST_H
