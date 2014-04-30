/*
** widgetquickstart.h
**
** Copyright © Quazaa Development Team, 2009-2012.
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

#ifndef WIZARDQUICKSTART_H
#define WIZARDQUICKSTART_H

#include <QWizard>

namespace Ui
{
class CWizardQuickStart;
}

class CWizardQuickStart : public QWizard
{
	Q_OBJECT

public:
	explicit CWizardQuickStart( QWidget* parent = NULL );
	~CWizardQuickStart();

protected:
	virtual void changeEvent( QEvent* e );

private:
	Ui::CWizardQuickStart* ui;

private slots:
	void accept();
	void on_toolButtonSharesAdd_clicked();
	void on_toolButtonSharesRemove_clicked();
	void setSkin();
	void on_checkBoxUPnP_stateChanged( int arg1 );
	void on_pushButtonRandomizePort_clicked();
};

#endif // WIZARDQUICKSTART_H

