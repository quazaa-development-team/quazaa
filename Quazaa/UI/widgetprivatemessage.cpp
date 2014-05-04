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

#include "widgetprivatemessage.h"
#include "ui_widgetprivatemessage.h"
#include "quazaasettings.h"
#include "chatconverter.h"
#include "skinsettings.h"

#include <QDesktopServices>
#include <QScrollBar>

#include "debug_new.h"

CWidgetPrivateMessage::CWidgetPrivateMessage( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::CWidgetPrivateMessage )
{
	ui->setupUi( this );
	m_pSession = 0;
	setSkin();
}

CWidgetPrivateMessage::~CWidgetPrivateMessage()
{
	delete ui;
}

void CWidgetPrivateMessage::changeEvent( QEvent* e )
{
	QWidget::changeEvent( e );
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CWidgetPrivateMessage::on_textEdit_anchorClicked( QUrl link )
{
	QDesktopServices::openUrl( link );
}

void CWidgetPrivateMessage::OnIncomingMessage( QString sMessage, bool bAction )
{
	qDebug() << "incoming message: " << sMessage;
	if ( bAction )
	{
		ui->textEdit->append( "* " + m_sNick + " " + sMessage );
	}
	else
	{
		ui->textEdit->append( "&lt;" + m_sNick + "&gt;: " + sMessage );
	}
}
void CWidgetPrivateMessage::OnSystemMessage( QString sMessage )
{
	qDebug() << "system message: " << sMessage;

	ui->textEdit->append( "<font color=\"#FF0000\"><b>[SYSTEM]</b> " + sMessage + "</font>" );
}
void CWidgetPrivateMessage::OnGUIDChanged( QUuid oGUID )
{
	m_oGUID = oGUID;
}
void CWidgetPrivateMessage::OnNickChanged( QString sNick )
{
	OnSystemMessage( m_sNick + " is now known as " + sNick );
	m_sNick = sNick;
	ui->labelName->setText( m_sNick );
}

void CWidgetPrivateMessage::SendPrivateMessage( QString sMessage, bool bAction )
{
	if ( bAction )
	{
		ui->textEdit->append( "* " + quazaaSettings.Profile.GnutellaScreenName + " " + sMessage );
	}
	else
	{
		ui->textEdit->append( "&lt;" + quazaaSettings.Profile.GnutellaScreenName + "&gt;: " + sMessage );
	}

	emit SendMessageS( sMessage, bAction );
}

void CWidgetPrivateMessage::SendPrivateMessage( QTextDocument* pMessage, bool bAction )
{
	ChatConverter oConv( pMessage );

	if ( bAction )
	{
		ui->textEdit->append( "* " + quazaaSettings.Profile.GnutellaScreenName + " " + oConv.toHtml() );
	}
	else
	{
		ui->textEdit->append( "&lt;" + quazaaSettings.Profile.GnutellaScreenName + "&gt;: " + oConv.toHtml() );
	}

	emit SendMessageS( pMessage->clone(), bAction );
}

void CWidgetPrivateMessage::setSkin()
{

}
