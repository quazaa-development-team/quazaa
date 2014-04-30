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

class EndPoint : public QHostAddress
{
protected:
	quint16	m_nPort;
	mutable QString m_sCountryCode;

public:
	EndPoint();
	explicit EndPoint( quint32 ip4Addr, quint16 nPort = 0 );
	explicit EndPoint( quint8* ip6Addr, quint16 nPort = 0 );
	explicit EndPoint( const Q_IPV6ADDR& ip6Addr, quint16 nPort = 0 );
	explicit EndPoint( const sockaddr* sockaddr, quint16 nPort = 0 );
	explicit EndPoint( const QString& address, quint16 nPort );
	explicit EndPoint( const QString& address );
	explicit EndPoint( const QHostAddress& address, quint16 nPort );
	EndPoint( const EndPoint& copy );
	EndPoint( SpecialAddress address, quint16 nPort = 0 );

	EndPoint& operator=( const EndPoint& rhs );
	EndPoint& operator++();
	EndPoint& operator--();
	EndPoint operator++( int );
	EndPoint operator--( int );

	bool operator==( const EndPoint& rhs ) const;
	bool operator!=( const EndPoint& rhs ) const;
	bool operator<( const EndPoint& rhs ) const;
	bool operator>( const EndPoint& rhs ) const;
	bool operator<=( const EndPoint& rhs ) const;
	bool operator>=( const EndPoint& rhs ) const;

	bool operator==( const QHostAddress& rhs ) const;
	bool operator!=( const QHostAddress& rhs ) const;

	void setAddressWithPort( const QString& address );
	QString toStringWithPort() const;

	quint16 port() const;
	void setPort( const quint16 nPort );

	bool isFirewalled() const;
	bool isValid() const;

	QString country() const;
	QString countryName() const;

	void clear();

	friend QDataStream& operator<<( QDataStream&, const EndPoint& );
	friend QDataStream& operator>>( QDataStream&, EndPoint& );
};

QDataStream& operator<<( QDataStream& s, const EndPoint& rhs );
QDataStream& operator>>( QDataStream& s, EndPoint& rhs );

#endif // ENDPOINT_H
