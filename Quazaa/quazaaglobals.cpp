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

#include "quazaaglobals.h"
#include "version.h"

#include "debug_new.h"

#include <QDir>
#include <QObject>
#include <QStandardPaths>

/*!
	\file quazaaglobals.h
	\brief \#include &qout;quazaaglobals.h&quot;
 */

/*!
	\class QuazaaGlobals quazaaglobals.h
	\ingroup base
	\brief Miscellaneous identifiers used throughout Quazaa.

	The QuazaaGlobals class provides a single place to provide global
	variables used throughout Quazaa.

	\section Accessing QuazaaGlobals

	It is recommended to access QuazaaGlobals instances via static
	QuazaaGlobals::FUNCTION_NAME() methods.
 */

/*!
	Returns the application name of Quazaa at run-time as a string.
 */
QString QuazaaGlobals::APPLICATION_NAME()
{
	return "Quazaa";
}

/*!
	Returns the version of Quazaa at run-time as a string.
 */
QString QuazaaGlobals::APPLICATION_VERSION()
{
#ifdef QT_DEBUG
	static const QString sVersion = QString( "%1.%2 Git:%3 (debug build)"
											 ).arg( QString::number( Version::MAJOR ),
													QString::number( Version::MINOR ),
													Version::REVISION );
#else
	static const QString sVersion = QString( "%1.%2 Git:%3"
											 ).arg( QString::number( Version::MAJOR ),
													QString::number( Version::MINOR ),
													Version::REVISION );
#endif
	return sVersion;
}

/*!
	Returns the version of Quazaa including build date at run-time as a string.
 */
QString QuazaaGlobals::APPLICATION_VERSION_STRING()
{
	static const QString sVersion = QString( "%1.%2 Git:%3 (%4)"
											 ).arg( QString::number( Version::MAJOR ),
													QString::number( Version::MINOR ),
													Version::REVISION,
													Version::BUILD_DATE );
	return sVersion;
}

/*!
	Returns the application fingerprint at run-time as a string.
 */
QString QuazaaGlobals::APPLICATION_FINGERPRINT()
{
	return "QZ" + APPLICATION_VERSION();
}

/*!
	Returns the organization name of Quazaa at run-time as a string.
 */
QString QuazaaGlobals::APPLICATION_ORGANIZATION_NAME()
{
	return "Quazaa Development Team";
}

/*!
	Returns the Quazaa website domain at run-time as a string.
 */
QString QuazaaGlobals::APPLICATION_ORGANIZATION_DOMAIN()
{
	return "quazaa.sourceforge.net";
}

/*!
	Returns the user agent of Quazaa at run-time as a string.
	This is used by most of the network protocols in user agent pakets.
 */
QString QuazaaGlobals::USER_AGENT_STRING()
{
	static const QString sUserAgent = APPLICATION_NAME() + "/"
									  + APPLICATION_VERSION().replace( ",", "." );
	return sUserAgent;
}

/*!
	Returns the vendor code of Quazaa at run-time as a string.
	This is used by most of the network protocols in vendor code pakets.
 */
QString QuazaaGlobals::VENDOR_CODE()
{
#ifdef QT_DEBUG
	return "QAZB";
#else
	return "QAZA";
#endif
}

/*!
	Returns the media open filter used by file open dialogs when opening media files.
 */
QString QuazaaGlobals::MEDIA_OPEN_FILTER()
{
	return QObject::tr( "All Media Files " )
			+ "(*.3g2 *.3gp *.3gp2 *.3gpp *.d2v *.divx *.amr *.amv *.asf *.avi *.bik *.drc *.dsa "
			+ "*.dsm *.dss *.dsv *.evo *.flc *.fli *.flic *.flv *.hdmov *.ifo *.ivf *.m1v *.m2p "
			+ "*.m2t *.m2ts *.m2v *.m4v *.mkv *.mov *.mp2v *.mp4 *.mp4v *.mpe *.mpeg *.mpg *.mpv2 "
			+ "*.mpv4 *.mts *.ogm *.ogv *.pss *.pva *.ram *.ratdvd *.rm *.rmm *.roq *.rp *.rmvb "
			+ "*.rpm *.rt *.smi *.smil *.smk *.swf *.tp *.tpr *.ts *.tta *.vob *.vp6 *.wm *.wmp "
			+ "*.wmv *.aac *.ac3 *.aif *.aifc *.aiff *.alac *.au *.cda *.dts *.flac *.mid *.midi "
			+ "*.m1a *.m2a *.m4a *.m4b *.mka *.mpa *.mpc *.mp2 *.mp3 *.oga *.ogg *.ra *.rmi *.snd "
			+ "*.wav *.wma);;"
			+ QObject::tr( "All Files " ) + "(*.*);;" + QObject::tr( "Video Files " )
			+ "(*.3g2 *.3gp *.3gp2 *.3gpp *.d2v *.divx *.amr *.amv *.asf *.avi *.bik *.drc *.dsa "
			+ "*.dsm *.dss *.dsv *.evo *.flc *.fli *.flic *.flv *.hdmov *.ifo *.ivf *.m1v *.m2p "
			+ "*.m2t *.m2ts *.m2v *.m4v *.mkv *.mov *.mp2v *.mp4 *.mp4v *.mpe *.mpeg *.mpg *.mpv2 "
			+ "*.mpv4 *.mts *.ogm *.ogv *.pss *.pva *.ram *.ratdvd *.rm *.rmm *.roq *.rp *.rmvb "
			+ "*.rpm *.rt *.smi *.smil *.smk *.swf *.tp *.tpr *.ts *.tta *.vob *.vp6 *.wm *.wmp "
			+ "*.wmv);;"
			+ QObject::tr( "Audio Files" )
			+ " (*.aac *.ac3 *.aif *.aifc *.aiff *.alac *.au *.cda *.dts *.flac *.mid *.midi *.m1a "
			+ "*.m2a *.m4a *.m4b *.mka *.mpa *.mpc *.mp2 *.mp3 *.oga *.ogg *.ra *.rmi *.snd *.wav "
			+ "*.wma)";
}

QString QuazaaGlobals::SETTINGS_PATH()
{
	QDir path;
	static const QString sPath = QDir::toNativeSeparators( HOME_PATH() + ".quazaa/" );
	path.mkpath( sPath );
	return sPath;
}

/*!
	Returns the path to store Quazaa data files.
*/
QString QuazaaGlobals::DATA_PATH()
{
	QDir path;
	static const QString sPath = QDir::toNativeSeparators( SETTINGS_PATH() + "Data/" );
	path.mkpath( sPath );
	return sPath;
}

QString QuazaaGlobals::STORAGE_PATH()
{
	QDir path;
	static const QString sPath = QDir::toNativeSeparators( HOME_PATH() + "Quazaa/" );
	path.mkpath( sPath );
	return sPath;
}

QString QuazaaGlobals::INI_FILE()
{
	static const QString sPath = SETTINGS_PATH() + "quazaa.ini";
	return sPath;
}

QString QuazaaGlobals::HOME_PATH()
{
	static const QString sPath = QStandardPaths::writableLocation( QStandardPaths::HomeLocation )
								 + "/";
	return sPath;
}
