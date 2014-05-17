/*
** $Id$
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

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "geoiplist.h"
#include "types.h"
#include "systemlog.h"
#include "quazaaglobals.h"

#include "debug_new.h"

GeoIPList geoIP;

GeoIPList::GeoIPList()
{
	m_bListLoaded = false;
}

void GeoIPList::loadGeoIP()
{
	const QString sOriginalFile( qApp->applicationDirPath() + "/GeoIP/geoip.dat" );
	const QString sSerializedFile( QuazaaGlobals::DATA_PATH() + "/geoIP.ser" );

	// try first to deserialize from .ser file
	bool readFromOriginalGeoIP = false;
	{
		QFile oSerializedFile( sSerializedFile );

		if ( QFile::exists( sSerializedFile ) && QFile::exists( sOriginalFile ) )
		{
			QFileInfo iOriginal( sOriginalFile );
			QFileInfo iSerialized( sSerializedFile );

			if ( iOriginal.lastModified() > iSerialized.lastModified() )
			{
				systemLog.postLog( LogSeverity::Warning,
								   QObject::tr( "GeoIP data modified, refreshing..." ) );
				oSerializedFile.remove();
			}
		}

		if ( ! oSerializedFile.open( QIODevice::ReadOnly ) )
		{
			systemLog.postLog( LogSeverity::Warning,
							   QObject::tr(
								   "Unable to open GeoIP serialization file for loading." ) );
			readFromOriginalGeoIP = true;
		}
		else
		{
			try
			{
				QDataStream iStream( &oSerializedFile );
				iStream >> m_lDatabase;
			}
			catch ( ... )
			{
				systemLog.postLog( LogSeverity::Warning,
								   QObject::tr( "Unable to deserialize GeoIP list." ) );
				readFromOriginalGeoIP = true;
			}
			oSerializedFile.close();
		}
	}

	if ( readFromOriginalGeoIP )
	{
		QFile oOriginalFile( sOriginalFile );
		if ( !oOriginalFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
		{
			return;
		}

		m_lDatabase.clear();

		QTextStream in( &oOriginalFile );
		while ( !in.atEnd() )
		{
			QStringList line = in.readLine().split( " " );

			if ( line.size() != 3 )
			{
				systemLog.postLog( LogSeverity::Warning, "[GeoIP] Bad line, skippig" );
				continue;
			}
			EndPoint rBegin( line[0] + ":0" );
			EndPoint rEnd( line[1] + ":0" );
			QString sCountry = line[2];

			GeoIPEntry item = GeoIPEntry( rBegin.toIPv4Address(),
										  QPair<quint32, QString>( rEnd.toIPv4Address(),
																   sCountry ) );
			m_lDatabase.append( item );
		}

		// sort the database before saving so binary search can work without the need for resorting
		// each time the database (list) is loaded
		qSort( m_lDatabase );

		QFile oSerializedFile( sSerializedFile );
		if ( ! oSerializedFile.open( QIODevice::WriteOnly ) )
		{
			systemLog.postLog( LogSeverity::Error,
							   QObject::tr(
								   "Unable to open GeoIP serialization file for saving." ) );
		}
		else
		{
			try
			{
				QDataStream oStream( &oSerializedFile );
				oStream << m_lDatabase;
			}
			catch ( ... )
			{
				systemLog.postLog( LogSeverity::Error,
								   QObject::tr( "Unable to serialize GeoIP list." ) );
			}
			oSerializedFile.close();
		}
	}

	// REMOVE for Quazaa 1.0
#ifdef _DEBUG
	// verify that loadad list is properly sorted
	for ( int i = 0, nMax = m_lDatabase.size() - 1; i < nMax; ++i )
	{
		Q_ASSERT( m_lDatabase[i] <= m_lDatabase[i + 1] );
	}
#else
	for ( int i = 0, nMax = m_lDatabase.size() - 1; i < nMax; ++i )
	{
		if ( m_lDatabase[i] > m_lDatabase[i + 1] )
		{
			qSort( m_lDatabase );
			systemLog.postLog( LogSeverity::Error,
							   QObject::tr( "Broken serialized GeoIP list loaded from disk." ) );
			break;
		}
	}
#endif // _DEBUG

	m_bListLoaded = !m_lDatabase.isEmpty();
}

QString GeoIPList::findCountryCode( const QString& sIP ) const
{
	EndPoint ipAddress( sIP );
	return findCountryCode( ipAddress );
}

QString GeoIPList::findCountryCode( const QHostAddress& ip ) const
{
	if ( ip.protocol() == 1 ) // IPv6
	{
		return "ZZ";
	}

	const quint32 ip4 = ip.toIPv4Address();
	return findCountryCode( ip4 );
}

QString GeoIPList::findCountryCode( const quint32 nIp ) const
{
	if ( !m_bListLoaded )
	{
		return "ZZ";
	}

	int nMiddle;
	int nBegin = 0;
	int nEnd = m_lDatabase.size();

	int n = nEnd - nBegin;

	int nHalf;

	while ( n > 0 )
	{
		nHalf = n >> 1;

		nMiddle = nBegin + nHalf;

		if ( nIp < m_lDatabase.at( nMiddle ).first )
		{
			n = nHalf;
		}
		else
		{
			if ( nIp <= m_lDatabase.at( nMiddle ).second.first )
			{
				return m_lDatabase.at( nMiddle ).second.second;
			}
			nBegin = nMiddle + 1;
			n -= nHalf + 1;
		}
	}

	return "ZZ";
}

QString GeoIPList::countryNameFromCode( const QString& code ) const
{
	// Leave the formatting this way as it is easier to update from text file.
	if ( code == "AD" )
	{
		return QObject::tr( "Andorra" );
	}
	if ( code == "AE" )
	{
		return QObject::tr( "United Arab Emirates" );
	}
	if ( code == "AF" )
	{
		return QObject::tr( "Afghanistan" );
	}
	if ( code == "AG" )
	{
		return QObject::tr( "Antigua and Barbuda" );
	}
	if ( code == "AI" )
	{
		return QObject::tr( "Anguilla" );
	}
	if ( code == "AL" )
	{
		return QObject::tr( "Albania" );
	}
	if ( code == "AM" )
	{
		return QObject::tr( "Armenia" );
	}
	if ( code == "AO" )
	{
		return QObject::tr( "Angola" );
	}
	if ( code == "AR" )
	{
		return QObject::tr( "Argentina" );
	}
	if ( code == "AS" )
	{
		return QObject::tr( "American Samoa" );
	}
	if ( code == "AT" )
	{
		return QObject::tr( "Austria" );
	}
	if ( code == "AU" )
	{
		return QObject::tr( "Australia" );
	}
	if ( code == "AW" )
	{
		return QObject::tr( "Aruba" );
	}
	if ( code == "AZ" )
	{
		return QObject::tr( "Azerbaijan" );
	}
	if ( code == "BA" )
	{
		return QObject::tr( "Bosnia and Herzegowina" );
	}
	if ( code == "BB" )
	{
		return QObject::tr( "Barbados" );
	}
	if ( code == "BD" )
	{
		return QObject::tr( "Bangladesh" );
	}
	if ( code == "BE" )
	{
		return QObject::tr( "Belgium" );
	}
	if ( code == "BF" )
	{
		return QObject::tr( "Burkina Faso" );
	}
	if ( code == "BG" )
	{
		return QObject::tr( "Bulgaria" );
	}
	if ( code == "BH" )
	{
		return QObject::tr( "Bahrain" );
	}
	if ( code == "BI" )
	{
		return QObject::tr( "Burundi" );
	}
	if ( code == "BJ" )
	{
		return QObject::tr( "Benin" );
	}
	if ( code == "BM" )
	{
		return QObject::tr( "Bermuda" );
	}
	if ( code == "BN" )
	{
		return QObject::tr( "Brunei Darussalam" );
	}
	if ( code == "BO" )
	{
		return QObject::tr( "Bolivia" );
	}
	if ( code == "BQ" )
	{
		return QObject::tr( "Bonaire, St. Eustatius and Saba" );
	}
	if ( code == "BR" )
	{
		return QObject::tr( "Brazil" );
	}
	if ( code == "BS" )
	{
		return QObject::tr( "Bahamas" );
	}
	if ( code == "BT" )
	{
		return QObject::tr( "Bhutan" );
	}
	if ( code == "BW" )
	{
		return QObject::tr( "Botswana" );
	}
	if ( code == "BY" )
	{
		return QObject::tr( "Belarus" );
	}
	if ( code == "BZ" )
	{
		return QObject::tr( "Belize" );
	}
	if ( code == "CA" )
	{
		return QObject::tr( "Canada" );
	}
	if ( code == "CD" )
	{
		return QObject::tr( "The Democratic Republic of The Congo" );
	}
	if ( code == "CF" )
	{
		return QObject::tr( "Central African Republic" );
	}
	if ( code == "CG" )
	{
		return QObject::tr( "Congo" );
	}
	if ( code == "CH" )
	{
		return QObject::tr( "Switzerland" );
	}
	if ( code == "CI" )
	{
		return QObject::tr( "Cote D'ivoire" );
	}
	if ( code == "CK" )
	{
		return QObject::tr( "Cook Islands" );
	}
	if ( code == "CL" )
	{
		return QObject::tr( "Chile" );
	}
	if ( code == "CM" )
	{
		return QObject::tr( "Cameroon" );
	}
	if ( code == "CN" )
	{
		return QObject::tr( "China" );
	}
	if ( code == "CO" )
	{
		return QObject::tr( "Colombia" );
	}
	if ( code == "CR" )
	{
		return QObject::tr( "Costa Rica" );
	}
	if ( code == "CU" )
	{
		return QObject::tr( "Cuba" );
	}
	if ( code == "CV" )
	{
		return QObject::tr( "Cape Verde" );
	}
	if ( code == "CW" )
	{
		return QObject::tr( "Curacao" );
	}
	if ( code == "CY" )
	{
		return QObject::tr( "Cyprus" );
	}
	if ( code == "CZ" )
	{
		return QObject::tr( "Czech Republic" );
	}
	if ( code == "DE" )
	{
		return QObject::tr( "Germany" );
	}
	if ( code == "DJ" )
	{
		return QObject::tr( "Djibouti" );
	}
	if ( code == "DK" )
	{
		return QObject::tr( "Denmark" );
	}
	if ( code == "DM" )
	{
		return QObject::tr( "Dominica" );
	}
	if ( code == "DO" )
	{
		return QObject::tr( "Dominican Republic" );
	}
	if ( code == "DZ" )
	{
		return QObject::tr( "Algeria" );
	}
	if ( code == "EC" )
	{
		return QObject::tr( "Ecuador" );
	}
	if ( code == "EE" )
	{
		return QObject::tr( "Estonia" );
	}
	if ( code == "EG" )
	{
		return QObject::tr( "Egypt" );
	}
	if ( code == "ER" )
	{
		return QObject::tr( "Eritrea" );
	}
	if ( code == "ES" )
	{
		return QObject::tr( "Spain" );
	}
	if ( code == "ET" )
	{
		return QObject::tr( "Ethiopia" );
	}
	if ( code == "EU" )
	{
		return QObject::tr( "European Union" );
	}
	if ( code == "FI" )
	{
		return QObject::tr( "Finland" );
	}
	if ( code == "FJ" )
	{
		return QObject::tr( "Fiji" );
	}
	if ( code == "FM" )
	{
		return QObject::tr( "Federated States of Micronesia" );
	}
	if ( code == "FO" )
	{
		return QObject::tr( "Faroe Islands" );
	}
	if ( code == "FR" )
	{
		return QObject::tr( "France" );
	}
	if ( code == "GA" )
	{
		return QObject::tr( "Gabon" );
	}
	if ( code == "GB" )
	{
		return QObject::tr( "United Kingdom" );
	}
	if ( code == "GD" )
	{
		return QObject::tr( "Grenada" );
	}
	if ( code == "GE" )
	{
		return QObject::tr( "Georgia" );
	}
	if ( code == "GF" )
	{
		return QObject::tr( "French Guiana" );
	}
	if ( code == "GG" )
	{
		return QObject::tr( "Guernsey" );
	}
	if ( code == "GH" )
	{
		return QObject::tr( "Ghana" );
	}
	if ( code == "GI" )
	{
		return QObject::tr( "Gibraltar" );
	}
	if ( code == "GL" )
	{
		return QObject::tr( "Greenland" );
	}
	if ( code == "GM" )
	{
		return QObject::tr( "Gambia" );
	}
	if ( code == "GN" )
	{
		return QObject::tr( "Guinea" );
	}
	if ( code == "GP" )
	{
		return QObject::tr( "Guadeloupe" );
	}
	if ( code == "GQ" )
	{
		return QObject::tr( "Equatorial Guinea" );
	}
	if ( code == "GR" )
	{
		return QObject::tr( "Greece" );
	}
	if ( code == "GT" )
	{
		return QObject::tr( "Guatemala" );
	}
	if ( code == "GU" )
	{
		return QObject::tr( "Guam" );
	}
	if ( code == "GW" )
	{
		return QObject::tr( "Guinea-bissau" );
	}
	if ( code == "GY" )
	{
		return QObject::tr( "Guyana" );
	}
	if ( code == "HK" )
	{
		return QObject::tr( "Hong Kong" );
	}
	if ( code == "HN" )
	{
		return QObject::tr( "Honduras" );
	}
	if ( code == "HR" )
	{
		return QObject::tr( "Croatia" );
	}
	if ( code == "HT" )
	{
		return QObject::tr( "Haiti" );
	}
	if ( code == "HU" )
	{
		return QObject::tr( "Hungary" );
	}
	if ( code == "ID" )
	{
		return QObject::tr( "Indonesia" );
	}
	if ( code == "IE" )
	{
		return QObject::tr( "Ireland" );
	}
	if ( code == "IL" )
	{
		return QObject::tr( "Israel" );
	}
	if ( code == "IM" )
	{
		return QObject::tr( "Isle of Man" );
	}
	if ( code == "IN" )
	{
		return QObject::tr( "India" );
	}
	if ( code == "IO" )
	{
		return QObject::tr( "British Indian Ocean Territory" );
	}
	if ( code == "IQ" )
	{
		return QObject::tr( "Iraq" );
	}
	if ( code == "IR" )
	{
		return QObject::tr( "Islamic Republic of Iran" );
	}
	if ( code == "IS" )
	{
		return QObject::tr( "Iceland" );
	}
	if ( code == "IT" )
	{
		return QObject::tr( "Italy" );
	}
	if ( code == "JE" )
	{
		return QObject::tr( "Jersey" );
	}
	if ( code == "JM" )
	{
		return QObject::tr( "Jamaica" );
	}
	if ( code == "JO" )
	{
		return QObject::tr( "Jordan" );
	}
	if ( code == "JP" )
	{
		return QObject::tr( "Japan" );
	}
	if ( code == "KE" )
	{
		return QObject::tr( "Kenya" );
	}
	if ( code == "KG" )
	{
		return QObject::tr( "Kyrgyzstan" );
	}
	if ( code == "KH" )
	{
		return QObject::tr( "Cambodia" );
	}
	if ( code == "KI" )
	{
		return QObject::tr( "Kiribati" );
	}
	if ( code == "KM" )
	{
		return QObject::tr( "Comoros" );
	}
	if ( code == "KN" )
	{
		return QObject::tr( "Saint Kitts and Nevis" );
	}
	if ( code == "KP" )
	{
		return QObject::tr( "Democratic People's Republic of Korea" );
	}
	if ( code == "KR" )
	{
		return QObject::tr( "Republic of Korea" );
	}
	if ( code == "KW" )
	{
		return QObject::tr( "Kuwait" );
	}
	if ( code == "KY" )
	{
		return QObject::tr( "Cayman Islands" );
	}
	if ( code == "KZ" )
	{
		return QObject::tr( "Kazakhstan" );
	}
	if ( code == "LA" )
	{
		return QObject::tr( "Lao People's Democratic Republic" );
	}
	if ( code == "LB" )
	{
		return QObject::tr( "Lebanon" );
	}
	if ( code == "LC" )
	{
		return QObject::tr( "Saint Lucia" );
	}
	if ( code == "LI" )
	{
		return QObject::tr( "Liechtenstein" );
	}
	if ( code == "LK" )
	{
		return QObject::tr( "Sri Lanka" );
	}
	if ( code == "LR" )
	{
		return QObject::tr( "Liberia" );
	}
	if ( code == "LS" )
	{
		return QObject::tr( "Lesotho" );
	}
	if ( code == "LT" )
	{
		return QObject::tr( "Lithuania" );
	}
	if ( code == "LU" )
	{
		return QObject::tr( "Luxembourg" );
	}
	if ( code == "LV" )
	{
		return QObject::tr( "Latvia" );
	}
	if ( code == "LY" )
	{
		return QObject::tr( "Libyan Arab Jamahiriya" );
	}
	if ( code == "MA" )
	{
		return QObject::tr( "Morocco" );
	}
	if ( code == "MC" )
	{
		return QObject::tr( "Monaco" );
	}
	if ( code == "MD" )
	{
		return QObject::tr( "Republic of Moldova" );
	}
	if ( code == "ME" )
	{
		return QObject::tr( "Montenegro" );
	}
	if ( code == "MF" )
	{
		return QObject::tr( "Saint Martin" );
	}
	if ( code == "MG" )
	{
		return QObject::tr( "Madagascar" );
	}
	if ( code == "MH" )
	{
		return QObject::tr( "Marshall Islands" );
	}
	if ( code == "MK" )
	{
		return QObject::tr( "Macedonia" );
	}
	if ( code == "ML" )
	{
		return QObject::tr( "Mali" );
	}
	if ( code == "MM" )
	{
		return QObject::tr( "Myanmar" );
	}
	if ( code == "MN" )
	{
		return QObject::tr( "Mongolia" );
	}
	if ( code == "MO" )
	{
		return QObject::tr( "Macau" );
	}
	if ( code == "MP" )
	{
		return QObject::tr( "Northern Mariana Islands" );
	}
	if ( code == "MQ" )
	{
		return QObject::tr( "Martinique" );
	}
	if ( code == "MR" )
	{
		return QObject::tr( "Mauritania" );
	}
	if ( code == "MS" )
	{
		return QObject::tr( "Montserrat" );
	}
	if ( code == "MT" )
	{
		return QObject::tr( "Malta" );
	}
	if ( code == "MU" )
	{
		return QObject::tr( "Mauritius" );
	}
	if ( code == "MV" )
	{
		return QObject::tr( "Maldives" );
	}
	if ( code == "MW" )
	{
		return QObject::tr( "Malawi" );
	}
	if ( code == "MX" )
	{
		return QObject::tr( "Mexico" );
	}
	if ( code == "MY" )
	{
		return QObject::tr( "Malaysia" );
	}
	if ( code == "MZ" )
	{
		return QObject::tr( "Mozambique" );
	}
	if ( code == "NA" )
	{
		return QObject::tr( "Namibia" );
	}
	if ( code == "NC" )
	{
		return QObject::tr( "New Caledonia" );
	}
	if ( code == "NE" )
	{
		return QObject::tr( "Niger" );
	}
	if ( code == "NF" )
	{
		return QObject::tr( "Norfolk Island" );
	}
	if ( code == "NG" )
	{
		return QObject::tr( "Nigeria" );
	}
	if ( code == "NI" )
	{
		return QObject::tr( "Nicaragua" );
	}
	if ( code == "NL" )
	{
		return QObject::tr( "Netherlands" );
	}
	if ( code == "NO" )
	{
		return QObject::tr( "Norway" );
	}
	if ( code == "NP" )
	{
		return QObject::tr( "Nepal" );
	}
	if ( code == "NR" )
	{
		return QObject::tr( "Nauru" );
	}
	if ( code == "NU" )
	{
		return QObject::tr( "Niue" );
	}
	if ( code == "NZ" )
	{
		return QObject::tr( "New Zealand" );
	}
	if ( code == "OM" )
	{
		return QObject::tr( "Oman" );
	}
	if ( code == "PA" )
	{
		return QObject::tr( "Panama" );
	}
	if ( code == "PE" )
	{
		return QObject::tr( "Peru" );
	}
	if ( code == "PF" )
	{
		return QObject::tr( "French Polynesia" );
	}
	if ( code == "PG" )
	{
		return QObject::tr( "Papua New Guinea" );
	}
	if ( code == "PH" )
	{
		return QObject::tr( "Philippines" );
	}
	if ( code == "PK" )
	{
		return QObject::tr( "Pakistan" );
	}
	if ( code == "PL" )
	{
		return QObject::tr( "Poland" );
	}
	if ( code == "PM" )
	{
		return QObject::tr( "St. Pierre and Miquelon" );
	}
	if ( code == "PR" )
	{
		return QObject::tr( "Puerto Rico" );
	}
	if ( code == "PS" )
	{
		return QObject::tr( "Palestinian Territory Occupied" );
	}
	if ( code == "PT" )
	{
		return QObject::tr( "Portugal" );
	}
	if ( code == "PW" )
	{
		return QObject::tr( "Palau" );
	}
	if ( code == "PY" )
	{
		return QObject::tr( "Paraguay" );
	}
	if ( code == "QA" )
	{
		return QObject::tr( "Qatar" );
	}
	if ( code == "RE" )
	{
		return QObject::tr( "Reunion" );
	}
	if ( code == "RO" )
	{
		return QObject::tr( "Romania" );
	}
	if ( code == "RS" )
	{
		return QObject::tr( "Serbia" );
	}
	if ( code == "RU" )
	{
		return QObject::tr( "Russian Federation" );
	}
	if ( code == "RW" )
	{
		return QObject::tr( "Rwanda" );
	}
	if ( code == "SA" )
	{
		return QObject::tr( "Saudi Arabia" );
	}
	if ( code == "SB" )
	{
		return QObject::tr( "Solomon Islands" );
	}
	if ( code == "SC" )
	{
		return QObject::tr( "Seychelles" );
	}
	if ( code == "SD" )
	{
		return QObject::tr( "Sudan" );
	}
	if ( code == "SE" )
	{
		return QObject::tr( "Sweden" );
	}
	if ( code == "SG" )
	{
		return QObject::tr( "Singapore" );
	}
	if ( code == "SI" )
	{
		return QObject::tr( "Slovenia" );
	}
	if ( code == "SK" )
	{
		return QObject::tr( "Slovakia" );
	}
	if ( code == "SL" )
	{
		return QObject::tr( "Sierra Leone" );
	}
	if ( code == "SM" )
	{
		return QObject::tr( "San Marino" );
	}
	if ( code == "SN" )
	{
		return QObject::tr( "Senegal" );
	}
	if ( code == "SO" )
	{
		return QObject::tr( "Somalia" );
	}
	if ( code == "SR" )
	{
		return QObject::tr( "Suriname" );
	}
	if ( code == "SS" )
	{
		return QObject::tr( "South Sudan" );
	}
	if ( code == "ST" )
	{
		return QObject::tr( "Sao Tome and Principe" );
	}
	if ( code == "SV" )
	{
		return QObject::tr( "El Salvador" );
	}
	if ( code == "SX" )
	{
		return QObject::tr( "Sint Maarten" );
	}
	if ( code == "SY" )
	{
		return QObject::tr( "Syrian Arab Republic" );
	}
	if ( code == "SZ" )
	{
		return QObject::tr( "Swaziland" );
	}
	if ( code == "TC" )
	{
		return QObject::tr( "Turks and Caicos Islands" );
	}
	if ( code == "TD" )
	{
		return QObject::tr( "Chad" );
	}
	if ( code == "TG" )
	{
		return QObject::tr( "Togo" );
	}
	if ( code == "TH" )
	{
		return QObject::tr( "Thailand" );
	}
	if ( code == "TJ" )
	{
		return QObject::tr( "Tajikistan" );
	}
	if ( code == "TK" )
	{
		return QObject::tr( "Tokelau" );
	}
	if ( code == "TL" )
	{
		return QObject::tr( "Timor-leste" );
	}
	if ( code == "TM" )
	{
		return QObject::tr( "Turkmenistan" );
	}
	if ( code == "TN" )
	{
		return QObject::tr( "Tunisia" );
	}
	if ( code == "TO" )
	{
		return QObject::tr( "Tonga" );
	}
	if ( code == "TR" )
	{
		return QObject::tr( "Turkey" );
	}
	if ( code == "TT" )
	{
		return QObject::tr( "Trinidad and Tobago" );
	}
	if ( code == "TV" )
	{
		return QObject::tr( "Tuvalu" );
	}
	if ( code == "TW" )
	{
		return QObject::tr( "Taiwan, Republic of China" );
	}
	if ( code == "TZ" )
	{
		return QObject::tr( "United Republic of Tanzania" );
	}
	if ( code == "UA" )
	{
		return QObject::tr( "Ukraine" );
	}
	if ( code == "UG" )
	{
		return QObject::tr( "Uganda" );
	}
	if ( code == "UM" )
	{
		return QObject::tr( "United States Minor Outlying Islands" );
	}
	if ( code == "US" )
	{
		return QObject::tr( "United States" );
	}
	if ( code == "UY" )
	{
		return QObject::tr( "Uruguay" );
	}
	if ( code == "UZ" )
	{
		return QObject::tr( "Uzbekistan" );
	}
	if ( code == "VA" )
	{
		return QObject::tr( "Holy See (VATICAN City State)" );
	}
	if ( code == "VC" )
	{
		return QObject::tr( "Saint Vincent and The Grenadines" );
	}
	if ( code == "VE" )
	{
		return QObject::tr( "Venezuela" );
	}
	if ( code == "VG" )
	{
		return QObject::tr( "Virgin Islands (BRITISH)" );
	}
	if ( code == "VI" )
	{
		return QObject::tr( "Virgin Islands (U.S.)" );
	}
	if ( code == "VN" )
	{
		return QObject::tr( "Viet Nam" );
	}
	if ( code == "VU" )
	{
		return QObject::tr( "Vanuatu" );
	}
	if ( code == "WF" )
	{
		return QObject::tr( "Wallis and Futuna Islands" );
	}
	if ( code == "WS" )
	{
		return QObject::tr( "Samoa" );
	}
	if ( code == "YE" )
	{
		return QObject::tr( "Yemen" );
	}
	if ( code == "YT" )
	{
		return QObject::tr( "Mayotte" );
	}
	if ( code == "ZA" )
	{
		return QObject::tr( "South Africa" );
	}
	if ( code == "ZM" )
	{
		return QObject::tr( "Zambia" );
	}
	if ( code == "ZW" )
	{
		return QObject::tr( "Zimbabwe" );
	}
	else
	{
		return QObject::tr( "Unknown" );
	}
}

