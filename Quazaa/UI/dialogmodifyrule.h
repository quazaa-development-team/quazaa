/*
** dialogmodifyrule.h
**
** Copyright  Quazaa Development Team, 2009-2012.
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

#ifndef DIALOGMODIFYRULE_H
#define DIALOGMODIFYRULE_H

#include <QDialog>

#include "securitymanager.h"
#include "securerule.h"
#include "contentrule.h"
#include "hashrule.h"
#include "iprangerule.h"
#include "iprule.h"
#include "regexprule.h"
#include "useragentrule.h"
#include "widgetsecurity.h"

namespace Ui
{
	class CDialogModifyRule;
}

namespace RuleIndex {
	enum Rows {
		IPAddress = 0, IPAddressRange = 1, Hash = 2, Content = 3, RegularExpression = 4, UserAgent = 5
	};
}

class CDialogModifyRule : public QDialog
{
	Q_OBJECT

private:
	Ui::CDialogModifyRule* ui;
	CSecureRule* m_pRule;
	CWidgetSecurity* m_pParent;
public:
	// Creates a dialog window using the provided rule as basis. To create a new rule, don't provide a rule.
	CDialogModifyRule(CWidgetSecurity* parent, CSecureRule* pRule = NULL);
	~CDialogModifyRule();

protected:
	void changeEvent(QEvent* e);

private:

signals:
	void closed();

private slots:
	void on_pushButtonCancel_clicked();
	void on_pushButtonOK_clicked();
	void on_comboBoxExpire_currentIndexChanged(int index);
	void setSkin();
	void on_lineEditDays_editingFinished();
	void on_lineEditHours_editingFinished();
	void on_lineEditMinutes_editingFinished();
};

#endif // DIALOGMODIFYRULE_H
