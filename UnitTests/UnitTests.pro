#
# UnitTests.pro
#
# Copyright © Quazaaa Development Team, 2013-2013.
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


QT += multimedia \
	  multimediawidgets \
	  network \
	  testlib \
	  widgets \
	  xml

TARGET = tst_unittestssecurity
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

Q_DIR = ../Quazaa

INCLUDEPATH += $$Q_DIR \
			   $$Q_DIR/3rdparty \
			   $$Q_DIR/3rdparty/nvwa \
			   $$Q_DIR/3rdparty/SingleApplication \
			   $$Q_DIR/Chat \
			   $$Q_DIR/Discovery \
			   $$Q_DIR/FileFragments \
			   $$Q_DIR/HostCache \
			   $$Q_DIR/Misc \
			   $$Q_DIR/Models \
			   $$Q_DIR/NetworkCore \
			   $$Q_DIR/ShareManager \
			   $$Q_DIR/Skin \
			   $$Q_DIR/Transfers \
			   $$Q_DIR/UI \
			   .

include( $$Q_DIR/Security/security.pri )

HEADERS += unittestsecurity.h \
		   $$Q_DIR/3rdparty/CyoEncode/CyoDecode.h \
		   $$Q_DIR/3rdparty/CyoEncode/CyoEncode.h \
		   $$Q_DIR/commonfunctions.h \
		   $$Q_DIR/geoiplist.h \
		   $$Q_DIR/Misc/timedsignalqueue.h \
		   $$Q_DIR/NetworkCore/buffer.h \
		   $$Q_DIR/NetworkCore/endpoint.h \
		   $$Q_DIR/NetworkCore/g2packet.h \
		   $$Q_DIR/NetworkCore/Hashes/hash.h \
		   $$Q_DIR/NetworkCore/queryhit.h \
		   $$Q_DIR/quazaaglobals.h \
		   $$Q_DIR/quazaasettings.h \
		   $$Q_DIR/systemlog.h

SOURCES += unitmain.cpp \
		   unittestssecurity.cpp \
		   $$Q_DIR/3rdparty/CyoEncode/CyoDecode.c \
		   $$Q_DIR/3rdparty/CyoEncode/CyoEncode.c \
		   $$Q_DIR/commonfunctions.cpp \
		   $$Q_DIR/geoiplist.cpp \
		   $$Q_DIR/Misc/timedsignalqueue.cpp \
		   $$Q_DIR/NetworkCore/buffer.cpp \
		   $$Q_DIR/NetworkCore/endpoint.cpp \
		   $$Q_DIR/NetworkCore/g2packet.cpp \
		   $$Q_DIR/NetworkCore/Hashes/hash.cpp \
		   $$Q_DIR/NetworkCore/queryhit.cpp \
		   $$Q_DIR/quazaaglobals.cpp \
		   $$Q_DIR/quazaasettings.cpp \
		   $$Q_DIR/systemlog.cpp


DEFINES += SRCDIR=\\\"$$PWD/\\\" \
		   QUAZAA_SETUP_UNIT_TESTS
