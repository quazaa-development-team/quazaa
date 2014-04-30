/*
** dialogirccolordialog.h
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

#ifndef DIALOGIRCCOLORDIALOG_H
#define DIALOGIRCCOLORDIALOG_H

#include <QDialog>

namespace Ui
{
class CDialogIrcColorDialog;
}

class CDialogIrcColorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CDialogIrcColorDialog( QColor initialColor, QWidget* parent = 0 );
	~CDialogIrcColorDialog();
	QColor color();
	bool isDefaultColor();


private slots:
	void on_toolButtonCancel_clicked();
	void on_toolButtonDefault_clicked();
	void on_toolButtonDarkBlue_clicked();
	void on_toolButtonDarkGreen_clicked();
	void on_toolButtonRed_clicked();
	void on_toolButtonDarkRed_clicked();
	void on_toolButtonDarkViolet_clicked();
	void on_toolButtonOrange_clicked();
	void on_toolButtonYellow_clicked();
	void on_toolButtonLightGreen_clicked();
	void on_toolButtonCornFlowerBlue_clicked();
	void on_toolButtonLightBlue_clicked();
	void on_toolButtonBlue_clicked();
	void on_toolButtonViolet_clicked();
	void on_toolButtonDarkGray_clicked();
	void on_toolButtonGray_clicked();

private:
	Ui::CDialogIrcColorDialog* ui;
	QColor m_oIrcColor;
	bool m_bDefault;
};

#endif // DIALOGIRCCOLORDIALOG_H
