/*
** widgetprivatemessage.h
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

#ifndef WIDGETPRIVATEMESSAGE_H
#define WIDGETPRIVATEMESSAGE_H

#include <QWidget>
#include <QUrl>
#include "types.h"
#include "chatsession.h"

class QTextDocument;

namespace Ui
{
class CWidgetPrivateMessage;
}

class CWidgetPrivateMessage : public QWidget
{
	Q_OBJECT
public:
	QUuid	m_oGUID;
	QString m_sNick;
	ChatSession* m_pSession;
public:
	CWidgetPrivateMessage( QWidget* parent = 0 );
	~CWidgetPrivateMessage();
	QString inputMessage;

	void SendPrivateMessage( QTextDocument* pMessage, bool bAction = false );
	void SendPrivateMessage( QString sMessage, bool bAction = false );

protected:
	void changeEvent( QEvent* e );

private:
	Ui::CWidgetPrivateMessage* ui;

signals:
	void SendMessageS( QTextDocument*, bool );
	void SendMessageS( QString, bool );

public slots:
	void OnIncomingMessage( QString sMessage, bool bAction = false );
	void OnSystemMessage( QString sMessage );
	void OnGUIDChanged( QUuid oGUID );
	void OnNickChanged( QString sNick );

private slots:
	void on_textEdit_anchorClicked( QUrl link );
	void setSkin();
};

#endif // WIDGETPRIVATEMESSAGE_H
