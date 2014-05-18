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

	void setAddress( quint32 ip4Addr );
	void setAddress( quint8* ip6Addr );
	void setAddress( const Q_IPV6ADDR& ip6Addr );
	bool setAddress( const QString& address );
	void setAddress( const sockaddr* sockaddr );
	void setAddress( const QHostAddress& rhs );
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

namespace std
{
template <>
/**
 * @brief The hash<QHostAddress> struct provides a 32 bit hash of a given QHostAddress, thus
 * allowing the usage of the QHostAddress class together with std::unordered_map.
 */
struct hash<QHostAddress> : public unary_function<QHostAddress, quint32>
{
	/**
	 * @brief operator() Hashes a given value and returns its 32 bit hash.
	 * @param value The QHostAddress to hash.
	 * @return The 32 bit hash of the QHostAddress.
	 */
	quint32 operator()( const QHostAddress& value ) const
	{
		if ( value.protocol() == QAbstractSocket::IPv4Protocol )
		{
			return value.toIPv4Address();
		}
		else
		{
			Q_IPV6ADDR addr128bit = value.toIPv6Address();
			quint32* addr32bit = ( quint32* )( &addr128bit );
			return addr32bit[0] + addr32bit[1] + addr32bit[2] + addr32bit[3];
		}
	}
};


template <>
/**
 * @brief The hash<EndPoint> struct provides a 32 bit hash of a given EndPoint, thus allowing the
 * usage of the EndPoint class together with std::unordered_map.
 */
struct hash<EndPoint> : public unary_function<EndPoint, quint32>
{
	/**
	 * @brief operator() Hashes a given value and returns its 32 bit hash.
	 * @param value The EndPoint to hash.
	 * @return The 32 bit hash of the EndPoint.
	 */
	quint32 operator()( const EndPoint& value ) const
	{
		if ( value.protocol() == QAbstractSocket::IPv4Protocol )
		{
			return value.toIPv4Address() * value.port();
		}
		else
		{
			Q_IPV6ADDR addr128bit = value.toIPv6Address();
			quint32* addr32bit = ( quint32* )( &addr128bit );
			return ( addr32bit[0] + addr32bit[1] + addr32bit[2] + addr32bit[3] ) * value.port();
		}
	}
};
}

#endif // ENDPOINT_H
