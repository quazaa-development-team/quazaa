/*
** quazaasettings.h
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

#ifndef QUAZAASETTINGS_H
#define QUAZAASETTINGS_H

#include <QObject>
#include <QMainWindow>
#include <QUuid>
#include <QTranslator>
#include <QVariant>

namespace IrcMessageType
{
enum MessageType
{
	Joins,
	Parts,
	Nicks,
	Modes,
	Kicks,
	Quits,
	Topics
};
}

namespace IrcColorType
{
enum ColorType
{
	Background,
	Default,
	Event,
	Notice,
	Action,
	Inactive,
	Alert,
	Highlight,
	TimeStamp,
	Link,
	White,
	Black,
	Blue,
	Green,
	Red,
	Brown,
	Purple,
	Orange,
	Yellow,
	LightGreen,
	Cyan,
	LightCyan,
	LightBlue,
	Pink,
	Gray,
	LightGray
};
}

namespace IrcShortcutType
{
enum ShortcutType
{
	NextView,
	PreviousView,
	ExpandView,
	CollapseView,
	SearchView,
	ResetViews,
	NextActiveView,
	PreviousActiveView,
	MostActiveView
};
}

namespace Settings
{
struct sAres
{
	bool		Enable;									// Connect to Ares
	int			MaximumDownloads;						// Max concurrent downloads
	int			MaximumUploads;							// Max concurrent uploads
	int			MaximumUploadsPerUser;					// Max uploads per user
};

struct sBitTorrent
{
	bool		AutoClear;								// Clear completed torrents when they meet the required share ratio
	bool		AutoSeed;								// Automatically re-seed most recently completed torrent on start-up
	int			BandwidthPercentage;					// Percentage of bandwidth to use when BT active.
	int			ClearRatio;								// Share ratio a torrent must reach to be cleared. (Minimum 100%)
	int			CodePage;								// The code page to assume for a .torrent file if it isn't UTF-8
	QString		DefaultTracker;							// Default tracker to use if trackers listed in torrent are down
	int			DefaultTrackerPeriod;					// Delay between tracker contact attempts if one is not specified by tracker
	int			DhtPruneTime;
	int			DownloadConnections;					// Number of active torrent connections allowed
	int			DownloadTorrents;						// Number of torrents to download at once
	bool		Endgame;								// Allow endgame mode when completing torrents. (Download same chunk from multiple sources)
	bool		ExtraKeys;								// Check for '.utf8' keys if there is an encoding error
	int			LinkPing;
	int			LinkTimeout;
	bool		Managed;								// Managed Torrents
	bool		PreferBTSources;						// Prefer downloading from BT sources when appropriate
	int			RandomPeriod;
	int			RequestLimit;
	int			RequestPipe;
	int			RequestSize;
	bool		ShowFilesInDownload;					// Display Individual Files In The Download Tree
	int			SourceExchangePeriod;					// Time to exchange sources with new peers
	bool		StartPaused;							// Start torrents in a paused state
	bool		TestPartials;							// Do on integrity check of Bittorrent partials
	QString		TorrentPath;							// Where .torrent files are stored
	bool		TrackerKey;								// Send a key (random value) to trackers
	int			UploadCount;							// Number of active torrent uploads allowed
	bool		UseKademlia;							// Use Kademlia with bittorrent
	bool		UseSaveDialog;							// Use torrent save dialog or use default settings
	bool		UseTemp;
};

struct sChat
{
	QFont       Font;                                   // The font used in IRC windows
	QVariant	Connections;							// Irc server connections
	bool		ConnectOnStartup;						// Connect to the chat server and enter rooms on startup
	bool		EnableFileTransfers;					// Enable Irc File Transfers
	bool		ShowTimestamp;							// Show timestamps at the beginning of messages
	QString     TimestampFormat;                        // The format of timestamps
	int			Scrollback;							// This limits how many blocks(lines) can be in a message view
	bool		StripNicks;								// Strip host information from nicks
	bool        HighlightSounds;                      // Mute the notification sounds
	bool        DarkTheme;                              // Has the user selected the dark or light theme?
	bool        SortViews;                              // Can we sort views?

	QStringList Hosts;									// The list of hosts in the connect dialog
	QStringList NickNames;								// The list of nick names in the connect dialog
	QStringList RealNames;								// The list of real names in the connect dialog
	QStringList UserNames;								// The list of user names in the connect dialog
	QStringList ConnectionNames;						// The list of connection names in the connect dialog
	QStringList Ignores;                                // Nicks to ignore

	QHash<int, bool> Messages;
	QHash<int, bool> Highlights;
	QHash<int, QString> Colors;
	QHash<int, QString> Shortcuts;
	QMap<QString, QString> Aliases;                     // Command aliases
};

struct sConnection
{
	bool		DetectConnectionLoss;					// Detect loss of internet
	bool		DetectConnectionReset;					// Detect regaining of internet connection
	quint8		FailureLimit;							// Max allowed connection failures (default = 3) (Neighbour connections)
	quint16		FailurePenalty;							// Delay after connection failure (seconds, default = 300) (Neighbour connections)
	quint64		InSpeed;								// Inbound internet connection speed in B/s
	quint64		OutSpeed;								// Outbound internet connection speed in B/s
	quint16		Port;									// Incoming port
	bool		RandomPort;								// Select a random incoming port
	quint32		SendBuffer;								// Size of data send blocks
	quint32		TimeoutConnect;							// Time to wait for a connection before dropping the connection
	quint32		TimeoutTraffic;							// Time to wait for general network communications before dropping a connection
	QStringList	PreferredCountries;						// Country preference
	quint32     UDPOutLimitPPS;                         // Packets per second limiter
};

struct sDiscovery
{
	quint16		AccessThrottle;							// Number of seconds to wait between consecutive requests for the same service.
	quint8		FailureLimit;							// Number of failures after which a cache should be autodisabled no matter its rating. (0 to disable)
	// Note that this setting will be ineffective if a value higher than MaximalServiceRating is chosen.
	quint8		MaximumServiceRating;					// The highest rating a service can reach.
	quint8		ServiceTimeout;							// Number of seconds after which a cache request will be cancelled automatically.
	quint32		ZeroRatingRevivalInterval;				// Number of seconds after which we try to revive a service with probability 0.
	quint8		ZeroRatingRevivalTries;					// Number of times we try to revive a cache with probability 0 before banning it for good.
};

struct sDownloads
{
	bool		AllowBackwards;							// Permit download to run in reverse when appropriate
	bool		AutoClear;								// Automatically clear finished downloads
	int			BufferSize;								// Size of download buffer
	int			ChunkSize;								// Size of download chunks
	int			ChunkStrap;								// Size of first download chunk
	int			ClearDelay;								// Time a download should be finished before clearing
	QString		CompletePath;							// Where downloads are moved when they complete
	int			ConnectThrottle;						// Delay between download attempts. (Very important for routers)
	int			DropFailedSourcesThreshold;				// The number of sources where Quazaa start dropping failed sources after only one attempt
	bool		ExpandDownloads;						// Expand new downloads by default
	bool		FlushSD;
	QString		IncompletePath;							// Where incomplete downloads are stored
	int			MaxAllowedFailures;						// Max download failures before dropping a download source
	int			MaxConnectingSources;					// The maximum number of sources that can be in the 'connecting' state. (Important for XPsp2)
	int			MaxFiles;								// How many files download at once
	int			MaxFileSearches;						// Number of files over the download limit that can search for sources for when they start. (Search, etc)
	int			MaxReviews;								// Maximum number of reviews to store per download
	int			MaxTransfers;							// How many total tranfers take place
	int			MaxTransfersPerFile;					// How many transfers are allowed per file
	bool		Metadata;								// Download metadata (ID3 tags, etc.)
	int			MinSources;								// The minimum number of sources a download has before Quazaa regards it as having a problem
	bool		NeverDrop;								// Do not drop bad sources (may pollute source list with many dead sources)
	int			PushTimeout;							// Lower transfer timeout for push sources
	int			QueueLimit;								// Longest queue to wait in. (0 to disable. This should be >800 or 0 to get good performance from ed2k)
	bool		RequestHash;							// Request unknown hashes from active download sources
	bool		RequestHTTP11;
	bool		RequestURLENC;
	int			RetryDelay;								// Time to wait between connection connection attempts to a particular download source
	int			SaveInterval;							// Time between writes to disc of downloads
	int			SearchPeriod;							// Time to send out search requests
	bool		ShowGroups;
	bool		ShowMonitorURLs;
	bool		ShowPercent;							// Display small green % complete bar on progress graphic
	bool		ShowSources;							// Display individual download sources in Downloads tree
	bool		SortColumns;							// Allow user to sort downloads by clicking column headers
	bool		SortSources;							// Automatically sort sources (Status, protocol, queue)
	int			SourcesWanted;							// Number of sources Quazaa 'wants'. (Will not request more than this number of sources from ed2k)
	bool		StaggardStart;							// Start file connections at different time intervals instead of all at the same time
	int			StarveGiveUp;							// How long (in hours) before Quazaa will give up on a download and try another if it gets no data. (+ 0-9 h, depending on sources)
	int			StarveTimeout;							// How long before Quazaa tries drops a connection for a particular file source
	bool		URIPrompt;								// Show prompt before opening URIs
	bool		VerifyED2K;								// Verify file integrity on eDonkey 2k downloads
	bool		VerifyFiles;							// Verify file integrity
	bool		VerifyTiger;							// Verify file integrity using Tiger hash
	bool		WebHookEnable;
	QStringList	WebHookExtensions;
};

struct sEDonkey
{
	QString		DefaultServerFlags;						// Default server flags (for UDP searches)
	int			DequeueTime;
	bool		Enable;									// Connect to  2k
	bool		EnableKad;								// Connect to the Kad discovery service
	bool		Endgame;								// Allow endgame mode when completing downloads. (Download same chunk from multiple sources)
	int			ExtendedRequest;
	bool		FastConnect;							// Try connecting to 2 servers to get online faster
	bool		ForceHighID;							// Reconnect if low-id. (once only)
	int			FrameSize;
	int			GetSourcesThrottle;						// Max rate a general GetSources can done
	bool		LargeFileSupport;						// Allow 64 bit file sizes
	bool		LearnNewServers;						// Get new servers from servers
	bool		LearnNewServersClient;					// Get new servers from clients
	bool		MagnetSearch;							// Search for magnets over ed2k (lower server load)
	int			MaxClients;								// Max ed2k client links
	int			MaxResults;								// Stop searching after this many search results
	int			MaxShareCount;							// Hard limit on file list sent to server
	bool		MetAutoQuery;							// Auto query for a new server list
	int			MinServerFileSize;						// Minimum size a file in the library must be in order to be included in the server file list. (In KB)
	int			NumServers;								// 1
	int			PacketThrottle;							// ED2K packet rate limiter
	int			QueryFileThrottle;						// Max rate a file can have GetSources done
	int			QueryGlobalThrottle;					// Global throttle for all ed2k searches (TCP, UDP, manual and auto)
	int			QueueRankThrottle;						// How frequently queue ranks are sent
	int			QueryServerThrottle;					// Max rate at which an individual server can be queried
	int			ReAskTime;
	int			RequestPipe;
	int			RequestSize;
	bool		SearchCachedServers;					// Search on known remote servers in the server cache
	bool		SendPortServer;							// Send port in tag to ed2k servers. (not needed for newer servers)
	QString		ServerListURL;							// URL address to auto query for a new server list
	bool		ServerWalk;								// Enable global UDP walk of servers
	int			SourceThrottle;							// ED2K source rate limiter
	int			StatsGlobalThrottle;					// Global throttle for server UDP stats requests
	int			StatsServerThrottle;					// Max rate at which an individual server can be asked for stats
};

struct sGnutella
{
	int			ConnectFactor;
	quint16		ConnectThrottle;						// Delay between connection attempts (seconds)
	int			HitsPerPacket;							// Max hits allowed for a single packet
	quint32		HostCacheSize;							// Max size of host cache; 0 to disable
	int			HostCacheView;
	int			MaxHits;								// Max hits allowed for a single query
	quint32		MaxResults;								// Maximum results to return to a single query
	int			RouteCache;								// Cache for route to peer
};

struct sGnutella2
{
	int			ClientMode;								// Desired mode of operation: MODE_AUTO, MODE_LEAF, MODE_HUB
	bool		Enable;									// Connect to G2
	int			HAWPeriod;
	int			HostCount;								// Number of hosts in X-Try-Hubs
	quint32		HostCurrent;							// Index of current host
	int			HostExpire;								// Inactive time before a host expires
	int			HubHorizonSize;
	bool		HubVerified;							// Verified we are operating as a hub
	int			KHLHubCount;
	int			KHLPeriod;
	int			LNIMinimumUpdate;
	quint32		NumHubs;								// Number of hubs a leaf has (Leaf to Hub)
	quint32		NumLeafs;								// Number of leafs a hub has (Hub to Leaf)
	quint32		NumPeers;								// Number of peers a hub has (Hub to Hub)
	quint32		PingRate;								// Time in seconds between pings
	quint32		PingTimeout;
	int			PingRelayLimit;							// Number of other leafs to forward a /PI/UDP to: 10 - 30
	quint32		QueryHostDeadline;						// Time before ending queries
	quint32		QueryHostThrottle;						// Bandwidth throttling for queries
	quint32		QueryLimit;								// Maximum amount of concurrent queries
	quint32		QueryKeyTime;							// Time in seconds before re-requesting query key
	int			RequeryDelay;							// Time before sending another query (seconds)
	int			UdpBuffers;								// UDP protocol buffer size
	quint32		UdpInExpire;							// Time before incoming an incloming UDP connection
	int			UdpInFrames;							// UDP protocol in frame size
	quint32		UdpMTU;									// UDP protocol maximum transmission units
	quint32		UdpOutExpire;							// Time before dropping a UDP connection
	int			UdpOutFrames;							// UDP protocol out frame size
	quint32		UdpOutResend;							// Time before resending a UDP protocol packet
	quint32		HubBalancePeriod;
	quint32		HubBalanceGrace;
	quint32		HubBalanceLow;
	quint32		HubBalanceLowTime;
	quint32		HubBalanceHigh;
	quint32		HubBalanceHighTime;

};

struct sLanguage
{
	QString		File;								// What language does the user prefer
};

struct sLibrary
{
	QString		FilterURI;
	bool		GhostFiles;								// Create ghost files specifying why files were deleted on deletion
	bool		HashWindow;								// Display hashing progress window
	bool		HighPriorityHashing;					// Use high priority hashing
	int			HighPriorityHashingSpeed;				// desired speed in MB/s when hashing with hi priority
	int			HistoryDays;							// Remember files for this many days
	int			HistoryTotal;							// Remember this many total files in Library history
	int			LowPriorityHashingSpeed;				// desired speed in MB/s when hashing with low priority
	QStringList	NeverShareTypes;						// Never share these file types
	bool		PartialMatch;
	bool		PreferAPETags;							// Read APE tags first and only then ID3 tags from audio files
	int			QueryRouteSize;
	bool		RememberViews;							// Remember custom folder views
	QStringList	SafeExecuteTypes;						// These file types are safe to open
	bool		ScanAPE;								// Enable .ape,.mac,.apl metadata extraction by internals
	bool		ScanASF;								// Enable .asf,.wma,.wmv metadata extraction by internals
	bool		ScanAVI;								// Enable .avi metadata extraction by internals
	bool		ScanCHM;								// Enable .chm metadata extraction by internals
	bool		ScanEXE;								// Enable .exe,.dll metadata extraction by internals
	bool		ScanImage;								// Enable .jpg,.jpeg,.gif,.png,.bmp metadata extraction by internals
	bool		ScanMP3;								// Enable .mp3 metadata extraction by internals
	bool		ScanMPC;								// Enable .mpc,.mpp,.mp+ metadata extraction by internals
	bool		ScanMPEG;								// Enable .mpeg,.mpg metadata extraction by internals
	bool		ScanMSI;								// Enable .msi metadata extraction by internals
	bool		ScanOGG;								// Enable .ogg metadata extraction by internals
	bool		ScanPDF;								// Enable .pdf metadata extraction by internals
	QString		SchemaURI;
	QStringList	Shares;									// A list of your shared folders
	bool		ShowCoverArt;
	bool		SmartSeriesDetection;					// Organize video files in Library by using predefined patterns
	int			SourceExpire;
	bool		SourceMesh;
	int			ThumbSize;
	int			TigerHeight;
	int			TreeSize;

	/*
	bool		ShowVirtual;
	int			PanelSize;
	bool		ShowPanel;
	bool		UseFolderGUID;							// Save/Load folder GUID using NTFS stream
	bool		MarkFileAsDownload;						// Mark downloaded file using NTFS stream as Internet Explorer
	bool		UseCustomFolders;						// Use desktop.ini
	*/
};

struct sLive
{
	bool		AdultWarning;							// Has the user been warned about the adult filter?
	bool		AutoClose;
	int			MonitorThrottle;						// Monitor slider settings
	bool		DiskSpaceStop;							// Has Quazaa paused all downloads due to critical disk space?
	bool		DiskSpaceWarning;						// Has the user been warned of low disk space?
	bool		DiskWriteWarning;						// Has the user been warned of write problems?
	bool		DonkeyServerWarning;					// Has the user been warned about having an empty server list?
	QString		LastDuplicateHash;						// Stores the hash of the file about which the warning was shown
	bool		MaliciousWarning;						// Is the warning dialog opened?
	bool		QueueLimitWarning;						// Has the user been warned about limiting the max Q position accepted?
	bool		UploadLimitWarning;						// Has the user been warned about the ed2k/BT ratio?
};

struct sLogging
{
	bool		SaveLog;								// Create a log file on crash
	bool		LogShowTimestamp;						// Show timestamps in the system log?
	bool		ShowInformation;						// Show Information messages
	bool		ShowSecurity;							// Show Security messages
	bool		ShowNotice;								// Show Notice messages
	bool		ShowDebug;								// Show Debug messages
	bool		ShowWarnings;							// Show Warning messages
	bool		ShowError;								// Show Error messages
	bool		ShowCritical;							// Show Critical messages
	bool		IsPaused;								// Is logging paused
};

struct sMedia
{
	int			Aspect;									// Size video is displayed in
	QString		AudioVisualPlugin;						// Path to visual plugin for audio. Null if none is used.
	QString		CustomPlayerPath;						// Path to the custom player executable
	int			Handler;								// Media Player (0 Built In, 1 Shell Open, 2 Custom)
	QStringList	FileTypes;								// Try to open these file types as media files
	bool		ListVisible;							// Is playlist visible
	bool		Mute;									// Is sound muted
	QString		OpenPath;								// The last folder where a media file was opened
	QStringList	Playlist;								// Save playlist so it can be reopened when closing/opening Quazaa
	bool		Shuffle;									// Is random turned on for media files?
	bool		Repeat;									// Is repeat turned on for media files?
	bool		StatusVisible;							// Display time remaining, etc
	qreal		Volume;									// Sound volume
	int			Zoom;									// The zoom level for video
	/*
	QString		ServicesCLSID;
	QString		Mpeg1PreviewCLSID;
	QString		Mp3PreviewCLSID;
	QString		AviPreviewCLSID;
	QString		VisWrapperCLSID;
	QString		VisSoniqueCLSID;
	QString		VisCLSID;
	QString		VisPath;
	int			VisSize;
	QString		ServicePath;
	bool		ShortPaths;								// Some players differently handle unicode paths but they can launch files using 8.3 paths
	*/
};

struct sParental
{
	QStringList	AdultFilter;							// List of words to censor
	bool		ChatAdultCensor;						// Censor adlt language from chat
	bool		FilterAdultSearchResults;				// Filter adult content from searches
};

struct sPrivateMessages
{
	bool		Gnutella2Enable;						// Are Gnutella 2 private messages enabled?
	bool		AresEnable;								// Are Gnutella 2 private messages enabled?
	bool		eDonkeyEnable;							// Are Gnutella 2 private messages enabled?
	int			AwayMessageIdleTime;					// Time in secs of idle system time before showing away message
	QString		AwayMessage;							// The away message.
};

struct sProfile
{
	int			Age;									// Age..
	QString		AolScreenName;							// Aol screen name
	QString		AvatarPath;								// Path to an avatar image file. Quazaa logo used if blank
	QString		Biography;								// Your life story..
	QString		City;									// Your city
	QString		Country;								// Your country..
	QString		Email;									// EMail address
	QStringList	Favorites;								// Favorite websites
	QStringList	FavoritesURL;							// Favorite Websites URLs
	int			Gender;									// Gender..
	QString		GnutellaScreenName;						// The name displayed in searches, uploads and downloads and when users browse your system
	QUuid		GUID;									// Unique ID for each client. Can be regenerated
	QString		ICQuin;									// Identification number in ICQ
	QStringList	Interests;								// What do you like to do other than use Quazaa
	QString		IrcAlternateNickname;					// Alternate nickname in Irc chat if first one is already used
	QString		IrcNickname;							// Nickname used in Irc chat
	QString		IrcUserName;							// User name for Irc chat
	QString		JabberID;								// Jabber ID
	QString		Latitude;								// Location for a disgruntled P2P user to aim a missile
	QString		Longitude;								// Location for a disgruntled P2P user to aim a missile
	QString		MSNPassport;							// Microsoft's Messenger ID
	QString		MyspaceProfile;							// Myspace profile excluding http://profile.myspace.com/
	QString		RealName;								// User's real name
	QString		StateProvince;							// Your state or province
	QString		YahooID;								// Yahoo Messenger ID
};

struct sScheduler
{

};

struct sSearch
{
	QString		BlankSchemaURI;
	int			BrowseTreeSize;
	int			ClearPrevious;							// Clear previous search results? 0 - ask user; 1 - no; 2 - yes.
	bool		ExpandSearchMatches;					// Expand multi-source search matches
	int			FilterMask;
	bool		HighlightNew;
	QString		LastSchemaURI;
	int			MaxPreviewLength;						// Max size of search previews
	QString		MonitorFilter;
	int			MonitorQueue;
	QString		MonitorSchemaURI;
	bool		SchemaTypes;
	int			Throttle;								// How often each individual search may run. Low values may cause source finding to get overlooked.
	QString		ShareMonkeyBaseURL;
	bool		ShowNames;
	bool		SwitchOnDownload;						// Switch to transfers view when starting downloads
	//bool		SanityCheck;							// Drop hits of banned hosts
};

struct sSecurity
{
	bool		AllowProfileBrowse;						// Allow others to browse your user profile
	bool		AllowSharesBrowse;						// Allow others to browse your shares
	QStringList	BlockedAgentUploadFilter;				// List of bad user agents to block
	bool		DeleteFirewallException;				// Delete Firewall exception on shutdown
	bool		DeleteUPnPPorts;						// Delete forwarded ports on shutdown (UPnP)
	bool		ED2kChatFilter;							// Filter known ed2k spam. (pretty bad- always on)
	bool		EnableFirewallException;				// Create Firewall exception at startup
	bool		EnableUPnP;								// Use UPnP to automatically set up firewalls/routers
	int			FirewallState;							// Is a firewall blocking incloming connections?
	bool		ChatFilter;								// Filter out chat spam
	int			IrcFloodLimit;							// Number of messages sent by a user in 1 second before ignoring them
	bool		IrcFloodProtection;						// Automatically ignore users that flood chat rooms with text
	//int			MaxMaliciousFileSize;					// Size for which to trigger malicious software search
	bool		RemoteEnable;							// Enable remote access?
	QString		RemotePassword;							// Remote access password
	QString		RemoteUsername;							// Remote access user name
	bool		SearchIgnoreOwnIP;						// Do not accept any ports on your external IP as a source
	int			SearchSpamFilterThreshold;				// Percentage of spam hits which triggers file sources to be treated as spam
	bool		UPnPSkipWANIPSetup;						// Skip WANIPConn1 device setup (UPnP)
	bool		UPnPSkipWANPPPSetup;					// Skip WANPPPConn1 device setup (UPnP)

	/* --- Settings used by security manager --- */
	// TODO: Make sure slot securitySettingsChanged() is called if these settings are changed to inform security manager about the changes.
	bool		LogIPCheckHits;							// Post log message on isDenied( QHostAdress ) call
	bool		IgnorePrivateIP;						// Ingnore all 'local' (LAN) IPs
	quint32		RuleExpiryInterval;						// Check the security manager for expired hosts each x seconds; 0 to disable
};

struct sSkin
{
	QString		File;									// Full path to the skin file
};

struct sSystem
{
	int			CloseMode;								// How does application react when the close button it is pressed
	bool		MinimizeToTray;							// How does application react when the minimize button it is pressed
	bool		ConnectOnStartup;						// Connect to networks when starting Quazaa
	int			DiskSpaceStop;							// Value at which to pause all downloads due to low disk space
	int			DiskSpaceWarning;						// Value at which to warn the user about low disk space
	bool		StartWithSystem;						// Start with operating system
};

struct sTransfers
{
	int			BandwidthHubIn;
	int			BandwidthHubOut;
	int			BandwidthHubUploads;
	int			BandwidthLeafIn;
	int			BandwidthLeafOut;
	int			BandwidthPeerIn;
	int			BandwidthPeerOut;
	int			BandwidthRequest;
	int			BandwidthUdpOut;
	int			MinTransfersRest;						// For how long at least to suspend Transfers each round
	bool		RequireConnectedNetwork;				// Only upload/download to connected networks
	bool		SimpleProgressBar;						// Displays a simplified progress bar (lower CPU use)
};

struct sUploads
{
	bool		AllowBackwards;							// Allow data to be sent from end of range to begining where supported
	bool		AutoClear;								// Automatically clear completed uploads ('Completed' queue)
	int			ClampdownFactor;						// % threshold where additional uploads are denied
	int			ClampdownFloor;							// Floor to base clampdown factor off of
	int			ClearDelay;								// Delay between auto-clears
	bool		DynamicPreviews;						// Dynamically create previews on request
	int			FreeBandwidthFactor;					// Amount of bandwidth allocated for each upload
	int			FreeBandwidthValue;						// Amount of bandwidth remaining for uploads
	bool		HubShareLimiting;						// Limit sharing in hub mode
	int			MaxPerHost;								// Max simultaneous uploads to one remote client
	int			PreviewQuality;							// Quality of dynamically created previews
	int			PreviewTransfers;						// Max simultaneous uploads of previews
	int			QueuePollMax;
	int			QueuePollMin;
	int			RewardQueuePercentage;					// The percentage of each reward queue reserved for uploaders
	int			RotateChunkLimit;						// Limit on the size of rotating chunks
	bool		ShareHashset;							// Share the hashset for a particular file
	bool		ShareMetadata;							// Share metadata (ID3 tags, etc.) for files
	bool		SharePartials;							// Share new partial downloads
	bool		SharePreviews;							// Share previews
	bool		ShareTiger;								// Share tiger tree hashes
	bool		ThrottleMode;							// Are we throttling upload bandwidth
};

struct sWeb
{
	bool		Ares;									// Use Quazaa to open Ares links (arlnk:)
	bool		BrowserIntegration;						// Use Quazaa to manage browser downloads?
	bool		ED2K;									// Use Quazaa to open eDonkey 2k links (eD2k:)
	bool		Foxy;									// Use Quazaa to open Foxy links (slightly altered magnet)
	bool		Magnet;									// Use Quazaa to open Magnet links (magnet:)
	QStringList	ManageDownloadTypes;					// Use Quazaa to manage these download types
	bool		Piolet;									// Use Quazaa to open Piolet links (mp2p:)
	bool		Torrent;								// Use Quazaa to open torrent links (.torrent files and torrent:)
};

struct sWebServices
{
	QString		BitziAgent;
	bool		BitziOkay;
	QString		BitziWebSubmit;
	QString		BitziWebView;
	QString		BitziXML;
	QString		ShareMonkeyCid;							// Affiliate ID
	bool		ShareMonkeyOkay;
	bool		ShareMonkeySaveThumbnail;
};

struct sWinMain
{
	int			ActiveTab;								// The active tab (Home, Library, Media, etc.)
	int			ActivitySplitterRestoreTop;				// The width left side of the search results splitter should restore to when right clicked
	int			ActivitySplitterRestoreBottom;			// The width left side of the search results splitter should restore to when right clicked
	bool		ChatRoomsTaskVisible;					// The Rooms task in the Chat sidebar
	bool		ChatFriendsTaskVisible;					// The Friends task in the Chat sidebar
	QByteArray	ChatUserListSplitter;                   // The chat splitter
	QByteArray	ChatTreeWidget;                         // The list order in the server/channel tree
	QByteArray  ChatTreeWidgetSplitter;                   // The splitter for the server/channel list
	QByteArray	ChatToolbars;							// Chat Toolbars
	QByteArray	DiscoveryHeader;						// Discovery header
	QByteArray	DiscoveryToolbar;						// Discovery Toolbar
	QByteArray	DownloadsHeader;						// Downloads header
	QByteArray	DownloadsToolbar;						// Downloads Toolbar
	QByteArray	DownloadsSplitter;						// Downloads splitter position
	int			DownloadsSplitterRestoreTop;			// The top height of the downloads splitter should restore when right clicked
	int			DownloadsSplitterRestoreBottom;			// The bottom height of the downloads splitter should restore when right clicked
	QByteArray	GraphSplitter;							// Graph splitter position
	int			GraphSplitterRestoreLeft;				// The width left side of the home splitter should restore to when right clicked
	int			GraphSplitterRestoreRight;				// The width left side of the home splitter should restore to when right clicked
	QByteArray	GraphToolbar;							// Graph Toolbar
	QByteArray	HitMonitorToolbar;						// Hit Monitor Toolbar
	QString		HomeSearchString;						// The text typed into the search text box on the home tab
	QByteArray	HostCacheSplitter;						// Host Cache splitter position
	int			HostCacheSplitterRestoreLeft;			// The width left side of the home splitter should restore to when right clicked
	int			HostCacheSplitterRestoreRight;			// The width left side of the home splitter should restore to when right clicked
	QByteArray	HostCacheToolbar;						// Host Cache Toolbar
	QByteArray	LibraryDetailsSplitter;					// Library details splitter position
	int			LibraryDetailsSplitterRestoreTop;		// The width left side of the home splitter should restore to when right clicked
	int			LibraryDetailsSplitterRestoreBottom;	// The width left side of the home splitter should restore to when right clicked
	int			LibraryNavigatorTab;					// Shared Folders or Organiser
	QByteArray	LibrarySplitter;						// Library splitter position
	int			LibrarySplitterRestoreLeft;				// The width left side of the home splitter should restore to when right clicked
	int			LibrarySplitterRestoreRight;			// The width left side of the home splitter should restore to when right clicked
	QByteArray	LibraryToolbar;							// Library View Toolbar
	QByteArray	MainToolbar;							// The Navigation Toolbar
	QByteArray	MediaSplitter;							// Media splitter position
	int			MediaSplitterRestoreLeft;				// The width left side of the home splitter should restore to when right clicked
	int			MediaSplitterRestoreRight;				// The width left side of the home splitter should restore to when right clicked
	QByteArray	MediaToolbars;							// Media Toolbars
	QByteArray	NeighboursToolbars;						// Neighbours Toolbars
	QByteArray	NeighboursHeader;						// Header in the neighbours list view
	QByteArray	PacketDumpToolbar;						// Packet Dump Toolbar
	QByteArray	ActivitySplitter;						// Activity splitter position
	QByteArray	SchedulerToolbar;						// Scheduler Toolbar
	QByteArray	SearchDetailsSplitter;					// Search details splitter position
	QByteArray	SearchHeader;							// Header for all searches
	int			SearchResultsSplitterRestoreTop;		// The width left side of the search results splitter should restore to when right clicked
	int			SearchResultsSplitterRestoreBottom;		// The width left side of the search results splitter should restore to when right clicked
	bool		SearchFileTypeTaskVisible;				// The File Type task in the Search sidebar
	QByteArray	SearchMonitorToolbar;					// Search Monitor Toolbar
	bool		SearchNetworksTaskVisible;				// The Networks task in the Search sidebar
	bool		SearchResultsTaskVisible;				// The Results task in the Search sidebar
	QByteArray	SearchSplitter;							// Search Splitter
	int			SearchSplitterRestoreLeft;				// The width left side of the home splitter should restore to when right clicked
	int			SearchSplitterRestoreRight;				// The width left side of the home splitter should restore to when right clicked
	bool		SearchTaskVisible;						// The Search task in the Search sidebar
	QByteArray	SearchToolbar;							// Search Toolbar
	QByteArray	SecurityAutomaticHeader;				// The header for the automatic security list
	QByteArray	SecurityManualHeader;					// The header for the manual security list
	QByteArray	SecurityToolbars;						// Security Toolbars
	QByteArray	SystemLogToolbar;						// System Log Toolbar
	QByteArray	TransfersSplitter;						// Transfers splitter position
	int			TransfersSplitterRestoreLeft;			// The width left side of the search results splitter should restore to when right clicked
	int			TransfersSplitterRestoreRight;			// The width left side of the search results splitter should restore to when right clicked
	QByteArray	TransfersNavigationSplitter;			// Transfers splitter position
	int			TransfersNavigationSplitterRestoreTop;	// The width left side of the search results splitter should restore to when right clicked
	int			TransfersNavigationSplitterRestoreBottom;// The width left side of the search results splitter should restore to when right clicked
	QByteArray	UploadsSplitter;						// Uploads splitter position
	int			UploadsSplitterRestoreTop;				// The top height of the uploads splitter should restore when right clicked
	int			UploadsSplitterRestoreBottom;			// The bottom height of the uploads splitter should restore when right clicked
	QByteArray	UploadsToolbar;							// Uploads Toolbar
	bool		Visible;								// Is the main window visible
};
}

class QuazaaSettings : public QObject
{
	Q_OBJECT

public:
	QTranslator translator;
	QuazaaSettings();
	void saveSettings();
	void loadSettings();
	void saveChatConnections();
	void loadChatConnections();
	void saveChatConnectionWizard();
	void loadChatConnectionWizard();
	void saveChat();
	void loadChat();
	void saveProfile();
	void loadProfile();
	void saveSkinSettings();
	void loadSkinSettings();
	void saveWindowSettings( QMainWindow* window );
	void loadWindowSettings( QMainWindow* window );
	void saveLanguageSettings();
	void loadLanguageSettings();
	void saveFirstRun( bool firstRun );
	bool isFirstRun();
	void saveLogSettings();
	void loadLogSettings();

public:
	Settings::sAres				Ares;
	Settings::sBitTorrent		BitTorrent;
	Settings::sChat				Chat;
	Settings::sConnection		Connection;
	Settings::sDiscovery		Discovery;
	Settings::sDownloads		Downloads;
	Settings::sEDonkey			EDonkey;
	Settings::sGnutella			Gnutella;
	Settings::sGnutella2		Gnutella2;
	Settings::sLanguage			Language;
	Settings::sLibrary			Library;
	Settings::sLive				Live;
	Settings::sLogging			Logging;
	Settings::sMedia			Media;
	Settings::sParental			Parental;
	Settings::sPrivateMessages	PrivateMessages;
	Settings::sProfile			Profile;
	Settings::sSearch			Search;
	Settings::sSecurity			Security;
	Settings::sSkin				Skin;
	Settings::sSystem			System;
	Settings::sTransfers		Transfers;
	Settings::sUploads			Uploads;
	Settings::sWeb				Web;
	Settings::sWebServices		WebServices;
	Settings::sWinMain			WinMain;

signals:
	void chatSettingsChanged();
	void securitySettingsChanged();
};

extern QuazaaSettings quazaaSettings;

#endif // QUAZAASETTINGS_H
