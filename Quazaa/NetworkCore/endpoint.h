/*
** endpoint.h
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

#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <QHostAddress>
#include "geoiplist.h"

class CEndPoint : public QHostAddress
{
protected:
	quint16	m_nPort;
	mutable QString m_sCountryCode;

public:
	CEndPoint();
	explicit CEndPoint(quint32 ip4Addr, quint16 nPort = 0);
	explicit CEndPoint(quint8* ip6Addr, quint16 nPort = 0);
	explicit CEndPoint(const Q_IPV6ADDR& ip6Addr, quint16 nPort = 0);
	explicit CEndPoint(const sockaddr* sockaddr, quint16 nPort = 0);
	explicit CEndPoint(const QString& address, quint16 nPort);
	explicit CEndPoint(const QString& address);
	explicit CEndPoint(const QHostAddress& address, quint16 nPort);
	CEndPoint(const CEndPoint& copy);
	CEndPoint(SpecialAddress address, quint16 nPort = 0);

	CEndPoint& operator=(const CEndPoint& rhs);
	CEndPoint& operator++();
	CEndPoint& operator--();
	CEndPoint operator++(int);
	CEndPoint operator--(int);

	bool operator==(const CEndPoint& rhs) const;
	bool operator!=(const CEndPoint& rhs) const;
	bool operator<(const CEndPoint& rhs) const;
	bool operator>(const CEndPoint& rhs) const;
	bool operator<=(const CEndPoint& rhs) const;
	bool operator>=(const CEndPoint& rhs) const;

	bool operator==(const QHostAddress& rhs) const;
	bool operator!=(const QHostAddress& rhs) const;

	void setAddressWithPort(const QString& address);
	QString toStringWithPort() const;

	quint16 port() const;
	void setPort(const quint16 nPort);

	bool isFirewalled() const;
	bool isValid() const;

	QString country() const;
	QString countryName() const;

	void clear();

	friend QDataStream &operator<<(QDataStream &, const CEndPoint &);
	friend QDataStream &operator>>(QDataStream &, CEndPoint &);
};

QDataStream &operator<<(QDataStream &s, const CEndPoint &rhs);
QDataStream &operator>>(QDataStream &s, CEndPoint &rhs);

#endif // ENDPOINT_H
