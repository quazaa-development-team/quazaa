/*
* Copyright (C) 2008-2013 J-P Nurmi <jpnurmi@gmail.com>
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

#include "listviewircusers.h"
#include "menufactory.h"
#include "ircuserlistmodel.h"
#include "session.h"
#include <QItemSelectionModel>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QAction>

CListViewIrcUsers::CListViewIrcUsers( QWidget* parent ) : QListView( parent )
{
	d.menuFactory = 0;
	d.userModel = new IrcUserListModel();
	connect( this, SIGNAL( doubleClicked( QModelIndex ) ), SLOT( onDoubleClicked( QModelIndex ) ) );
}

CListViewIrcUsers::~CListViewIrcUsers()
{
}

QSize CListViewIrcUsers::sizeHint() const
{
	return QSize( 16 * fontMetrics().width( '#' ) + verticalScrollBar()->sizeHint().width(),
				  QListView::sizeHint().height() );
}

Session* CListViewIrcUsers::session() const
{
	return d.userModel->session();
}

void CListViewIrcUsers::setSession( Session* session )
{
	d.userModel->setSession( session );
}

QString CListViewIrcUsers::channel() const
{
	return d.userModel->channel();
}

void CListViewIrcUsers::setChannel( const QString& channel )
{
	d.userModel->setChannel( channel );
}

IrcUserListModel* CListViewIrcUsers::userModel() const
{
	return d.userModel;
}

bool CListViewIrcUsers::hasUser( const QString& user ) const
{
	return d.userModel->hasUser( user );
}

MenuFactory* CListViewIrcUsers::menuFactory() const
{
	if ( !d.menuFactory )
	{
		CListViewIrcUsers* that = const_cast<CListViewIrcUsers*>( this );
		that->d.menuFactory = new MenuFactory( that );
	}
	return d.menuFactory;
}

void CListViewIrcUsers::setMenuFactory( MenuFactory* factory )
{
	if ( d.menuFactory && d.menuFactory->parent() == this )
	{
		delete d.menuFactory;
	}
	d.menuFactory = factory;
}

void CListViewIrcUsers::processMessage( IrcMessage* message )
{
	d.userModel->processMessage( message );
}

void CListViewIrcUsers::contextMenuEvent( QContextMenuEvent* event )
{
	QModelIndex index = indexAt( event->pos() );
	if ( index.isValid() )
	{
		QMenu* menu = menuFactory()->createUserListMenu( index.data().toString(), this );
		menu->exec( event->globalPos() );
		menu->deleteLater();
	}
}

void CListViewIrcUsers::mousePressEvent( QMouseEvent* event )
{
	QListView::mousePressEvent( event );
	if ( !indexAt( event->pos() ).isValid() )
	{
		selectionModel()->clear();
	}
}

void CListViewIrcUsers::showEvent( QShowEvent* event )
{
	QListView::showEvent( event );
	if ( !model() )
	{
		setModel( d.userModel );
	}
}

void CListViewIrcUsers::onDoubleClicked( const QModelIndex& index )
{
	if ( index.isValid() )
	{
		emit doubleClicked( index.data( Qt::DisplayRole ).toString() );
	}
}
