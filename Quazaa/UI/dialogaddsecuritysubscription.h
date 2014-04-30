/*
** dialogaddsecuritysubscription.h
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

#ifndef DIALOGADDSECURITYSUBSCRIPTION_H
#define DIALOGADDSECURITYSUBSCRIPTION_H

#include <QDialog>

namespace Ui
{
class CDialogAddSecuritySubscription;
}

class CDialogAddSecuritySubscription : public QDialog
{
	Q_OBJECT
public:
	CDialogAddSecuritySubscription( QWidget* parent = 0 );
	~CDialogAddSecuritySubscription();

protected:
	void changeEvent( QEvent* e );

private:
	Ui::CDialogAddSecuritySubscription* ui;

signals:
	void closed();

private slots:
	void on_pushButtonCancel_clicked();
	void on_pushButtonSubscribe_clicked();
	void setSkin();
};

#endif // DIALOGADDSECURITYSUBSCRIPTION_H
