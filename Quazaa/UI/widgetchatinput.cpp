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

#include "widgetchatinput.h"
#include "ui_widgetchatinput.h"
#include "dialogconnectto.h"
#include "dialogirccolordialog.h"
#include "skinsettings.h"
#include "quazaasettings.h"

#include "chatsessiong2.h"

#include <QColorDialog>
#include <QPalette>
#include <QSpacerItem>
#include <QRegularExpression>

#include "debug_new.h"

CWidgetChatInput::CWidgetChatInput( QWidget* parent, bool isIrc ) :
	QMainWindow( parent ),
	ui( new Ui::CWidgetChatInput )
{
	ui->setupUi( this );
	bIsIrc = isIrc;

	defaultColor = ui->textEditInput->textColor();

	QTextCharFormat format;
	format.setFontStyleHint( QFont::TypeWriter );

	connect( &quazaaSettings, SIGNAL( chatSettingsChanged() ), this, SLOT( applySettings() ) );

	ui->textEditInput->setCurrentCharFormat( format );

	connect( ui->textEditInput, SIGNAL( cursorPositionChanged() ), this, SLOT( updateToolbar() ) );

	connect( ui->textEditInput, SIGNAL( returnPressed() ), ui->toolButtonSend, SLOT( click() ) );
	connect( ui->textEditInput, SIGNAL( currentCharFormatChanged( QTextCharFormat ) ), this,
			 SLOT( onTextFormatChange( QTextCharFormat ) ) );

	toolButtonSmilies = new QToolButton();
	toolButtonSmilies->setPopupMode( QToolButton::InstantPopup );
	toolButtonSmilies->setToolTip( tr( "Smilies" ) );
	toolButtonSmilies->setIcon( QIcon( ":/Resource/Smileys/0.png" ) );
	widgetSmileyList = new CWidgetSmileyList( this );
	toolButtonSmilies->setMenu( widgetSmileyList );

	toolButtonPickColor = new QToolButton( this );
	toolButtonPickColor->setIconSize( QSize( 24, 24 ) );
	if ( bIsIrc )
	{
		if ( quazaaSettings.Chat.DarkTheme )
		{
			toolButtonPickColor->setIcon( QIcon( ":/Resource/Chat/PaletteDark.png" ) );
		}
		else
		{
			toolButtonPickColor->setIcon( QIcon( ":/Resource/Chat/Palette.png" ) );
		}
		toolButtonPickColor->setStyleSheet( "" );
	}
	else
	{
		toolButtonPickColor->setIcon( QIcon() );
		toolButtonPickColor->setStyleSheet(
			QString( "QToolButton { background-color: %1; border-style: outset; border-width: 2px;	border-radius: 6px; border-color: lightgrey; }" ).arg(
				ui->textEditInput->textColor().name() ) );
	}
	toolButtonPickColor->setToolTip( tr( "Font Color" ) );
	connect( toolButtonPickColor, SIGNAL( clicked() ), this, SLOT( pickColor() ) );

	QWidget* spacer = new QWidget( this );
	spacer->setStyleSheet( "background-color: transparent;" );
	spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
	labelLag = new QLabel( this );

	ui->toolBarTextTools->insertWidget( ui->actionBold, toolButtonPickColor );
	ui->toolBarTextTools->addSeparator();
	ui->toolBarTextTools->addWidget( toolButtonSmilies );
	ui->toolBarTextTools->addWidget( spacer );
	ui->toolBarTextTools->addWidget( labelLag );
	ui->actionBold->setChecked( ui->textEditInput->fontWeight() == QFont::Bold );
	ui->actionItalic->setChecked( ui->textEditInput->fontItalic() );
	ui->actionUnderline->setChecked( ui->textEditInput->fontUnderline() );
	connect( ui->actionItalic, SIGNAL( toggled( bool ) ), ui->textEditInput, SLOT( setFontItalic( bool ) ) );
	connect( ui->actionUnderline, SIGNAL( toggled( bool ) ), ui->textEditInput, SLOT( setFontUnderline( bool ) ) );
	applySettings();
	updateToolbar();
}

CWidgetChatInput::~CWidgetChatInput()
{
	delete widgetSmileyList;
	delete ui;
}

void CWidgetChatInput::changeEvent( QEvent* e )
{
	QMainWindow::changeEvent( e );
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CWidgetChatInput::on_toolButtonSend_clicked()
{
	if ( !ui->textEditInput->document()->isEmpty() )
	{
		if ( ui->textEditInput->document()->lineCount() > 1 )
		{
			QStringList lineList = ui->textEditInput->document()->toHtml().split( QRegularExpression( "[\\r\\n]" ),
																				  QString::KeepEmptyParts );

			for ( int i = 4; i < lineList.size(); i++ )
			{
				QTextDocument* line = new QTextDocument();
				line->setHtml( lineList.at( i ) );
				if ( line->toPlainText().startsWith( "/" ) )
				{
					emit messageSent( line->toPlainText() );
				}
				else
				{
					emit messageSent( line );
				}
			}
		}
		else
		{
			if ( ui->textEditInput->document()->toPlainText().startsWith( "/" ) )
			{
				emit messageSent( ui->textEditInput->document()->toPlainText() );
			}
			else
			{
				emit messageSent( ui->textEditInput->document() );
			}
		}
		ui->textEditInput->addHistory( ui->textEditInput->document() );
		ui->textEditInput->resetHistoryIndex();
		QTextCharFormat oldFormat = ui->textEditInput->currentCharFormat();
		ui->textEditInput->document()->clear();
		ui->textEditInput->setCurrentCharFormat( oldFormat );
	}
}

void CWidgetChatInput::setText( QString text )
{
	ui->textEditInput->setHtml( text );
}

void CWidgetChatInput::onTextFormatChange( QTextCharFormat newFormat )
{
	if ( newFormat.fontWeight() == QFont::Normal )
	{
		ui->actionBold->setChecked( false );
	}
	else if ( newFormat.fontWeight() == QFont::Bold )
	{
		ui->actionBold->setChecked( true );
	}

	ui->actionItalic->setChecked( newFormat.fontItalic() );
	ui->actionUnderline->setChecked( newFormat.fontUnderline() );
}

void CWidgetChatInput::on_actionBold_toggled( bool checked )
{
	ui->textEditInput->setFontWeight( ( checked ? QFont::Bold : QFont::Normal ) );
}

void CWidgetChatInput::on_actionItalic_toggled( bool checked )
{
	ui->textEditInput->setFontItalic( checked );
}

void CWidgetChatInput::on_actionUnderline_toggled( bool checked )
{
	ui->textEditInput->setFontUnderline( checked );
}

void CWidgetChatInput::pickColor()
{
	QColor fontColor;
	if ( bIsIrc )
	{
		CDialogIrcColorDialog* dlgIrcColor = new CDialogIrcColorDialog( ui->textEditInput->textColor(), this );
		bool accepted = dlgIrcColor->exec();
		if ( accepted )
		{
			if ( !dlgIrcColor->isDefaultColor() )
			{
				fontColor = dlgIrcColor->color();
				ui->textEditInput->setTextColor( fontColor );
			}
			else
			{
				fontColor = defaultColor;
				ui->textEditInput->setTextColor( fontColor );
			}
		}
	}
	else
	{
		fontColor = QColorDialog::getColor( ui->textEditInput->textColor(), this, tr( "Select Font Color" ) );

		if ( fontColor.isValid() )
		{
			ui->textEditInput->setTextColor( fontColor );
		}
	}

	updateToolbar();
}

void CWidgetChatInput::updateToolbar()
{
	if ( bIsIrc )
	{
		if ( ( ui->textEditInput->textColor() == defaultColor ) )
		{
			if ( quazaaSettings.Chat.DarkTheme )
			{
				toolButtonPickColor->setIcon( QIcon( ":/Resource/Chat/PaletteDark.png" ) );
			}
			else
			{
				toolButtonPickColor->setIcon( QIcon( ":/Resource/Chat/Palette.png" ) );
			}
			toolButtonPickColor->setStyleSheet( "" );
		}
		else
		{
			toolButtonPickColor->setIcon( QIcon() );
			toolButtonPickColor->setStyleSheet(
				QString( "QToolButton { background-color: %1; border-style: outset; border-width: 2px;	border-radius: 6px; border-color: lightgrey; }" ).arg(
					ui->textEditInput->textColor().name() ) );
		}
	}
	else
	{
		toolButtonPickColor->setStyleSheet(
			QString( "QToolButton { background-color: %1; border-style: outset; border-width: 2px;	border-radius: 6px; border-color: lightgrey; }" ).arg(
				ui->textEditInput->textColor().name() ) );
	}
	ui->actionBold->setChecked( ui->textEditInput->fontWeight() == QFont::Bold );
	ui->actionItalic->setChecked( ui->textEditInput->fontItalic() );
	ui->actionUnderline->setChecked( ui->textEditInput->fontUnderline() );
}

CWidgetReturnEmitTextEdit* CWidgetChatInput::textEdit()
{
	return ui->textEditInput;
}

QLabel* CWidgetChatInput::helpLabel()
{
	return ui->helpLabel;
}

QLabel* CWidgetChatInput::lagLabel()
{
	return labelLag;
}

void CWidgetChatInput::applySettings()
{
	if ( bIsIrc ) // Only apply Irc theme if text input belongs to irc
	{
		if ( quazaaSettings.Chat.DarkTheme )
		{
			QString baseStyleSheet( "CWidgetChatInput { background: #222222; } " );
			baseStyleSheet +=
				QString( "QToolButton { border: none; background-position: center; background-repeat: no-repeat; padding: 2px; color: %1; } " ).arg(
					quazaaSettings.Chat.Colors[IrcColorType::Default] );
			baseStyleSheet += QString( "QToolButton[popupMode=\"2\"] { padding-right: 10px; } " );
			baseStyleSheet += QString( "QToolButton:hover, QToolButton:pressed { background: #666666; border-radius: 4px; } " );
			setStyleSheet( baseStyleSheet );
			labelLag->setStyleSheet( QString( "color: %11; padding-right: 5px;" ).arg(
										 quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			ui->textEditInput->setStyleSheet(
				QString( "border: 1px solid transparent; background: #222222; selection-color: #dedede; selection-background-color: #444444; color: %1" ).arg(
					quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			ui->helpLabel->setStyleSheet(
				QString( "QLabel#helpLabel { color: %1; border: 1px solid transparent; border-top-color: palette(dark); background: #222222; } " ).arg(
					quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			toolButtonPickColor->setIcon( QIcon( ":/Resource/Chat/PaletteDark.png" ) );
			ui->actionBold->setIcon( QIcon( ":/Resource/Generic/FormatTextBoldDark.png" ) );
			ui->actionItalic->setIcon( QIcon( ":/Resource/Generic/FormatTextItalicDark.png" ) );
			ui->actionUnderline->setIcon( QIcon( ":/Resource/Generic/FormatTextUnderlineDark.png" ) );
			ui->toolBarTextTools->setStyleSheet( QString( "QToolBar { background: #222222; color: %1; }" ).arg(
													 quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			ui->toolButtonSend->setIcon( QIcon( ":/Resource/Chat/SendDark.png" ) );
		}
		else
		{
			QString baseStyleSheet( "CWidgetChatInput { background: palette(alternate-base); } " );
			baseStyleSheet += QString( "QToolButton { color: %1; } " ).arg( quazaaSettings.Chat.Colors[IrcColorType::Default] );
			setStyleSheet( baseStyleSheet );
			labelLag->setStyleSheet( QString( "color: %1; padding-right: 5px;" ).arg(
										 quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			ui->textEditInput->setStyleSheet(
				QString( "background: palette(alternate-base); border: 1px solid transparent; color: %1" ).arg(
					quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			ui->helpLabel->setStyleSheet(
				QString( "QLabel#helpLabel { color: %1; border: 1px solid transparent; border-top-color: palette(dark); } " ).arg(
					quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			toolButtonPickColor->setIcon( QIcon( ":/Resource/Chat/Palette.png" ) );
			ui->actionBold->setIcon( QIcon( ":/Resource/Generic/FormatTextBold.png" ) );
			ui->actionItalic->setIcon( QIcon( ":/Resource/Generic/FormatTextItalic.png" ) );
			ui->actionUnderline->setIcon( QIcon( ":/Resource/Generic/FormatTextUnderline.png" ) );
			ui->toolBarTextTools->setStyleSheet( QString( "QToolBar { background: palette(alternate-base); color: %1; }" ).arg(
													 quazaaSettings.Chat.Colors[IrcColorType::Default] ) );
			ui->toolButtonSend->setIcon( QIcon( ":/Resource/Chat/Send.png" ) );
		}
	}
}

void CWidgetChatInput::setLag( qint64 lag )
{
	if ( lag == -1 )
	{
		labelLag->setText( "" );
	}
	else
	{
		labelLag->setText( tr( "%1 ms" ).arg( lag ) );
	}
}
