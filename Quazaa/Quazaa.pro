#
# Quazaa.pro
#
# Copyright © Quazaaa Development Team, 2009-2014.
# This file is part of QUAZAA (quazaa.sourceforge.net)
#
# Quazaa is free software; this file may be used under the terms of the GNU
# General Public License version 3.0 or later or later as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.
#
# Quazaa is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# Please review the following information to ensure the GNU General Public
# License version 3.0 requirements will be met:
# http://www.gnu.org/copyleft/gpl.html.
#
# You should have received a copy of the GNU General Public License version
# 3.0 along with Quazaa; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

greaterThan(QT_VER_MAJ, 4) {
		QT += multimedia \
			  multimediawidgets \
			  network \
			  widgets \
			  xml
}

QT += network \
		sql \
		xml

unix:!mac {
		QT += dbus
}

TARGET = Quazaa

# Paths
DESTDIR = ./bin

CONFIG(debug, debug|release) {
		OBJECTS_DIR = temp/obj/debug
		RCC_DIR = temp/qrc/debug
}
else {
		OBJECTS_DIR = temp/obj/release
		RCC_DIR = temp/qrc/release
}

MOC_DIR = temp/moc
UI_DIR = temp/uic

INCLUDEPATH += 3rdparty \
		3rdparty/nvwa \
		3rdparty/SingleApplication \
		Chat \
		Discovery \
		FileFragments \
		HostCache \
		Misc \
		Models \
		NetworkCore \
		ShareManager \
		Skin \
		Transfers \
		UI \
		.

include(3rdparty/communi-desktop/src/src.pri)
include(Security/security.pri)

# Version stuff
MAJOR = 0
MINOR = 1
VERSION_HEADER = version.h
VERSION_HEADER_PATH = $$clean_path($$relative_path($$PWD/$$VERSION_HEADER, $$OUT_PWD))

versiontarget.target = $$VERSION_HEADER_PATH
CONFIG(debug, debug|release): versiontarget.commands = cd \"$$PWD\" && \"$$OUT_PWD/../VersionTool/debug/VersionTool\" $$MAJOR $$MINOR $$VERSION_HEADER
CONFIG(release, debug|release): versiontarget.commands = cd \"$$PWD\" && \"$$OUT_PWD/../VersionTool/release/VersionTool\" $$MAJOR $$MINOR $$VERSION_HEADER
win32-*{
	versiontarget.commands = $$replace(versiontarget.commands, '/', '\\') # for nmake
}
versiontarget.depends = FORCE
PRE_TARGETDEPS += $$VERSION_HEADER_PATH
QMAKE_EXTRA_TARGETS += versiontarget
QMAKE_CLEAN += $$VERSION_HEADER_PATH

# Language stuff
isEmpty(QMAKE_LRELEASE) {
		win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
		else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updateqm.input = TRANSLATIONS
updateqm.output = $$DESTDIR/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm $$DESTDIR/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all

# Other resources that need to be in build folder
!equals(PWD, $$OUT_PWD){
		!build_pass:message(Configuring for shadow build)
		O_SRC = $$PWD/bin/*
		O_TARGET = $$OUT_PWD/bin/
		win32:O_SRC ~= s,/,\\,g
		win32:O_TARGET ~= s,/,\\,g
		others.commands = $(COPY_DIR) $$quote($$O_SRC) $$quote($$O_TARGET)
		others.depends = FORCE
		QMAKE_EXTRA_TARGETS += others
		PRE_TARGETDEPS += others
}
equals(PWD, $$OUT_PWD){
		!build_pass:message(Configuring for in-place build)
}

# Append _debug to executable name when compiling using debug config
CONFIG(debug, debug|release):TARGET = $$join(TARGET,,,_debug)

# Additional config

CONFIG(debug, debug|release){
		DEFINES += _DEBUG
		QT_FATAL_WARNINGS = 1
}


win32 {
		LIBS += -Lbin -luser32 -lole32 -lshell32 # if you are at windows os
}
mac {
		LIBS += -lz
}
linux {
		LIBS += -lz -L/usr/lib
}
unix {
		LIBS += -lz -L/usr/lib
}

TEMPLATE = app

# MinGW-specific compiler flags (enable exception handling and disable new/delete overload)
win32-g++ {
		CONFIG += exceptions
		LIBS += libuuid
}

# MSVC-specific compiler flags
win32-msvc2008 {
		!build_pass:message(Setting up MSVC 2008 Compiler flags)
		QMAKE_CFLAGS_DEBUG += /Gd \
				/Gm \
				/RTC1
		QMAKE_CFLAGS_RELEASE += /Gd \
				/GA
		QMAKE_CXXFLAGS_DEBUG += /Gd \
				/Gm \
				/RTC1
		QMAKE_CXXFLAGS_RELEASE += /Gd \
				/GA
		QMAKE_LFLAGS_DEBUG += /FIXED:NO

		DEFINES += _CRT_SECURE_NO_WARNINGS
}

win32-msvc201* {
		!build_pass:message(Setting up MSVC 201x Compiler flags)
		QMAKE_CFLAGS_DEBUG += /Gd \
				#/Gm \
				/RTC1 \
				/MDd \
				/Zi \
				/GS \
				/MP
		QMAKE_CFLAGS_RELEASE += /Gd \
				/GA \
				/O2 \
				/MD \
				/MP
		QMAKE_CXXFLAGS_DEBUG += /Gd \
				#/Gm \
				/RTC1 \
				/MDd \
				/Zi \
				/GS \
				/MP
		QMAKE_CXXFLAGS_RELEASE += /Gd \
				/GA \
				/MD \
				/MP
		QMAKE_LFLAGS_DEBUG += /FIXED:NO

		DEFINES += _CRT_SECURE_NO_WARNINGS
}

# use CONFIG-=sse2 to disable SSE2
sse2 {
		# MSVC
		win32-msvc* {
				!build_pass:message( "SSE2 Enabled")
				QMAKE_CFLAGS_DEBUG +=/arch:SSE2
				QMAKE_CFLAGS_RELEASE += /arch:SSE2
				QMAKE_CXXFLAGS_DEBUG += /arch:SSE2
				QMAKE_CXXFLAGS_RELEASE += /arch:SSE2
		}
}

contains(DEFINES, _USE_DEBUG_NEW){
		!build_pass:message( "Building with DEBUG_NEW" )
}

# Use Qt's Zlib
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

# Headers
HEADERS += \
		$$[QT_INSTALL_HEADERS]/QtZlib/zlib.h \
		3rdparty/CyoEncode/CyoDecode.h \
		3rdparty/CyoEncode/CyoEncode.h \
		3rdparty/nvwa/debug_new.h \
		3rdparty/nvwa/fast_mutex.h \
		3rdparty/nvwa/static_assert.h \
		3rdparty/SingleApplication/localserver.h \
		3rdparty/SingleApplication/singleapplication.h \
		Chat/chatcompleter.h \
		Chat/chatconverter.h \
		Chat/chatcore.h \
		Chat/chatsession.h \
		Chat/chatsessiong2.h \
		commonfunctions.h \
		Discovery/banneddiscoveryservice.h \
		Discovery/discovery.h \
		Discovery/discoveryservice.h \
		Discovery/gwc.h \
		Discovery/networktype.h \
		FileFragments/Compatibility.hpp \
		FileFragments/Exception.hpp \
		FileFragments/FileFragments.hpp \
		FileFragments/List.hpp \
		FileFragments/Queue.hpp \
		FileFragments/Range.hpp \
		FileFragments/Ranges.hpp \
		geoiplist.h \
		HostCache/g2hostcache.h \
		HostCache/g2hostcachehost.h \
		HostCache/hostcachehost.h \
		HostCache/hostcache.h \
		Metalink/magnetlink.h \
		Metalink/metalinkhandler.h \
		Metalink/metalink4handler.h \
		Misc/debugrwlock.h \
		Misc/fileiconprovider.h \
		Misc/idprovider.h \
		Misc/networkiconprovider.h \
		Misc/timedsignalqueue.h \
		Misc/timeoutreadlocker.h \
		Misc/timeoutwritelocker.h \
		Misc/unorderedptrvector.h \
		Models/categorynavigatortreemodel.h \
		Models/discoverytablemodel.h \
		Models/downloadstreemodel.h \
		Models/downloadspeermodel.h \
		Models/hostcachetablemodel.h \
		Models/ircuserlistmodel.h \
		Models/neighbourstablemodel.h \
		Models/searchfilter.h \
		Models/searchsortfilterproxymodel.h \
		Models/searchtreemodel.h \
		Models/securityfiltermodel.h \
		Models/securitytablemodel.h \
		Models/sharesnavigatortreemodel.h \
		NetworkCore/buffer.h \
		NetworkCore/compressedconnection.h \
		NetworkCore/datagramfrags.h \
		NetworkCore/datagrams.h \
		NetworkCore/endpoint.h \
		NetworkCore/g2node.h \
		NetworkCore/g2packet.h \
		NetworkCore/handshake.h \
		NetworkCore/handshakes.h \
		NetworkCore/Hashes/hash.h \
		NetworkCore/Hashes/hashset.h \
		NetworkCore/hubhorizon.h \
		NetworkCore/managedsearch.h \
		NetworkCore/neighbour.h \
		NetworkCore/neighbours.h \
		NetworkCore/neighboursbase.h \
		NetworkCore/neighboursconnections.h \
		NetworkCore/neighboursg2.h \
		NetworkCore/neighboursrouting.h \
		NetworkCore/network.h \
		NetworkCore/networkconnection.h \
		NetworkCore/parser.h \
		NetworkCore/query.h \
		NetworkCore/queryhashgroup.h \
		NetworkCore/queryhashmaster.h \
		NetworkCore/queryhashtable.h \
		NetworkCore/queryhit.h \
		NetworkCore/querykeys.h \
		NetworkCore/ratecontroller.h \
		NetworkCore/routetable.h \
		NetworkCore/searchmanager.h \
		NetworkCore/thread.h \
		NetworkCore/types.h \
		NetworkCore/types.h \
		NetworkCore/zlibutils.h \
		quazaaglobals.h \
		quazaasettings.h \
		quazaasysinfo.h \
		ShareManager/file.h \
		ShareManager/filehasher.h \
		ShareManager/sharedfile.h \
		ShareManager/sharemanager.h \
		Skin/skinsettings.h \
		systemlog.h \
		Transfers/download.h \
		Transfers/downloads.h \
		Transfers/downloadsource.h \
		Transfers/downloadtransfer.h \
		Transfers/transfer.h \
		Transfers/transfers.h \
		UI/completerlineedit.h \
		UI/dialogabout.h \
		UI/dialogadddownload.h \
		UI/dialogaddsecuritysubscription.h \
		UI/dialogclosetype.h \
		UI/dialogconnectto.h \
		UI/dialogcreatetorrent.h \
		UI/dialogdownloadmonitor.h \
		UI/dialogdownloadproperties.h \
		UI/dialogdownloadsimport.h \
		UI/dialogeditshares.h \
		UI/dialogfiltersearch.h \
		UI/dialoghashprogress.h \
		UI/dialogimportsecurity.h \
		UI/dialogirccolordialog.h \
		UI/dialogircsettings.h \
		UI/dialoglanguage.h \
		UI/dialoglibrarysearch.h \
		UI/dialogmodifyrule.h \
		UI/dialogneighbourinfo.h \
		UI/dialogopentorrent.h \
		UI/dialogpreviewprepare.h \
		UI/dialogprivatemessages.h \
		UI/dialogprofile.h \
		UI/dialogscheduler.h \
		UI/dialogsecuritysubscriptions.h \
		UI/dialogselectvisualisation.h \
		UI/dialogsettings.h \
		UI/dialogsplash.h \
		UI/dialogtorrentproperties.h \
		UI/dialogtransferprogresstooltip.h \
		UI/ircchannelstackview.h \
		UI/mediacontrols.h \
		UI/suggestedlineedit.h \
		UI/tableview.h \
		UI/tabwidget.h \
		UI/tabwidget_p.h \
		UI/videocontainer.h \
		UI/videowidget.h \
		UI/widgetactivity.h \
		UI/widgetchatinput.h \
		UI/widgetdiscovery.h \
		UI/widgetdownloads.h \
		UI/widgetgraph.h \
		UI/widgethitmonitor.h \
		UI/widgethome.h \
		UI/widgethostcache.h \
		UI/widgetircmain.h \
		UI/widgetlibrary.h \
		UI/widgetlibraryview.h \
		UI/widgetmedia.h \
		UI/widgetneighbours.h \
		UI/widgetpacketdump.h \
		UI/widgetprivatemessage.h \
		UI/widgetreturnemittextedit.h \
		UI/widgetscheduler.h \
		UI/widgetsearch.h \
		UI/widgetsearchmonitor.h \
		UI/widgetsearchresults.h \
		UI/widgetsearchtemplate.h \
		UI/widgetsecurity.h \
		UI/widgetsmileylist.h \
		UI/widgetsystemlog.h \
		UI/widgettransfers.h \
		UI/widgetuploads.h \
		UI/winmain.h \
		UI/wizardircconnection.h \
		UI/wizardquickstart.h \
		UI/wizardtreewidget.h

# Sources
SOURCES += \
		3rdparty/CyoEncode/CyoDecode.c \
		3rdparty/CyoEncode/CyoEncode.c \
		3rdparty/nvwa/debug_new.cpp \
		3rdparty/SingleApplication/localserver.cpp \
		3rdparty/SingleApplication/singleapplication.cpp \
		Chat/chatcompleter.cpp \
		Chat/chatconverter.cpp \
		Chat/chatcore.cpp \
		Chat/chatsession.cpp \
		Chat/chatsessiong2.cpp \
		commonfunctions.cpp \
		Discovery/banneddiscoveryservice.cpp \
		Discovery/discovery.cpp \
		Discovery/discoveryservice.cpp \
		Discovery/gwc.cpp \
		Discovery/networktype.cpp \
		geoiplist.cpp \
		HostCache/g2hostcache.cpp \
		HostCache/g2hostcachehost.cpp \
		HostCache/hostcachehost.cpp \
		HostCache/hostcache.cpp \
		main.cpp \
		Misc/debugrwlock.cpp \
		Misc/fileiconprovider.cpp \
		Misc/networkiconprovider.cpp \
		Misc/timedsignalqueue.cpp \
		Metalink/magnetlink.cpp \
		Metalink/metalinkhandler.cpp \
		Metalink/metalink4handler.cpp \
		Models/categorynavigatortreemodel.cpp \
		Models/discoverytablemodel.cpp \
		Models/downloadstreemodel.cpp \
		Models/downloadspeermodel.cpp \
		Models/hostcachetablemodel.cpp \
		Models/ircuserlistmodel.cpp \
		Models/neighbourstablemodel.cpp \
		Models/searchfilter.cpp \
		Models/searchsortfilterproxymodel.cpp \
		Models/searchtreemodel.cpp \
		Models/securityfiltermodel.cpp \
		Models/securitytablemodel.cpp \
		Models/sharesnavigatortreemodel.cpp \
		NetworkCore/buffer.cpp \
		NetworkCore/compressedconnection.cpp \
		NetworkCore/datagramfrags.cpp \
		NetworkCore/datagrams.cpp \
		NetworkCore/endpoint.cpp \
		NetworkCore/g2node.cpp \
		NetworkCore/g2packet.cpp \
		NetworkCore/handshake.cpp \
		NetworkCore/handshakes.cpp \
		NetworkCore/Hashes/hash.cpp \
		NetworkCore/Hashes/hashset.cpp \
		NetworkCore/hubhorizon.cpp \
		NetworkCore/managedsearch.cpp \
		NetworkCore/neighbour.cpp \
		NetworkCore/neighbours.cpp \
		NetworkCore/neighboursbase.cpp \
		NetworkCore/neighboursconnections.cpp \
		NetworkCore/neighboursg2.cpp \
		NetworkCore/neighboursrouting.cpp \
		NetworkCore/network.cpp \
		NetworkCore/networkconnection.cpp \
		NetworkCore/parser.cpp \
		NetworkCore/query.cpp \
		NetworkCore/queryhashgroup.cpp \
		NetworkCore/queryhashmaster.cpp \
		NetworkCore/queryhashtable.cpp \
		NetworkCore/queryhit.cpp \
		NetworkCore/querykeys.cpp \
		NetworkCore/ratecontroller.cpp \
		NetworkCore/routetable.cpp \
		NetworkCore/searchmanager.cpp \
		NetworkCore/thread.cpp \
		NetworkCore/types.cpp \
		NetworkCore/zlibutils.cpp \
		quazaaglobals.cpp \
		quazaasettings.cpp \
		quazaasysinfo.cpp \
		ShareManager/file.cpp \
		ShareManager/filehasher.cpp \
		ShareManager/sharedfile.cpp \
		ShareManager/sharemanager.cpp \
		Skin/skinsettings.cpp \
		systemlog.cpp \
		Transfers/download.cpp \
		Transfers/downloads.cpp \
		Transfers/downloadsource.cpp \
		Transfers/downloadtransfer.cpp \
		Transfers/transfer.cpp \
		Transfers/transfers.cpp \
		UI/completerlineedit.cpp \
		UI/dialogabout.cpp \
		UI/dialogadddownload.cpp \
		UI/dialogaddsecuritysubscription.cpp \
		UI/dialogclosetype.cpp \
		UI/dialogconnectto.cpp \
		UI/dialogcreatetorrent.cpp \
		UI/dialogdownloadmonitor.cpp \
		UI/dialogdownloadproperties.cpp \
		UI/dialogdownloadsimport.cpp \
		UI/dialogeditshares.cpp \
		UI/dialogfiltersearch.cpp \
		UI/dialoghashprogress.cpp \
		UI/dialogimportsecurity.cpp \
		UI/dialogirccolordialog.cpp \
		UI/dialogircsettings.cpp \
		UI/dialoglanguage.cpp \
		UI/dialoglibrarysearch.cpp \
		UI/dialogmodifyrule.cpp \
		UI/dialogneighbourinfo.cpp \
		UI/dialogopentorrent.cpp \
		UI/dialogpreviewprepare.cpp \
		UI/dialogprivatemessages.cpp \
		UI/dialogprofile.cpp \
		UI/dialogscheduler.cpp \
		UI/dialogsecuritysubscriptions.cpp \
		UI/dialogselectvisualisation.cpp \
		UI/dialogsettings.cpp \
		UI/dialogsplash.cpp \
		UI/dialogtorrentproperties.cpp \
		UI/dialogtransferprogresstooltip.cpp \
		UI/ircchannelstackview.cpp \
		UI/mediacontrols.cpp \
		UI/suggestedlineedit.cpp \
		UI/tableview.cpp \
		UI/tabwidget.cpp \
		UI/videowidget.cpp \
		UI/videocontainer.cpp \
		UI/widgetactivity.cpp \
		UI/widgetchatinput.cpp \
		UI/widgetdiscovery.cpp \
		UI/widgetdownloads.cpp \
		UI/widgetgraph.cpp \
		UI/widgethitmonitor.cpp \
		UI/widgethome.cpp \
		UI/widgethostcache.cpp \
		UI/widgetircmain.cpp \
		UI/widgetlibrary.cpp \
		UI/widgetlibraryview.cpp \
		UI/widgetmedia.cpp \
		UI/widgetneighbours.cpp \
		UI/widgetpacketdump.cpp \
		UI/widgetprivatemessage.cpp \
		UI/widgetreturnemittextedit.cpp \
		UI/widgetscheduler.cpp \
		UI/widgetsearch.cpp \
		UI/widgetsearchmonitor.cpp \
		UI/widgetsearchresults.cpp \
		UI/widgetsearchtemplate.cpp \
		UI/widgetsecurity.cpp \
		UI/widgetsmileylist.cpp \
		UI/widgetsystemlog.cpp \
		UI/widgettransfers.cpp \
		UI/widgetuploads.cpp \
		UI/winmain.cpp \
		UI/wizardircconnection.cpp \
		UI/wizardtreewidget.cpp \
		UI/wizardquickstart.cpp

FORMS += \
		UI/dialogabout.ui \
		UI/dialogadddownload.ui \
		UI/dialogaddsecuritysubscription.ui \
		UI/dialogclosetype.ui \
		UI/dialogconnectto.ui \
		UI/dialogcreatetorrent.ui \
		UI/dialogdownloadmonitor.ui \
		UI/dialogdownloadproperties.ui \
		UI/dialogdownloadsimport.ui \
		UI/dialogeditshares.ui \
		UI/dialogfiltersearch.ui \
		UI/dialoghashprogress.ui \
		UI/dialogimportsecurity.ui \
		UI/dialogirccolordialog.ui \
		UI/dialogircsettings.ui \
		UI/dialoglanguage.ui \
		UI/dialoglibrarysearch.ui \
		UI/dialogmodifyrule.ui \
		UI/dialogneighbourinfo.ui \
		UI/dialogopentorrent.ui \
		UI/dialogpreviewprepare.ui \
		UI/dialogprivatemessages.ui \
		UI/dialogprofile.ui \
		UI/dialogscheduler.ui \
		UI/dialogsecuritysubscriptions.ui \
		UI/dialogselectvisualisation.ui \
		UI/dialogsettings.ui \
		UI/dialogsplash.ui \
		UI/dialogtorrentproperties.ui \
		UI/dialogtransferprogresstooltip.ui \
		UI/mediacontrols.ui \
		UI/mediasettings.ui \
		UI/videocontainer.ui \
		UI/widgetactivity.ui \
		UI/widgetchatinput.ui \
		UI/widgetdiscovery.ui \
		UI/widgetdownloads.ui \
		UI/widgetgraph.ui \
		UI/widgethitmonitor.ui \
		UI/widgethome.ui \
		UI/widgethostcache.ui \
		UI/widgetlibrary.ui \
		UI/widgetlibraryview.ui \
		UI/widgetmedia.ui \
		UI/widgetneighbours.ui \
		UI/widgetpacketdump.ui \
		UI/widgetprivatemessage.ui \
		UI/widgetscheduler.ui \
		UI/widgetsearch.ui \
		UI/widgetsearchmonitor.ui \
		UI/widgetsearchresults.ui \
		UI/widgetsearchtemplate.ui \
		UI/widgetsecurity.ui \
		UI/widgetsmileylist.ui \
		UI/widgetsystemlog.ui \
		UI/widgettransfers.ui \
		UI/widgetuploads.ui \
		UI/winmain.ui \
		UI/wizardircconnection.ui \
		UI/wizardquickstart.ui

TRANSLATIONS = \
		Language/quazaa_af.ts \
		Language/quazaa_ar.ts \
		Language/quazaa_ca.ts \
		Language/quazaa_chs.ts \
		Language/quazaa_cz.ts \
		Language/quazaa_de.ts \
		Language/quazaa_default_en.ts \
		Language/quazaa_ee.ts \
		Language/quazaa_es.ts \
		Language/quazaa_fa.ts \
		Language/quazaa_fi.ts \
		Language/quazaa_fr.ts \
		Language/quazaa_gr.ts \
		Language/quazaa_heb.ts \
		Language/quazaa_hr.ts \
		Language/quazaa_hu.ts \
		Language/quazaa_it.ts \
		Language/quazaa_ja.ts \
		Language/quazaa_lt.ts \
		Language/quazaa_nl.ts \
		Language/quazaa_no.ts \
		Language/quazaa_pl.ts \
		Language/quazaa_pt-br.ts \
		Language/quazaa_pt.ts \
		Language/quazaa_ru.ts \
		Language/quazaa_sl-si.ts \
		Language/quazaa_sq.ts \
		Language/quazaa_sr.ts \
		Language/quazaa_sv.ts \
		Language/quazaa_tr.ts \
		Language/quazaa_tw.ts

RESOURCES += Resource.qrc
RC_FILE = Quazaa.rc
OTHER_FILES += LICENSE.GPL3

mac {
		QM_DATA.path = Contents/MacOS
		QM_DATA.files += $$DESTDIR/DefaultSecurity.dat
		QM_DATA.files += $$DESTDIR/DefaultServices.dat
		QM_DATA.files += $$DESTDIR/notify.mp3
		QM_DATA.files += $$DESTDIR/portable.ini
		for(translation, TRANSLATIONS) {
				translation ~= s/\\.ts$/.qm
				QM_DATA.files += $$DESTDIR/$$basename(translation)
		}
		QMAKE_BUNDLE_DATA += QM_DATA

		GEOIP_DATA.path = Contents/MacOS
		GEOIP_DATA.files += $$DESTDIR/GeoIP
		QMAKE_BUNDLE_DATA += GEOIP_DATA
}
