/*
** commonfunctions.cpp
**
** Copyright © Quazaa Development Team, 2009-2014.
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

#include <QDir>

#include <QRegularExpression>
#include <QDesktopServices>
#include <QUrl>
#include <QtGlobal>

#include "types.h"
#include "commonfunctions.h"

#include "debug_new.h"

void common::folderOpen( const QString& sPath )
{
	QDir completePath( sPath );

	if ( !completePath.exists() )
	{
		completePath.mkpath( sPath );
	}
	QDesktopServices::openUrl( QUrl::fromLocalFile( sPath ) );
}

QString common::vendorCodeToName( const QString& vendorCode )
{
	if ( vendorCode == "RAZA" )
	{
		return "Shareaza";
	}
	if ( vendorCode == "RAZB" )
	{
		return "Shareaza Beta";
	}
	if ( vendorCode == "QAZA" )
	{
		return "Quazaa";
	}
	if ( vendorCode == "QAZB" )
	{
		return "Quazaa Beta";
	}
	if ( vendorCode == "SHLN" )
	{
		return "Sharelin";
	}
	if ( vendorCode == "RZCB" )
	{
		return "Shareaza Plus";
	}
	if ( vendorCode == "FSCP" )
	{
		return "FileScope";
	}
	if ( vendorCode == "AGIO" )
	{
		return "Adiago";
	}
	if ( vendorCode == "GNUC" )
	{
		return "Gnucleus";
	}
	if ( vendorCode == "MLDK" )
	{
		return "MLDonkey";
	}
	if ( vendorCode == "MMMM" || vendorCode == "MRPH" )
	{
		return "Morpheus";
	}
	if ( vendorCode == "PEER" )
	{
		return "PeerProject";
	}
	else
	{
		return vendorCode;
	}
}

QString common::fixFileName(const QString& sName )
{
	QString sRet = sName;
	// \ * / ? % : | " > <
	sRet.replace( QRegularExpression( "[\\\\\\*\\/\\?\\%\\:\\|\\\"\\>\\<\\x0000]" ), "_" );
	return sRet.left( 255 );
}

QString common::getIncompleteFileName( const HashSet& vHashes )
{
	const Hash* const pHash = vHashes.mostImportant();

	if ( pHash )
	{
		return pHash->getFamilyName() + "_" + pHash->toString();
	}
	else
	{
		return QString();
	}
}

QString common::formatBytes( quint64 nBytesPerSec )
{
	const char* szUnit[5] = { "B", "KiB", "MiB", "GiB", "TiB" };

	double nBPS = nBytesPerSec;

	int nStep = 0;
	while ( nBPS > 1024 )
	{
		nBPS /= 1024;
		++nStep;
		if ( nStep == 4 )
		{
			break;
		}
	}

	if ( nStep )
	{
		return QString().sprintf( "%1.2f %s", nBPS, szUnit[nStep] );
	}
	else
	{
		return QString().sprintf( "%1.0f %s", nBPS, szUnit[nStep] );
	}
}

QString common::writeSizeInWholeBytes( quint64 nBytes )
{
	if ( !nBytes )
	{
		return QString( "0 B" );
	}

	const char* sz1000[5] = { "B", "KB", "MB", "GB", "TB" };
	const char* sz1024[5] = { "B", "KiB", "MiB", "GiB", "TiB" };

	quint64 n1 = nBytes, n2 = nBytes;
	quint8 n1count = 0, n2count = 0;

	while ( n1 % 1000 == 0 )
	{
		n1 /= 1000;
		++n1count;
	}

	while ( n2 % 1024 == 0 )
	{
		n2 /= 1024;
		++n2count;
	}

	if ( n1count > n2count )
	{
		return QString::number( n1 ) + " " + sz1000[n1count];
	}
	else
	{
		return QString::number( n2 ) + " " + sz1024[n2count];
	}
}

quint64 common::readSizeInBytes( QString sInput, bool& bOK )
{
	sInput.remove( QRegularExpression( "\\s" ) );

	// \A and \z force it to match the entire string, not only the line like ^ and $ do.
	QRegularExpression oMatch( "\\A[0-9]+(|B|KB|KiB|MB|MiB|GB|GiB|TB|TiB)\\z",
							   QRegularExpression::CaseInsensitiveOption );
	bOK = oMatch.match( sInput, 0, QRegularExpression::NormalMatch ).hasMatch();

	if ( !bOK )
	{
		return 0;
	}

	int nSuffixPos = QRegularExpression( "(B|KB|KiB|MB|MiB|GB|GiB|TB|TiB)\\z",
										  QRegularExpression::CaseInsensitiveOption
										  ).match( sInput ).capturedStart();

	QString sNumber = sInput.left( nSuffixPos );
	QString sSuffix = sInput.right( sInput.length() - sNumber.size() );

#ifdef _DEBUG
	Q_ASSERT( QRegularExpression( "\\A(|B|KB|KiB|MB|MiB|GB|GiB|TB|TiB)\\z",
								  QRegularExpression::CaseInsensitiveOption
								  ).match( sSuffix, 0, QRegularExpression::NormalMatch ).hasMatch()
			);

	Q_ASSERT( QRegularExpression( "\\A[0-9]+\\z", QRegularExpression::CaseInsensitiveOption
								  ).match( sNumber, 0, QRegularExpression::NormalMatch ).hasMatch()
			);
#endif //_DEBUG

	quint64 nMultiplier = 1;

	if ( sSuffix.isEmpty() )
	{
		nMultiplier = 1;
	}
	else if ( !sSuffix.compare( "B", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1;
	}
	else if ( !sSuffix.compare( "KB", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1000;
	}
	else if ( !sSuffix.compare( "KiB", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1024;
	}
	else if ( !sSuffix.compare( "MB", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1000 * 1000;
	}
	else if ( !sSuffix.compare( "MiB", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1024 * 1024;
	}
	else if ( !sSuffix.compare( "TB", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1000 * 1000 * 1000;
	}
	else if ( !sSuffix.compare( "TiB", Qt::CaseInsensitive ) )
	{
		nMultiplier = 1024 * 1024 * 1024;
	}
	else
	{
		bOK = false;
	}

	if ( !bOK )
	{
		return 0;
	}

	quint64 nValue = sNumber.toLongLong( &bOK );

	return bOK ? nValue * nMultiplier : 0;
}

quint32 common::securedSaveFile( const QString& sPath, const QString& sFileName,
								 Component oComponent, const void* const pManager,
								 quint32 ( *writeData )( const void* const, QFile& ) )
{
	QFileInfo oFileInfo( sPath + sFileName );

	const QString sAbsolutePath         = oFileInfo.absolutePath() + "/";

	QDir path = QDir( sAbsolutePath );
	if ( !path.exists() )
	{
		path.mkpath( sAbsolutePath );
	}

	const QString sCompletePathAndName  =
		QDir::toNativeSeparators( sAbsolutePath + oFileInfo.fileName() );
	const QString sBackupPathAndName    =
		QDir::toNativeSeparators( sAbsolutePath + oFileInfo.completeBaseName() + "_backup."
								  + oFileInfo.suffix() );
	const QString sTemporaryPathAndName =
		QDir::toNativeSeparators( sAbsolutePath + oFileInfo.completeBaseName() + "_tmp."
								  + oFileInfo.suffix() );

	systemLog.postLog( LogSeverity::Debug, oComponent,
					   QObject::tr( "Saving to file: " ) + sCompletePathAndName );

	if ( QFile::exists( sTemporaryPathAndName ) && !QFile::remove( sTemporaryPathAndName ) )
	{
		systemLog.postLog( LogSeverity::Error, oComponent,
						   QObject::tr( "Error: Could not free space required for temporary file:" )
						   + " " + sTemporaryPathAndName );
		return 0;
	}

	QFile oFile( sTemporaryPathAndName );

	if ( !oFile.open( QIODevice::WriteOnly ) )
	{
		systemLog.postLog( LogSeverity::Error, oComponent,
						   QObject::tr( "Error: Could open temporary file for write: " )
						   + sTemporaryPathAndName );
		return 0;
	}

	quint32 nPieces;
	try
	{
		nPieces = writeData( pManager, oFile );
	}
	catch ( ... )
	{
		systemLog.postLog( LogSeverity::Error, oComponent,
						   QObject::tr( "Error while writing to file: " ) + sTemporaryPathAndName );
		return 0;
	}

	oFile.close();

	if ( QFile::exists( sCompletePathAndName ) && !QFile::remove( sCompletePathAndName ) )
	{
		systemLog.postLog( LogSeverity::Error, oComponent,
						   QObject::tr( "Error: Could not remove old data file: " )
						   + sCompletePathAndName );
		return 0;
	}

	if ( !QFile::rename( sTemporaryPathAndName, sCompletePathAndName ) )
	{
		systemLog.postLog( LogSeverity::Error, oComponent,
						   QObject::tr( "Error: Could not rename data file: " )
						   + sCompletePathAndName );
		return 0;
	}

	if ( QFile::exists( sBackupPathAndName ) && !QFile::remove( sBackupPathAndName ) )
	{
		systemLog.postLog( LogSeverity::Warning, oComponent,
						   QObject::tr( "Warning: Could not remove old backup file: " )
						   + sBackupPathAndName );
	}

	if ( !QFile::copy( sCompletePathAndName, sBackupPathAndName ) )
	{
		systemLog.postLog( LogSeverity::Warning, oComponent,
						   QObject::tr( "Warning: Could not create create new backup file: " )
						   + sBackupPathAndName );
	}

	return nPieces;
}

quint16 common::getRandomUnusedPort( bool bClear )
{
	/**
	 * This list contains are all ports that are known to be used by other applications.
	 * Source: http://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers (2012-08-31)
	 * The range 49152-65535 - above the registered ports - contains dynamic or private ports
	 * that cannot be registered with IANA.[106] This range is used for custom or temporary
	 * purposes and for automatic allocation of ephemeral ports.
	 */
	static QSet<quint16> oUsedPorts;

	/**
	 * This can be used to free the memory occupied by oUsedPorts if the functionality is not
	 * needed any longer.
	 */
	if ( bClear )
	{
		oUsedPorts.clear();		// Remove all elements.
		oUsedPorts.squeeze();	// Set internal capacity to 0.
		return 0;
	}

	if ( oUsedPorts.isEmpty() )
	{
		// avoid reallocation while building set
		oUsedPorts.reserve( 1826 );

		// fill set with ports
		oUsedPorts << 1024	//  TCP	UDP	Reserved
				   << 1025	//  TCP		NFS or IIS or Teradata
				   << 1026	//  TCP		Often used by Microsoft DCOM services
				   << 1029	//  TCP		Often used by Microsoft DCOM services
				   << 1058	//  TCP	UDP	nim IBM AIX Network Installation Manager (NIM)
				   << 1059	//  TCP	UDP	nimreg IBM AIX Network Installation Manager (NIM)
				   << 1080	//  TCP		SOCKS proxy
				   << 1085	//  TCP	UDP	WebObjects
				   << 1098	//  TCP	UDP	rmiactivation RMI Activation
				   << 1099	//  TCP	UDP	rmiregistry RMI Registry
				   << 1109	//  	UDP	Reserved[1]
				   << 1109	//  TCP		Reserved[1]
				   << 1109	//  TCP		Kerberos Post Office Protocol (KPOP)
				   << 1110	//  	UDP	EasyBits School network discovery protocol (for Intel's CMPC platform)
				   << 1140	//  TCP	UDP	AutoNOC protocol
				   << 1167	//  	UDP	phone conference calling
				   << 1169	//  TCP	UDP	Tripwire
				   << 1176	//  TCP		Perceptive Automation Indigo Home automation server
				   << 1182	//  TCP	UDP	AcceleNet Intelligent Transfer Protocol
				   << 1194	//  TCP	UDP	OpenVPN
				   << 1198	//  TCP	UDP	The cajo project Free dynamic transparent distributed computing in Java
				   << 1200	//  TCP		scol protocol used by SCOL 3D virtual worlds server to answer world name resolution client request[41]
				   << 1200	//  	UDP	scol protocol used by SCOL 3D virtual worlds server to answer world name resolution client request
				   << 1200	//  	UDP	Steam Friends Applet
				   << 1214	//  TCP		Kazaa
				   << 1217	//  TCP		Uvora Online
				   << 1220	//  TCP		QuickTime Streaming Server administration
				   << 1223	//  TCP	UDP	TGP TrulyGlobal Protocol also known as "The Gur Protocol" (named for Gur Kimchi of TrulyGlobal)
				   << 1234	//  	UDP	VLC media player default port for UDP/RTP stream
				   << 1236	//  TCP		Symantec BindView Control UNIX Default port for TCP management server connections
				   << 1241	//  TCP	UDP	Nessus Security Scanner
				   << 1270	//  TCP	UDP	Microsoft System Center Operations Manager (SCOM) (formerly Microsoft Operations Manager (MOM)) agent
				   << 1293	//  TCP	UDP	IPSec (Internet Protocol Security)
				   << 1301	//  TCP		Palmer Performance OBDNet
				   << 1309	//  TCP		Altera Quartus jtagd
				   << 1311	//  TCP		Dell OpenManage HTTPS
				   << 1313	//  TCP		Xbiim (Canvii server)
				   << 1314	//  TCP		Festival Speech Synthesis System
				   << 1319	//  TCP		AMX ICSP
				   << 1319	//  	UDP	AMX ICSP
				   << 1337	//  	UDP	Men and Mice DNS
				   << 1337	//  TCP		Men and Mice DNS
				   << 1337	//  TCP		PowerFolder P2P Encrypted File Synchronization Program
				   << 1337	//  TCP		WASTE Encrypted File Sharing Program
				   << 1344	//  TCP		Internet Content Adaptation Protocol
				   << 1352	//  TCP		IBM Lotus Notes/Domino[42] (RPC) protocol
				   << 1387	//  TCP	UDP	cadsi-lm LMS International (formerly Computer Aided Design Software Inc. (CADSI)) LM
				   << 1414	//  TCP		IBM WebSphere MQ (formerly known as MQSeries)
				   << 1417	//  TCP	UDP	Timbuktu Service 1 Port
				   << 1418	//  TCP	UDP	Timbuktu Service 2 Port
				   << 1419	//  TCP	UDP	Timbuktu Service 3 Port
				   << 1420	//  TCP	UDP	Timbuktu Service 4 Port
				   << 1431	//  TCP		Reverse Gossip Transport Protocol (RGTP) used to access a General-purpose Reverse-Ordered Gossip Gathering System (GROGGS) bulletin board such as that implemented on the Cambridge University's Phoenix system
				   << 1433	//  TCP		MSSQL (Microsoft SQL Server database management system) Server
				   << 1434	//  TCP	UDP	MSSQL (Microsoft SQL Server database management system) Monitor
				   << 1453	//  TCP	UDP	National Port of Turkey
				   << 1470	//  TCP		Solarwinds Kiwi Log Server
				   << 1494	//  TCP		Citrix XenApp Independent Computing Architecture (ICA) thin client protocol
				   << 1500	//  TCP		NetGuard GuardianPro firewall (NT4-based) Remote Management
				   << 1501	//  	UDP	NetGuard GuardianPro firewall (NT4-based) Authentication Client
				   << 1503	//  TCP	UDP	Windows Live Messenger (Whiteboard and Application Sharing)
				   << 1512	//  TCP	UDP	Microsoft Windows Internet Name Service (WINS)
				   << 1513	//  TCP	UDP	Garena Garena Gaming Client
				   << 1521	//  TCP		nCube License Manager
				   << 1521	//  TCP		Oracle database default listener in future releases official port 2483
				   << 1524	//  TCP	UDP	ingreslock ingres
				   << 1526	//  TCP		Oracle database common alternative for listener
				   << 1527	//  TCP		Apache Derby Network Server default port
				   << 1533	//  TCP		IBM Sametime IM—Virtual Places Chat Microsoft SQL Server
				   << 1534	//  	UDP	Eclipse Target Communication Framework (TCF) agent discovery[43]
				   << 1547	//  TCP	UDP	Laplink
				   << 1550	//  TCP	UDP	3m-image-lm Image Storage license manager 3M Company
				   << 1550	//  		Gadu-Gadu (direct client-to-client)
				   << 1581	//  	UDP	MIL STD 2045-47001 VMF
				   << 1589	//  	UDP	Cisco VQP (VLAN Query Protocol) / VMPS
				   << 1590	//  TCP		GE Smallworld Datastore Server (SWMFS/Smallworld Master Filesystem)
				   << 1627	//  		iSketch
				   << 1645	//  TCP	UDP	radius auth RADIUS authentication protocol (default for Cisco and Juniper Networks RADIUS servers but see port 1812 below)
				   << 1646	//  TCP	UDP	radius acct RADIUS authentication protocol (default for Cisco and Juniper Networks RADIUS servers but see port 1813 below)
				   << 1666	//  TCP		Perforce
				   << 1677	//  TCP	UDP	Novell GroupWise clients in client/server access mode
				   << 1688	//  TCP		Microsoft Key Management Service for KMS Windows Activation
				   << 1700	//  	UDP	Cisco RADIUS Change of Authorization for TrustSec
				   << 1701	//  	UDP	Layer 2 Forwarding Protocol (L2F) & Layer 2 Tunneling Protocol (L2TP)
				   << 1707	//  TCP	UDP	Windward Studios
				   << 1707	//  TCP	Romtoc Packet Protocol (L2F) & Layer 2 Tunneling Protocol (L2TP)
				   << 1716	//  TCP		America's Army Massively multiplayer online game (MMO)
				   << 1719	//  	UDP	H.323 Registration and alternate communication
				   << 1720	//  TCP		H.323 Call signalling
				   << 1723	//  TCP	UDP	Microsoft Point-to-Point Tunneling Protocol (PPTP)
				   << 1725	//  	UDP	Valve Steam Client
				   << 1755	//  TCP	UDP	Microsoft Media Services (MMS ms-streaming)
				   << 1761	//  	UDP	cft-0
				   << 1761	//  TCP		cft-0
				   << 1761	//  TCP		Novell Zenworks Remote Control utility
				   << 1762	//  Start range: cft-1 to cft-7
				   << 1763
				   << 1764
				   << 1765
				   << 1766
				   << 1767
				   << 1768	//  TCP	UDP	cft-1 to cft-7
				   << 1792	//  TCP	UDP	Moby
				   << 1801	//  TCP	UDP	Microsoft Message Queuing
				   << 1812	//  TCP	UDP	radius RADIUS authentication protocol
				   << 1813	//  TCP	UDP	radacct RADIUS accounting protocol
				   << 1863	//  TCP		MSNP (Microsoft Notification Protocol) used by the .NET Messenger Service and a number of Instant Messaging clients
				   << 1883	//  TCP	UDP	MQ Telemetry Transport (MQTT) formerly known as MQIsdp (MQSeries SCADA protocol)
				   << 1886	//  TCP		Leonardo over IP Pro2col Ltd
				   << 1900	//  	UDP	Microsoft SSDP Enables discovery of UPnP devices
				   << 1920	//  TCP		IBM Tivoli monitoring console
				   << 1935	//  TCP		Adobe Systems Macromedia Flash Real Time Messaging Protocol (RTMP) "plain" protocol
				   << 1947	//  TCP	UDP	SentinelSRM (hasplm) Aladdin HASP License Manager
				   << 1967	//  	UDP	Cisco IOS IP Service Level Agreements (IP SLAs) Control Protocol
				   << 1970	//  TCP	UDP	Netop Business Solutions Netop Remote Control
				   << 1971	//  TCP	UDP	Netop Business Solutions Netop School
				   << 1972	//  TCP	UDP	InterSystems Caché
				   << 1975	//  	UDP	Cisco TCO (Documentation)
				   << 1976	//  	UDP	Cisco TCO (Documentation)
				   << 1977	//  	UDP	Cisco TCO (Documentation)
				   << 1984	//  TCP		Big Brother and related Xymon (formerly Hobbit) System and Network Monitor
				   << 1985	//  	UDP	Cisco HSRP
				   << 1994	//  TCP	UDP	Cisco STUN-SDLC (Serial Tunneling—Synchronous Data Link Control) protocol
				   << 1997	//  TCP		Chizmo Networks Transfer Tool
				   << 1998	//  TCP	UDP	Cisco X.25 over TCP (XOT) service
				   << 2000	//  TCP	UDP	Cisco SCCP (Skinny)
				   << 2001	//  	UDP	CAPTAN Test Stand System
				   << 2002	//  TCP		Secure Access Control Server (ACS) for Windows
				   << 2010	//  TCP		Artemis: Spaceship Bridge Simulator default port
				   << 2030	//  		Oracle services for Microsoft Transaction Server
				   << 2031	//  TCP	UDP	mobrien-chat(http://chat.mobrien.com:2031)
				   << 2041	//  TCP		Mail.Ru Agent communication protocol
				   << 2049	//  TCP	UDP	NFS (Network File System)
				   << 2049	//  	UDP	shilp
				   << 2053	//  TCP		knetd Kerberos de-multiplexor
				   << 2056	//  	UDP	Civilization 4 multiplayer
				   << 2074	//  TCP	UDP	Vertel VMF SA (i.e. App.. SpeakFreely)
				   << 2080	//  TCP	UDP	Autodesk NLM (FLEXlm)
				   << 2082	//  TCP		Infowave Mobility Server
				   << 2082	//  TCP		CPanel default
				   << 2083	//  TCP		Secure Radius Service (radsec)
				   << 2083	//  TCP		CPanel default SSL
				   << 2086	//  TCP		GNUnet
				   << 2086	//  TCP		WebHost Manager default
				   << 2087	//  TCP		WebHost Manager default SSL
				   << 2095	//  TCP		CPanel default Web mail
				   << 2096	//  TCP		CPanel default SSL Web mail
				   << 2102	//  TCP	UDP	zephyr-srv Project Athena Zephyr Notification Service server
				   << 2103	//  TCP	UDP	zephyr-clt Project Athena Zephyr Notification Service serv-hm connection
				   << 2104	//  TCP	UDP	zephyr-hm Project Athena Zephyr Notification Service hostmanager
				   << 2105	//  TCP	UDP	IBM MiniPay
				   << 2105	//  TCP	UDP	eklogin Kerberos encrypted remote login (rlogin)
				   << 2105	//  TCP	UDP	zephyr-hm-srv Project Athena Zephyr Notification Service hm-serv connection (should use port 2102)
				   << 2121	//  TCP		FTP proxy
				   << 2144	//  TCP		Iron Mountain LiveVault Agent
				   << 2145	//  TCP		Iron Mountain LiveVault Agent
				   << 2156	//  	UDP	Talari Reliable Protocol
				   << 2160	//  TCP		APC Agent
				   << 2161	//  TCP		APC Agent
				   << 2181	//  TCP	UDP	EForward-document transport system
				   << 2190	//  	UDP	TiVoConnect Beacon
				   << 2200	//  	UDP	Tuxanci game server[44]
				   << 2210	//  	UDP	NOAAPORT Broadcast Network
				   << 2210	//  TCP		NOAAPORT Broadcast Network
				   << 2210	//  TCP		MikroTik Remote management for "The Dude"
				   << 2211	//  	UDP	EMWIN
				   << 2211	//  TCP		EMWIN
				   << 2211	//  TCP		MikroTik Secure management for "The Dude"
				   << 2212	//  	UDP	LeeCO POS Server Service
				   << 2212	//  TCP		LeeCO POS Server Service
				   << 2212	//  TCP		Port-A-Pour Remote WinBatch
				   << 2219	//  TCP	UDP	NetIQ NCAP Protocol
				   << 2220	//  TCP	UDP	NetIQ End2End
				   << 2221	//  TCP		ESET Anti-virus updates
				   << 2222	//  TCP		DirectAdmin default & ESET Remote Administration
				   << 2223	//  	UDP	Microsoft Office OS X antipiracy network monitor
				   << 2261	//  TCP	UDP	CoMotion Master
				   << 2262	//  TCP	UDP	CoMotion Backup
				   << 2301	//  TCP		HP System Management Redirect to port 2381
				   << 2302	//  	UDP	ArmA multiplayer (default for game)
				   << 2302	//  	UDP	Halo: Combat Evolved multiplayer
				   << 2303	//  	UDP	ArmA multiplayer (default for server reporting) (default port for game +1)
				   << 2305	//  	UDP	ArmA multiplayer (default for VoN) (default port for game +3)
				   << 2323	//  TCP		Philips TVs based on jointSPACE [45]
				   << 2369	//  TCP		Default for BMC Software Control-M/Server—Configuration Agent though often changed during installation
				   << 2370	//  TCP		Default for BMC Software Control-M/Server—to allow the Control-M/Enterprise Manager to connect to the Control-M/Server though often changed during installation
				   << 2379	//  TCP		KGS Go Server
				   << 2381	//  TCP		HP Insight Manager default for Web server
				   << 2399	//  TCP		FileMaker Data Access Layer (ODBC/JDBC)
				   << 2401	//  TCP		CVS version control system
				   << 2404	//  TCP		IEC 60870-5 -104 used to send electric power telecontrol messages between two systems via directly connected data circuits
				   << 2420	//  	UDP	Westell Remote Access
				   << 2424	//  TCP		OrientDB database listening for Binary client connections
				   << 2427	//  	UDP	Cisco MGCP
				   << 2447	//  TCP	UDP	ovwdb—OpenView Network Node Manager (NNM) daemon
				   << 2463	//  TCP	UDP	LSI RAID Management formerly Symbios Logic
				   << 2480	//  TCP		OrientDB database listening for HTTP client connections
				   << 2483	//  TCP	UDP	Oracle database listening for unsecure client connections to the listener replaces port 1521
				   << 2484	//  TCP	UDP	Oracle database listening for SSL client connections to the listener
				   << 2500	//  TCP		THEÒSMESSENGER listening for TheòsMessenger client connections
				   << 2501	//  TCP		TheosNet-Admin listening for TheòsMessenger client connections
				   << 2518	//  TCP	UDP	Willy
				   << 2525	//  TCP		SMTP alternate
				   << 2535	//  TCP		MADCAP - Multicast Address Dynamic Client Allocation Protocol
				   << 2546	//  TCP	UDP	EVault data protection services
				   << 2593	//  TCP	UDP	RunUO—Ultima Online server
				   << 2598	//  TCP		new ICA (Citrix) —when Session Reliability is enabled TCP port 2598 replaces port 1494
				   << 2599	//  TCP		SonicWALL anti-spam traffic between Remote Analyzer (RA) and Control Center (CC)
				   << 2610	//  TCP		Dark Ages (video game)
				   << 2612	//  TCP	UDP	QPasa from MQSoftware
				   << 2636	//  TCP		Solve Service
				   << 2638	//  TCP		Sybase database listener
				   << 2641	//  TCP	UDP	HDL Server from CNRI
				   << 2642	//  TCP	UDP	Tragic
				   << 2698	//  TCP	UDP	Citel / MCK IVPIP
				   << 2700	//  Start range: KnowShowGo P2P
				   << 1701
				   << 1702
				   << 1703
				   << 1704
				   << 1705
				   << 1706
				   << 1707
				   << 1708
				   << 1709
				   << 1710
				   << 1711
				   << 1712
				   << 1713
				   << 1714
				   << 1715
				   << 1716
				   << 1717
				   << 1718
				   << 1719
				   << 1720
				   << 1721
				   << 1722
				   << 1723
				   << 1724
				   << 1725
				   << 1726
				   << 1727
				   << 1728
				   << 1729
				   << 1730
				   << 1731
				   << 1732
				   << 1733
				   << 1734
				   << 1735
				   << 1736
				   << 1737
				   << 1738
				   << 1739
				   << 1740
				   << 1741
				   << 1742
				   << 1743
				   << 1744
				   << 1745
				   << 1746
				   << 1747
				   << 1748
				   << 1749
				   << 1750
				   << 1751
				   << 1752
				   << 1753
				   << 1754
				   << 1755
				   << 1756
				   << 1757
				   << 1758
				   << 1759
				   << 1760
				   << 1761
				   << 1762
				   << 1763
				   << 1764
				   << 1765
				   << 1766
				   << 1767
				   << 1768
				   << 1769
				   << 1770
				   << 1771
				   << 1772
				   << 1773
				   << 1774
				   << 1775
				   << 1776
				   << 1777
				   << 1778
				   << 1779
				   << 1780
				   << 1781
				   << 1782
				   << 1783
				   << 1784
				   << 1785
				   << 1786
				   << 1787
				   << 1788
				   << 1789
				   << 1790
				   << 1791
				   << 1792
				   << 1793
				   << 1794
				   << 1795
				   << 1796
				   << 1797
				   << 1798
				   << 1799
				   << 2800	//  TCP		KnowShowGo P2P
				   << 2710	//  TCP		XBT Tracker
				   << 2710	//  	UDP	XBT Tracker experimental UDP tracker extension
				   << 2710	//  TCP		Knuddels.de
				   << 2735	//  TCP	UDP	NetIQ Monitor Console
				   << 2809	//  TCP		corbaloc:iiop URL per the CORBA 3.0.3 specification
				   << 2809	//  TCP		IBM WebSphere Application Server (WAS) Bootstrap/rmi default
				   << 2809	//  	UDP	corbaloc:iiop URL per the CORBA 3.0.3 specification.
				   << 2827	//  TCP		I2P Basic Open Bridge API
				   << 2868	//  TCP	UDP	Norman Proprietary Event Protocol NPEP
				   << 2944	//  	UDP	Megaco text H.248
				   << 2945	//  	UDP	Megaco binary (ASN.1) H.248
				   << 2947	//  TCP		gpsd GPS daemon
				   << 2948	//  TCP	UDP	WAP-push Multimedia Messaging Service (MMS)
				   << 2949	//  TCP	UDP	WAP-pushsecure Multimedia Messaging Service (MMS)
				   << 2967	//  TCP		Symantec AntiVirus Corporate Edition
				   << 3000	//  TCP		Miralix License server
				   << 3000	//  TCP		Cloud9 Integrated Development Environment server
				   << 3000	//  	UDP	Distributed Interactive Simulation (DIS) modifiable default
				   << 3000	//  TCP		Ruby on Rails development default[46]
				   << 3001	//  TCP		Miralix Phone Monitor
				   << 3001	//  TCP		Opsware server (Satellite)
				   << 3002	//  TCP		Miralix CSTA
				   << 3003	//  TCP		Miralix GreenBox API
				   << 3004	//  TCP		Miralix InfoLink
				   << 3005	//  TCP		Miralix TimeOut
				   << 3006	//  TCP		Miralix SMS Client Connector
				   << 3007	//  TCP		Miralix OM Server
				   << 3008	//  TCP		Miralix Proxy
				   << 3017	//  TCP		Miralix IVR and Voicemail
				   << 3025	//  TCP		netpd.org
				   << 3030	//  TCP	UDP	NetPanzer
				   << 3050	//  TCP	UDP	gds_db (Interbase/Firebird)
				   << 3051	//  TCP	UDP	Galaxy Server (Gateway Ticketing Systems)
				   << 3052	//  TCP	UDP	APC PowerChute Network [1]
				   << 3074	//  TCP	UDP	Xbox LIVE and/or Games for Windows - LIVE
				   << 3100	//  TCP		SMAUSA OpCon Scheduler as the default listen port
				   << 3101	//  TCP		BlackBerry Enterprise Server communication to cloud
				   << 3119	//  TCP		D2000 Entis/Actis Application server
				   << 3128	//  TCP		Web caches and the default for the Squid (software)
				   << 3128	//  TCP		Tatsoft default client connection
				   << 3162	//  TCP	UDP	SFLM (Standard Floating License Manager)
				   << 3225	//  TCP	UDP	FCIP (Fiber Channel over Internet Protocol)
				   << 3233	//  TCP	UDP	WhiskerControl research control protocol
				   << 3235	//  TCP	UDP	Galaxy Network Service (Gateway Ticketing Systems)
				   << 3260	//  TCP		iSCSI target
				   << 3268	//  TCP	UDP	msft-gc Microsoft Global Catalog (LDAP service which contains data from Active Directory forests)
				   << 3269	//  TCP	UDP	msft-gc-ssl Microsoft Global Catalog over SSL (similar to port 3268 LDAP over SSL)
				   << 3283	//  TCP		Apple Remote Desktop reporting (officially Net Assistant referring to an earlier product)
				   << 3299	//  TCP		SAP-Router (routing application proxy for SAP R/3)
				   << 3300	//  TCP	UDP	Debate Gopher backend database system
				   << 3305	//  TCP	UDP	odette-ftp Odette File Transfer Protocol (OFTP)
				   << 3306	//  TCP	UDP	MySQL database system
				   << 3313	//  TCP		Verisys file integrity monitoring software
				   << 3333	//  TCP		Network Caller ID server
				   << 3333	//  TCP		CruiseControl.rb[47]
				   << 3386	//  TCP	UDP	GTP' 3GPP GSM/UMTS CDR logging protocol
				   << 3389	//  TCP	UDP	Microsoft Terminal Server (RDP) officially registered as Windows Based Terminal (WBT) - Link
				   << 3396	//  TCP	UDP	Novell NDPS Printer Agent
				   << 3412	//  TCP	UDP	xmlBlaster
				   << 3455	//  TCP	UDP	[RSVP] Reservation Protocol
				   << 3423	//  TCP		Xware xTrm Communication Protocol
				   << 3424	//  TCP		Xware xTrm Communication Protocol over SSL
				   << 3478	//  TCP	UDP	STUN a protocol for NAT traversal[48]
				   << 3478	//  TCP	UDP	TURN a protocol for NAT traversal[49]
				   << 3483	//  	UDP	Slim Devices discovery protocol
				   << 3483	//  TCP		Slim Devices SlimProto protocol
				   << 3516	//  TCP	UDP	Smartcard Port
				   << 3527	//  	UDP	Microsoft Message Queuing
				   << 3535	//  TCP		SMTP alternate[50]
				   << 3537	//  TCP	UDP	ni-visa-remote
				   << 3544	//  	UDP	Teredo tunneling
				   << 3605	//  	UDP	ComCam IO Port
				   << 3606	//  TCP	UDP	Splitlock Server
				   << 3632	//  TCP		distributed compiler
				   << 3689	//  TCP		Digital Audio Access Protocol (DAAP)—used by Apple’s iTunes and AirPort Express
				   << 3690	//  TCP	UDP	Subversion (SVN) version control system
				   << 3702	//  TCP	UDP	Web Services Dynamic Discovery (WS-Discovery) used by various components of Windows Vista
				   << 3723	//  TCP	UDP	Used by many Battle.net Blizzard games (Diablo II Warcraft II Warcraft III StarCraft)
				   << 3724	//  TCP		World of Warcraft Online gaming MMORPG
				   << 3724	//  TCP		Club Penguin Disney online game for kids
				   << 3724	//  	UDP	World of Warcraft Online gaming MMORPG
				   << 3784	//  TCP	UDP	VoIP program used by Ventrilo
				   << 3785	//  	UDP	VoIP program used by Ventrilo
				   << 3799	//  	UDP	RADIUS change of authorization
				   << 3800	//  TCP		Used by HGG programs
				   << 3825	//  TCP		Used by RedSeal Networks client/server connection
				   << 3826	//  TCP		Used by RedSeal Networks client/server connection
				   << 3835	//  TCP		Used by RedSeal Networks client/server connection
				   << 3880	//  TCP	UDP	IGRS
				   << 3868	//  TCP	SCTP	Diameter base protocol (RFC 3588)
				   << 3872	//  TCP		Oracle Management Remote Agent
				   << 3899	//  TCP		Remote Administrator
				   << 3900	//  TCP		udt_os IBM UniData UDT OS[51]
				   << 3945	//  TCP	UDP	EMCADS service a Giritech product used by G/On
				   << 3978	//  TCP	UDP	OpenTTD game (masterserver and content service)
				   << 3979	//  TCP	UDP	OpenTTD game
				   << 3999	//  TCP	UDP	Norman distributed scanning service
				   << 4000	//  TCP	UDP	Diablo II game
				   << 4001	//  TCP		Microsoft Ants game
				   << 4007	//  TCP		PrintBuzzer printer monitoring socket server
				   << 4018	//  TCP	UDP	protocol information and warnings
				   << 4035	//  TCP	//  TCP	IBM Rational Developer for System z Remote System Explorer Daemon
				   << 4045	//  TCP	UDP	Solaris lockd NFS lock daemon/manager
				   << 4069	//  	UDP	Minger Email Address Verification Protocol[52]
				   << 4089	//  TCP	UDP	OpenCORE Remote Control Service
				   << 4093	//  TCP	UDP	PxPlus Client server interface ProvideX
				   << 4096	//  TCP	UDP	Ascom Timeplex BRE (Bridge Relay Element)
				   << 4100	//  		WatchGuard authentication applet default
				   << 4111	//  TCP		Xgrid
				   << 4116	//  TCP	UDP	Smartcard-TLS
				   << 4125	//  TCP		Microsoft Remote Web Workplace administration
				   << 4172	//  TCP	UDP	Teradici PCoIP
				   << 4201	//  TCP		TinyMUD and various derivatives
				   << 4226	//  TCP	UDP	Aleph One (game)
				   << 4224	//  TCP		Cisco Audio Session Tunneling
				   << 4242	//  TCP		Reverse Battle Tetris
				   << 4321	//  TCP		Referral Whois (RWhois) Protocol[53]
				   << 4323	//  	UDP	Lincoln Electric's ArcLink/XT
				   << 4433	//  TCP		Axence nVision
				   << 4434	//  TCP		Axence nVision
				   << 4435	//  TCP		Axence nVision
				   << 4436	//  TCP		Axence nVision
				   << 4444	//  TCP		I2P HTTP/S proxy
				   << 4445	//  TCP		I2P HTTP/S proxy
				   << 4486	//  TCP	UDP	Integrated Client Message Service (ICMS)
				   << 4500	//  	UDP	IPSec NAT Traversal (RFC 3947)
				   << 4502	// Start range: Microsoft Silverlight[...]
				   << 4503
				   << 4504
				   << 4505
				   << 4506
				   << 4507
				   << 4508
				   << 4509
				   << 4510
				   << 4511
				   << 4512
				   << 4513
				   << 4514
				   << 4515
				   << 4516
				   << 4517
				   << 4518
				   << 4519
				   << 4520
				   << 4521
				   << 4522
				   << 4523
				   << 4524
				   << 4525
				   << 4526
				   << 4527
				   << 4528
				   << 4529
				   << 4530
				   << 4531
				   << 4532
				   << 4533
				   << 4534	//  TCP		Microsoft Silverlight connectable ports under non-elevated trust
				   << 4534	//  	UDP	Armagetron Advanced default server port
				   << 4567	//  TCP		Sinatra default server port in development mode (HTTP)
				   << 4569	//  	UDP	Inter-Asterisk eXchange (IAX2)
				   << 4610	// Start range: QualiSystems TestShell Suite Services
				   << 4611
				   << 4612
				   << 4613
				   << 4614
				   << 4615
				   << 4616
				   << 4617
				   << 4618
				   << 4619
				   << 4620
				   << 4621
				   << 4622
				   << 4623
				   << 4624
				   << 4625
				   << 4626
				   << 4627
				   << 4628
				   << 4629
				   << 4630
				   << 4631
				   << 4632
				   << 4633
				   << 4634
				   << 4635
				   << 4636
				   << 4637
				   << 4638
				   << 4639
				   << 4640	//  TCP		QualiSystems TestShell Suite Services
				   << 4662	//  	UDP	OrbitNet Message Service
				   << 4662	//  TCP		OrbitNet Message Service
				   << 4662	//  TCP		Default for older versions of eMule[54]
				   << 4664	//  TCP		Google Desktop Search
				   << 4672	//  	UDP	Default for older versions of eMule[54]
				   << 4711	//  TCP		eMule optional web interface[54]
				   << 4711	//  TCP		McAfee Web Gateway 7 - Default GUI Port HTTP
				   << 4712	//  TCP		McAfee Web Gateway 7 - Default GUI Port HTTPS
				   << 4713	//  TCP		PulseAudio sound server
				   << 4728	//  TCP		Computer Associates Desktop and Server Management (DMP)/Port Multiplexer [55]
				   << 4732	//  	UDP	Digital Airways' OHM server's commands to mobile devices (used mainly for binary SMS)
				   << 4747	//  TCP		Apprentice
				   << 4750	//  TCP		BladeLogic Agent
				   << 4840	//  TCP	UDP	OPC UA TCP Protocol for OPC Unified Architecture from OPC Foundation
				   << 4843	//  TCP	UDP	OPC UA TCP Protocol over TLS/SSL for OPC Unified Architecture from OPC Foundation
				   << 4847	//  TCP	UDP	Web Fresh Communication Quadrion Software & Odorless Entertainment
				   << 4894	//  TCP	UDP	LysKOM Protocol A
				   << 4899	//  TCP	UDP	Radmin remote administration tool
				   << 4949	//  TCP		Munin Resource Monitoring Tool
				   << 4950	//  TCP	UDP	Cylon Controls UC32 Communications Port
				   << 4982	//  TCP	UDP	Solar Data Log (JK client app for PV solar inverters)
				   << 4993	//  TCP	UDP	Home FTP Server web Interface Default Port
				   << 5000	//  TCP		commplex-main
				   << 5000	//  TCP		UPnP—Windows network device interoperability
				   << 5000	//  TCP		VTun—VPN Software
				   << 5000	//  	UDP	FlightGear multiplayer[56]
				   << 5000	//  	UDP	VTun—VPN Software
				   << 5001	//  TCP		commplex-link
				   << 5001	//  TCP		Slingbox and Slingplayer
				   << 5001	//  TCP		Iperf (Tool for measuring TCP and UDP bandwidth performance)
				   << 5001	//  	UDP	Iperf (Tool for measuring TCP and UDP bandwidth performance)
				   << 5002	//  TCP		SOLICARD ARX[57]
				   << 5002	//  	UDP	Drobo Dashboard[58]
				   << 5003	//  TCP	UDP	FileMaker
				   << 5004	//  TCP	UDP	RTP (Real-time Transport Protocol) media data (RFC 3551 RFC 4571)
				   << 5004	// DCCP		RTP (Real-time Transport Protocol) media data (RFC 3551 RFC 4571)
				   << 5005	//  TCP	UDP	RTP (Real-time Transport Protocol) control protocol (RFC 3551 RFC 4571)
				   << 5005	// DCCP		RTP (Real-time Transport Protocol) control protocol (RFC 3551 RFC 4571)
				   << 5010	//  TCP	UDP	Registered to: TelePath (the IBM FlowMark workflow-management system messaging platform)[59]
				   //  		The TCP port is now used for: IBM WebSphere MQ Workflow
				   << 5011	//  TCP	UDP	TelePath (the IBM FlowMark workflow-management system messaging platform)[59]
				   << 5029	//  TCP		Sonic Robo Blast 2 : Multiplayer[60]
				   << 5031	//  TCP	UDP	AVM CAPI-over-TCP (ISDN over Ethernet tunneling)
				   << 5037	//  TCP		Android ADB server
				   << 5050	//  TCP		Yahoo! Messenger
				   << 5051	//  TCP		ita-agent Symantec Intruder Alert[61]
				   << 5060	//  TCP	UDP	Session Initiation Protocol (SIP)
				   << 5061	//  TCP		Session Initiation Protocol (SIP) over TLS
				   << 5070	//  TCP		Binary Floor Control Protocol (BFCP)[62] published as RFC 4582 is a protocol that allows for an additional video channel (known as the content channel) alongside the main video channel in a video-conferencing call that uses SIP. Also used for Session Initiation Protocol (SIP) preferred port for PUBLISH on SIP Trunk to Cisco Unified Presence Server (CUPS)
				   << 5082	//  TCP	UDP	Qpur Communication Protocol
				   << 5083	//  TCP	UDP	Qpur File Protocol
				   << 5084	//  TCP	UDP	EPCglobal Low Level Reader Protocol (LLRP)
				   << 5085	//  TCP	UDP	EPCglobal Low Level Reader Protocol (LLRP) over TLS
				   << 5093	//  	UDP	SafeNet Inc Sentinel LM Sentinel RMS License Manager Client-to-Server
				   << 5099	//  TCP	UDP	SafeNet Inc Sentinel LM Sentinel RMS License Manager Server-to-Server
				   << 5104	//  TCP		IBM Tivoli Framework NetCOOL/Impact[63] HTTP Service
				   << 5106	//  TCP		A-Talk Common connection
				   << 5107	//  TCP		A-Talk Remote server connection
				   << 5108	//  TCP		VPOP3 Mail Server Webmail
				   << 5109	//  TCP	UDP	VPOP3 Mail Server Status
				   << 5110	//  TCP		ProRat Server
				   << 5121	//  TCP		Neverwinter Nights
				   << 5150	//  TCP	UDP	ATMP Ascend Tunnel Management Protocol[64]
				   << 5150	//  TCP	UDP	Malware Cerberus RAT
				   << 5151	//  TCP		ESRI SDE Instance
				   << 5151	//  	UDP	ESRI SDE Remote Start
				   << 5154	//  TCP	UDP	BZFlag
				   << 5176	//  TCP		ConsoleWorks default UI interface
				   << 5190	//  TCP		ICQ and AOL Instant Messenger
				   << 5222	//  TCP		Extensible Messaging and Presence Protocol (XMPP) client connection[65][66]
				   << 5223	//  TCP		Extensible Messaging and Presence Protocol (XMPP) client connection over SSL
				   << 5228	//  TCP		HP Virtual Room Service
				   << 5228	//  TCP		Android Market
				   << 5246	//  	UDP	Control And Provisioning of Wireless Access Points (CAPWAP) CAPWAP control[67]
				   << 5247	//  	UDP	Control And Provisioning of Wireless Access Points (CAPWAP) CAPWAP data[67]
				   << 5269	//  TCP		Extensible Messaging and Presence Protocol (XMPP) server connection[65][66]
				   << 5280	//  TCP		Extensible Messaging and Presence Protocol (XMPP) XEP-0124: Bidirectional-streams Over Synchronous HTTP (BOSH)
				   << 5281	//  TCP		Undo License Manager
				   << 5281	//  TCP		Extensible Messaging and Presence Protocol (XMPP)[68]
				   << 5298	//  TCP	UDP	Extensible Messaging and Presence Protocol (XMPP)[69]
				   << 5310	//  TCP	UDP	Outlaws (1997 video game). Both UDP and TCP are reserved but only UDP is used
				   << 5310	//  TCP	UDP	Ginever.net data communication port
				   << 5311	//  TCP	UDP	Ginever.net data communication port
				   << 5312	//  TCP	UDP	Ginever.net data communication port
				   << 5313	//  TCP	UDP	Ginever.net data communication port
				   << 5314	//  TCP	UDP	Ginever.net data communication port
				   << 5315	//  TCP	UDP	Ginever.net data communication port
				   << 5349	//  TCP		STUN a protocol for NAT traversal (UDP is reserved)[48]
				   << 5349	//  TCP		TURN a protocol for NAT traversal (UDP is reserved)[49]
				   << 5351	//  TCP	UDP	NAT Port Mapping Protocol—client-requested configuration for inbound connections through network address translators
				   << 5353	//  	UDP	Multicast DNS (mDNS)
				   << 5355	//  TCP	UDP	LLMNR—Link-Local Multicast Name Resolution allows hosts to perform name resolution for hosts on the same local link (only provided by Windows Vista and Server 2008)
				   << 5357	//  TCP	UDP	Web Services for Devices (WSDAPI) (only provided by Windows Vista Windows 7 and Server 2008)
				   << 5358	//  TCP	UDP	WSDAPI Applications to Use a Secure Channel (only provided by Windows Vista Windows 7 and Server 2008)
				   << 5394	//  	UDP	Kega Fusion a Sega multi-console emulator
				   << 5402	//  TCP	UDP	mftp Stratacache OmniCast content delivery system MFTP file sharing protocol
				   << 5405	//  TCP	UDP	NetSupport Manager
				   << 5412	//  TCP	UDP	IBM Rational Synergy (Telelogic Synergy) (Continuus CM) Message Router
				   << 5421	//  TCP	UDP	NetSupport Manager
				   << 5432	//  TCP	UDP	PostgreSQL database system
				   << 5433	//  TCP		Bouwsoft file/webserver[70]
				   << 5445	//  	UDP	Cisco Unified Video Advantage
				   << 5450	//  TCP		OSIsoft PI Server Client Access
				   << 5457	//  TCP		OSIsoft PI Asset Framework Client Access
				   << 5458	//  TCP		OSIsoft PI Notifications Client Access
				   << 5495	//  TCP		Applix TM1 Admin server
				   << 5498	//  TCP		Hotline tracker server connection
				   << 5499	//  	UDP	Hotline tracker server discovery
				   << 5500	//  TCP		VNC remote desktop protocol—for incoming listening viewer Hotline control connection
				   << 5501	//  TCP		Hotline file transfer connection
				   << 5517	//  TCP		Setiqueue Proxy server client for SETI@Home project
				   << 5550	//  TCP		Hewlett-Packard Data Protector
				   << 5555	//  TCP		Freeciv versions up to 2.0 Hewlett-Packard Data Protector McAfee EndPoint Encryption Database Server SAP Default for Microsoft Dynamics CRM 4.0
				   << 5556	//  TCP	UDP	Freeciv
				   << 5591	//  TCP		Default for Tidal Enterprise Scheduler master-Socket used for communication between Agent-to-Master though can be changed
				   << 5631	//  TCP		pcANYWHEREdata Symantec pcAnywhere (version 7.52 and later[71])[72] data
				   << 5632	//  	UDP	pcANYWHEREstat Symantec pcAnywhere (version 7.52 and later) status
				   << 5656	//  TCP		IBM Lotus Sametime p2p file transfer
				   << 5666	//  TCP		NRPE (Nagios)
				   << 5667	//  TCP		NSCA (Nagios)
				   << 5678	//  	UDP	Mikrotik RouterOS Neighbor Discovery Protocol (MNDP)
				   << 5721	//  TCP	UDP	Kaseya
				   << 5723	//  TCP		Operations Manager[73]
				   << 5741	//  TCP	UDP	IDA Discover Port 1
				   << 5742	//  TCP	UDP	IDA Discover Port 2
				   << 5800	//  TCP		VNC remote desktop protocol—for use over HTTP
				   << 5814	//  TCP	UDP	Hewlett-Packard Support Automation (HP OpenView Self-Healing Services)
				   << 5850	//  TCP		COMIT SE (PCR)
				   << 5852	//  TCP		Adeona client: communications to OpenDHT
				   << 5900	//  TCP	UDP	Virtual Network Computing (VNC) remote desktop protocol (used by Apple Remote Desktop and others)
				   << 5912	//  TCP		Default for Tidal Enterprise Scheduler agent-Socket used for communication between Master-to-Agent though can be changed
				   << 5938	//  TCP	UDP	TeamViewer remote desktop protocol
				   << 5984	//  TCP	UDP	CouchDB database server
				   << 5999	//  TCP		CVSup file update tool[74]
				   << 6000	//  TCP		X11—used between an X client and server over the network
				   << 6001	//  	UDP	X11—used between an X client and server over the network
				   << 6005	//  TCP		Default for BMC Software Control-M/Server—Socket used for communication between Control-M processes—though often changed during installation
				   << 6005	//  TCP		Default for Camfrog chat & cam client
				   << 6050	//  TCP		Arcserve backup
				   << 6050	//  TCP		Nortel software
				   << 6051	//  TCP		Arcserve backup
				   << 6072	//  TCP		iOperator Protocol Signal Port
				   << 6086	//  TCP		PDTP—FTP like file server in a P2P network
				   << 6100	//  TCP		Vizrt System
				   << 6100	//  TCP		Ventrilo This is the authentication port that must be allowed outbound for version 3 of Ventrilo
				   << 6101	//  TCP		Backup Exec Agent Browser
				   << 6110	//  TCP	UDP	softcm HP Softbench CM
				   << 6111	//  TCP	UDP	spc HP Softbench Sub-Process Control
				   << 6112	//  	UDP	"dtspcd"—a network daemon that accepts requests from clients to execute commands and launch applications remotely
				   << 6112	//  TCP		"dtspcd"—a network daemon that accepts requests from clients to execute commands and launch applications remotely
				   << 6112	//  TCP		Blizzard's Battle.net gaming service ArenaNet gaming service Relic gaming sercive
				   << 6112	//  TCP		Club Penguin Disney online game for kids
				   << 6113	//  TCP		Club Penguin Disney online game for kids
				   << 6129	//  TCP		DameWare Remote Control
				   << 6257	//  	UDP	WinMX (see also 6699)
				   << 6260	//  TCP	UDP	planet M.U.L.E.
				   << 6262	//  TCP		Sybase Advantage Database Server
				   << 6324	//  TCP	UDP	Hall Research Device discovery and configuration
				   << 6343	//  	UDP	SFlow sFlow traffic monitoring
				   << 6346	//  TCP	UDP	gnutella-svc gnutella (FrostWire Limewire Shareaza etc.)
				   << 6347	//  TCP	UDP	gnutella-rtr Gnutella alternate
				   << 6350	//  TCP	UDP	App Discovery and Access Protocol
				   << 6389	//  TCP		EMC CLARiiON
				   << 6432	//  TCP		PgBouncer - A connection pooler for PostgreSQL
				   << 6444	//  TCP	UDP	Sun Grid Engine—Qmaster Service
				   << 6445	//  TCP	UDP	Sun Grid Engine—Execution Service
				   << 6502	//  TCP	UDP	Netop Business Solutions - NetOp Remote Control
				   << 6503	//  	UDP	Netop Business Solutions - NetOp School
				   << 6515	//  TCP	UDP	Elipse RPC Protocol (REC)
				   << 6522	//  TCP		Gobby (and other libobby-based software)
				   << 6523	//  TCP		Gobby 0.5 (and other libinfinity-based software)
				   << 6543	//  	UDP	Paradigm Research & Development Jetnet[75] default
				   << 6566	//  TCP		SANE (Scanner Access Now Easy)—SANE network scanner daemon
				   << 6560	//  TCP		Speech-Dispatcher daemon
				   << 6561	//  TCP		Speech-Dispatcher daemon
				   << 6571	//  		Windows Live FolderShare client
				   << 6600	//  TCP		Music Playing Daemon (MPD)
				   << 6619	//  TCP	UDP	odette-ftps Odette File Transfer Protocol (OFTP) over TLS/SSL
				   << 6646	//  	UDP	McAfee Network Agent
				   << 6660	//  Start range: Internet Relay Chat (IRC)
				   << 6661
				   << 6662
				   << 6663
				   << 6664
				   << 6665
				   << 6666
				   << 6667
				   << 6668
				   << 6669	//  TCP		Internet Relay Chat (IRC)
				   << 6679	//  TCP	UDP	Osorno Automation Protocol (OSAUT)
				   << 6679	//  TCP		IRC SSL (Secure Internet Relay Chat)—often used
				   << 6697	//  TCP		IRC SSL (Secure Internet Relay Chat)—often used
				   << 6699	//  TCP		WinMX (see also 6257)
				   << 6702	//  TCP		Default for Tidal Enterprise Scheduler client-Socket used for communication between Client-to-Master though can be changed
				   << 6771	//  	UDP	Polycom server broadcast
				   << 6789	//  TCP		Campbell Scientific Loggernet Software[76]
				   << 6881
				   << 6882
				   << 6883
				   << 6884
				   << 6885
				   << 6886
				   << 6887	//  TCP	UDP	BitTorrent part of full range of ports used most often
				   << 6888	//  TCP	UDP	MUSE
				   << 6888	//  Start range: BitTorrent part of full range of ports used most often
				   << 6889
				   << 6890
				   << 6891
				   << 6892
				   << 6893
				   << 6894
				   << 6895
				   << 6896
				   << 6897
				   << 6898
				   << 6899
				   << 6900
				   << 6901
				   << 6902
				   << 6903
				   << 6904
				   << 6905
				   << 6906
				   << 6907
				   << 6908
				   << 6909
				   << 6910
				   << 6911
				   << 6912
				   << 6913
				   << 6914
				   << 6915
				   << 6916
				   << 6917
				   << 6918
				   << 6919
				   << 6920
				   << 6921
				   << 6922
				   << 6923
				   << 6924
				   << 6925
				   << 6926
				   << 6927
				   << 6928
				   << 6929
				   << 6930
				   << 6931
				   << 6932
				   << 6933
				   << 6934
				   << 6935
				   << 6936
				   << 6937
				   << 6938
				   << 6939
				   << 6940
				   << 6941
				   << 6942
				   << 6943
				   << 6944
				   << 6945
				   << 6946
				   << 6947
				   << 6948
				   << 6949
				   << 6950
				   << 6951
				   << 6952
				   << 6953
				   << 6954
				   << 6955
				   << 6956
				   << 6957
				   << 6958
				   << 6959
				   << 6960
				   << 6961
				   << 6962
				   << 6963
				   << 6964
				   << 6965
				   << 6966
				   << 6967
				   << 6968
				   << 6969
				   << 6970
				   << 6971
				   << 6972
				   << 6973
				   << 6974
				   << 6975
				   << 6976
				   << 6977
				   << 6978
				   << 6979
				   << 6980
				   << 6981
				   << 6982
				   << 6983
				   << 6984
				   << 6985
				   << 6986
				   << 6987
				   << 6988
				   << 6989
				   << 6990
				   << 6991
				   << 6992
				   << 6993
				   << 6994
				   << 6995
				   << 6996
				   << 6997
				   << 6998
				   << 6999	//  TCP	UDP	BitTorrent part of full range of ports used most often
				   << 6891	//  Start range: Windows Live Messenger (File transfer)
				   << 6892
				   << 6893
				   << 6894
				   << 6895
				   << 6896
				   << 6897
				   << 6898
				   << 6899
				   << 6900	//  TCP	UDP	Windows Live Messenger (File transfer)
				   << 6901	//  TCP	UDP	Windows Live Messenger (Voice)
				   << 6969	//  TCP	UDP	acmsoda
				   << 6969	//  TCP		BitTorrent tracker
				   << 7000	//  TCP		Default for Vuze's built in HTTPS Bittorrent Tracker
				   << 7000	//  TCP		Avira Server Management Console
				   << 7001	//  TCP		Avira Server Management Console
				   << 7001	//  TCP		Default for BEA WebLogic Server's HTTP server though often changed during installation
				   << 7002	//  TCP		Default for BEA WebLogic Server's HTTPS server though often changed during installation
				   << 7005	//  TCP		Default for BMC Software Control-M/Server and Control-M/Agent for Agent-to-Server though often changed during installation
				   << 7006	//  TCP		Default for BMC Software Control-M/Server and Control-M/Agent for Server-to-Agent though often changed during installation
				   << 7010	//  TCP		Default for Cisco AON AMC (AON Management Console) [77]
				   << 7022	//  TCP		Database mirroring endpoints
				   << 7023	//  	UDP	Bryan Wilcutt T2-NMCS Protocol for SatCom Modems
				   << 7025	//  TCP		Zimbra LMTP [mailbox]—local mail delivery
				   << 7047	//  TCP		Zimbra conversion server
				   << 7133	//  TCP		Enemy Territory: Quake Wars
				   << 7144	//  TCP		Peercast
				   << 7145	//  TCP		Peercast
				   << 7171	//  TCP		Tibia
				   << 7306	//  TCP		Zimbra mysql [mailbox]
				   << 7307	//  TCP		Zimbra mysql [logger]
				   << 7312	//  	UDP	Sibelius License Server
				   << 7400	//  TCP	UDP	RTPS (Real Time Publish Subscribe) DDS Discovery
				   << 7401	//  TCP	UDP	RTPS (Real Time Publish Subscribe) DDS User-Traffic
				   << 7402	//  TCP	UDP	RTPS (Real Time Publish Subscribe) DDS Meta-Traffic
				   << 7473	//  TCP	UDP	Rise: The Vieneo Province
				   << 7547	//  TCP	UDP	CPE WAN Management Protocol Technical Report 069
				   << 7615	//  TCP		ISL Online[78] communication protocol
				   << 7624	//  TCP	UDP	Instrument Neutral Distributed Interface
				   << 7652	//  TCP		I2P anonymizing overlay network
				   << 7653	//  TCP		I2P anonymizing overlay network
				   << 7654	//  TCP		I2P anonymizing overlay network
				   << 7655	//  	UDP	I2P SAM Bridge Socket API
				   << 7656	//  TCP		I2P anonymizing overlay network
				   << 7657	//  TCP		I2P anonymizing overlay network
				   << 7658	//  TCP		I2P anonymizing overlay network
				   << 7659	//  TCP		I2P anonymizing overlay network
				   << 7660	//  TCP		I2P anonymizing overlay network
				   << 7670	//  TCP		BrettspielWelt BSW Boardgame Portal
				   << 7676	//  TCP		Aqumin AlphaVision Remote Command Interface
				   << 7700	//  	UDP	P2P DC (RedHub)
				   << 7707	//  	UDP	Killing Floor
				   << 7708	//  	UDP	Killing Floor
				   << 7717	//  	UDP	Killing Floor
				   << 7777	//  TCP		iChat server file transfer proxy
				   << 7777	//  TCP		Oracle Cluster File System 2
				   << 7777	//  TCP		Windows backdoor program tini.exe default
				   << 7777	//  TCP		Xivio default Chat Server
				   << 7777	//  TCP		Terraria default server
				   << 7777	//  	UDP	San Andreas Multiplayer default server
				   << 7778	//  TCP		Bad Trip MUD
				   << 7777	//  Start range: Unreal Tournament series default server
				   << 7778
				   << 7779
				   << 7780
				   << 7781
				   << 7782
				   << 7783
				   << 7784
				   << 7785
				   << 7786
				   << 7787
				   << 7788	//  TCP	UDP	Unreal Tournament series default server
				   << 7787	//  TCP		GFI EventsManager 7 & 8
				   << 7788	//  TCP		GFI EventsManager 7 & 8
				   << 7831	//  TCP		Default used by Smartlaunch Internet Cafe Administration[79] software
				   << 7880	//  TCP	UDP	PowerSchool Gradebook Server
				   << 7890	//  TCP		Default that will be used by the iControl Internet Cafe Suite Administration software
				   << 7915	//  TCP		Default for YSFlight server[80]
				   << 7935	//  TCP		Fixed port used for Adobe Flash Debug Player to communicate with a debugger (Flash IDE Flex Builder or fdb).[81]
				   << 9936	//  TCP	UDP	EMC2 (Legato) Networker or Sun Solcitice Backup
				   << 7937	//  TCP	UDP	EMC2 (Legato) Networker or Sun Solcitice Backup
				   << 8000	//  	UDP	iRDMI (Intel Remote Desktop Management Interface)[82]—sometimes erroneously used instead of port 8080
				   << 8000	//  TCP		iRDMI (Intel Remote Desktop Management Interface)[82]—sometimes erroneously used instead of port 8080
				   << 8000	//  TCP		Commonly used for internet radio streams such as those using SHOUTcast
				   << 8000	//  TCP		FreemakeVideoCapture service a part of Freemake Video Downloader [83]
				   << 8001	//  TCP		Commonly used for internet radio streams such as those using SHOUTcast
				   << 8002	//  TCP		Cisco Systems Unified Call Manager Intercluster
				   << 8008	//  TCP		HTTP Alternate
				   << 8008	//  TCP		IBM HTTP Server administration default
				   << 8009	//  TCP		ajp13—Apache JServ Protocol AJP Connector
				   << 8010	//  TCP		XMPP File transfers
				   << 8011	//  TCP		HTTP/TCP Symon Communications Event and Query Engine
				   << 8012	//  TCP		HTTP/TCP Symon Communications Event and Query Engine
				   << 8013	//  TCP		HTTP/TCP Symon Communications Event and Query Engine
				   << 8014	//  TCP		HTTP/TCP Symon Communications Event and Query Engine
				   << 8014	//  TCP	UDP	Perseus SDR Receiver default remote connection port
				   << 8020	//  TCP		360Works SuperContainer[84]
				   << 8074	//  TCP		Gadu-Gadu
				   << 8075	//  TCP		Killing Floor
				   << 8078	//  TCP	UDP	Default port for most Endless Online-based servers
				   << 8080	//  TCP		HTTP alternate (http_alt)—commonly used for Web proxy and caching server or for running a Web server as a non-root user
				   << 8080	//  TCP		Apache Tomcat
				   << 8080	//  	UDP	FilePhile Master/Relay
				   << 8081	//  TCP		HTTP alternate VibeStreamer e.g. McAfee ePolicy Orchestrator (ePO)
				   << 8086	//  TCP		HELM Web Host Automation Windows Control Panel
				   << 8086	//  TCP		Kaspersky AV Control Center
				   << 8087	//  TCP		Hosting Accelerator Control Panel
				   << 8087	//  TCP		Parallels Plesk Control Panel
				   << 8087	//  	UDP	Kaspersky AV Control Center
				   << 8088	//  TCP		Asterisk (PBX) Web Configuration utility (GUI Addon)
				   << 8089	//  TCP		Splunk Daemon
				   << 8090	//  TCP		HTTP Alternate (http_alt_alt)—used as an alternative to port 8080
				   << 8100	//  TCP		Console Gateway License Verification
				   << 8116	//  	UDP	Check Point Cluster Control Protocol
				   << 8118	//  TCP		Privoxy—advertisement-filtering Web proxy
				   << 8123	//  TCP		Polipo Web proxy
				   << 8192	//  TCP		Sophos Remote Management System
				   << 8193	//  TCP		Sophos Remote Management System
				   << 8194	//  TCP		Sophos Remote Management System
				   << 8194	//  TCP		Bloomberg Application
				   << 8195	//  TCP		Bloomberg Application
				   << 8200	//  TCP		GoToMyPC
				   << 8222	//  TCP		VMware Server Management User Interface[85] (insecure Web interface).[86] See also port 8333
				   << 8243	//  TCP	UDP	HTTPS listener for Apache Synapse [87]
				   << 8280	//  TCP	UDP	HTTP listener for Apache Synapse [87]
				   << 8291	//  TCP		Winbox—Default on a MikroTik RouterOS for a Windows application used to administer MikroTik RouterOS
				   << 8303	//  	UDP	Teeworlds Server
				   << 8331	//  TCP		MultiBit [5]
				   << 8332	//  TCP		Bitcoin JSON-RPC server[88]
				   << 8333	//  TCP		Bitcoin[89]
				   << 8333	//  TCP		VMware Server Management User Interface[85] (secure Web interface).[86] See also port 8222
				   << 8400	//  TCP	UDP	cvp Commvault Unified Data Management
				   << 8442	//  TCP	UDP	CyBro A-bus Cybrotech Ltd.
				   << 8443	//  TCP		SW Soft Plesk Control Panel Apache Tomcat SSL Promise WebPAM SSL McAfee ePolicy Orchestrator (ePO)
				   << 8484	//  TCP	UDP	MapleStory
				   << 8500	//  TCP	UDP	ColdFusion Macromedia/Adobe ColdFusion default and Duke Nukem 3D—default
				   << 8501	//  TCP		[6] DukesterX —default
				   << 8601	//  TCP		Wavestore CCTV protocol[90]
				   << 8602	//  TCP	UDP	Wavestore Notification protocol[90]
				   << 8642	//  TCP		Lotus Traveller
				   << 8691	//  TCP		Ultra Fractal default server port for distributing calculations over network computers
				   << 8701	//  	UDP	SoftPerfect Bandwidth Manager
				   << 8702	//  	UDP	SoftPerfect Bandwidth Manager
				   << 8767	//  	UDP	TeamSpeak—default
				   << 8768	//  	UDP	TeamSpeak—alternate
				   << 8778	//  TCP		EPOS Speech Synthesis System
				   << 8840	//  TCP		Opera Unite server
				   << 8880	//  	UDP	cddbp-alt CD DataBase (CDDB) protocol (CDDBP) alternate
				   << 8880	//  TCP		cddbp-alt CD DataBase (CDDB) protocol (CDDBP) alternate
				   << 8880	//  TCP		WebSphere Application Server SOAP connector default
				   << 8880	//  TCP		Win Media Streamer to Server SOAP connector default
				   << 8881	//  TCP		Atlasz Informatics Research Ltd Secure Application Server
				   << 8882	//  TCP		Atlasz Informatics Research Ltd Secure Application Server
				   << 8883	//  TCP	UDP	Secure MQ Telemetry Transport (MQTT over SSL)
				   << 8886	//  TCP		PPM3 (Padtec Management Protocol version 3)
				   << 8887	//  TCP		HyperVM HTTP
				   << 8888	//  TCP		HyperVM HTTPS
				   << 8888	//  TCP		Freenet HTTP
				   << 8888	//  TCP	UDP	NewsEDGE server
				   << 8888	//  TCP		Sun Answerbook dwhttpd server (deprecated by docs.sun.com)
				   << 8888	//  TCP		GNUmp3d HTTP music streaming and Web interface
				   << 8888	//  TCP		LoLo Catcher HTTP Web interface (www.optiform.com)
				   << 8888	//  TCP		D2GS Admin Console Telnet administration console for D2GS servers (Diablo 2)
				   << 8888	//  TCP		Earthland Relams 2 Server (AU1_2)
				   << 8888	//  TCP		MAMP Server
				   << 8889	//  TCP		MAMP Server
				   << 8889	//  TCP		Earthland Relams 2 Server (AU1_1)
				   << 8983	//  TCP		Default for Apache Solr 1.4
				   << 8998	//  TCP		I2P Monotone Repository
				   << 9000	//  TCP		Buffalo LinkSystem Web access
				   << 9000	//  TCP		DBGp
				   << 9000	//  TCP		SqueezeCenter web server & streaming
				   << 9000	//  	UDP	UDPCast
				   << 9000	//  TCP		Play! Framework web server
				   << 9001	//  TCP	UDP	ETL Service Manager[91]
				   << 9001	//  		Microsoft SharePoint authoring environment
				   << 9001	//  		cisco-xremote router configuration
				   << 9001	//  		Tor network default
				   << 9001	//  TCP		DBGp Proxy
				   << 9009	//  TCP	UDP	Pichat Server—Peer to peer chat software
				   << 9010	//  TCP		TISERVICEMANAGEMENT Numara Track-It!
				   << 9020	//  TCP		WiT WiT Services
				   << 9025	//  TCP		WiT WiT Services
				   << 9030	//  TCP		Tor often used
				   << 9043	//  TCP		WebSphere Application Server Administration Console secure
				   << 9050	//  TCP		Tor
				   << 9051	//  TCP		Tor
				   << 9060	//  TCP		WebSphere Application Server Administration Console
				   << 9080	//  	UDP	glrpc Groove Collaboration software GLRPC
				   << 9080	//  TCP		glrpc Groove Collaboration software GLRPC
				   << 9080	//  TCP		WebSphere Application Server HTTP Transport (port 1) default
				   << 9090	//  TCP		Webwasher Secure Web McAfee Web Gateway - Default Proxy Port
				   << 9090	//  TCP		Openfire Administration Console
				   << 9090	//  TCP		SqueezeCenter control (CLI)
				   << 9091	//  TCP		Openfire Administration Console (SSL Secured)
				   << 9091	//  TCP		Transmission (BitTorrent client) Web Interface
				   << 9100	//  TCP		PDL Data Stream
				   << 9101	//  TCP	UDP	Bacula Director
				   << 9102	//  TCP	UDP	Bacula File Daemon
				   << 9103	//  TCP	UDP	Bacula Storage Daemon
				   << 9105	//  TCP	UDP	Xadmin Control Daemon
				   << 9106	//  TCP	UDP	Astergate Control Daemon
				   << 9107	//  TCP		Astergate-FAX Control Daemon
				   << 9110	//  	UDP	SSMP Message protocol
				   << 9119	//  TCP	UDP	MXit Instant Messenger
				   << 9191	//  TCP		Catamount Software - PocketMoney Sync
				   << 9293	//  TCP		Sony PlayStation RemotePlay
				   << 9300	//  TCP		IBM Cognos 8 SOAP Business Intelligence and Performance Management
				   << 9303	//  	UDP	D-Link Shareport Share storage and MFP printers
				   << 9306	//  TCP		Sphinx Native API
				   << 9312	//  TCP		Sphinx SphinxQL
				   << 9418	//  TCP	UDP	git Git pack transfer service
				   << 9420	//  TCP		MooseFS distributed file system—master server to chunk servers
				   << 9421	//  TCP		MooseFS distributed file system—master server to clients
				   << 9422	//  TCP		MooseFS distributed file system—chunk servers to clients
				   << 9535	//  TCP	UDP	mngsuite LANDesk Management Suite Remote Control
				   << 9536	//  TCP	UDP	laes-bf IP Fabrics Surveillance buffering function
				   << 9600	//  	UDP	Omron FINS OMRON FINS PLC communication
				   << 9675	//  TCP	UDP	Spiceworks Desktop IT Helpdesk Software
				   << 9676	//  TCP	UDP	Spiceworks Desktop IT Helpdesk Software
				   << 9695	//  	UDP	CCNx
				   << 9800	//  TCP	UDP	WebDAV Source
				   << 9800	//  		WebCT e-learning portal
				   << 9875	//  TCP		Club Penguin Disney online game for kids
				   << 9898	//  	UDP	MonkeyCom
				   << 9898	//  TCP		MonkeyCom
				   << 9898	//  TCP		Tripwire—File Integrity Monitoring Software
				   << 9987	//  	UDP	TeamSpeak 3 server default (voice) port (for the conflicting service see the IANA list)
				   << 9996	//  TCP	UDP	Ryan's App "Ryan's App" Trading Software
				   << 9996	//  TCP	UDP	The Palace "The Palace" Virtual Reality Chat software.—5
				   << 9998	//  TCP	UDP	The Palace "The Palace" Virtual Reality Chat software.—5
				   << 9999	//  		Hydranode—edonkey2000 TELNET control
				   << 9999	//  TCP		Lantronix UDS-10/UDS100[92] RS-485 to Ethernet Converter TELNET control
				   << 9999	//  		Urchin Web Analytics
				   << 10000	//  		Webmin—Web-based Linux admin tool
				   << 10000	//  		BackupExec
				   << 10000	//  		Ericsson Account Manager (avim)
				   << 10001	//  TCP		Lantronix UDS-10/UDS100[93] RS-485 to Ethernet Converter default
				   << 10008	//  TCP	UDP	Octopus Multiplexer primary port for the CROMP protocol which provides a platform-independent means for communication of objects across a network
				   << 10009	//  TCP	UDP	Cross Fire a multiplayer online First Person Shooter
				   << 10010	//  TCP		Open Object Rexx (ooRexx) rxapi daemon
				   << 10017	//  		AIXNeXT HPUX—rexd daemon control
				   << 10024	//  TCP		Zimbra smtp [mta]—to amavis from postfix
				   << 10025	//  TCP		Zimbra smtp [mta]—back to postfix from amavis
				   << 10050	//  TCP	UDP	Zabbix-Agent
				   << 10051	//  TCP	UDP	Zabbix-Trapper
				   << 10110	//  TCP	UDP	NMEA 0183 Navigational Data. Transport of NMEA 0183 sentences over TCP or UDP
				   << 10113	//  TCP	UDP	NetIQ Endpoint
				   << 10114	//  TCP	UDP	NetIQ Qcheck
				   << 10115	//  TCP	UDP	NetIQ Endpoint
				   << 10116	//  TCP	UDP	NetIQ VoIP Assessor
				   << 10172	//  TCP		Intuit Quickbooks client
				   << 10200	//  TCP		FRISK Software International's fpscand virus scanning daemon for Unix platforms [94]
				   << 10200	//  TCP		FRISK Software International's f-protd virus scanning daemon for Unix platforms [95]
				   << 10201	//  TCP		FRISK Software International's f-protd virus scanning daemon for Unix platforms [95]
				   << 10202	//  TCP		FRISK Software International's f-protd virus scanning daemon for Unix platforms [95]
				   << 10203	//  TCP		FRISK Software International's f-protd virus scanning daemon for Unix platforms [95]
				   << 10204	//  TCP		FRISK Software International's f-protd virus scanning daemon for Unix platforms [95]
				   << 10301	//  TCP		VoiceIP-ACS UMP default device provisioning endpoint
				   << 10302	//  TCP		VoiceIP-ACS UMP default device provisioning endpoint (SSL)
				   << 10308	//  		Lock-on: Modern Air Combat
				   << 10480	//  		SWAT 4 Dedicated Server
				   << 10823	//  	UDP	Farming Simulator 2011 Default Server
				   << 10891	//  TCP		Jungle Disk (this port is opened by the Jungle Disk Monitor service on the localhost)
				   << 11001	//  TCP	UDP	metasys ( Johnson Controls Metasys java AC control environment )
				   << 11211	//  TCP	UDP	memcached
				   << 11235	//  		Savage:Battle for Newerth Server Hosting
				   << 11294	//  		Blood Quest Online Server
				   << 11371	//  		OpenPGP HTTP key server
				   << 11576	//  		IPStor Server management communication
				   << 12010	//  TCP		ElevateDB default database port [96]
				   << 12011	//  TCP		Axence nVision
				   << 12012	//  TCP		Axence nVision
				   << 12012	//  TCP		Audition Online Dance Battle Korea Server—Status/Version Check
				   << 12012	//  	UDP	Audition Online Dance Battle Korea Server—Status/Version Check
				   << 12013	//  TCP	UDP	Audition Online Dance Battle Korea Server
				   << 12035	//  	UDP	Linden Lab viewer to sim on SecondLife
				   << 12222	//  	UDP	Light Weight Access Point Protocol (LWAPP) LWAPP data (RFC 5412)
				   << 12223	//  	UDP	Light Weight Access Point Protocol (LWAPP) LWAPP control (RFC 5412)
				   << 12345	//  		NetBus—remote administration tool (often Trojan horse). Also used by NetBuster. Little Fighter 2 (TCP).
				   << 12489	//  TCP		NSClient/NSClient++/NC_Net (Nagios)
				   << 12975	//  TCP		LogMeIn Hamachi (VPN tunnel software; also port 32976)—used to connect to Mediation Server (bibi.hamachi.cc will attempt to use SSL (TCP port 443) if both 12975 & 32976 fail to connect
				   << 12998	//  	UDP	Takenaka RDI Mirror World on SecondLife
				   << 12999	//  	UDP	Takenaka RDI Mirror World on SecondLife
				   << 13000	//  Start range: Linden Lab viewer to sim on SecondLife
				   << 13001
				   << 13002
				   << 13003
				   << 13004
				   << 13005
				   << 13006
				   << 13007
				   << 13008
				   << 13009
				   << 13010
				   << 13011
				   << 13012
				   << 13013
				   << 13014
				   << 13015
				   << 13016
				   << 13017
				   << 13018
				   << 13019
				   << 13020
				   << 13021
				   << 13022
				   << 13023
				   << 13024
				   << 13025
				   << 13026
				   << 13027
				   << 13028
				   << 13029
				   << 13030
				   << 13031
				   << 13032
				   << 13033
				   << 13034
				   << 13035
				   << 13036
				   << 13037
				   << 13038
				   << 13039
				   << 13040
				   << 13041
				   << 13042
				   << 13043
				   << 13044
				   << 13045
				   << 13046
				   << 13047
				   << 13048
				   << 13049
				   << 13050	//  	UDP	Linden Lab viewer to sim on SecondLife
				   << 13008	//  TCP	UDP	Cross Fire a multiplayer online First Person Shooter
				   << 13075	//  TCP		Default[97] for BMC Software Control-M/Enterprise Manager Corba communication though often changed during installation
				   << 13195	//  TCP	UDP	Ontolux Ontolux 2D
				   << 13196	//  TCP	UDP	Ontolux Ontolux 2D
				   << 13337	//  TCP	UDP	ÆtherNet peer-to-peer networking
				   << 13338	//  TCP	UDP	ÆtherNet peer-to-peer networking
				   << 13339	//  TCP	UDP	ÆtherNet peer-to-peer networking
				   << 13340	//  TCP	UDP	ÆtherNet peer-to-peer networking
				   << 13720	//  TCP	UDP	Symantec NetBackup—bprd (formerly VERITAS)
				   << 13721	//  TCP	UDP	Symantec NetBackup—bpdbm (formerly VERITAS)
				   << 13724	//  TCP	UDP	Symantec Network Utility—vnetd (formerly VERITAS)
				   << 13782	//  TCP	UDP	Symantec NetBackup—bpcd (formerly VERITAS)
				   << 13783	//  TCP	UDP	Symantec VOPIED protocol (formerly VERITAS)
				   << 13785	//  TCP	UDP	Symantec NetBackup Database—nbdb (formerly VERITAS)
				   << 13786	//  TCP	UDP	Symantec nomdb (formerly VERITAS)
				   << 14439	//  TCP		APRS UI-View Amateur Radio[98] UI-WebServer
				   << 14567	//  	UDP	Battlefield 1942 and mods
				   << 14900	//  TCP		K3 SYSPRO K3 Framework WCF Backbone
				   << 15000	//  TCP		psyBNC
				   << 15000	//  TCP		Wesnoth
				   << 15000	//  TCP		Kaspersky Network Agent
				   << 15000	//  TCP		hydap Hypack Hydrographic Software Packages Data Acquisition
				   << 15000	//  	UDP	hydap Hypack Hydrographic Software Packages Data Acquisition
				   << 15556	//  TCP	UDP	Jeex.EU Artesia (direct client-to-db.service)
				   << 15567	//  	UDP	Battlefield Vietnam and mods
				   << 15345	//  TCP	UDP	XPilot Contact
				   << 16000	//  TCP		shroudBNC
				   << 16080	//  TCP		Mac OS X Server Web (HTTP) service with performance cache[99]
				   << 16200	//  TCP		Oracle Universal Content Management Content Server
				   << 16250	//  TCP		Oracle Universal Content Management Inbound Refinery
				   << 16384	//  	UDP	Iron Mountain Digital online backup
				   << 16567	//  	UDP	Battlefield 2 and mods
				   << 17500	//  TCP		Dropbox LanSync Protocol (db-lsp used to synchronize file catalogs between Dropbox clients on your local network.
				   << 17500	//  	UDP	Dropbox LanSync Discovery (db-lsp-disc used to synchronize file catalogs between Dropbox clients on your local network; is transmitted to broadcast addresses.
				   << 18010	//  TCP		Super Dancer Online Extreme(SDO-X)—CiB Net Station Malaysia Server
				   << 18104	//  TCP		RAD PDF Service
				   << 18180	//  TCP		DART Reporting server
				   << 18200	//  TCP	UDP	Audition Online Dance Battle AsiaSoft Thailand Server—Status/Version Check
				   << 18201	//  TCP	UDP	Audition Online Dance Battle AsiaSoft Thailand Server
				   << 18206	//  TCP	UDP	Audition Online Dance Battle AsiaSoft Thailand Server—FAM Database
				   << 18300	//  TCP	UDP	Audition Online Dance Battle AsiaSoft SEA Server—Status/Version Check
				   << 18301	//  TCP	UDP	Audition Online Dance Battle AsiaSoft SEA Server
				   << 18306	//  TCP	UDP	Audition Online Dance Battle AsiaSoft SEA Server—FAM Database
				   << 18333	//  TCP		Bitcoin testnet[89]
				   << 18400	//  TCP	UDP	Audition Online Dance Battle KAIZEN Brazil Server—Status/Version Check
				   << 18401	//  TCP	UDP	Audition Online Dance Battle KAIZEN Brazil Server
				   << 18505	//  TCP	UDP	Audition Online Dance Battle Nexon Server—Status/Version Check
				   << 18506	//  TCP	UDP	Audition Online Dance Battle Nexon Server
				   << 18605	//  TCP	UDP	X-BEAT—Status/Version Check
				   << 18606	//  TCP	UDP	X-BEAT
				   << 19000	//  TCP	UDP	Audition Online Dance Battle G10/alaplaya Server—Status/Version Check
				   << 19000	//  	UDP	JACK sound server
				   << 19001	//  TCP	UDP	Audition Online Dance Battle G10/alaplaya Server
				   << 19226	//  TCP		Panda Software AdminSecure Communication Agent
				   << 19283	//  TCP	UDP	K2 - KeyAuditor & KeyServer Sassafras Software Inc. Software Asset Management tools
				   << 19294	//  TCP		Google Talk Voice and Video connections [100]
				   << 19295	//  	UDP	Google Talk Voice and Video connections [100]
				   << 19302	//  	UDP	Google Talk Voice and Video connections [100]
				   << 19315	//  TCP	UDP	KeyShadow for K2 - KeyAuditor & KeyServer Sassafras Software Inc. Software Asset Management tools
				   << 19540	//  TCP	UDP	Belkin Network USB Hub
				   << 19638	//  TCP		Ensim Control Panel
				   << 19812	//  TCP		4D database SQL Communication
				   << 19813	//  TCP		4D database Client Server Communication
				   << 19814	//  TCP		4D database DB4D Communication
				   << 19999	//  		DNP - Secure (Distributed Network Protocol - Secure) a secure version of the protocol used in SCADA systems between communicating RTU's and IED's
				   << 20000	//  		DNP (Distributed Network Protocol) a protocol used in SCADA systems between communicating RTU's and IED's
				   << 20000	//  		Usermin Web-based user tool
				   << 20014	//  TCP		DART Reporting server
				   << 20560	//  TCP	UDP	Killing Floor
				   << 20702	//  TCP		Precise TPM Listener Agent
				   << 20720	//  TCP		Symantec i3 Web GUI server
				   << 20790	//  TCP		Precise TPM Web GUI server
				   << 21001	//  TCP		AMLFilter AMLFilter Inc. amlf-admin default port
				   << 21011	//  TCP		AMLFilter AMLFilter Inc. amlf-engine-01 default http port
				   << 21012	//  TCP		AMLFilter AMLFilter Inc. amlf-engine-01 default https port
				   << 21021	//  TCP		AMLFilter AMLFilter Inc. amlf-engine-02 default http port
				   << 21022	//  TCP		AMLFilter AMLFilter Inc. amlf-engine-02 default https port
				   << 22136	//  TCP		FLIR Systems Camera Resource Protocol
				   << 22222	//  TCP		Davis Instruments WeatherLink IP
				   << 22347	//  TCP	UDP	WibuKey WIBU-SYSTEMS AG Software protection system
				   << 22349	//  TCP		Wolfson Microelectronics WISCEBridge Debug Protocol[101]
				   << 22350	//  TCP	UDP	CodeMeter WIBU-SYSTEMS AG Software protection system
				   << 23073	//  		Soldat Dedicated Server
				   << 23399	//  		Skype Default Protocol
				   << 23513	//  		Duke Nukem 3D#Source code Duke Nukem Ports
				   << 24444	//  		NetBeans integrated development environment
				   << 24465	//  TCP	UDP	Tonido Directory Server for Tonido which is a Personal Web App and P2P platform
				   << 24554	//  TCP	UDP	BINKP Fidonet mail transfers over TCP/IP
				   << 24800	//  		Synergy: keyboard/mouse sharing software
				   << 24842	//  		StepMania: Online: Dance Dance Revolution Simulator
				   << 25000	//  TCP		Teamware Office standard client connection
				   << 25003	//  TCP		Teamware Office client notifier
				   << 25005	//  TCP		Teamware Office message transfer
				   << 25007	//  TCP		Teamware Office MIME Connector
				   << 25010	//  TCP		Teamware Office Agent server
				   << 25560	//  TCP		codeheart.js Relay Server
				   << 25565	//  TCP	UDP	Minecraft Dedicated Server
				   << 25565	//  		MySQL Standard MySQL port
				   << 25570	//  		Manic Digger default single player port
				   << 25826	//  	UDP	collectd default port[102]
				   << 25888	//  	UDP	Xfire (Firewall Report UDP_IN) IP Address (206.220.40.146) resolves to gameservertracking.xfire.com. Use unknown.
				   << 25999	//  TCP		Xfire
				   << 26000	//  	UDP	id Software's Quake server
				   << 26000	//  TCP		id Software's Quake server
				   << 26000	//  TCP		CCP's EVE Online Online gaming MMORPG
				   << 26850	//  TCP		War of No Return Server Port
				   << 26900	//  TCP		CCP's EVE Online Online gaming MMORPG
				   << 26901	//  TCP		CCP's EVE Online Online gaming MMORPG
				   << 27000	//  Start range: Steam Client
				   << 27001
				   << 27002
				   << 27003
				   << 27004
				   << 27005
				   << 27006
				   << 27007
				   << 27008
				   << 27009
				   << 27010
				   << 27011
				   << 27012
				   << 27013
				   << 27014
				   << 27015
				   << 27016
				   << 27017
				   << 27018
				   << 27019
				   << 27020
				   << 27021
				   << 27022
				   << 27023
				   << 27024
				   << 27025
				   << 27026
				   << 27027
				   << 27028
				   << 27029
				   << 27030	//  	UDP	Steam Client
				   << 27000	//  	UDP	(through 27006) id Software's QuakeWorld master server
				   << 27000	//  Start range: FlexNet Publisher's License server (from the range of default ports)
				   << 27001
				   << 27002
				   << 27003
				   << 27004
				   << 27005
				   << 27006
				   << 27007
				   << 27008
				   << 27009	//  TCP		FlexNet Publisher's License server (from the range of default ports)
				   << 27010	//  		Source engine dedicated server port
				   << 27014	//  Start range: Steam Downloads
				   << 27015
				   << 27016
				   << 27017
				   << 27018
				   << 27019
				   << 27020
				   << 27021
				   << 27022
				   << 27023
				   << 27024
				   << 27025
				   << 27026
				   << 27027
				   << 27028
				   << 27029
				   << 27030
				   << 27031
				   << 27032
				   << 27033
				   << 27034
				   << 27035
				   << 27036
				   << 27037
				   << 27038
				   << 27039
				   << 27040
				   << 27041
				   << 27042
				   << 27043
				   << 27044
				   << 27045
				   << 27046
				   << 27047
				   << 27048
				   << 27049
				   << 27050	//  TCP		Steam Downloads
				   << 27014	//  		Source engine dedicated server port (rare)
				   << 27015	//  		GoldSrc and Source engine dedicated server port
				   << 27016	//  		Magicka server port
				   << 27017	//  		mongoDB server port
				   << 27374	//  		Sub7 default.
				   << 27888	//  	UDP	Kaillera server
				   << 27900	//  		Nintendo Wi-Fi Connection
				   << 27901	//  		Nintendo Wi-Fi Connection
				   << 27902	//  Start range: id Software's Quake II master server
				   << 27903
				   << 27904
				   << 27905
				   << 27906
				   << 27907
				   << 27908
				   << 27909
				   << 27910	//  	UDP	id Software's Quake II master server
				   << 27950	//  	UDP	OpenArena outgoing
				   << 27960	//  Start range: Activision's Enemy Territory and id Software's Quake III Arena Quake III and Quake Live and some ioquake3 derived games (OpenArena incoming)
				   << 27961
				   << 27962
				   << 27963
				   << 27964
				   << 27965
				   << 27966
				   << 27967
				   << 27968
				   << 27969	//  	UDP	Activision's Enemy Territory and id Software's Quake III Arena Quake III and Quake Live and some ioquake3 derived games (OpenArena incoming)
				   << 28000	//  		Bitfighter Common/default Bitfighter Server
				   << 28001	//  		Starsiege: Tribes Common/default Tribes v.1 Server
				   << 28395	//  TCP		www.SmartSystemsLLC.com Used by Smart Sale 5.0
				   << 28785	//  	UDP	Cube 2 Sauerbraten[103]
				   << 28786	//  	UDP	Cube 2 Sauerbraten Port 2[103]
				   << 28852	//  TCP	UDP	Killing Floor
				   << 28910	//  		Nintendo Wi-Fi Connection
				   << 28960	//  	UDP	Call of Duty; Call of Duty: United Offensive; Call of Duty 2; Call of Duty 4: Modern Warfare; Call of Duty: World at War (PC Version)
				   << 29000	//  		Perfect World International Used by the Perfect World International Client
				   << 29292	//  TCP		TMO Integration Service Communications Port Used by Transaction Manager SaaS (HighJump Software)
				   << 29900	//  		Nintendo Wi-Fi Connection
				   << 29901	//  		Nintendo Wi-Fi Connection
				   << 29920	//  		Nintendo Wi-Fi Connection
				   << 30000	//  		Pokémon Netbattle
				   << 30301	//  		BitTorrent
				   << 30564	//  TCP		Multiplicity: keyboard/mouse/clipboard sharing software
				   << 30718	//  	UDP	Lantronix Discovery for Lantronix serial-to-ethernet devices
				   << 30777	//  TCP		ZangZing agent
				   << 31337	//  TCP		Back Orifice—remote administration tool (often Trojan horse)
				   << 31415	//  		ThoughtSignal—Server Communication Service (often Informational)
				   << 31456	//  TCP		TetriNET IRC gateway on some servers
				   << 31457	//  TCP		TetriNET
				   << 31458	//  TCP		TetriNET Used for game spectators
				   << 31620	//  TCP	UDP	LM-MON (Standard Floating License Manager LM-MON)
				   << 32123	//  TCP		x3Lobby Used by x3Lobby an internet application.
				   << 32245	//  TCP		MMTSG-mutualed over MMT (encrypted transmission)
				   << 32769	//  TCP		FileNet RPC
				   << 32887	//  TCP		Use by "Ace of Spades" game
				   << 32976	//  TCP		LogMeIn Hamachi (VPN tunnel software; also port 12975)—used to connect to Mediation Server (bibi.hamachi.cc will attempt to use SSL (TCP port 443) if both 12975 & 32976 fail to connect
				   << 33434	//  TCP	UDP	traceroute
				   << 33982	//  TCP	UDP	Dezta software
				   << 34443	//  		Linksys PSUS4 print server
				   << 34567	//  TCP		EDI service[104]
				   << 36963	//  	UDP	Any of the USGN online games most notably Counter Strike 2D multiplayer (2D clone of popular CounterStrike computer game)
				   << 37659	//  TCP		Axence nVision
				   << 37777	//  TCP		Digital Video Recorder hardware
				   << 40000	//  TCP	UDP	SafetyNET p Real-time Industrial Ethernet protocol
				   << 40123	//  	UDP	Flatcast
				   << 41823	//  TCP	UDP	Murealm Client
				   << 43047	//  TCP		TheòsMessenger second port for service TheòsMessenger
				   << 43034	//  TCP	UDP	LarmX.com™ database update mtr port
				   << 43048	//  TCP		TheòsMessenger third port for service TheòsMessenger
				   << 43594	//  TCP		RuneScape FunOrb Runescape Private Servers game servers
				   << 43595	//  TCP		RuneScape JAGGRAB servers
				   << 45824	//  TCP		Server for the DAI family of client-server products
				   << 47001	//  TCP		WinRM - Windows Remote Management Service [105]
				   << 47808	//  TCP	UDP	BACnet Building Automation and Control Networks (4780810 = BAC016)
				   << 49151;	//  TCP	UDP	Reserved


		for ( int i = 27500; i <= 27900; ++i )
		{
			oUsedPorts <<  i;    //27500-27900	//  	UDP	id Software's QuakeWorld
		}
	}

	int nPort = 1024; // reserved port

	while ( oUsedPorts.contains( nPort ) )
	{
		nPort = common::getRandomNum( 1024, 49151 );
	}

	return nPort;
}


quint32 common::intToUint(qint32 n)
{
	static const qint32 nMin = std::numeric_limits<qint32>::min();

	if ( n < 0 )
	{
		return static_cast<quint32>( n - nMin ) + nMin;
	}
	else
	{
		return static_cast<quint32>( n );
	}

}


qint32 common::uintToInt(quint32 n)
{
	static const quint32 nMax = ( quint32 )std::numeric_limits<qint32>::max();

	if ( n > nMax )
	{
		return static_cast<qint32>( n - nMax ) + nMax;
	}
	else
	{
		return static_cast<qint32>( n );
	}
}

