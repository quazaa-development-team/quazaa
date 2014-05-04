/*
** networktype.h
**
** Copyright © Quazaa Development Team, 2012.
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

#ifndef NETWORKTYPE_H
#define NETWORKTYPE_H

#include <QtGlobal>
#include <QString>

namespace DiscoveryProtocol
{
enum Protocol
{
	None        = 0,
	Gnutella    = 1,
	G2          = 2,
	Ares        = 4,
	eDonkey2000 = 8
};
}

class NetworkType
{
private:
	quint16 m_nNetworks;

public:
	NetworkType();
	NetworkType( quint16 type );
	NetworkType( DiscoveryProtocol::Protocol type );

	inline void registerMetaType() const;

	bool operator==( const NetworkType& type ) const;
	bool operator!=( const NetworkType& type ) const;

	bool isNull() const;

	bool isGnutella() const;
	void setGnutella( bool );

	bool isG2() const;
	void setG2( bool );

	bool isAres() const;
	void setAres( bool );

	bool isEDonkey2000() const;
	void setEDonkey2000( bool );

	bool isNetwork( const NetworkType& type ) const;
	void setNetwork( const NetworkType& type );

	bool isMulti() const;

	quint16 toQuint16() const;

	QString toString() const;
};

void NetworkType::registerMetaType() const
{
	static int foo = qRegisterMetaType<NetworkType>( "NetworkType" );
	Q_UNUSED( foo );
}

#endif // NETWORKTYPE_H
