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

#include "quazaasettings.h"
#include "quazaaglobals.h"
#include <QtCore>
#include <QSettings>
#include <QUuid>

#include "debug_new.h"


/*!
	\file quazaasettings.h
	\brief \#include &qout;quazaasettings.h&quot;
 */

/*!
	\class CQuazaaSettings quazaasettings.h
	\ingroup base
	\brief Persistent settings used throughout quazaa.

	The CQuazaaSettings class provides a single place to store, save and load
	persistent settings used throughout Quazaa.

	\section Accessing CQuazaaSettings

	It is recommended to access CQuazaaSettings via the extern
	quazaaSettings.Struct.Variable and quazaaSettings.function methods.
 */

// This is the extern declaration used everywhere else in quazaa.
QuazaaSettings quazaaSettings;

/*!
	Creates a quazaaSettings object. Generally not necessary.
 */
QuazaaSettings::QuazaaSettings()
{
}

/*!
	Saves most of Quazaa's settings to persistent .ini file.
 */
void QuazaaSettings::saveSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Ares" );
	m_qSettings.setValue( "Enable", quazaaSettings.Ares.Enable );
	m_qSettings.setValue( "MaximumDownloads", quazaaSettings.Ares.MaximumDownloads );
	m_qSettings.setValue( "MaximumUploads", quazaaSettings.Ares.MaximumUploads );
	m_qSettings.setValue( "MaximumUploadsPerUser", quazaaSettings.Ares.MaximumUploadsPerUser );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "BitTorrent" );
	m_qSettings.setValue( "AutoClear", quazaaSettings.BitTorrent.AutoClear );
	m_qSettings.setValue( "AutoSeed", quazaaSettings.BitTorrent.AutoSeed );
	m_qSettings.setValue( "BandwidthPercentage", quazaaSettings.BitTorrent.BandwidthPercentage );
	m_qSettings.setValue( "ClearRatio", quazaaSettings.BitTorrent.ClearRatio );
	m_qSettings.setValue( "CodePage", quazaaSettings.BitTorrent.CodePage );
	m_qSettings.setValue( "DefaultTracker", quazaaSettings.BitTorrent.DefaultTracker );
	m_qSettings.setValue( "DefaultTrackerPeriod", quazaaSettings.BitTorrent.DefaultTrackerPeriod );
	m_qSettings.setValue( "DhtPruneTime", quazaaSettings.BitTorrent.DhtPruneTime );
	m_qSettings.setValue( "DownloadConnections", quazaaSettings.BitTorrent.DownloadConnections );
	m_qSettings.setValue( "DownloadTorrents", quazaaSettings.BitTorrent.DownloadTorrents );
	m_qSettings.setValue( "Endgame", quazaaSettings.BitTorrent.Endgame );
	m_qSettings.setValue( "ExtraKeys", quazaaSettings.BitTorrent.ExtraKeys );
	m_qSettings.setValue( "LinkPing", quazaaSettings.BitTorrent.LinkPing );
	m_qSettings.setValue( "LinkTimeout", quazaaSettings.BitTorrent.LinkTimeout );
	m_qSettings.setValue( "Managed", quazaaSettings.BitTorrent.Managed );
	m_qSettings.setValue( "PreferBTSources", quazaaSettings.BitTorrent.PreferBTSources );
	m_qSettings.setValue( "RandomPeriod", quazaaSettings.BitTorrent.RandomPeriod );
	m_qSettings.setValue( "RequestLimit", quazaaSettings.BitTorrent.RequestLimit );
	m_qSettings.setValue( "RequestPipe", quazaaSettings.BitTorrent.RequestPipe );
	m_qSettings.setValue( "RequestSize", quazaaSettings.BitTorrent.RequestSize );
	m_qSettings.setValue( "ShowFilesInDownload", quazaaSettings.BitTorrent.ShowFilesInDownload );
	m_qSettings.setValue( "SourceExchangePeriod", quazaaSettings.BitTorrent.SourceExchangePeriod );
	m_qSettings.setValue( "StartPaused", quazaaSettings.BitTorrent.StartPaused );
	m_qSettings.setValue( "TestPartials", quazaaSettings.BitTorrent.TestPartials );
	m_qSettings.setValue( "TorrentPath", quazaaSettings.BitTorrent.TorrentPath );
	m_qSettings.setValue( "TrackerKey", quazaaSettings.BitTorrent.TrackerKey );
	m_qSettings.setValue( "UploadCount", quazaaSettings.BitTorrent.UploadCount );
	m_qSettings.setValue( "UseKademlia", quazaaSettings.BitTorrent.UseKademlia );
	m_qSettings.setValue( "UseSaveDialog", quazaaSettings.BitTorrent.UseSaveDialog );
	m_qSettings.setValue( "UseTemp", quazaaSettings.BitTorrent.UseTemp );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Connection" );
	m_qSettings.setValue( "DetectConnectionLoss", quazaaSettings.Connection.DetectConnectionLoss );
	m_qSettings.setValue( "DetectConnectionReset", quazaaSettings.Connection.DetectConnectionReset );
	m_qSettings.setValue( "FailureLimit", quazaaSettings.Connection.FailureLimit );
	m_qSettings.setValue( "FailurePenalty", quazaaSettings.Connection.FailurePenalty );
	m_qSettings.setValue( "InSpeed", quazaaSettings.Connection.InSpeed );
	m_qSettings.setValue( "OutSpeed", quazaaSettings.Connection.OutSpeed );
	m_qSettings.setValue( "Port", quazaaSettings.Connection.Port );
	m_qSettings.setValue( "RandomPort", quazaaSettings.Connection.RandomPort );
	m_qSettings.setValue( "SendBuffer", quazaaSettings.Connection.SendBuffer );
	m_qSettings.setValue( "TimeoutConnect", quazaaSettings.Connection.TimeoutConnect );
	m_qSettings.setValue( "TimeoutTraffic", quazaaSettings.Connection.TimeoutTraffic );
	m_qSettings.setValue( "PreferredCountries", quazaaSettings.Connection.PreferredCountries );
	m_qSettings.setValue( "UDPOutLimitPPS", quazaaSettings.Connection.UDPOutLimitPPS );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Discovery" );
	m_qSettings.setValue( "AccessThrottle",            quazaaSettings.Discovery.AccessThrottle );
	m_qSettings.setValue( "FailureLimit",              quazaaSettings.Discovery.FailureLimit );
	m_qSettings.setValue( "MaximalServiceRating",      quazaaSettings.Discovery.MaximumServiceRating );
	m_qSettings.setValue( "ServiceTimeout",            quazaaSettings.Discovery.ServiceTimeout );
	m_qSettings.setValue( "ZeroRatingRevivalInterval", quazaaSettings.Discovery.ZeroRatingRevivalInterval );
	m_qSettings.setValue( "ZeroRatingRevivalTries",    quazaaSettings.Discovery.ZeroRatingRevivalTries );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Downloads" );
	m_qSettings.setValue( "AllowBackwards", quazaaSettings.Downloads.AllowBackwards );
	m_qSettings.setValue( "AutoClear", quazaaSettings.Downloads.AutoClear );
	m_qSettings.setValue( "BufferSize", quazaaSettings.Downloads.BufferSize );
	m_qSettings.setValue( "ChunkSize", quazaaSettings.Downloads.ChunkSize );
	m_qSettings.setValue( "ChunkStrap", quazaaSettings.Downloads.ChunkStrap );
	m_qSettings.setValue( "ClearDelay", quazaaSettings.Downloads.ClearDelay );
	m_qSettings.setValue( "CompletePath", quazaaSettings.Downloads.CompletePath );
	m_qSettings.setValue( "ConnectThrottle", quazaaSettings.Downloads.ConnectThrottle );
	m_qSettings.setValue( "DropFailedSourcesThreshold", quazaaSettings.Downloads.DropFailedSourcesThreshold );
	m_qSettings.setValue( "ExpandDownloads", quazaaSettings.Downloads.ExpandDownloads );
	m_qSettings.setValue( "FlushSD", quazaaSettings.Downloads.FlushSD );
	m_qSettings.setValue( "IncompletePath", quazaaSettings.Downloads.IncompletePath );
	m_qSettings.setValue( "MaxAllowedFailures", quazaaSettings.Downloads.MaxAllowedFailures );
	m_qSettings.setValue( "MaxConnectingSources", quazaaSettings.Downloads.MaxConnectingSources );
	m_qSettings.setValue( "MaxFiles", quazaaSettings.Downloads.MaxFiles );
	m_qSettings.setValue( "MaxFileSearches", quazaaSettings.Downloads.MaxFileSearches );
	m_qSettings.setValue( "MaxReviews", quazaaSettings.Downloads.MaxReviews );
	m_qSettings.setValue( "MaxTransfers", quazaaSettings.Downloads.MaxTransfers );
	m_qSettings.setValue( "MaxTransfersPerFile", quazaaSettings.Downloads.MaxTransfersPerFile );
	m_qSettings.setValue( "Metadata", quazaaSettings.Downloads.Metadata );
	m_qSettings.setValue( "MinSources", quazaaSettings.Downloads.MinSources );
	m_qSettings.setValue( "NeverDrop", quazaaSettings.Downloads.NeverDrop );
	m_qSettings.setValue( "PushTimeout", quazaaSettings.Downloads.PushTimeout );
	m_qSettings.setValue( "QueueLimit", quazaaSettings.Downloads.QueueLimit );
	m_qSettings.setValue( "RequestHash", quazaaSettings.Downloads.RequestHash );
	m_qSettings.setValue( "RequestHTTP11", quazaaSettings.Downloads.RequestHTTP11 );
	m_qSettings.setValue( "RequestURLENC", quazaaSettings.Downloads.RequestURLENC );
	m_qSettings.setValue( "RetryDelay", quazaaSettings.Downloads.RetryDelay );
	m_qSettings.setValue( "SaveInterval", quazaaSettings.Downloads.SaveInterval );
	m_qSettings.setValue( "SearchPeriod", quazaaSettings.Downloads.SearchPeriod );
	m_qSettings.setValue( "ShowGroups", quazaaSettings.Downloads.ShowGroups );
	m_qSettings.setValue( "ShowMonitorURLs", quazaaSettings.Downloads.ShowMonitorURLs );
	m_qSettings.setValue( "ShowPercent", quazaaSettings.Downloads.ShowPercent );
	m_qSettings.setValue( "ShowSources", quazaaSettings.Downloads.ShowSources );
	m_qSettings.setValue( "SortColumns", quazaaSettings.Downloads.SortColumns );
	m_qSettings.setValue( "SortSources", quazaaSettings.Downloads.SortSources );
	m_qSettings.setValue( "SourcesWanted", quazaaSettings.Downloads.SourcesWanted );
	m_qSettings.setValue( "StaggardStart", quazaaSettings.Downloads.StaggardStart );
	m_qSettings.setValue( "StarveGiveUp", quazaaSettings.Downloads.StarveGiveUp );
	m_qSettings.setValue( "StarveTimeout", quazaaSettings.Downloads.StarveTimeout );
	m_qSettings.setValue( "URIPrompt", quazaaSettings.Downloads.URIPrompt );
	m_qSettings.setValue( "VerifyED2K", quazaaSettings.Downloads.VerifyED2K );
	m_qSettings.setValue( "VerifyFiles", quazaaSettings.Downloads.VerifyFiles );
	m_qSettings.setValue( "VerifyTiger", quazaaSettings.Downloads.VerifyTiger );
	m_qSettings.setValue( "WebHookEnable", quazaaSettings.Downloads.WebHookEnable );
	m_qSettings.setValue( "WebHookExtensions", quazaaSettings.Downloads.WebHookExtensions );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "eDonkey" );
	m_qSettings.setValue( "DefaultServerFlags", quazaaSettings.EDonkey.DefaultServerFlags );
	m_qSettings.setValue( "DequeueTime", quazaaSettings.EDonkey.DequeueTime );
	m_qSettings.setValue( "Enable", quazaaSettings.EDonkey.Enable );
	m_qSettings.setValue( "EnableKad", quazaaSettings.EDonkey.EnableKad );
	m_qSettings.setValue( "Endgame", quazaaSettings.EDonkey.Endgame );
	m_qSettings.setValue( "ExtendedRequest", quazaaSettings.EDonkey.ExtendedRequest );
	m_qSettings.setValue( "FastConnect", quazaaSettings.EDonkey.FastConnect );
	m_qSettings.setValue( "ForceHighID", quazaaSettings.EDonkey.ForceHighID );
	m_qSettings.setValue( "FrameSize", quazaaSettings.EDonkey.FrameSize );
	m_qSettings.setValue( "GetSourcesThrottle", quazaaSettings.EDonkey.GetSourcesThrottle );
	m_qSettings.setValue( "LargeFileSupport", quazaaSettings.EDonkey.LargeFileSupport );
	m_qSettings.setValue( "LearnNewServers", quazaaSettings.EDonkey.LearnNewServers );
	m_qSettings.setValue( "LearnNewServersClient", quazaaSettings.EDonkey.LearnNewServersClient );
	m_qSettings.setValue( "MagnetSearch", quazaaSettings.EDonkey.MagnetSearch );
	m_qSettings.setValue( "MaxClients", quazaaSettings.EDonkey.MaxClients );
	m_qSettings.setValue( "MaxResults", quazaaSettings.EDonkey.MaxResults );
	m_qSettings.setValue( "MaxShareCount", quazaaSettings.EDonkey.MaxShareCount );
	m_qSettings.setValue( "MetAutoQuery", quazaaSettings.EDonkey.MetAutoQuery );
	m_qSettings.setValue( "MinServerFileSize", quazaaSettings.EDonkey.MinServerFileSize );
	m_qSettings.setValue( "NumServers", quazaaSettings.EDonkey.NumServers );
	m_qSettings.setValue( "PacketThrottle", quazaaSettings.EDonkey.PacketThrottle );
	m_qSettings.setValue( "QueryFileThrottle", quazaaSettings.EDonkey.QueryFileThrottle );
	m_qSettings.setValue( "QueryGlobalThrottle", quazaaSettings.EDonkey.QueryGlobalThrottle );
	m_qSettings.setValue( "QueueRankThrottle", quazaaSettings.EDonkey.QueueRankThrottle );
	m_qSettings.setValue( "QueryServerThrottle", quazaaSettings.EDonkey.QueryServerThrottle );
	m_qSettings.setValue( "ReAskTime", quazaaSettings.EDonkey.ReAskTime );
	m_qSettings.setValue( "RequestPipe", quazaaSettings.EDonkey.RequestPipe );
	m_qSettings.setValue( "RequestSize", quazaaSettings.EDonkey.RequestSize );
	m_qSettings.setValue( "SearchCachedServers", quazaaSettings.EDonkey.SearchCachedServers );
	m_qSettings.setValue( "SendPortServer", quazaaSettings.EDonkey.SendPortServer );
	m_qSettings.setValue( "ServerListURL", quazaaSettings.EDonkey.ServerListURL );
	m_qSettings.setValue( "ServerWalk", quazaaSettings.EDonkey.ServerWalk );
	m_qSettings.setValue( "SourceThrottle", quazaaSettings.EDonkey.SourceThrottle );
	m_qSettings.setValue( "StatsGlobalThrottle", quazaaSettings.EDonkey.StatsGlobalThrottle );
	m_qSettings.setValue( "StatsServerThrottle", quazaaSettings.EDonkey.StatsServerThrottle );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Gnutella" );
	m_qSettings.setValue( "ConnectFactor", quazaaSettings.Gnutella.ConnectFactor );
	m_qSettings.setValue( "ConnectThrottle", quazaaSettings.Gnutella.ConnectThrottle );
	m_qSettings.setValue( "HitsPerPacket", quazaaSettings.Gnutella.HitsPerPacket );
	m_qSettings.setValue( "HostCacheSize", quazaaSettings.Gnutella.HostCacheSize );
	m_qSettings.setValue( "HostCacheView", quazaaSettings.Gnutella.HostCacheView );
	m_qSettings.setValue( "MaxHits", quazaaSettings.Gnutella.MaxHits );
	m_qSettings.setValue( "MaxResults", quazaaSettings.Gnutella.MaxResults );
	m_qSettings.setValue( "RouteCache", quazaaSettings.Gnutella.RouteCache );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Gnutella2" );
	m_qSettings.setValue( "ClientMode", quazaaSettings.Gnutella2.ClientMode );
	m_qSettings.setValue( "Enable", quazaaSettings.Gnutella2.Enable );
	m_qSettings.setValue( "HAWPeriod", quazaaSettings.Gnutella2.HAWPeriod );
	m_qSettings.setValue( "HostCount", quazaaSettings.Gnutella2.HostCount );
	m_qSettings.setValue( "HostCurrent", quazaaSettings.Gnutella2.HostCurrent );
	m_qSettings.setValue( "HostExpire", quazaaSettings.Gnutella2.HostExpire );
	m_qSettings.setValue( "HubHorizonSize", quazaaSettings.Gnutella2.HubHorizonSize );
	m_qSettings.setValue( "HubVerified", quazaaSettings.Gnutella2.HubVerified );
	m_qSettings.setValue( "KHLHubCount", quazaaSettings.Gnutella2.KHLHubCount );
	m_qSettings.setValue( "KHLPeriod", quazaaSettings.Gnutella2.KHLPeriod );
	m_qSettings.setValue( "LNIPeriod", quazaaSettings.Gnutella2.LNIMinimumUpdate );
	m_qSettings.setValue( "NumHubs", quazaaSettings.Gnutella2.NumHubs );
	m_qSettings.setValue( "NumLeafs", quazaaSettings.Gnutella2.NumLeafs );
	m_qSettings.setValue( "NumPeers", quazaaSettings.Gnutella2.NumPeers );
	m_qSettings.setValue( "PingRate", quazaaSettings.Gnutella2.PingRate );
	m_qSettings.setValue( "PingTimeout", quazaaSettings.Gnutella2.PingTimeout );
	m_qSettings.setValue( "PingRelayLimit", quazaaSettings.Gnutella2.PingRelayLimit );
	m_qSettings.setValue( "QueryHostDeadline", quazaaSettings.Gnutella2.QueryHostDeadline );
	m_qSettings.setValue( "QueryHostThrottle", quazaaSettings.Gnutella2.QueryHostThrottle );
	m_qSettings.setValue( "QueryKeyTime", quazaaSettings.Gnutella2.QueryKeyTime );
	m_qSettings.setValue( "QueryLimit", quazaaSettings.Gnutella2.QueryLimit );
	m_qSettings.setValue( "RequeryDelay", quazaaSettings.Gnutella2.RequeryDelay );
	m_qSettings.setValue( "UdpBuffers", quazaaSettings.Gnutella2.UdpBuffers );
	m_qSettings.setValue( "UdpInExpire", quazaaSettings.Gnutella2.UdpInExpire );
	m_qSettings.setValue( "UdpInFrames", quazaaSettings.Gnutella2.UdpInFrames );
	m_qSettings.setValue( "UdpOutFrames", quazaaSettings.Gnutella2.UdpOutFrames );
	m_qSettings.setValue( "UdpMTU", quazaaSettings.Gnutella2.UdpMTU );
	m_qSettings.setValue( "UdpOutResend", quazaaSettings.Gnutella2.UdpOutResend );
	m_qSettings.setValue( "HubBalancePeriod", quazaaSettings.Gnutella2.HubBalancePeriod );
	m_qSettings.setValue( "HubBalanceGrace", quazaaSettings.Gnutella2.HubBalanceGrace );
	m_qSettings.setValue( "HubBalanceLow", quazaaSettings.Gnutella2.HubBalanceLow );
	m_qSettings.setValue( "HubBalanceLowTime", quazaaSettings.Gnutella2.HubBalanceLowTime );
	m_qSettings.setValue( "HubBalanceHigh", quazaaSettings.Gnutella2.HubBalanceHigh );
	m_qSettings.setValue( "HubBalanceHighTime", quazaaSettings.Gnutella2.HubBalanceHighTime );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Library" );
	m_qSettings.setValue( "FilterURI", quazaaSettings.Library.FilterURI );
	m_qSettings.setValue( "GhostFiles", quazaaSettings.Library.GhostFiles );
	m_qSettings.setValue( "HashWindow", quazaaSettings.Library.HashWindow );
	m_qSettings.setValue( "HighPriorityHashing", quazaaSettings.Library.HighPriorityHashing );
	m_qSettings.setValue( "HighPriorityHashingSpeed", quazaaSettings.Library.HighPriorityHashingSpeed );
	m_qSettings.setValue( "HistoryDays", quazaaSettings.Library.HistoryDays );
	m_qSettings.setValue( "HistoryTotal", quazaaSettings.Library.HistoryTotal );
	m_qSettings.setValue( "LowPriorityHashingSpeed", quazaaSettings.Library.LowPriorityHashingSpeed );
	m_qSettings.setValue( "NeverShareTypes", quazaaSettings.Library.NeverShareTypes );
	m_qSettings.setValue( "PartialMatch", quazaaSettings.Library.PartialMatch );
	m_qSettings.setValue( "PreferAPETags", quazaaSettings.Library.PreferAPETags );
	m_qSettings.setValue( "QueryRouteSize", quazaaSettings.Library.QueryRouteSize );
	m_qSettings.setValue( "RememberViews", quazaaSettings.Library.RememberViews );
	m_qSettings.setValue( "SafeExecuteTypes", quazaaSettings.Library.SafeExecuteTypes );
	m_qSettings.setValue( "ScanAPE", quazaaSettings.Library.ScanAPE );
	m_qSettings.setValue( "ScanASF", quazaaSettings.Library.ScanASF );
	m_qSettings.setValue( "ScanAVI", quazaaSettings.Library.ScanAVI );
	m_qSettings.setValue( "ScanCHM", quazaaSettings.Library.ScanCHM );
	m_qSettings.setValue( "ScanEXE", quazaaSettings.Library.ScanEXE );
	m_qSettings.setValue( "ScanImage", quazaaSettings.Library.ScanImage );
	m_qSettings.setValue( "ScanMP3", quazaaSettings.Library.ScanMP3 );
	m_qSettings.setValue( "ScanMPC", quazaaSettings.Library.ScanMPC );
	m_qSettings.setValue( "ScanMPEG", quazaaSettings.Library.ScanMPEG );
	m_qSettings.setValue( "ScanMSI", quazaaSettings.Library.ScanMSI );
	m_qSettings.setValue( "ScanOGG", quazaaSettings.Library.ScanOGG );
	m_qSettings.setValue( "ScanPDF", quazaaSettings.Library.ScanPDF );
	m_qSettings.setValue( "SchemaURI", quazaaSettings.Library.SchemaURI );
	quazaaSettings.Library.Shares.removeDuplicates();
	m_qSettings.setValue( "Shares", quazaaSettings.Library.Shares );
	m_qSettings.setValue( "ShowCoverArt", quazaaSettings.Library.ShowCoverArt );
	m_qSettings.setValue( "SmartSeriesDetection", quazaaSettings.Library.SmartSeriesDetection );
	m_qSettings.setValue( "SourceExpire", quazaaSettings.Library.SourceExpire );
	m_qSettings.setValue( "SourceMesh", quazaaSettings.Library.SourceMesh );
	m_qSettings.setValue( "ThumbSize", quazaaSettings.Library.ThumbSize );
	m_qSettings.setValue( "TigerHeight", quazaaSettings.Library.TigerHeight );
	m_qSettings.setValue( "TreeSize", quazaaSettings.Library.TreeSize );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Live" );
	m_qSettings.setValue( "AdultWarning", quazaaSettings.Live.AdultWarning );
	m_qSettings.setValue( "AutoClose", quazaaSettings.Live.AutoClose );
	m_qSettings.setValue( "MonitorThrottle", quazaaSettings.Live.MonitorThrottle );
	m_qSettings.setValue( "DiskSpaceStop", quazaaSettings.Live.DiskSpaceStop );
	m_qSettings.setValue( "DiskSpaceWarning", quazaaSettings.Live.DiskSpaceWarning );
	m_qSettings.setValue( "DiskWriteWarning", quazaaSettings.Live.DiskWriteWarning );
	m_qSettings.setValue( "DonkeyServerWarning", quazaaSettings.Live.DonkeyServerWarning );
	m_qSettings.setValue( "LastDuplicateHash", quazaaSettings.Live.LastDuplicateHash );
	m_qSettings.setValue( "MaliciousWarning", quazaaSettings.Live.MaliciousWarning );
	m_qSettings.setValue( "QueueLimitWarning", quazaaSettings.Live.QueueLimitWarning );
	m_qSettings.setValue( "UploadLimitWarning", quazaaSettings.Live.UploadLimitWarning );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "MediaPlayer" );
	m_qSettings.setValue( "Aspect", quazaaSettings.Media.Aspect );
	m_qSettings.setValue( "AudioVisualPlugin", quazaaSettings.Media.AudioVisualPlugin );
	m_qSettings.setValue( "CustomPlayerPath", quazaaSettings.Media.CustomPlayerPath );
	m_qSettings.setValue( "MediaHandler", quazaaSettings.Media.Handler );
	m_qSettings.setValue( "FileTypes", quazaaSettings.Media.FileTypes );
	m_qSettings.setValue( "ListVisible", quazaaSettings.Media.ListVisible );
	m_qSettings.setValue( "Mute", quazaaSettings.Media.Mute );
	m_qSettings.setValue( "OpenPath", quazaaSettings.Media.OpenPath );
	m_qSettings.setValue( "Playlist", quazaaSettings.Media.Playlist );
	m_qSettings.setValue( "Shuffle", quazaaSettings.Media.Shuffle );
	m_qSettings.setValue( "Repeat", quazaaSettings.Media.Repeat );
	m_qSettings.setValue( "StatusVisible", quazaaSettings.Media.StatusVisible );
	m_qSettings.setValue( "Volume", quazaaSettings.Media.Volume );
	m_qSettings.setValue( "Zoom", quazaaSettings.Media.Zoom );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Parental" );
	m_qSettings.setValue( "AdultFilter", quazaaSettings.Parental.AdultFilter );
	m_qSettings.setValue( "ChatAdultCensor", quazaaSettings.Parental.ChatAdultCensor );
	m_qSettings.setValue( "FilterAdultSearchResults", quazaaSettings.Parental.FilterAdultSearchResults );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "PrivateMessages" );
	m_qSettings.setValue( "AresEnable", quazaaSettings.PrivateMessages.AresEnable );
	m_qSettings.setValue( "AwayMessage", quazaaSettings.PrivateMessages.AwayMessage );
	m_qSettings.setValue( "AwayMessageIdleTime", quazaaSettings.PrivateMessages.AwayMessageIdleTime );
	m_qSettings.setValue( "eDonkeyEnable", quazaaSettings.PrivateMessages.eDonkeyEnable );
	m_qSettings.setValue( "Gnutella2Enable", quazaaSettings.PrivateMessages.Gnutella2Enable );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Scheduler" );

	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Search" );
	m_qSettings.setValue( "BlankSchemaURI", quazaaSettings.Search.BlankSchemaURI );
	m_qSettings.setValue( "BrowseTreeSize", quazaaSettings.Search.BrowseTreeSize );
	m_qSettings.setValue( "ClearPrevious", quazaaSettings.Search.ClearPrevious );
	m_qSettings.setValue( "ExpandSearchMatches", quazaaSettings.Search.ExpandSearchMatches );
	m_qSettings.setValue( "FilterMask", quazaaSettings.Search.FilterMask );
	m_qSettings.setValue( "HighlightNew", quazaaSettings.Search.HighlightNew );
	m_qSettings.setValue( "LastSchemaURI", quazaaSettings.Search.LastSchemaURI );
	m_qSettings.setValue( "MaxPreviewLength", quazaaSettings.Search.MaxPreviewLength );
	m_qSettings.setValue( "MonitorFilter", quazaaSettings.Search.MonitorFilter );
	m_qSettings.setValue( "MonitorQueue", quazaaSettings.Search.MonitorQueue );
	m_qSettings.setValue( "MonitorSchemaURI", quazaaSettings.Search.MonitorSchemaURI );
	m_qSettings.setValue( "SchemaTypes", quazaaSettings.Search.SchemaTypes );
	m_qSettings.setValue( "SearchThrottle", quazaaSettings.Search.Throttle );
	m_qSettings.setValue( "ShareMonkeyBaseURL", quazaaSettings.Search.ShareMonkeyBaseURL );
	m_qSettings.setValue( "ShowNames", quazaaSettings.Search.ShowNames );
	m_qSettings.setValue( "SwitchOnDownload", quazaaSettings.Search.SwitchOnDownload );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Security" );
	m_qSettings.setValue( "AllowProfileBrowse", quazaaSettings.Security.AllowProfileBrowse );
	m_qSettings.setValue( "AllowSharesBrowse", quazaaSettings.Security.AllowSharesBrowse );
	m_qSettings.setValue( "BlockedAgentUploadFilter", quazaaSettings.Security.BlockedAgentUploadFilter );
	m_qSettings.setValue( "DeleteFirewallException", quazaaSettings.Security.DeleteFirewallException );
	m_qSettings.setValue( "DeleteUPnPPorts", quazaaSettings.Security.DeleteUPnPPorts );
	m_qSettings.setValue( "ED2kChatFilter", quazaaSettings.Security.ED2kChatFilter );
	m_qSettings.setValue( "EnableFirewallException", quazaaSettings.Security.EnableFirewallException );
	m_qSettings.setValue( "EnableUPnP", quazaaSettings.Security.EnableUPnP );
	m_qSettings.setValue( "FirewallState", quazaaSettings.Security.FirewallState );
	m_qSettings.setValue( "ChatFilter", quazaaSettings.Security.ChatFilter );
	m_qSettings.setValue( "IrcFloodLimit", quazaaSettings.Security.IrcFloodLimit );
	m_qSettings.setValue( "IrcFloodProtection", quazaaSettings.Security.IrcFloodProtection );
	//m_qSettings.setValue("MaxMaliciousFileSize", quazaaSettings.Security.MaxMaliciousFileSize);
	m_qSettings.setValue( "RemoteEnable", quazaaSettings.Security.RemoteEnable );
	m_qSettings.setValue( "RemotePassword", quazaaSettings.Security.RemotePassword );
	m_qSettings.setValue( "RemoteUsername", quazaaSettings.Security.RemoteUsername );
	m_qSettings.setValue( "SearchIgnoreLocalIP", quazaaSettings.Security.IgnorePrivateIP );
	m_qSettings.setValue( "SearchIgnoreOwnIP", quazaaSettings.Security.SearchIgnoreOwnIP );
	m_qSettings.setValue( "SearchSpamFilterThreshold", quazaaSettings.Security.SearchSpamFilterThreshold );
	m_qSettings.setValue( "UPnPSkipWANIPSetup", quazaaSettings.Security.UPnPSkipWANIPSetup );
	m_qSettings.setValue( "UPnPSkipWANPPPSetup", quazaaSettings.Security.UPnPSkipWANPPPSetup );

	m_qSettings.setValue( "LogIPCheckHits", quazaaSettings.Security.LogIPCheckHits );
	m_qSettings.setValue( "RuleExpiryInterval", quazaaSettings.Security.RuleExpiryInterval );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "System" );
	m_qSettings.setValue( "CloseMode", quazaaSettings.System.CloseMode );
	m_qSettings.setValue( "ConnectOnStartup", quazaaSettings.System.ConnectOnStartup );
	m_qSettings.setValue( "DiskSpaceStop", quazaaSettings.System.DiskSpaceStop );
	m_qSettings.setValue( "DiskSpaceWarning", quazaaSettings.System.DiskSpaceWarning );
	m_qSettings.setValue( "MinimizeToTray", quazaaSettings.System.MinimizeToTray );
	m_qSettings.setValue( "StartWithSystem", quazaaSettings.System.StartWithSystem );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Transfers" );
	m_qSettings.setValue( "BandwidthHubIn", quazaaSettings.Transfers.BandwidthHubIn );
	m_qSettings.setValue( "BandwidthHubOut", quazaaSettings.Transfers.BandwidthHubOut );
	m_qSettings.setValue( "BandwidthHubUploads", quazaaSettings.Transfers.BandwidthHubUploads );
	m_qSettings.setValue( "BandwidthLeafIn", quazaaSettings.Transfers.BandwidthLeafIn );
	m_qSettings.setValue( "BandwidthLeafOut", quazaaSettings.Transfers.BandwidthLeafOut );
	m_qSettings.setValue( "BandwidthPeerIn", quazaaSettings.Transfers.BandwidthPeerIn );
	m_qSettings.setValue( "BandwidthPeerOut", quazaaSettings.Transfers.BandwidthPeerOut );
	m_qSettings.setValue( "BandwidthRequest", quazaaSettings.Transfers.BandwidthRequest );
	m_qSettings.setValue( "BandwidthUdpOut", quazaaSettings.Transfers.BandwidthUdpOut );
	m_qSettings.setValue( "MinTransfersRest", quazaaSettings.Transfers.MinTransfersRest );
	m_qSettings.setValue( "RequireConnectedNetwork", quazaaSettings.Transfers.RequireConnectedNetwork );
	m_qSettings.setValue( "SimpleProgressBar", quazaaSettings.Transfers.SimpleProgressBar );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Uploads" );
	m_qSettings.setValue( "AllowBackwards", quazaaSettings.Uploads.AllowBackwards );
	m_qSettings.setValue( "AutoClear", quazaaSettings.Uploads.AutoClear );
	m_qSettings.setValue( "ClampdownFactor", quazaaSettings.Uploads.ClampdownFactor );
	m_qSettings.setValue( "ClampdownFloor", quazaaSettings.Uploads.ClampdownFloor );
	m_qSettings.setValue( "ClearDelay", quazaaSettings.Uploads.ClearDelay );
	m_qSettings.setValue( "DynamicPreviews", quazaaSettings.Uploads.DynamicPreviews );
	m_qSettings.setValue( "FreeBandwidthFactor", quazaaSettings.Uploads.FreeBandwidthFactor );
	m_qSettings.setValue( "FreeBandwidthValue", quazaaSettings.Uploads.FreeBandwidthValue );
	m_qSettings.setValue( "HubShareLimiting", quazaaSettings.Uploads.HubShareLimiting );
	m_qSettings.setValue( "MaxPerHost", quazaaSettings.Uploads.MaxPerHost );
	m_qSettings.setValue( "PreviewQuality", quazaaSettings.Uploads.PreviewQuality );
	m_qSettings.setValue( "PreviewTransfers", quazaaSettings.Uploads.PreviewTransfers );
	m_qSettings.setValue( "QueuePollMax", quazaaSettings.Uploads.QueuePollMax );
	m_qSettings.setValue( "QueuePollMin", quazaaSettings.Uploads.QueuePollMin );
	m_qSettings.setValue( "RewardQueuePercentage", quazaaSettings.Uploads.RewardQueuePercentage );
	m_qSettings.setValue( "RotateChunkLimit", quazaaSettings.Uploads.RotateChunkLimit );
	m_qSettings.setValue( "ShareHashset", quazaaSettings.Uploads.ShareHashset );
	m_qSettings.setValue( "ShareMetadata", quazaaSettings.Uploads.ShareMetadata );
	m_qSettings.setValue( "SharePartials", quazaaSettings.Uploads.SharePartials );
	m_qSettings.setValue( "SharePreviews", quazaaSettings.Uploads.SharePreviews );
	m_qSettings.setValue( "ShareTiger", quazaaSettings.Uploads.ShareTiger );
	m_qSettings.setValue( "ThrottleMode", quazaaSettings.Uploads.ThrottleMode );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Web" );
	m_qSettings.setValue( "Ares", quazaaSettings.Web.Ares );
	m_qSettings.setValue( "BrowserIntegration", quazaaSettings.Web.BrowserIntegration );
	m_qSettings.setValue( "ED2K", quazaaSettings.Web.ED2K );
	m_qSettings.setValue( "Foxy", quazaaSettings.Web.Foxy );
	m_qSettings.setValue( "Magnet", quazaaSettings.Web.Magnet );
	m_qSettings.setValue( "ManageDownloadTypes", quazaaSettings.Web.ManageDownloadTypes );
	m_qSettings.setValue( "Piolet", quazaaSettings.Web.Piolet );
	m_qSettings.setValue( "Torrent", quazaaSettings.Web.Torrent );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "WebServices" );
	m_qSettings.setValue( "BitziAgent", quazaaSettings.WebServices.BitziAgent );
	m_qSettings.setValue( "BitziOkay", quazaaSettings.WebServices.BitziOkay );
	m_qSettings.setValue( "BitziWebSubmit", quazaaSettings.WebServices.BitziWebSubmit );
	m_qSettings.setValue( "BitziWebView", quazaaSettings.WebServices.BitziWebView );
	m_qSettings.setValue( "BitziXML", quazaaSettings.WebServices.BitziXML );
	m_qSettings.setValue( "ShareMonkeyCid", quazaaSettings.WebServices.ShareMonkeyCid );
	m_qSettings.setValue( "ShareMonkeyOkay", quazaaSettings.WebServices.ShareMonkeyOkay );
	m_qSettings.setValue( "ShareMonkeySaveThumbnail", quazaaSettings.WebServices.ShareMonkeySaveThumbnail );
	m_qSettings.endGroup();
}

/*!
	Loads most of Quazaa's settings from persistent .ini file.
 */
void QuazaaSettings::loadSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Ares" );
	quazaaSettings.Ares.Enable = m_qSettings.value( "Enable", true ).toBool();
	quazaaSettings.Ares.MaximumDownloads = m_qSettings.value( "MaximumDownloads", 10 ).toInt();
	quazaaSettings.Ares.MaximumUploads = m_qSettings.value( "MaximumUploads", 6 ).toInt();
	quazaaSettings.Ares.MaximumUploadsPerUser = m_qSettings.value( "MaximumUploadsPerUser", 3 ).toInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "BitTorrent" );
	quazaaSettings.BitTorrent.AutoClear = m_qSettings.value( "AutoClear", false ).toBool();
	quazaaSettings.BitTorrent.AutoSeed = m_qSettings.value( "AutoSeed", true ).toBool();
	quazaaSettings.BitTorrent.BandwidthPercentage = m_qSettings.value( "BandwidthPercentage", 80 ).toInt();
	quazaaSettings.BitTorrent.ClearRatio = m_qSettings.value( "ClearRatio", 120 ).toInt();
	quazaaSettings.BitTorrent.CodePage = m_qSettings.value( "CodePage", 0 ).toInt();
	quazaaSettings.BitTorrent.DefaultTracker = m_qSettings.value( "DefaultTracker", "" ).toString();
	quazaaSettings.BitTorrent.DefaultTrackerPeriod = m_qSettings.value( "DefaultTrackerPeriod", 5 ).toInt();
	quazaaSettings.BitTorrent.DhtPruneTime = m_qSettings.value( "DhtPruneTime", 30 ).toInt();
	quazaaSettings.BitTorrent.DownloadConnections = m_qSettings.value( "DownloadConnections", 40 ).toInt();
	quazaaSettings.BitTorrent.DownloadTorrents = m_qSettings.value( "DownloadTorrents", 4 ).toInt();
	quazaaSettings.BitTorrent.Endgame = m_qSettings.value( "Endgame", true ).toBool();
	quazaaSettings.BitTorrent.ExtraKeys = m_qSettings.value( "ExtraKeys", true ).toBool();
	quazaaSettings.BitTorrent.LinkPing = m_qSettings.value( "LinkPing", 120 ).toInt();
	quazaaSettings.BitTorrent.LinkTimeout = m_qSettings.value( "LinkTimeout", 180 ).toInt();
	quazaaSettings.BitTorrent.Managed = m_qSettings.value( "Managed", true ).toBool();
	quazaaSettings.BitTorrent.PreferBTSources = m_qSettings.value( "PreferBTSources", true ).toBool();
	quazaaSettings.BitTorrent.RandomPeriod = m_qSettings.value( "RandomPeriod", 30 ).toInt();
	quazaaSettings.BitTorrent.RequestLimit = m_qSettings.value( "RequestLimit", 128 ).toInt();
	quazaaSettings.BitTorrent.RequestPipe = m_qSettings.value( "RequestPipe", 4 ).toInt();
	quazaaSettings.BitTorrent.RequestSize = m_qSettings.value( "RequestSize", 16 ).toInt();
	quazaaSettings.BitTorrent.ShowFilesInDownload = m_qSettings.value( "ShowFilesInDownload", true ).toBool();
	quazaaSettings.BitTorrent.SourceExchangePeriod = m_qSettings.value( "SourceExchangePeriod", 10 ).toInt();
	quazaaSettings.BitTorrent.StartPaused = m_qSettings.value( "StartPaused", false ).toBool();
	quazaaSettings.BitTorrent.TestPartials = m_qSettings.value( "TestPartials", true ).toBool();
	quazaaSettings.BitTorrent.TorrentPath = m_qSettings.value( "TorrentPath",
															   QuazaaGlobals::STORAGE_PATH() + "Torrents" ).toString();
	quazaaSettings.BitTorrent.TrackerKey = m_qSettings.value( "TrackerKey", true ).toBool();
	quazaaSettings.BitTorrent.UploadCount = m_qSettings.value( "UploadCount", 4 ).toInt();
	quazaaSettings.BitTorrent.UseKademlia = m_qSettings.value( "UseKademlia", true ).toBool();
	quazaaSettings.BitTorrent.UseSaveDialog = m_qSettings.value( "UseSaveDialog", false ).toBool();
	quazaaSettings.BitTorrent.UseTemp = m_qSettings.value( "UseTemp", false ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Connection" );
	quazaaSettings.Connection.DetectConnectionLoss = m_qSettings.value( "DetectConnectionLoss", true ).toBool();
	quazaaSettings.Connection.DetectConnectionReset = m_qSettings.value( "DetectConnectionReset", false ).toBool();
	quazaaSettings.Connection.FailureLimit = m_qSettings.value( "FailureLimit", 3 ).toUInt();
	quazaaSettings.Connection.FailurePenalty = m_qSettings.value( "FailurePenalty", 300 ).toUInt();
	quazaaSettings.Connection.InSpeed = m_qSettings.value( "InSpeed", 1024 * 1024 ).toULongLong(); // 1Mbit
	quazaaSettings.Connection.OutSpeed = m_qSettings.value( "OutSpeed", 16384 ).toULongLong();	 // 16KB/s
	quazaaSettings.Connection.Port = m_qSettings.value( "Port", 6350 ).toUInt();
	quazaaSettings.Connection.RandomPort = m_qSettings.value( "RandomPort", false ).toBool();
	quazaaSettings.Connection.SendBuffer = m_qSettings.value( "SendBuffer", 2048 ).toUInt();
	quazaaSettings.Connection.TimeoutConnect = m_qSettings.value( "TimeoutConnect", 16 ).toUInt();
	quazaaSettings.Connection.TimeoutTraffic = m_qSettings.value( "TimeoutTraffic", 60 ).toUInt();
	quazaaSettings.Connection.PreferredCountries = m_qSettings.value( "PreferredCountries", QStringList() ).toStringList();
	quazaaSettings.Connection.UDPOutLimitPPS = m_qSettings.value( "UDPOutLimitPPS", 128 ).toUInt();
	if ( quazaaSettings.Connection.UDPOutLimitPPS < 10 )
	{
		quazaaSettings.Connection.UDPOutLimitPPS = 10;    // failsafe
	}
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Discovery" );
	quazaaSettings.Discovery.AccessThrottle            = m_qSettings.value( "AccessThrottle", 60 ).toUInt();
	quazaaSettings.Discovery.FailureLimit              = m_qSettings.value( "FailureLimit", 2 ).toUInt();
	quazaaSettings.Discovery.MaximumServiceRating      = m_qSettings.value( "MaximalServiceRating", 10 ).toUInt();
	quazaaSettings.Discovery.ServiceTimeout            = m_qSettings.value( "ServiceTimeout", 10 ).toUInt();
	quazaaSettings.Discovery.ZeroRatingRevivalInterval = m_qSettings.value( "ZeroRatingRevivalInterval",
																			60 * 24 * 10 ).toUInt();
	quazaaSettings.Discovery.ZeroRatingRevivalTries    = m_qSettings.value( "ZeroRatingRevivalTries", 2 ).toUInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Downloads" );
	quazaaSettings.Downloads.AllowBackwards = m_qSettings.value( "AllowBackwards", true ).toBool();
	quazaaSettings.Downloads.AutoClear = m_qSettings.value( "AutoClear", false ).toBool();
	quazaaSettings.Downloads.BufferSize = m_qSettings.value( "BufferSize", 81920 ).toInt();
	quazaaSettings.Downloads.ChunkSize = m_qSettings.value( "ChunkSize", 524288 ).toInt();
	quazaaSettings.Downloads.ChunkStrap = m_qSettings.value( "ChunkStrap", 131072 ).toInt();
	quazaaSettings.Downloads.ClearDelay = m_qSettings.value( "ClearDelay", 30000 ).toInt();
	quazaaSettings.Downloads.CompletePath = m_qSettings.value( "CompletePath",
															   QStandardPaths::writableLocation( QStandardPaths::DownloadLocation ) ).toString();
	quazaaSettings.Downloads.ConnectThrottle = m_qSettings.value( "ConnectThrottle", 800 ).toInt();
	quazaaSettings.Downloads.DropFailedSourcesThreshold = m_qSettings.value( "DropFailedSourcesThreshold", 20 ).toInt();
	quazaaSettings.Downloads.ExpandDownloads = m_qSettings.value( "ExpandDownloads", false ).toBool();
	quazaaSettings.Downloads.FlushSD = m_qSettings.value( "FlushSD", true ).toBool();
	quazaaSettings.Downloads.IncompletePath = m_qSettings.value( "IncompletePath",
																 QuazaaGlobals::STORAGE_PATH() + "Incomplete" ).toString();
	quazaaSettings.Downloads.MaxAllowedFailures = m_qSettings.value( "MaxAllowedFailures", 10 ).toInt();
	quazaaSettings.Downloads.MaxConnectingSources = m_qSettings.value( "MaxConnectingSources", 8 ).toInt();
	quazaaSettings.Downloads.MaxFiles = m_qSettings.value( "MaxFiles", 26 ).toInt();
	quazaaSettings.Downloads.MaxFileSearches = m_qSettings.value( "MaxFileSearches", 2 ).toInt();
	quazaaSettings.Downloads.MaxReviews = m_qSettings.value( "MaxReviews", 64 ).toInt();
	quazaaSettings.Downloads.MaxTransfers = m_qSettings.value( "MaxTransfers", 100 ).toInt();
	quazaaSettings.Downloads.MaxTransfersPerFile = m_qSettings.value( "MaxTransfersPerFile", 10 ).toInt();
	quazaaSettings.Downloads.Metadata = m_qSettings.value( "Metadata", true ).toBool();
	quazaaSettings.Downloads.MinSources = m_qSettings.value( "MinSources", 1 ).toInt();
	quazaaSettings.Downloads.NeverDrop = m_qSettings.value( "NeverDrop", false ).toBool();
	quazaaSettings.Downloads.PushTimeout = m_qSettings.value( "PushTimeout", 45000 ).toInt();
	quazaaSettings.Downloads.QueueLimit = m_qSettings.value( "QueueLimit", 3 ).toInt();
	quazaaSettings.Downloads.RequestHash = m_qSettings.value( "RequestHash", true ).toBool();
	quazaaSettings.Downloads.RequestHTTP11 = m_qSettings.value( "RequestHTTP11", true ).toBool();
	quazaaSettings.Downloads.RequestURLENC = m_qSettings.value( "RequestURLENC", true ).toBool();
	quazaaSettings.Downloads.RetryDelay = m_qSettings.value( "RetryDelay", 600000 ).toUInt();
	quazaaSettings.Downloads.SaveInterval = m_qSettings.value( "SaveInterval", 60000 ).toUInt();
	quazaaSettings.Downloads.SearchPeriod = m_qSettings.value( "SearchPeriod", 120000 ).toInt();
	quazaaSettings.Downloads.ShowPercent = m_qSettings.value( "ShowPercent", false ).toBool();
	quazaaSettings.Downloads.ShowSources = m_qSettings.value( "ShowSources", false ).toBool();
	quazaaSettings.Downloads.SortColumns = m_qSettings.value( "SortColumns", true ).toBool();
	quazaaSettings.Downloads.SortSources = m_qSettings.value( "SortSources", true ).toBool();
	quazaaSettings.Downloads.SourcesWanted = m_qSettings.value( "SourcesWanted", 500 ).toInt();
	quazaaSettings.Downloads.StaggardStart = m_qSettings.value( "StaggardStart", false ).toBool();
	quazaaSettings.Downloads.StarveGiveUp = m_qSettings.value( "StarveGiveUp", 3 ).toInt();
	quazaaSettings.Downloads.StarveTimeout = m_qSettings.value( "StarveTimeout", 2700 ).toInt();
	quazaaSettings.Downloads.URIPrompt = m_qSettings.value( "URIPrompt", true ).toBool();
	quazaaSettings.Downloads.VerifyED2K = m_qSettings.value( "VerifyED2K", true ).toBool();
	quazaaSettings.Downloads.VerifyFiles = m_qSettings.value( "VerifyFiles", true ).toBool();
	quazaaSettings.Downloads.VerifyTiger = m_qSettings.value( "VerifyTiger", true ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "eDonkey" );
	quazaaSettings.EDonkey.DefaultServerFlags = m_qSettings.value( "DefaultServerFlags", "ffffffff" ).toString();
	quazaaSettings.EDonkey.DequeueTime = m_qSettings.value( "DequeueTime", 60 ).toInt();
	quazaaSettings.EDonkey.Enable = m_qSettings.value( "Enable", false ).toBool();
	quazaaSettings.EDonkey.EnableKad = m_qSettings.value( "EnableKad", false ).toBool();
	quazaaSettings.EDonkey.Endgame = m_qSettings.value( "Endgame", true ).toBool();
	quazaaSettings.EDonkey.ExtendedRequest = m_qSettings.value( "ExtendedRequest", 2 ).toInt();
	quazaaSettings.EDonkey.FastConnect = m_qSettings.value( "FastConnect", false ).toBool();
	quazaaSettings.EDonkey.ForceHighID = m_qSettings.value( "ForceHighID", true ).toBool();
	quazaaSettings.EDonkey.FrameSize = m_qSettings.value( "FrameSize", 10 ).toInt();
	quazaaSettings.EDonkey.GetSourcesThrottle = m_qSettings.value( "GetSourcesThrottle", 8 ).toInt();
	quazaaSettings.EDonkey.LargeFileSupport = m_qSettings.value( "LargeFileSupport", false ).toBool();
	quazaaSettings.EDonkey.LearnNewServers = m_qSettings.value( "LearnNewServers", false ).toBool();
	quazaaSettings.EDonkey.LearnNewServersClient = m_qSettings.value( "LearnNewServersClient", false ).toBool();
	quazaaSettings.EDonkey.MagnetSearch = m_qSettings.value( "MagnetSearch", true ).toBool();
	quazaaSettings.EDonkey.MaxClients = m_qSettings.value( "MaxClients", 35 ).toInt();
	quazaaSettings.EDonkey.MaxResults = m_qSettings.value( "MaxResults", 100 ).toInt();
	quazaaSettings.EDonkey.MaxShareCount = m_qSettings.value( "MaxShareCount", 1000 ).toInt();
	quazaaSettings.EDonkey.MetAutoQuery = m_qSettings.value( "MetAutoQuery", true ).toBool();
	quazaaSettings.EDonkey.MinServerFileSize = m_qSettings.value( "MinServerFileSize", 0 ).toInt();
	quazaaSettings.EDonkey.NumServers = m_qSettings.value( "NumServers", 1 ).toInt();
	quazaaSettings.EDonkey.PacketThrottle = m_qSettings.value( "PacketThrottle", 500 ).toInt();
	quazaaSettings.EDonkey.QueryFileThrottle = m_qSettings.value( "QueryFileThrottle", 60 ).toInt();
	quazaaSettings.EDonkey.QueryGlobalThrottle = m_qSettings.value( "QueryGlobalThrottle", 1000 ).toInt();
	quazaaSettings.EDonkey.QueueRankThrottle = m_qSettings.value( "QueueRankThrottle", 120 ).toInt();
	quazaaSettings.EDonkey.QueryServerThrottle = m_qSettings.value( "QueryServerThrottle", 120 ).toInt();
	quazaaSettings.EDonkey.ReAskTime = m_qSettings.value( "ReAskTime", 29 ).toInt();
	quazaaSettings.EDonkey.RequestPipe = m_qSettings.value( "RequestPipe", 3 ).toInt();
	quazaaSettings.EDonkey.RequestSize = m_qSettings.value( "RequestSize", 90 ).toInt();
	quazaaSettings.EDonkey.SearchCachedServers = m_qSettings.value( "SearchCachedServers", true ).toBool();
	quazaaSettings.EDonkey.SendPortServer = m_qSettings.value( "SendPortServer", false ).toBool();
	quazaaSettings.EDonkey.ServerListURL = m_qSettings.value( "ServerListURL",
															  "http://ocbmaurice.dyndns.org/pl/slist.pl/server.met?download/server-best.met" ).toString();
	quazaaSettings.EDonkey.ServerWalk = m_qSettings.value( "ServerWalk", true ).toBool();
	quazaaSettings.EDonkey.SourceThrottle = m_qSettings.value( "SourceThrottle", 1000 ).toInt();
	quazaaSettings.EDonkey.StatsGlobalThrottle = m_qSettings.value( "StatsGlobalThrottle", 30 ).toInt();
	quazaaSettings.EDonkey.StatsServerThrottle = m_qSettings.value( "StatsServerThrottle", 7 ).toInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Gnutella" );
	quazaaSettings.Gnutella.ConnectFactor = m_qSettings.value( "ConnectFactor", 3 ).toInt();
	quazaaSettings.Gnutella.ConnectThrottle = m_qSettings.value( "ConnectThrottle", 120 ).toUInt();
	quazaaSettings.Gnutella.HitsPerPacket = m_qSettings.value( "HitsPerPacket", 64 ).toInt();
	quazaaSettings.Gnutella.HostCacheSize = m_qSettings.value( "HostCacheSize", 2048 ).toUInt();
	quazaaSettings.Gnutella.HostCacheView = m_qSettings.value( "HostCacheView", 3 ).toInt();
	quazaaSettings.Gnutella.MaxHits = m_qSettings.value( "MaxHits", 64 ).toInt();
	quazaaSettings.Gnutella.MaxResults = m_qSettings.value( "MaxResults", 150 ).toInt();
	quazaaSettings.Gnutella.RouteCache = m_qSettings.value( "RouteCache", 10 ).toInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Gnutella2" );
	quazaaSettings.Gnutella2.ClientMode = m_qSettings.value( "ClientMode", 0 ).toInt();
	quazaaSettings.Gnutella2.Enable = m_qSettings.value( "Enable", true ).toBool();
	quazaaSettings.Gnutella2.HAWPeriod = m_qSettings.value( "HAWPeriod", 300 ).toInt();
	quazaaSettings.Gnutella2.HostCount = m_qSettings.value( "HostCount", 15 ).toInt();
	quazaaSettings.Gnutella2.HostCurrent = m_qSettings.value( "HostCurrent", 600 ).toUInt();
	quazaaSettings.Gnutella2.HostExpire = m_qSettings.value( "HostExpire", 172800 ).toInt();
	quazaaSettings.Gnutella2.HubHorizonSize = m_qSettings.value( "HubHorizonSize", 128 ).toInt();
	quazaaSettings.Gnutella2.HubVerified = m_qSettings.value( "HubVerified", false ).toBool();
	quazaaSettings.Gnutella2.KHLHubCount = m_qSettings.value( "KHLHubCount", 50 ).toInt();
	quazaaSettings.Gnutella2.KHLPeriod = m_qSettings.value( "KHLPeriod", 60 ).toInt();
	quazaaSettings.Gnutella2.LNIMinimumUpdate = m_qSettings.value( "LNIMinimumUpdate", 60 ).toInt();
	quazaaSettings.Gnutella2.NumHubs = m_qSettings.value( "NumHubs", 3 ).toInt();
	quazaaSettings.Gnutella2.NumLeafs = m_qSettings.value( "NumLeafs", 300 ).toInt();
	quazaaSettings.Gnutella2.NumPeers = m_qSettings.value( "NumPeers", 6 ).toInt();
	quazaaSettings.Gnutella2.PingRate = m_qSettings.value( "PingRate", 120 ).toInt();
	quazaaSettings.Gnutella2.PingTimeout = m_qSettings.value( "PingTimeout", 120 ).toUInt();
	quazaaSettings.Gnutella2.PingRelayLimit = m_qSettings.value( "PingRelayLimit", 10 ).toInt();
	quazaaSettings.Gnutella2.QueryHostDeadline = m_qSettings.value( "QueryHostDeadline", 600 ).toInt();
	quazaaSettings.Gnutella2.QueryHostThrottle = m_qSettings.value( "QueryHostThrottle", 120 ).toInt();
	quazaaSettings.Gnutella2.QueryKeyTime = m_qSettings.value( "QueryKeyTime", 7200 ).toUInt(); // 2h
	quazaaSettings.Gnutella2.QueryLimit = m_qSettings.value( "QueryLimit", 2400 ).toInt();
	quazaaSettings.Gnutella2.RequeryDelay = m_qSettings.value( "RequeryDelay", 1800 ).toInt();
	quazaaSettings.Gnutella2.UdpBuffers = m_qSettings.value( "UdpBuffers",
															 1024 ).toInt(); // fragmented packets use one buffer / fragment
	quazaaSettings.Gnutella2.UdpInExpire = m_qSettings.value( "UdpInExpire", 30 ).toInt();
	quazaaSettings.Gnutella2.UdpInFrames = m_qSettings.value( "UdpInFrames", 256 ).toInt();
	quazaaSettings.Gnutella2.UdpMTU = m_qSettings.value( "UdpMTU", 500 ).toInt();
	quazaaSettings.Gnutella2.UdpOutExpire = m_qSettings.value( "UdpOutExpire", 26 ).toInt();
	quazaaSettings.Gnutella2.UdpOutFrames = m_qSettings.value( "UdpOutFrames", 512 ).toInt();
	quazaaSettings.Gnutella2.UdpOutResend = m_qSettings.value( "UdpOutResend", 6 ).toInt();
	quazaaSettings.Gnutella2.HubBalancePeriod = m_qSettings.value( "HubBalancePeriod", 60 ).toUInt();
	quazaaSettings.Gnutella2.HubBalanceGrace = m_qSettings.value( "HubBalanceGrace", 3600 ).toUInt();
	quazaaSettings.Gnutella2.HubBalanceLow = m_qSettings.value( "HubBalanceLow", 50 ).toUInt();
	quazaaSettings.Gnutella2.HubBalanceLowTime = m_qSettings.value( "HubBalanceLowTime", 30 ).toUInt();
	quazaaSettings.Gnutella2.HubBalanceHigh = m_qSettings.value( "HubBalanceHigh", 90 ).toUInt();
	quazaaSettings.Gnutella2.HubBalanceHighTime = m_qSettings.value( "HubBalanceHighTime", 30 ).toUInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Library" );
	quazaaSettings.Library.FilterURI = m_qSettings.value( "FilterURI", "" ).toString();
	quazaaSettings.Library.GhostFiles = m_qSettings.value( "GhostFiles", true ).toBool();
	quazaaSettings.Library.HashWindow = m_qSettings.value( "HashWindow", true ).toBool();
	quazaaSettings.Library.HighPriorityHashing = m_qSettings.value( "HighPriorityHashing", false ).toInt();
	quazaaSettings.Library.HighPriorityHashingSpeed = m_qSettings.value( "HighPriorityHashingSpeed", 20 ).toBool();
	quazaaSettings.Library.HistoryDays = m_qSettings.value( "HistoryDays", 3 ).toInt();
	quazaaSettings.Library.HistoryTotal = m_qSettings.value( "HistoryTotal", 32 ).toInt();
	quazaaSettings.Library.LowPriorityHashingSpeed = m_qSettings.value( "LowPriorityHashingSpeed", 2 ).toInt();
	quazaaSettings.Library.NeverShareTypes = m_qSettings.value( "NeverShareTypes",
																QStringList() << "vbs" << "js" << "jc!" << "fb!" << "bc!" << "!ut"
																<< "dbx" << "part" << "partial" << "pst" << "reget" << "getright"
																<< "pif" << "lnk" << "sd" << "url" << "wab" << "m4p" << "infodb"
																<< "racestats" << "chk" << "tmp" << "temp" << "ini" << "inf" << "log"
																<< "old" << "manifest" << "met" << "bak" << "$$$" << "---" << "~~~"
																<< "###" << "__incomplete___" << "wma" << "wmv" ).toStringList();
	quazaaSettings.Library.PartialMatch = m_qSettings.value( "PartialMatch", true ).toBool();
	quazaaSettings.Library.PreferAPETags = m_qSettings.value( "PreferAPETags", true ).toBool();
	quazaaSettings.Library.QueryRouteSize = m_qSettings.value( "QueryRouteSize", 20 ).toInt();
	quazaaSettings.Library.RememberViews = m_qSettings.value( "RememberViews", true ).toBool();
	quazaaSettings.Library.SafeExecuteTypes = m_qSettings.value( "SafeExecuteTypes",
																 QStringList() << "3gp" << "7z" << "aac" << "ace" << "ape" << "avi"
																 << "bmp" << "co" << "collection" << "flv" << "gif" << "iso"
																 << "jpg" << "jpeg" << "lit" << "mid" << "mov" << "m1v" << "m2v"
																 << "m3u" << "m4a" << "mkv" << "mp2" << "mp3" << "mp4" << "mpa"
																 << "mpe" << "mpg" << "mpeg" << "ogg" << "ogm" << "pdf" << "png"
																 << "qt" << "rar" << "rm" << "sks" << "tar" << "tgz" << "torrent"
																 << "txt" << "wav" << "zip" ).toStringList();
	quazaaSettings.Library.ScanAPE = m_qSettings.value( "ScanAPE", true ).toBool();
	quazaaSettings.Library.ScanASF = m_qSettings.value( "ScanASF", true ).toBool();
	quazaaSettings.Library.ScanAVI = m_qSettings.value( "ScanAVI", true ).toBool();
	quazaaSettings.Library.ScanCHM = m_qSettings.value( "ScanCHM", true ).toBool();
	quazaaSettings.Library.ScanEXE = m_qSettings.value( "ScanEXE", true ).toBool();
	quazaaSettings.Library.ScanImage = m_qSettings.value( "ScanImage", true ).toBool();
	quazaaSettings.Library.ScanMP3 = m_qSettings.value( "ScanMP3", true ).toBool();
	quazaaSettings.Library.ScanMPC = m_qSettings.value( "ScanMPC", true ).toBool();
	quazaaSettings.Library.ScanMPEG = m_qSettings.value( "ScanMPEG", true ).toBool();
	quazaaSettings.Library.ScanMSI = m_qSettings.value( "ScanMSI", true ).toBool();
	quazaaSettings.Library.ScanOGG = m_qSettings.value( "ScanOGG", true ).toBool();
	quazaaSettings.Library.ScanPDF = m_qSettings.value( "ScanPDF", true ).toBool();
	quazaaSettings.Library.SchemaURI = m_qSettings.value( "SchemaURI",
														  "http://www.limewire.com/schemas/audio.xsd" ).toString();
	quazaaSettings.Library.Shares = m_qSettings.value( "Shares", QStringList() << quazaaSettings.Downloads.CompletePath
													   << quazaaSettings.BitTorrent.TorrentPath ).toStringList();
	quazaaSettings.Library.ShowCoverArt = m_qSettings.value( "ShowCoverArt", true ).toBool();
	quazaaSettings.Library.SmartSeriesDetection = m_qSettings.value( "SmartSeriesDetection", false ).toBool();
	quazaaSettings.Library.SourceExpire = m_qSettings.value( "SourceExpire", 86400 ).toInt();
	quazaaSettings.Library.SourceMesh = m_qSettings.value( "SourceMesh", true ).toBool();
	quazaaSettings.Library.ThumbSize = m_qSettings.value( "ThumbSize", 96 ).toInt();
	quazaaSettings.Library.TigerHeight = m_qSettings.value( "TigerHeight", 9 ).toInt();
	quazaaSettings.Library.TreeSize = m_qSettings.value( "TreeSize", 200 ).toInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Live" );
	quazaaSettings.Live.AdultWarning = m_qSettings.value( "AdultWarning", false ).toBool();
	quazaaSettings.Live.AutoClose = m_qSettings.value( "AutoClose", false ).toBool();
	quazaaSettings.Live.MonitorThrottle = m_qSettings.value( "MonitorThrottle", 100 ).toInt();
	quazaaSettings.Live.DiskSpaceStop = m_qSettings.value( "DiskSpaceStop", false ).toBool();
	quazaaSettings.Live.DiskSpaceWarning = m_qSettings.value( "DiskSpaceWarning", false ).toBool();
	quazaaSettings.Live.DiskWriteWarning = m_qSettings.value( "DiskWriteWarning", false ).toBool();
	quazaaSettings.Live.DonkeyServerWarning = m_qSettings.value( "DonkeyServerWarning", false ).toBool();
	quazaaSettings.Live.LastDuplicateHash = m_qSettings.value( "LastDuplicateHash", "" ).toString();
	quazaaSettings.Live.MaliciousWarning = m_qSettings.value( "MaliciousWarning", false ).toBool();
	quazaaSettings.Live.QueueLimitWarning = m_qSettings.value( "QueueLimitWarning", false ).toBool();
	quazaaSettings.Live.UploadLimitWarning = m_qSettings.value( "UploadLimitWarning", false ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "MediaPlayer" );
	quazaaSettings.Media.Aspect = m_qSettings.value( "Aspect", 0 ).toInt();
	quazaaSettings.Media.AudioVisualPlugin = m_qSettings.value( "AudioVisualPlugin", "" ).toString();
	quazaaSettings.Media.CustomPlayerPath = m_qSettings.value( "CustomPlayerPath", "" ).toString();
	quazaaSettings.Media.Handler = m_qSettings.value( "MediaHandler", 0 ).toInt();
	quazaaSettings.Media.FileTypes = m_qSettings.value( "FileTypes",
														QStringList() << "asx" << "wax" << "m3u" << "wvx" << "wmx"
														<< "asf" << "wav" << "snd" << "au" << "aif" << "aifc" << "aiff" << "mp3"
														<< "cda" << "mid" << "rmi" << "midi" << "avi" << "mpeg" << "mpg"
														<< "m1v" << "mp2" << "mpa" << "mpe" ).toStringList();
	quazaaSettings.Media.ListVisible = m_qSettings.value( "ListVisible", true ).toBool();
	quazaaSettings.Media.Mute = m_qSettings.value( "Mute", false ).toBool();
	quazaaSettings.Media.OpenPath = m_qSettings.value( "OpenPath", quazaaSettings.Downloads.CompletePath ).toString();
	quazaaSettings.Media.Playlist = m_qSettings.value( "Playlist", QStringList() ).toStringList();
	quazaaSettings.Media.Shuffle = m_qSettings.value( "Shuffle", false ).toBool();
	quazaaSettings.Media.Repeat = m_qSettings.value( "Repeat", false ).toBool();
	quazaaSettings.Media.StatusVisible = m_qSettings.value( "StatusVisible", true ).toBool();
	quazaaSettings.Media.Volume = m_qSettings.value( "Volume", 1.0 ).toReal();
	quazaaSettings.Media.Zoom = m_qSettings.value( "Zoom", 0 ).toInt();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Parental" );
	quazaaSettings.Parental.AdultFilter = m_qSettings.value( "AdultFilter",
															 QStringList() << "shit" << "fuck" << "damn" << "bitch"
															 << "asshole" << "ass" << "pussy" << "cock" << "cunt" << "dick"
															 << "whore" << "slut" << "clit" ).toStringList();
	quazaaSettings.Parental.ChatAdultCensor = m_qSettings.value( "ChatAdultCensor", true ).toBool();
	quazaaSettings.Parental.FilterAdultSearchResults = m_qSettings.value( "FilterAdultSearchResults", true ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "PrivateMessages" );
	quazaaSettings.PrivateMessages.AresEnable = m_qSettings.value( "AresEnable", true ).toBool();
	quazaaSettings.PrivateMessages.AwayMessage = m_qSettings.value( "AwayMessage",
																	"This Quazaa user is currently away from their computer." ).toBool();
	quazaaSettings.PrivateMessages.AwayMessageIdleTime = m_qSettings.value( "AwayMessageIdleTime", 30 ).toInt();
	quazaaSettings.PrivateMessages.eDonkeyEnable = m_qSettings.value( "eDonkeyEnable", false ).toBool();
	quazaaSettings.PrivateMessages.Gnutella2Enable = m_qSettings.value( "Gnutella2Enable", false ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Search" );
	quazaaSettings.Search.BlankSchemaURI = m_qSettings.value( "BlankSchemaURI", "" ).toString();
	quazaaSettings.Search.BrowseTreeSize = m_qSettings.value( "BrowseTreeSize", 180 ).toInt();
	quazaaSettings.Search.ClearPrevious = m_qSettings.value( "ClearPrevious", 0 ).toInt();
	quazaaSettings.Search.ExpandSearchMatches = m_qSettings.value( "ExpandSearchMatches", false ).toBool();
	quazaaSettings.Search.FilterMask = m_qSettings.value( "FilterMask", 360 ).toInt();
	quazaaSettings.Search.HighlightNew = m_qSettings.value( "HighlightNew", true ).toBool();
	quazaaSettings.Search.LastSchemaURI = m_qSettings.value( "LastSchemaURI", "" ).toString();
	quazaaSettings.Search.MaxPreviewLength = m_qSettings.value( "MaxPreviewLength", 20480 ).toInt();
	quazaaSettings.Search.MonitorFilter = m_qSettings.value( "MonitorFilter", "" ).toString();
	quazaaSettings.Search.MonitorQueue = m_qSettings.value( "MonitorQueue", 128 ).toInt();
	quazaaSettings.Search.MonitorSchemaURI = m_qSettings.value( "MonitorSchemaURI",
																"http://www.limewire.com/schemas/audio.xsd" ).toString();
	quazaaSettings.Search.SchemaTypes = m_qSettings.value( "SchemaTypes", true ).toBool();
	quazaaSettings.Search.Throttle = m_qSettings.value( "SearchThrottle", 200 ).toInt();
	quazaaSettings.Search.ShareMonkeyBaseURL = m_qSettings.value( "ShareMonkeyBaseURL",
																  "http://tools.sharemonkey.com/xml/" ).toString();
	quazaaSettings.Search.ShowNames = m_qSettings.value( "ShowNames", true ).toBool();
	quazaaSettings.Search.SwitchOnDownload = m_qSettings.value( "SwitchOnDownload", true ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Security" );
	quazaaSettings.Security.AllowProfileBrowse = m_qSettings.value( "AllowProfileBrowse", true ).toBool();
	quazaaSettings.Security.AllowSharesBrowse = m_qSettings.value( "AllowSharesBrowse", true ).toBool();
	quazaaSettings.Security.BlockedAgentUploadFilter = m_qSettings.value( "BlockedAgentUploadFilter",
																		  QStringList() << "Mozilla" ).toStringList();
	quazaaSettings.Security.DeleteFirewallException = m_qSettings.value( "DeleteFirewallException", true ).toBool();
	quazaaSettings.Security.DeleteUPnPPorts = m_qSettings.value( "DeleteUPnPPorts", true ).toBool();
	quazaaSettings.Security.ED2kChatFilter = m_qSettings.value( "ED2kChatFilter", true ).toBool();
	quazaaSettings.Security.EnableFirewallException = m_qSettings.value( "EnableFirewallException", true ).toBool();
	quazaaSettings.Security.EnableUPnP = m_qSettings.value( "EnableUPnP", true ).toBool();
	quazaaSettings.Security.FirewallState = m_qSettings.value( "FirewallState", 0 ).toInt();
	quazaaSettings.Security.ChatFilter = m_qSettings.value( "ChatFilter", true ).toBool();
	quazaaSettings.Security.IrcFloodLimit = m_qSettings.value( "IrcFloodLimit", 24 ).toInt();
	quazaaSettings.Security.IrcFloodProtection = m_qSettings.value( "IrcFloodProtection", true ).toBool();
	quazaaSettings.Security.RemoteEnable = m_qSettings.value( "RemoteEnable", false ).toBool();
	quazaaSettings.Security.RemotePassword = m_qSettings.value( "RemotePassword", "" ).toString();
	quazaaSettings.Security.RemoteUsername = m_qSettings.value( "RemoteUsername", "" ).toString();
	quazaaSettings.Security.IgnorePrivateIP = m_qSettings.value( "SearchIgnoreLocalIP", true ).toBool();
	quazaaSettings.Security.SearchIgnoreOwnIP = m_qSettings.value( "SearchIgnoreOwnIP", true ).toBool();
	quazaaSettings.Security.SearchSpamFilterThreshold = m_qSettings.value( "SearchSpamFilterThreshold", 20 ).toInt();
	quazaaSettings.Security.UPnPSkipWANIPSetup = m_qSettings.value( "UPnPSkipWANIPSetup", false ).toBool();
	quazaaSettings.Security.UPnPSkipWANPPPSetup = m_qSettings.value( "UPnPSkipWANPPPSetup", false ).toBool();
	quazaaSettings.Security.LogIPCheckHits = m_qSettings.value( "LogIPCheckHits", false ).toBool();
	quazaaSettings.Security.RuleExpiryInterval = m_qSettings.value( "RuleExpiryInterval", 0 ).toUInt();
	m_qSettings.endGroup();

	// notify Security manager about new settings
	emit securitySettingsChanged();

	m_qSettings.beginGroup( "Scheduler" );

	m_qSettings.endGroup();

	m_qSettings.beginGroup( "System" );
	quazaaSettings.System.CloseMode = m_qSettings.value( "CloseMode", 0 ).toInt();
	quazaaSettings.System.ConnectOnStartup = m_qSettings.value( "ConnectOnStartup", true ).toBool();
	quazaaSettings.System.DiskSpaceStop = m_qSettings.value( "DiskSpaceStop", 25 ).toInt();
	quazaaSettings.System.DiskSpaceWarning = m_qSettings.value( "DiskSpaceWarning", 500 ).toInt();
	quazaaSettings.System.MinimizeToTray = m_qSettings.value( "MinimizeToTray", false ).toBool();
	quazaaSettings.System.StartWithSystem = m_qSettings.value( "StartWithSystem", false ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Transfers" );
	quazaaSettings.Transfers.BandwidthHubIn = m_qSettings.value( "HubIn", 0 ).toInt();
	quazaaSettings.Transfers.BandwidthHubOut = m_qSettings.value( "HubOut", 0 ).toInt();
	quazaaSettings.Transfers.BandwidthHubUploads = m_qSettings.value( "HubUploads", 40 ).toInt();
	quazaaSettings.Transfers.BandwidthLeafIn = m_qSettings.value( "LeafIn", 0 ).toInt();
	quazaaSettings.Transfers.BandwidthLeafOut = m_qSettings.value( "LeafOut", 0 ).toInt();
	quazaaSettings.Transfers.BandwidthPeerIn = m_qSettings.value( "PeerIn", 0 ).toInt();
	quazaaSettings.Transfers.BandwidthPeerOut = m_qSettings.value( "PeerOut", 0 ).toInt();
	quazaaSettings.Transfers.BandwidthRequest = m_qSettings.value( "Request", 32 ).toInt();
	quazaaSettings.Transfers.BandwidthUdpOut = m_qSettings.value( "UdpOut", 0 ).toInt();
	quazaaSettings.Transfers.MinTransfersRest = m_qSettings.value( "MinTransfersRest", 15 ).toInt();
	quazaaSettings.Transfers.RequireConnectedNetwork = m_qSettings.value( "RequireConnectedNetwork", true ).toBool();
	quazaaSettings.Transfers.SimpleProgressBar = m_qSettings.value( "SimpleProgressBar", false ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Uploads" );
	quazaaSettings.Uploads.AllowBackwards = m_qSettings.value( "AllowBackwards", true ).toBool();
	quazaaSettings.Uploads.AutoClear = m_qSettings.value( "AutoClear", false ).toBool();
	quazaaSettings.Uploads.ClampdownFactor = m_qSettings.value( "ClampdownFactor", 20 ).toInt();
	quazaaSettings.Uploads.ClampdownFloor = m_qSettings.value( "SharePartials", 8 ).toInt();
	quazaaSettings.Uploads.ClearDelay = m_qSettings.value( "ClearDelay", 30 ).toInt();
	quazaaSettings.Uploads.DynamicPreviews = m_qSettings.value( "DynamicPreviews", true ).toBool();
	quazaaSettings.Uploads.FreeBandwidthFactor = m_qSettings.value( "FreeBandwidthFactor", 15 ).toInt();
	quazaaSettings.Uploads.FreeBandwidthValue = m_qSettings.value( "FreeBandwidthValue", 20 ).toInt();
	quazaaSettings.Uploads.HubShareLimiting = m_qSettings.value( "HubShareLimiting", true ).toBool();
	quazaaSettings.Uploads.MaxPerHost = m_qSettings.value( "MaxPerHost", 2 ).toInt();
	quazaaSettings.Uploads.PreviewQuality = m_qSettings.value( "PreviewQuality", 70 ).toInt();
	quazaaSettings.Uploads.PreviewTransfers = m_qSettings.value( "PreviewTransfers", 3 ).toInt();
	quazaaSettings.Uploads.QueuePollMax = m_qSettings.value( "QueuePollMax", 120000 ).toInt();
	quazaaSettings.Uploads.QueuePollMin = m_qSettings.value( "QueuePollMin", 45000 ).toInt();
	quazaaSettings.Uploads.RewardQueuePercentage = m_qSettings.value( "RewardQueuePercentage", 10 ).toInt();
	quazaaSettings.Uploads.RotateChunkLimit = m_qSettings.value( "RotateChunkLimit", 1024 ).toInt();
	quazaaSettings.Uploads.ShareHashset = m_qSettings.value( "ShareHashset", true ).toBool();
	quazaaSettings.Uploads.ShareMetadata = m_qSettings.value( "ShareMetadata", true ).toBool();
	quazaaSettings.Uploads.SharePartials = m_qSettings.value( "SharePartials", true ).toBool();
	quazaaSettings.Uploads.SharePreviews = m_qSettings.value( "SharePreviews", true ).toBool();
	quazaaSettings.Uploads.ShareTiger = m_qSettings.value( "ShareTiger", true ).toBool();
	quazaaSettings.Uploads.ThrottleMode = m_qSettings.value( "ThrottleMode", true ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "Web" );
	quazaaSettings.Web.BrowserIntegration = m_qSettings.value( "BrowserIntegration", false ).toBool();
	quazaaSettings.Web.Ares = m_qSettings.value( "Ares", true ).toBool();
	quazaaSettings.Web.ED2K = m_qSettings.value( "ED2K", true ).toBool();
	quazaaSettings.Web.Foxy = m_qSettings.value( "Foxy", true ).toBool();
	quazaaSettings.Web.Magnet = m_qSettings.value( "Magnet", true ).toBool();
	quazaaSettings.Web.ManageDownloadTypes = m_qSettings.value( "ManageDownloadTypes",
																QStringList() << "7z" << "ace" << "arj"
																<< "bin" << "exe" << "fml" << "grs" << "gz" << "hqx" << "iso"
																<< "lzh" << "mp3" << "msi" << "r0" << "rar" << "sit" << "tar"
																<< "tgz" << "z" << "zip" ).toStringList();
	quazaaSettings.Web.Piolet = m_qSettings.value( "Piolet", true ).toBool();
	quazaaSettings.Web.Torrent = m_qSettings.value( "Torrent", true ).toBool();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "WebServices" );
	quazaaSettings.WebServices.BitziAgent = m_qSettings.value( "BitziAgent", "." ).toString();
	quazaaSettings.WebServices.BitziOkay = m_qSettings.value( "BitziOkay", false ).toBool();
	quazaaSettings.WebServices.BitziWebSubmit = m_qSettings.value( "BitziWebSubmit",
																   "http://bitzi.com/lookup/(SHA1).(TTH)?fl=(SIZE)&ff=(FIRST20)&fn=(NAME)&tag.ed2k.ed2khash=(ED2K)&(INFO)&a=(AGENT)&v=Q0.4&ref=quazaa" ).toString();
	quazaaSettings.WebServices.BitziWebView = m_qSettings.value( "BitziWebView",
																 "http://bitzi.com/lookup/(URN)?v=detail&ref=quazaa" ).toString();
	quazaaSettings.WebServices.BitziXML = m_qSettings.value( "BitziXML",
															 "http://ticket.bitzi.com/rdf/(SHA1).(TTH)" ).toString();
	quazaaSettings.WebServices.ShareMonkeyCid = m_qSettings.value( "ShareMonkeyCid", "197506" ).toString();
	quazaaSettings.WebServices.ShareMonkeyOkay = m_qSettings.value( "ShareMonkeyOkay", false ).toBool();
	quazaaSettings.WebServices.ShareMonkeySaveThumbnail = m_qSettings.value( "ShareMonkeySaveThumbnail", false ).toBool();
	m_qSettings.endGroup();
}

/*!
	Saves the IRC chat connections to persistent .ini file.
 */
void QuazaaSettings::saveChatConnections()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Chat" );
	m_qSettings.setValue( "Connections", quazaaSettings.Chat.Connections );
	m_qSettings.endGroup();
}

/*!
	Loads the IRC chat connections from persistent .ini file.
 */
void QuazaaSettings::loadChatConnections()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Chat" );
	quazaaSettings.Chat.Connections = m_qSettings.value( "Connections" );
	m_qSettings.endGroup();
}

/*!
	Saves the IRC chat connection dialog's autocomplete lists to persistent .ini file.
 */
void QuazaaSettings::saveChatConnectionWizard()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Chat" );
	m_qSettings.setValue( "Hosts", quazaaSettings.Chat.Hosts );
	m_qSettings.setValue( "NickNames", quazaaSettings.Chat.NickNames );
	m_qSettings.setValue( "RealNames", quazaaSettings.Chat.RealNames );
	m_qSettings.setValue( "UserNames", quazaaSettings.Chat.UserNames );
	m_qSettings.setValue( "ConnectionNames", quazaaSettings.Chat.ConnectionNames );
	m_qSettings.endGroup();
}

/*!
	Loads the IRC chat connection dialog's autocomplete lists from persistent .ini file.
 */
void QuazaaSettings::loadChatConnectionWizard()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Chat" );
	quazaaSettings.Chat.Hosts = m_qSettings.value( "Hosts", QStringList() << "irc.paradoxirc.net" << "us.paradoxirc.net"
												   << "eu.paradoxirc.net" << "irc.p2pchat.net" << "irc.gigirc.com"
												   << "irc.freenode.net" << "irc.undernet.org" << "irc.quakenet.org"
												   << "irc.uni-erlangen.de" << "irc.servercentral.net" << "irc.dal.net" ).toStringList();
	quazaaSettings.Chat.NickNames = m_qSettings.value( "NickNames", QStringList() ).toStringList();
	quazaaSettings.Chat.RealNames = m_qSettings.value( "RealNames", QStringList() ).toStringList();
	quazaaSettings.Chat.UserNames = m_qSettings.value( "UserNames", QStringList() ).toStringList();
	quazaaSettings.Chat.ConnectionNames = m_qSettings.value( "ConnectionNames", QStringList() ).toStringList();
	m_qSettings.endGroup();
}

/*!
	Saves the IRC chat settings to persistent .ini file.
 */
void QuazaaSettings::saveChat()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Chat" );
	m_qSettings.setValue( "Font", quazaaSettings.Chat.Font );
	m_qSettings.setValue( "ConnectOnStartup", quazaaSettings.Chat.ConnectOnStartup );
	m_qSettings.setValue( "EnableFileTransfers", quazaaSettings.Chat.EnableFileTransfers );
	m_qSettings.setValue( "ShowTimestamp", quazaaSettings.Chat.ShowTimestamp );
	m_qSettings.setValue( "TimeStampFormat", quazaaSettings.Chat.TimestampFormat );
	m_qSettings.setValue( "MaxBlockCount", quazaaSettings.Chat.Scrollback );
	m_qSettings.setValue( "StripNicks", quazaaSettings.Chat.StripNicks );
	m_qSettings.setValue( "HighlightSounds", quazaaSettings.Chat.HighlightSounds );
	m_qSettings.setValue( "DarkTheme", quazaaSettings.Chat.DarkTheme );

	m_qSettings.setValue( "ShortcutNextView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::NextView] );
	m_qSettings.setValue( "ShortcutPreviousView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::PreviousView] );
	m_qSettings.setValue( "ShortcutNextActiveView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::NextActiveView] );
	m_qSettings.setValue( "ShortcutPreviousActiveView",
						  quazaaSettings.Chat.Shortcuts[IrcShortcutType::PreviousActiveView] );
	m_qSettings.setValue( "ShortcutMostActiveView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::MostActiveView] );
	m_qSettings.setValue( "ShortcutExpandView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::ExpandView] );
	m_qSettings.setValue( "ShortcutCollapseView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::CollapseView] );
	m_qSettings.setValue( "ShortcutSearchView", quazaaSettings.Chat.Shortcuts[IrcShortcutType::SearchView] );
	m_qSettings.setValue( "ShortcutResetViews", quazaaSettings.Chat.Shortcuts[IrcShortcutType::ResetViews] );

	m_qSettings.setValue( "MessagesJoins", quazaaSettings.Chat.Messages[IrcMessageType::Joins] );
	m_qSettings.setValue( "MessagesParts", quazaaSettings.Chat.Messages[IrcMessageType::Parts] );
	m_qSettings.setValue( "MessagesNicks", quazaaSettings.Chat.Messages[IrcMessageType::Nicks] );
	m_qSettings.setValue( "MessagesModes", quazaaSettings.Chat.Messages[IrcMessageType::Modes] );
	m_qSettings.setValue( "MessagesKicks", quazaaSettings.Chat.Messages[IrcMessageType::Kicks] );
	m_qSettings.setValue( "MessagesQuits", quazaaSettings.Chat.Messages[IrcMessageType::Quits] );
	m_qSettings.setValue( "MessagesTopics", quazaaSettings.Chat.Messages[IrcMessageType::Topics] );

	m_qSettings.setValue( "HighlightsJoins", quazaaSettings.Chat.Highlights[IrcMessageType::Joins] );
	m_qSettings.setValue( "HighlightsParts", quazaaSettings.Chat.Highlights[IrcMessageType::Parts] );
	m_qSettings.setValue( "HighlightsNicks", quazaaSettings.Chat.Highlights[IrcMessageType::Nicks] );
	m_qSettings.setValue( "HighlightsModes", quazaaSettings.Chat.Highlights[IrcMessageType::Modes] );
	m_qSettings.setValue( "HighlightsKicks", quazaaSettings.Chat.Highlights[IrcMessageType::Kicks] );
	m_qSettings.setValue( "HighlightsQuits", quazaaSettings.Chat.Highlights[IrcMessageType::Quits] );
	m_qSettings.setValue( "HighlightsTopics", quazaaSettings.Chat.Highlights[IrcMessageType::Topics] );

	// TODO: the default values should respect palette
	m_qSettings.setValue( "ColorsBackground", quazaaSettings.Chat.Colors[IrcColorType::Background] );
	m_qSettings.setValue( "ColorsDefault", quazaaSettings.Chat.Colors[IrcColorType::Default] );
	m_qSettings.setValue( "ColorsEvent", quazaaSettings.Chat.Colors[IrcColorType::Event] );
	m_qSettings.setValue( "ColorsNotice", quazaaSettings.Chat.Colors[IrcColorType::Notice] );
	m_qSettings.setValue( "ColorsAction", quazaaSettings.Chat.Colors[IrcColorType::Action] );
	m_qSettings.setValue( "ColorsInactive", quazaaSettings.Chat.Colors[IrcColorType::Inactive] );
	m_qSettings.setValue( "ColorsAlert", quazaaSettings.Chat.Colors[IrcColorType::Alert] );
	m_qSettings.setValue( "ColorsHighlight", quazaaSettings.Chat.Colors[IrcColorType::Highlight] );
	m_qSettings.setValue( "ColorsTimestamp", quazaaSettings.Chat.Colors[IrcColorType::TimeStamp] );
	m_qSettings.setValue( "ColorsLink", quazaaSettings.Chat.Colors[IrcColorType::Link] );
	m_qSettings.setValue( "ColorsWhite", quazaaSettings.Chat.Colors[IrcColorType::White] );
	m_qSettings.setValue( "ColorsBlack", quazaaSettings.Chat.Colors[IrcColorType::Black] );
	m_qSettings.setValue( "ColorsBlue", quazaaSettings.Chat.Colors[IrcColorType::Blue] );
	m_qSettings.setValue( "ColorsGreen", quazaaSettings.Chat.Colors[IrcColorType::Green] );
	m_qSettings.setValue( "ColorsRed", quazaaSettings.Chat.Colors[IrcColorType::Red] );
	m_qSettings.setValue( "ColorsBrown", quazaaSettings.Chat.Colors[IrcColorType::Brown] );
	m_qSettings.setValue( "ColorsPurple", quazaaSettings.Chat.Colors[IrcColorType::Purple] );
	m_qSettings.setValue( "ColorsOrange", quazaaSettings.Chat.Colors[IrcColorType::Orange] );
	m_qSettings.setValue( "ColorsYellow", quazaaSettings.Chat.Colors[IrcColorType::Yellow] );
	m_qSettings.setValue( "ColorsLightGreen", quazaaSettings.Chat.Colors[IrcColorType::LightGreen] );
	m_qSettings.setValue( "ColorsCyan", quazaaSettings.Chat.Colors[IrcColorType::Cyan] );
	m_qSettings.setValue( "ColorsLightCyan", quazaaSettings.Chat.Colors[IrcColorType::LightCyan] );
	m_qSettings.setValue( "ColorsLightBlue", quazaaSettings.Chat.Colors[IrcColorType::LightBlue] );
	m_qSettings.setValue( "ColorsPink", quazaaSettings.Chat.Colors[IrcColorType::Pink] );
	m_qSettings.setValue( "ColorsGray", quazaaSettings.Chat.Colors[IrcColorType::Gray] );
	m_qSettings.setValue( "ColorsLightGray", quazaaSettings.Chat.Colors[IrcColorType::LightGray] );
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "ChatAliases" );
	QMap<QString, QString>::const_iterator i = quazaaSettings.Chat.Aliases.constBegin();
	while ( i != quazaaSettings.Chat.Aliases.constEnd() )
	{
		m_qSettings.setValue( i.key(), i.value() );
		++i;
	}
	m_qSettings.endGroup();

	emit chatSettingsChanged();
}

/*!
	Loads the IRC chat settings from persistent .ini file.
 */
void QuazaaSettings::loadChat()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Chat" );

	quazaaSettings.Chat.Font = m_qSettings.value( "Font", QFont() ).value<QFont>();
	quazaaSettings.Chat.ConnectOnStartup = m_qSettings.value( "ConnectOnStartup", false ).toBool();
	quazaaSettings.Chat.EnableFileTransfers = m_qSettings.value( "EnableFileTransfers", true ).toBool();
	quazaaSettings.Chat.ShowTimestamp = m_qSettings.value( "ShowTimestamp", false ).toBool();
	quazaaSettings.Chat.TimestampFormat = m_qSettings.value( "TimeStampFormat", "[hh:mm:ss]" ).toString();
	quazaaSettings.Chat.Scrollback = m_qSettings.value( "MaxBlockCount", -1 ).toInt();
	quazaaSettings.Chat.StripNicks = m_qSettings.value( "StripNicks", false ).toBool();
	quazaaSettings.Chat.HighlightSounds = m_qSettings.value( "HighlightSounds", true ).toBool();
	quazaaSettings.Chat.DarkTheme = m_qSettings.value( "DarkTheme", false ).toBool();

#ifdef Q_OS_MAC
	QString navigate( "Ctrl+Alt+%1" );
	QString nextActive( "Shift+Ctrl+Alt+%1" );
#else
	QString navigate( "Alt+%1" );
	QString nextActive( "Shift+Alt+%1" );
#endif

	quazaaSettings.Chat.Shortcuts[IrcShortcutType::PreviousView] = m_qSettings.value( "ShortcutPreviousView",
																					  navigate.arg( "Up" ) ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::NextView] = m_qSettings.value( "ShortcutNextView",
																				  navigate.arg( "Down" ) ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::CollapseView] = m_qSettings.value( "ShortcutCollapseView",
																					  navigate.arg( "Left" ) ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::ExpandView] = m_qSettings.value( "ShortcutExpandView",
																					navigate.arg( "Right" ) ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::PreviousActiveView] = m_qSettings.value( "ShortcutPreviousActiveView",
																							nextActive.arg( "Up" ) ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::NextActiveView] = m_qSettings.value( "ShortcutNextActiveView",
																						nextActive.arg( "Down" ) ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::MostActiveView] = m_qSettings.value( "ShortcutMostActiveView",
																						"Ctrl+L" ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::SearchView] = m_qSettings.value( "ShortcutSearchView",
																					"Ctrl+S" ).toString();
	quazaaSettings.Chat.Shortcuts[IrcShortcutType::ResetViews] = m_qSettings.value( "ShortcutResetViews",
																					"Ctrl+R" ).toString();

	quazaaSettings.Chat.Messages[IrcMessageType::Joins] =  m_qSettings.value( "MessagesJoins", true ).toBool();
	quazaaSettings.Chat.Messages[IrcMessageType::Parts] = m_qSettings.value( "MessagesParts", true ).toBool();
	quazaaSettings.Chat.Messages[IrcMessageType::Nicks] = m_qSettings.value( "MessagesNicks", true ).toBool();
	quazaaSettings.Chat.Messages[IrcMessageType::Modes] = m_qSettings.value( "MessagesModes", true ).toBool();
	quazaaSettings.Chat.Messages[IrcMessageType::Kicks] = m_qSettings.value( "MessagesKicks", true ).toBool();
	quazaaSettings.Chat.Messages[IrcMessageType::Quits] = m_qSettings.value( "MessagesQuits", true ).toBool();
	quazaaSettings.Chat.Messages[IrcMessageType::Topics] = m_qSettings.value( "MessagesTopics", true ).toBool();

	quazaaSettings.Chat.Highlights[IrcMessageType::Joins] = m_qSettings.value( "HighlightsJoins", false ).toBool();
	quazaaSettings.Chat.Highlights[IrcMessageType::Parts] = m_qSettings.value( "HighlightsParts", false ).toBool();
	quazaaSettings.Chat.Highlights[IrcMessageType::Nicks] = m_qSettings.value( "HighlightsNicks", false ).toBool();
	quazaaSettings.Chat.Highlights[IrcMessageType::Modes] = m_qSettings.value( "HighlightsModes", false ).toBool();
	quazaaSettings.Chat.Highlights[IrcMessageType::Kicks] = m_qSettings.value( "HighlightsKicks", false ).toBool();
	quazaaSettings.Chat.Highlights[IrcMessageType::Quits] = m_qSettings.value( "HighlightsQuits", false ).toBool();
	quazaaSettings.Chat.Highlights[IrcMessageType::Topics] = m_qSettings.value( "HighlightsTopics", false ).toBool();

	// TODO: the default values should respect palette
	quazaaSettings.Chat.Colors[IrcColorType::Background] = m_qSettings.value( "ColorsBackground", "white" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Default] = m_qSettings.value( "ColorsDefault", "black" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Event] = m_qSettings.value( "ColorsEvent", "chocolate" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Notice] = m_qSettings.value( "ColorsNotice", "indianred" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Action] = m_qSettings.value( "ColorsAction", "darkmagenta" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Inactive] = m_qSettings.value( "ColorsInactive", "gray" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Alert] = m_qSettings.value( "ColorsAlert", "red" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Highlight] = m_qSettings.value( "ColorsHighlight", "maroon" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::TimeStamp] = m_qSettings.value( "ColorsTimeStamp", "gray" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Link] = m_qSettings.value( "ColorsLink", "dodgerblue" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::White] = m_qSettings.value( "ColorsWhite", "white" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Black] = m_qSettings.value( "ColorsBlack", "black" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Blue] = m_qSettings.value( "ColorsBlue", "blue" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Green] = m_qSettings.value( "ColorsGreen", "green" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Red] = m_qSettings.value( "ColorsRed", "red" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Brown] = m_qSettings.value( "ColorsBrown", "brown" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Purple] = m_qSettings.value( "ColorsPurple", "purple" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Orange] = m_qSettings.value( "ColorsOrange", "orange" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Yellow] = m_qSettings.value( "ColorsYellow", "yellow" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::LightGreen] = m_qSettings.value( "ColorsLightGreen", "lightgreen" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Cyan] = m_qSettings.value( "ColorsCyan", "cyan" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::LightCyan] = m_qSettings.value( "ColorsLightCyan", "lightcyan" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::LightBlue] = m_qSettings.value( "ColorsLightBlue", "lightblue" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Pink] = m_qSettings.value( "ColorsPink", "pink" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::Gray] = m_qSettings.value( "ColorsGray", "gray" ).toString();
	quazaaSettings.Chat.Colors[IrcColorType::LightGray] = m_qSettings.value( "ColorsLightGray", "lightgray" ).toString();
	m_qSettings.endGroup();

	m_qSettings.beginGroup( "ChatAliases" );
	QStringList keys = m_qSettings.childKeys();
	foreach ( const QString & key, keys )
	{
		quazaaSettings.Chat.Aliases[key] = m_qSettings.value( key ).toString();
	}

	if ( quazaaSettings.Chat.Aliases.isEmpty() )
	{
		quazaaSettings.Chat.Aliases["Q"] = "QUERY $*";
		quazaaSettings.Chat.Aliases["M"] = "MSG $*";
		quazaaSettings.Chat.Aliases["TELL"] = "MSG $*";
		quazaaSettings.Chat.Aliases["J"] = "JOIN $*";
		quazaaSettings.Chat.Aliases["P"] = "PART $*";
		quazaaSettings.Chat.Aliases["LEAVE"] = "PART $*";
		quazaaSettings.Chat.Aliases["BS"] = "MSG BotServ $*";
		quazaaSettings.Chat.Aliases["BOTSERV"] = "MSG BotServ $*";
		quazaaSettings.Chat.Aliases["CS"] = "MSG ChanServ $*";
		quazaaSettings.Chat.Aliases["CHANSERV"] = "MSG ChanServ $*";
		quazaaSettings.Chat.Aliases["HS"] = "MSG HostServ $*";
		quazaaSettings.Chat.Aliases["HOSTSERV"] = "MSG HostServ $*";
		quazaaSettings.Chat.Aliases["MS"] = "MSG MemoServ $*";
		quazaaSettings.Chat.Aliases["MEMOSERV"] = "MSG MemoServ $*";
		quazaaSettings.Chat.Aliases["NS"] = "MSG NickServ $*";
		quazaaSettings.Chat.Aliases["NICKSERV"] = "MSG NickServ $*";
		quazaaSettings.Chat.Aliases["OS"] = "MSG OperServ $*";
		quazaaSettings.Chat.Aliases["OPERSERV"] = "MSG OperServ $*";
	}
	m_qSettings.endGroup();
}

/*!
	Saves the profile settings to persistent .ini file.
 */
void QuazaaSettings::saveProfile()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Profile" );
	m_qSettings.setValue( "Age", quazaaSettings.Profile.Age );
	m_qSettings.setValue( "AolScreenName", quazaaSettings.Profile.AolScreenName );             // Aol screen name
	m_qSettings.setValue( "AvatarPath",
						  quazaaSettings.Profile.AvatarPath );                   // Path to an avatar image file. Quazaa logo used if blank
	m_qSettings.setValue( "Biography", quazaaSettings.Profile.Biography );                     // Your life story..
	m_qSettings.setValue( "City", quazaaSettings.Profile.City );                               // Your city
	m_qSettings.setValue( "Country", quazaaSettings.Profile.Country );                         // Your country..
	m_qSettings.setValue( "Email", quazaaSettings.Profile.Email );                             // EMail address
	m_qSettings.setValue( "Favorites", quazaaSettings.Profile.Favorites );                     // Favorite websites
	m_qSettings.setValue( "FavoritesURL", quazaaSettings.Profile.FavoritesURL );               // Favorite Websites URLs
	m_qSettings.setValue( "Gender", quazaaSettings.Profile.Gender );                           // Gender..
	m_qSettings.setValue( "GnutellaScreenName",
						  quazaaSettings.Profile.GnutellaScreenName );   // The name displayed in searches, uploads and downloads and when users browse your system
	m_qSettings.setValue( "GUID",
						  quazaaSettings.Profile.GUID.toString() );                    // Unique ID for each client. Can be regenerated
	m_qSettings.setValue( "ICQuin",
						  quazaaSettings.Profile.ICQuin );                           // Identification number in ICQ
	m_qSettings.setValue( "Interests",
						  quazaaSettings.Profile.Interests );                     // What do you like to do other than use Quazaa
	m_qSettings.setValue( "IrcAlternateNickname",
						  quazaaSettings.Profile.IrcAlternateNickname ); // Alternate nickname in Irc chat if first one is already used
	m_qSettings.setValue( "IrcNickname", quazaaSettings.Profile.IrcNickname );                 // Nickname used in Irc chat
	m_qSettings.setValue( "IrcUserName", quazaaSettings.Profile.IrcUserName );                 // User name for Irc chat
	m_qSettings.setValue( "JabberID", quazaaSettings.Profile.JabberID );                       // Jabber ID
	m_qSettings.setValue( "Latitude",
						  quazaaSettings.Profile.Latitude );                       // Location for a disgruntled P2P user to aim a missile
	m_qSettings.setValue( "Longitude",
						  quazaaSettings.Profile.Longitude );                     // Location for a disgruntled P2P user to aim a missile
	m_qSettings.setValue( "MSNPassport", quazaaSettings.Profile.MSNPassport );                 // Microsoft's Messenger ID
	m_qSettings.setValue( "MyspaceProfile",
						  quazaaSettings.Profile.MyspaceProfile );           // Myspace profile excluding http://quazaaSettings.Profile.myspace.com/
	m_qSettings.setValue( "RealName", quazaaSettings.Profile.RealName );                       // User's real name
	m_qSettings.setValue( "StateProvince", quazaaSettings.Profile.StateProvince );             // Your state or province
	m_qSettings.setValue( "YahooID", quazaaSettings.Profile.YahooID );                         // Yahoo Messenger ID
	m_qSettings.endGroup();
}

/*!
	Loads the profile settings from persistent .ini file.
 */
void QuazaaSettings::loadProfile()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Profile" );

	quazaaSettings.Profile.Age = m_qSettings.value( "Age", 18 ).toInt();
	quazaaSettings.Profile.AolScreenName = m_qSettings.value( "AolScreenName", "" ).toString();
	quazaaSettings.Profile.AvatarPath = m_qSettings.value( "AvatarPath", ":/Resource/Chat/DefaultAvatar.png" ).toString();
	quazaaSettings.Profile.Biography = m_qSettings.value( "Biography", "" ).toString();
	quazaaSettings.Profile.City = m_qSettings.value( "City", "" ).toString();
	quazaaSettings.Profile.Country = m_qSettings.value( "Country", "" ).toString();
	quazaaSettings.Profile.Email = m_qSettings.value( "Email", "" ).toString();
	quazaaSettings.Profile.Favorites = m_qSettings.value( "Favorites", QStringList() ).toStringList();
	quazaaSettings.Profile.FavoritesURL = m_qSettings.value( "FavoritesURL", QStringList() ).toStringList();
	quazaaSettings.Profile.Gender = m_qSettings.value( "Gender", 0 ).toInt();
	quazaaSettings.Profile.GUID = m_qSettings.value( "GUID", QUuid::createUuid().toString() ).toString();
	QString tempNick = tr( "Quazaa-%1" ).arg( QUuid::createUuid().data1 );
	quazaaSettings.Profile.GnutellaScreenName = m_qSettings.value( "GnutellaScreenName", "" ).toString();
	if ( quazaaSettings.Profile.GnutellaScreenName == "" )
	{
		quazaaSettings.Profile.GnutellaScreenName = tempNick;
	}
	quazaaSettings.Profile.ICQuin = m_qSettings.value( "ICQuin", "" ).toString();
	quazaaSettings.Profile.Interests = m_qSettings.value( "Interests", QStringList() ).toStringList();
	quazaaSettings.Profile.IrcAlternateNickname = m_qSettings.value( "IrcAlternateNickname", "" ).toString();
	quazaaSettings.Profile.IrcNickname = m_qSettings.value( "IrcNickname", "" ).toString();
	if ( quazaaSettings.Profile.IrcNickname == "" )
	{
		quazaaSettings.Profile.IrcNickname = tempNick;
	}
	quazaaSettings.Profile.ICQuin = m_qSettings.value( "ICQuin", "" ).toString();
	quazaaSettings.Profile.IrcUserName = m_qSettings.value( "IrcUserName", "" ).toString();
	if ( quazaaSettings.Profile.IrcUserName == "" )
	{
		quazaaSettings.Profile.IrcUserName = tempNick;
	}
	quazaaSettings.Profile.ICQuin = m_qSettings.value( "ICQuin", "" ).toString();
	quazaaSettings.Profile.JabberID = m_qSettings.value( "JabberID", "" ).toString();
	quazaaSettings.Profile.Latitude = m_qSettings.value( "Latitude", "" ).toString();
	quazaaSettings.Profile.Longitude = m_qSettings.value( "Longitude", "" ).toString();
	quazaaSettings.Profile.MSNPassport = m_qSettings.value( "MSNPassport", "" ).toString();
	quazaaSettings.Profile.MyspaceProfile = m_qSettings.value( "MyspaceProfile", "" ).toString();
	quazaaSettings.Profile.RealName = m_qSettings.value( "RealName", "" ).toString();
	quazaaSettings.Profile.StateProvince = m_qSettings.value( "StateProvince", "" ).toString();
	quazaaSettings.Profile.YahooID = m_qSettings.value( "YahooID", "" ).toString();
	m_qSettings.endGroup();
}

/*!
	Saves the window settings to persistent .ini file.
 */
void QuazaaSettings::saveWindowSettings( QMainWindow* window )
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.setValue( "WindowGeometry", window->saveGeometry() );
	m_qSettings.setValue( "WindowState", window->saveState() );
	m_qSettings.setValue( "WindowVisible", window->isVisible() );

	m_qSettings.setValue( "ActiveTab", quazaaSettings.WinMain.ActiveTab );
	m_qSettings.setValue( "ActivitySplitter", quazaaSettings.WinMain.ActivitySplitter );
	m_qSettings.setValue( "ActivitySplitterRestoreTop", quazaaSettings.WinMain.ActivitySplitterRestoreTop );
	m_qSettings.setValue( "ActivitySplitterRestoreBottom", quazaaSettings.WinMain.ActivitySplitterRestoreBottom );
	m_qSettings.setValue( "ChatRoomsTaskVisible", quazaaSettings.WinMain.ChatRoomsTaskVisible );
	m_qSettings.setValue( "ChatFriendsTaskVisible", quazaaSettings.WinMain.ChatFriendsTaskVisible );
	m_qSettings.setValue( "ChatListSplitter", quazaaSettings.WinMain.ChatUserListSplitter );
	m_qSettings.setValue( "ChatTreeWidget", quazaaSettings.WinMain.ChatTreeWidget );
	m_qSettings.setValue( "ChatTreeWidgetSplitter", quazaaSettings.WinMain.ChatTreeWidgetSplitter );
	m_qSettings.setValue( "ChatToolbars", quazaaSettings.WinMain.ChatToolbars );
	m_qSettings.setValue( "DiscoveryHeader", quazaaSettings.WinMain.DiscoveryHeader );
	m_qSettings.setValue( "DiscoveryToolbar", quazaaSettings.WinMain.DiscoveryToolbar );
	m_qSettings.setValue( "DownloadsHeader", quazaaSettings.WinMain.DownloadsHeader );
	m_qSettings.setValue( "DownloadsToolbar", quazaaSettings.WinMain.DownloadsToolbar );
	m_qSettings.setValue( "DownloadsSplitter", quazaaSettings.WinMain.DownloadsSplitter );
	m_qSettings.setValue( "DownloadsSplitterRestoreTop", quazaaSettings.WinMain.DownloadsSplitterRestoreTop );
	m_qSettings.setValue( "DownloadsSplitterRestoreBottom", quazaaSettings.WinMain.DownloadsSplitterRestoreBottom );
	m_qSettings.setValue( "GraphSplitter", quazaaSettings.WinMain.GraphSplitter );
	m_qSettings.setValue( "GraphSplitterRestoreLeft", quazaaSettings.WinMain.GraphSplitterRestoreLeft );
	m_qSettings.setValue( "GraphSplitterRestoreRight", quazaaSettings.WinMain.GraphSplitterRestoreRight );
	m_qSettings.setValue( "GraphToolbar", quazaaSettings.WinMain.GraphToolbar );
	m_qSettings.setValue( "HomeSearchString", quazaaSettings.WinMain.HomeSearchString );
	m_qSettings.setValue( "HostCacheSplitter", quazaaSettings.WinMain.HostCacheSplitter );
	m_qSettings.setValue( "HostCacheSplitterRestoreLeft", quazaaSettings.WinMain.HostCacheSplitterRestoreLeft );
	m_qSettings.setValue( "HostCacheSplitterRestoreRight", quazaaSettings.WinMain.HostCacheSplitterRestoreRight );
	m_qSettings.setValue( "HostCacheToolbar", quazaaSettings.WinMain.HostCacheToolbar );
	m_qSettings.setValue( "LibraryDetailsSplitter", quazaaSettings.WinMain.LibraryDetailsSplitter );
	m_qSettings.setValue( "LibraryDetailsSplitterRestoreTop", quazaaSettings.WinMain.LibraryDetailsSplitterRestoreTop );
	m_qSettings.setValue( "LibraryDetailsSplitterRestoreBottom",
						  quazaaSettings.WinMain.LibraryDetailsSplitterRestoreBottom );
	m_qSettings.setValue( "LibraryNavigatorTab", quazaaSettings.WinMain.LibraryNavigatorTab );
	m_qSettings.setValue( "LibrarySplitter", quazaaSettings.WinMain.LibrarySplitter );
	m_qSettings.setValue( "LibrarySplitterRestoreLeft", quazaaSettings.WinMain.LibrarySplitterRestoreLeft );
	m_qSettings.setValue( "LibrarySplitterRestoreRight", quazaaSettings.WinMain.LibrarySplitterRestoreRight );
	m_qSettings.setValue( "LibraryToolbar", quazaaSettings.WinMain.LibraryToolbar );
	m_qSettings.setValue( "MainToolbar", quazaaSettings.WinMain.MainToolbar );
	m_qSettings.setValue( "MediaSplitter", quazaaSettings.WinMain.MediaSplitter );
	m_qSettings.setValue( "MediaSplitterRestoreLeft", quazaaSettings.WinMain.MediaSplitterRestoreLeft );
	m_qSettings.setValue( "MediaSplitterRestoreRight", quazaaSettings.WinMain.MediaSplitterRestoreRight );
	m_qSettings.setValue( "MediaToolbars", quazaaSettings.WinMain.MediaToolbars );
	m_qSettings.setValue( "NeighboursToolbars", quazaaSettings.WinMain.NeighboursToolbars );
	m_qSettings.setValue( "NeighboursHeader", quazaaSettings.WinMain.NeighboursHeader );
	m_qSettings.setValue( "PacketDumpToolbar", quazaaSettings.WinMain.PacketDumpToolbar );
	m_qSettings.setValue( "SchedulerToolbar", quazaaSettings.WinMain.SchedulerToolbar );
	m_qSettings.setValue( "SearchDetailsSplitter", quazaaSettings.WinMain.SearchDetailsSplitter );
	m_qSettings.setValue( "SearchFileTypeTaskVisible", quazaaSettings.WinMain.SearchFileTypeTaskVisible );
	m_qSettings.setValue( "SearchHeader", quazaaSettings.WinMain.SearchHeader );
	m_qSettings.setValue( "SearchMonitorToolbar", quazaaSettings.WinMain.SearchMonitorToolbar );
	m_qSettings.setValue( "SearchNetworksTaskVisible", quazaaSettings.WinMain.SearchNetworksTaskVisible );
	m_qSettings.setValue( "SearchResultsSplitterRestoreTop", quazaaSettings.WinMain.SearchResultsSplitterRestoreTop );
	m_qSettings.setValue( "SearchResultsSplitterRestoreBottom", quazaaSettings.WinMain.SearchResultsSplitterRestoreBottom );
	m_qSettings.setValue( "SearchResultsTaskVisible", quazaaSettings.WinMain.SearchResultsTaskVisible );
	m_qSettings.setValue( "SearchSplitter", quazaaSettings.WinMain.SearchSplitter );
	m_qSettings.setValue( "SearchSplitterRestoreLeft", quazaaSettings.WinMain.SearchSplitterRestoreLeft );
	m_qSettings.setValue( "SearchSplitterRestoreRight", quazaaSettings.WinMain.SearchSplitterRestoreRight );
	m_qSettings.setValue( "SearchTaskVisible", quazaaSettings.WinMain.SearchTaskVisible );
	m_qSettings.setValue( "SearchToolbar", quazaaSettings.WinMain.SearchToolbar );
	m_qSettings.setValue( "SecurityAutomaticHeader", quazaaSettings.WinMain.SecurityAutomaticHeader );
	m_qSettings.setValue( "SecurityManualHeader", quazaaSettings.WinMain.SecurityManualHeader );
	m_qSettings.setValue( "SecurityToolbars", quazaaSettings.WinMain.SecurityToolbars );
	m_qSettings.setValue( "SystemLogToolbar", quazaaSettings.WinMain.SystemLogToolbar );
	m_qSettings.setValue( "TransfersSplitter", quazaaSettings.WinMain.TransfersSplitter );
	m_qSettings.setValue( "TransfersSplitterRestoreLeft", quazaaSettings.WinMain.TransfersSplitterRestoreLeft );
	m_qSettings.setValue( "TransfersSplitterRestoreRight", quazaaSettings.WinMain.TransfersSplitterRestoreRight );
	m_qSettings.setValue( "TransfersNavigationSplitter", quazaaSettings.WinMain.TransfersNavigationSplitter );
	m_qSettings.setValue( "TransfersNavigationSplitterRestoreTop",
						  quazaaSettings.WinMain.TransfersNavigationSplitterRestoreTop );
	m_qSettings.setValue( "TransfersNavigationSplitterRestoreBottom",
						  quazaaSettings.WinMain.TransfersNavigationSplitterRestoreBottom );
	m_qSettings.setValue( "UploadsSplitter", quazaaSettings.WinMain.UploadsSplitter );
	m_qSettings.setValue( "UploadsSplitterRestoreTop", quazaaSettings.WinMain.UploadsSplitterRestoreTop );
	m_qSettings.setValue( "UploadsSplitterRestoreBottom", quazaaSettings.WinMain.UploadsSplitterRestoreBottom );
	m_qSettings.setValue( "UploadsToolbar", quazaaSettings.WinMain.UploadsToolbar );
}

/*!
	Loads the profile settings from persistent .ini file.
 */
void QuazaaSettings::loadWindowSettings( QMainWindow* window )
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	QList<QVariant> intListInitializer;
	intListInitializer << 0 << 0;

	window->restoreGeometry( m_qSettings.value( "WindowGeometry" ).toByteArray() );
	window->restoreState( m_qSettings.value( "WindowState" ).toByteArray() );
	quazaaSettings.WinMain.Visible = m_qSettings.value( "WindowVisible", true ).toBool();

	quazaaSettings.WinMain.ActiveTab = m_qSettings.value( "ActiveTab", 0 ).toInt();
	quazaaSettings.WinMain.ActivitySplitter = m_qSettings.value( "ActivitySplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.ActivitySplitterRestoreTop = m_qSettings.value( "ActivitySplitterRestoreTop", 0 ).toInt();
	quazaaSettings.WinMain.ActivitySplitterRestoreBottom = m_qSettings.value( "ActivitySplitterRestoreBottom", 0 ).toInt();
	quazaaSettings.WinMain.ChatRoomsTaskVisible = m_qSettings.value( "ChatRoomsTaskVisible", true ).toBool();
	quazaaSettings.WinMain.ChatFriendsTaskVisible = m_qSettings.value( "ChatFriendsTaskVisible", true ).toBool();
	quazaaSettings.WinMain.ChatUserListSplitter = m_qSettings.value( "ChatListSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.ChatTreeWidget = m_qSettings.value( "ChatTreeWidget", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.ChatTreeWidgetSplitter = m_qSettings.value( "ChatTreeWidgetSplitter",
																	   QByteArray() ).toByteArray();
	quazaaSettings.WinMain.ChatToolbars = m_qSettings.value( "ChatToolbars", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.DiscoveryHeader  = m_qSettings.value( "DiscoveryHeader",  QByteArray() ).toByteArray();
	quazaaSettings.WinMain.DiscoveryToolbar = m_qSettings.value( "DiscoveryToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.DownloadsHeader  = m_qSettings.value( "DownloadsHeader",  QByteArray() ).toByteArray();
	quazaaSettings.WinMain.DownloadsToolbar = m_qSettings.value( "DownloadsToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.DownloadsSplitter = m_qSettings.value( "DownloadsSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.DownloadsSplitterRestoreTop = m_qSettings.value( "DownloadsSplitterRestoreTop", 0 ).toInt();
	quazaaSettings.WinMain.DownloadsSplitterRestoreBottom = m_qSettings.value( "DownloadsSplitterRestoreBottom",
																			   0 ).toInt();
	quazaaSettings.WinMain.GraphSplitter = m_qSettings.value( "GraphSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.GraphToolbar = m_qSettings.value( "GraphToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.HomeSearchString = m_qSettings.value( "HomeSearchString", "" ).toString();
	quazaaSettings.WinMain.HostCacheSplitter = m_qSettings.value( "HostCacheSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.HostCacheToolbar = m_qSettings.value( "HostCacheToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.LibraryDetailsSplitter = m_qSettings.value( "LibraryDetailsSplitter",
																	   QByteArray() ).toByteArray();
	quazaaSettings.WinMain.LibraryDetailsSplitterRestoreTop = m_qSettings.value( "LibraryDetailsSplitterRestoreTop",
																				 0 ).toInt();
	quazaaSettings.WinMain.LibraryDetailsSplitterRestoreBottom = m_qSettings.value( "LibraryDetailsSplitterRestoreBottom",
																					0 ).toInt();
	quazaaSettings.WinMain.LibrarySplitter = m_qSettings.value( "LibrarySplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.LibrarySplitterRestoreLeft = m_qSettings.value( "LibrarySplitterRestoreLeft", 0 ).toInt();
	quazaaSettings.WinMain.LibrarySplitterRestoreRight = m_qSettings.value( "LibrarySplitterRestoreRight", 0 ).toInt();
	quazaaSettings.WinMain.LibraryNavigatorTab = m_qSettings.value( "LibraryNavigatorTab", 0 ).toInt();
	quazaaSettings.WinMain.LibraryToolbar = m_qSettings.value( "LibraryToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.MainToolbar = m_qSettings.value( "MainToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.MediaSplitter = m_qSettings.value( "MediaSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.MediaSplitterRestoreLeft = m_qSettings.value( "MediaSplitterRestoreLeft", 0 ).toInt();
	quazaaSettings.WinMain.MediaSplitterRestoreRight = m_qSettings.value( "MediaSplitterRestoreRight", 0 ).toInt();
	quazaaSettings.WinMain.MediaToolbars = m_qSettings.value( "MediaToolbars", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.NeighboursToolbars = m_qSettings.value( "NeighboursToolbars", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.NeighboursHeader = m_qSettings.value( "NeighboursHeader", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.PacketDumpToolbar = m_qSettings.value( "PacketDumpToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SchedulerToolbar = m_qSettings.value( "SchedulerToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SearchDetailsSplitter = m_qSettings.value( "SearchDetailsSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SearchHeader = m_qSettings.value( "SearchHeader", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SearchTaskVisible = m_qSettings.value( "SearchTaskVisible", true ).toBool();
	quazaaSettings.WinMain.SearchNetworksTaskVisible = m_qSettings.value( "SearchNetworksTaskVisible", true ).toBool();
	quazaaSettings.WinMain.SearchFileTypeTaskVisible = m_qSettings.value( "SearchFileTypeTaskVisible", true ).toBool();
	quazaaSettings.WinMain.SearchMonitorToolbar = m_qSettings.value( "SearchMonitorToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SearchResultsSplitterRestoreTop = m_qSettings.value( "SearchResultsSplitterRestoreTop",
																				0 ).toInt();
	quazaaSettings.WinMain.SearchResultsSplitterRestoreBottom = m_qSettings.value( "SearchResultsSplitterRestoreBottom",
																				   0 ).toInt();
	quazaaSettings.WinMain.SearchResultsTaskVisible = m_qSettings.value( "SearchResultsTaskVisible", true ).toBool();
	quazaaSettings.WinMain.SearchSplitter = m_qSettings.value( "SearchSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SearchSplitterRestoreLeft = m_qSettings.value( "SearchSplitterRestoreLeft", 0 ).toInt();
	quazaaSettings.WinMain.SearchSplitterRestoreRight = m_qSettings.value( "SearchSplitterRestoreRight", 0 ).toInt();
	quazaaSettings.WinMain.SearchToolbar = m_qSettings.value( "SearchToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SecurityAutomaticHeader = m_qSettings.value( "SecurityAutomaticHeader",
																		QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SecurityManualHeader = m_qSettings.value( "SecurityManualHeader", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SecurityToolbars = m_qSettings.value( "SecurityToolbars", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.SystemLogToolbar = m_qSettings.value( "SystemLogToolbar", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.TransfersSplitter = m_qSettings.value( "TransfersSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.TransfersSplitterRestoreLeft = m_qSettings.value( "TransfersSplitterRestoreLeft", 0 ).toInt();
	quazaaSettings.WinMain.TransfersSplitterRestoreRight = m_qSettings.value( "TransfersSplitterRestoreRight", 0 ).toInt();
	quazaaSettings.WinMain.TransfersNavigationSplitter = m_qSettings.value( "TransfersNavigationSplitter",
																			QByteArray() ).toByteArray();
	quazaaSettings.WinMain.TransfersNavigationSplitterRestoreTop =
		m_qSettings.value( "TransfersNavigationSplitterRestoreTop", 0 ).toInt();
	quazaaSettings.WinMain.TransfersNavigationSplitterRestoreBottom =
		m_qSettings.value( "TransfersNavigationSplitterRestoreBottom", 0 ).toInt();
	quazaaSettings.WinMain.UploadsSplitter = m_qSettings.value( "UploadsSplitter", QByteArray() ).toByteArray();
	quazaaSettings.WinMain.UploadsSplitterRestoreTop = m_qSettings.value( "UploadsSplitterRestoreTop", 0 ).toInt();
	quazaaSettings.WinMain.UploadsSplitterRestoreBottom = m_qSettings.value( "UploadsSplitterRestoreBottom", 0 ).toInt();
	quazaaSettings.WinMain.UploadsToolbar = m_qSettings.value( "UploadsToolbar", QByteArray() ).toByteArray();
}

/*!
	Saves the language settings to persistent .ini file.
 */
void QuazaaSettings::saveLanguageSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Language" );
	m_qSettings.setValue( "LanguageFile", quazaaSettings.Language.File );
	m_qSettings.endGroup();
}

/*!
	Loads the language settings from persistent .ini file.
 */
void QuazaaSettings::loadLanguageSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Language" );
	quazaaSettings.Language.File = m_qSettings.value( "LanguageFile", ( "quazaa_default_en" ) ).toString();
	m_qSettings.endGroup();
}

/*!
	Saves if this is Quazaa's first run to persistent .ini file.
 */
void QuazaaSettings::saveFirstRun( bool firstRun )
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.setValue( "FirstRun", firstRun );
}

/*!
	Loads if this is Quazaa's first run from persistent .ini file.
 */
bool QuazaaSettings::isFirstRun()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	return m_qSettings.value( "FirstRun", true ).toBool();
}

/*!
	Saves Quazaa's skin settings to persistent .ini file.
 */
void QuazaaSettings::saveSkinSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.setValue( "SkinFile", Skin.File );
}

/*!
	Loads Quazaa's skin settings from persistent .ini file.
 */
void QuazaaSettings::loadSkinSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	Skin.File = m_qSettings.value( "SkinFile", qApp->applicationDirPath() + "/Skin/Greenery/Greenery.qsk" ).toString();
}

/*!
	Saves Quazaa's log settings to persistent .ini file.
 */
void QuazaaSettings::saveLogSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Logging" );
	m_qSettings.setValue( "SaveLog", Logging.SaveLog );
	m_qSettings.setValue( "LogShowTimestamp", Logging.LogShowTimestamp );
	m_qSettings.setValue( "ShowInformation", Logging.ShowInformation );
	m_qSettings.setValue( "ShowSecurity", Logging.ShowSecurity );
	m_qSettings.setValue( "ShowNotice", Logging.ShowNotice );
	m_qSettings.setValue( "ShowDebug", Logging.ShowDebug );
	m_qSettings.setValue( "ShowWarnings", Logging.ShowWarnings );
	m_qSettings.setValue( "ShowError", Logging.ShowError );
	m_qSettings.setValue( "ShowCritical", Logging.ShowCritical );
	m_qSettings.setValue( "IsPaused", Logging.IsPaused );
	m_qSettings.endGroup();
}

/*!
	Loads Quazaa's log settings from persistent .ini file.
 */
void QuazaaSettings::loadLogSettings()
{
	QSettings m_qSettings( QuazaaGlobals::INI_FILE(), QSettings::IniFormat );

	m_qSettings.beginGroup( "Logging" );
	Logging.SaveLog          = m_qSettings.value( "SaveLog",          false ).toBool();
	Logging.LogShowTimestamp = m_qSettings.value( "LogShowTimestamp", true  ).toBool();
	Logging.ShowInformation  = m_qSettings.value( "ShowInformation",  true  ).toBool();
	Logging.ShowSecurity     = m_qSettings.value( "ShowSecurity",     true  ).toBool();
	Logging.ShowNotice       = m_qSettings.value( "ShowNotice",       true  ).toBool();
#ifdef QT_DEBUG
	Logging.ShowDebug        = m_qSettings.value( "ShowDebug",        true  ).toBool();
#else
	Logging.ShowDebug        = m_qSettings.value( "ShowDebug",        false ).toBool();
#endif
	Logging.ShowWarnings     = m_qSettings.value( "ShowWarnings",     true  ).toBool();
	Logging.ShowError        = m_qSettings.value( "ShowError",        true  ).toBool();
	Logging.ShowCritical     = m_qSettings.value( "ShowCritical",     true  ).toBool();
	Logging.IsPaused         = m_qSettings.value( "IsPaused",         false ).toBool();
	m_qSettings.endGroup();
}
