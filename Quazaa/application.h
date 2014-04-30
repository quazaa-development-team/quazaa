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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class CApplication : public QApplication
{
	Q_OBJECT

public:
	CApplication( int& argc, char* argv[] );
	~CApplication();

	static QString applicationSlogan();
	static void setApplicationSlogan( const QString& slogan );

	static QByteArray encoding();
	static void setEncoding( const QByteArray& encoding );

public slots:

private:
	struct ApplicationData
	{
		static QString slogan;
		static QByteArray encoding;
	};
};

#endif // APPLICATION_H
