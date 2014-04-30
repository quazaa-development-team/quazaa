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

#ifndef WIDGETIRCMAIN_H
#define WIDGETIRCMAIN_H

#include <QMainWindow>
#include <QPointer>

class SessionStackView;
class SessionTreeWidget;
class SoundNotification;
class SessionTreeItem;
struct ConnectionInfo;
class MessageView;
class IrcMessage;
class QtDockTile;
class IrcConnection;
class QShortcut;
class HomePage;
class Overlay;
class QSplitter;

class CWidgetIrcMain : public QMainWindow
{
	Q_OBJECT

public:
	explicit CWidgetIrcMain( QWidget* parent = 0 );
	~CWidgetIrcMain();

	QSize sizeHint() const;
	void saveWidget();

public slots:
	void connectTo( const QString& host = QString(), quint16 port = 6667,
					const QString& nick = QString(), const QString& password = QString() );
	void connectTo( const ConnectionInfo& connection );
	void connectToImpl( const ConnectionInfo& connection );

protected:
	void closeEvent( QCloseEvent* event );
	void changeEvent( QEvent* event );

private slots:
	void initialize();
	void editConnection( IrcConnection* connection );
	void applySettings();
	void highlighted( IrcMessage* message );
	void missed( IrcMessage* message );
	void viewAdded( MessageView* view );
	void viewActivated( MessageView* view );
	void closeTreeItem( SessionTreeItem* item );
	void currentTreeItemChanged( IrcConnection* connection, const QString& view );
	void splitterChanged( const QByteArray& state );
	void updateOverlay();
	void reconnect();
	void addView();
	void closeView();
	void searchView();
	void showSettings();
	void alert();

private:
	void createTree();
	void createHome();
	QString generateStyleSheet( bool dark );

	QSplitter* splitterIrcMain;
	SessionTreeWidget* treeWidget;
	SessionStackView* stackView;
	QtDockTile* dockTile;
	QShortcut* searchShortcut;
	QPointer<Overlay> overlay;
	SoundNotification* sound;
	HomePage* homePage;
};

#endif // WIDGETIRCMAIN_H
