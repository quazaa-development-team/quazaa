/*
** $Id: networkiconprovider.cpp 928 2012-02-15 17:19:00Z brov $
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

#include <QPixmapCache>
#include <QIcon>

#include "networkiconprovider.h"

NetworkIconProvider::NetworkIconProvider()
{
}

QIcon NetworkIconProvider::icon( DiscoveryProtocol::Protocol protocol )
{
	QIcon icon;
	QString key = QLatin1String( "nip_dp_" ) + QString::number( protocol );

	QPixmap pixIcon;
	if ( QPixmapCache::find( key, pixIcon ) )
	{
		// pixIcon internally uses shared data to avoid needless copying.
		icon.addPixmap( pixIcon );
		return icon;
	}

	switch ( protocol )
	{
	case DiscoveryProtocol::None:
		break;
	case DiscoveryProtocol::G2:
		pixIcon.load( ":/Resource/Networks/Gnutella2.png" );
		break;
	case DiscoveryProtocol::Gnutella:
		pixIcon.load( ":/Resource/Networks/Gnutella2.png" );
		break;
	case DiscoveryProtocol::Ares:
		pixIcon.load( ":/Resource/Networks/Ares.png" );
		break;
	case DiscoveryProtocol::eDonkey2000:
		pixIcon.load( ":/Resource/Networks/EDonkey.png" );
		break;
	}

	if ( pixIcon.isNull() )
	{
		return QIcon();
	}

	icon.addPixmap( pixIcon );

	// Default cache size: 2048 KB on embedded platforms, 10240 KB on desktop platforms
	QPixmapCache::insert( key, pixIcon );

	return icon;
}

QIcon NetworkIconProvider::icon( TransferProtocol protocol )
{
	QIcon icon;
	QString key = QLatin1String( "nip_tp_" ) + QString::number( protocol );

	QPixmap pixIcon;
	if ( QPixmapCache::find( key, pixIcon ) )
	{
		// pixIcon internally uses shared data to avoid needless copying.
		icon.addPixmap( pixIcon );
		return icon;
	}

	switch ( protocol )
	{
	case tpNull:
		break;
	case tpHTTP:
		pixIcon.load( ":/Resource/Networks/http.png" );
		break;
	case tpBitTorrent:
		pixIcon.load( ":/Resource/Networks/BitTorrent.png" );
	}

	if ( pixIcon.isNull() )
	{
		return QIcon();
	}

	icon.addPixmap( pixIcon );

	// Default cache size: 2048 KB on embedded platforms, 10240 KB on desktop platforms
	QPixmapCache::insert( key, pixIcon );

	return icon;
}

QIcon NetworkIconProvider::icon( const QString& sCountryCode )
{
	QIcon icon;
	QPixmap pixIcon;
	if ( QPixmapCache::find( sCountryCode, pixIcon ) )
	{
		// pixIcon internally uses shared data to avoid needless copying.
		icon.addPixmap( pixIcon );
		return icon;
	}

	pixIcon.load( ":/Resource/Flags/" + sCountryCode.toLower() + ".png" );

	if ( pixIcon.isNull() )
	{
		return QIcon();
	}

	icon.addPixmap( pixIcon );

	// Default cache size: 2048 KB on embedded platforms, 10240 KB on desktop platforms
	QPixmapCache::insert( sCountryCode, pixIcon );

	return icon;
}
