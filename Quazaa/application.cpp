/*
* Copyright (C) 2008-2013 J-P Nurmi <jpnurmi@gmail.com>
* Copyright (C) 2010-2013 SmokeX <smokexjc@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include "application.h"
#include "sharedtimer.h"
#include "connectioninfo.h"
#include <QMessageBox>
#include <QIcon>
#include <QFile>
#include <Irc>

QString CApplication::ApplicationData::slogan;
QByteArray CApplication::ApplicationData::encoding( "ISO-8859-15" );

CApplication::CApplication( int& argc, char* argv[] ) : QApplication( argc, argv )
{
}

CApplication::~CApplication()
{
}

QString CApplication::applicationSlogan()
{
	return ApplicationData::slogan;
}

void CApplication::setApplicationSlogan( const QString& slogan )
{
	ApplicationData::slogan = slogan;
}

QByteArray CApplication::encoding()
{
	return ApplicationData::encoding;
}

void CApplication::setEncoding( const QByteArray& encoding )
{
	ApplicationData::encoding = encoding;
}
