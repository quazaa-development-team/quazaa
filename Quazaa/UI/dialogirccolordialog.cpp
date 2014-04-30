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

#include "dialogirccolordialog.h"
#include "ui_dialogirccolordialog.h"

#include "debug_new.h"

CDialogIrcColorDialog::CDialogIrcColorDialog( QColor initialColor, QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::CDialogIrcColorDialog )
{
	ui->setupUi( this );
	m_oIrcColor = initialColor;
	m_bDefault = false;
}

CDialogIrcColorDialog::~CDialogIrcColorDialog()
{
	delete ui;
}

void CDialogIrcColorDialog::on_toolButtonCancel_clicked()
{
	reject();
}

bool CDialogIrcColorDialog::isDefaultColor()
{
	return m_bDefault;
}

QColor CDialogIrcColorDialog::color()
{
	return m_oIrcColor;
}

void CDialogIrcColorDialog::on_toolButtonDefault_clicked()
{
	m_bDefault = true;
	accept();
}

void CDialogIrcColorDialog::on_toolButtonDarkBlue_clicked()
{
	m_oIrcColor.setNamedColor( "darkblue" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonDarkGreen_clicked()
{
	m_oIrcColor.setNamedColor( "darkgreen" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonRed_clicked()
{
	m_oIrcColor.setNamedColor( "red" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonDarkRed_clicked()
{
	m_oIrcColor.setNamedColor( "darkred" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonDarkViolet_clicked()
{
	m_oIrcColor.setNamedColor( "darkviolet" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonOrange_clicked()
{
	m_oIrcColor.setNamedColor( "orange" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonYellow_clicked()
{
	m_oIrcColor.setNamedColor( "yellow" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonLightGreen_clicked()
{
	m_oIrcColor.setNamedColor( "lightgreen" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonCornFlowerBlue_clicked()
{
	m_oIrcColor.setNamedColor( "cornflowerblue" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonLightBlue_clicked()
{
	m_oIrcColor.setNamedColor( "lightblue" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonBlue_clicked()
{
	m_oIrcColor.setNamedColor( "blue" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonViolet_clicked()
{
	m_oIrcColor.setNamedColor( "violet" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonDarkGray_clicked()
{
	m_oIrcColor.setNamedColor( "gray" );
	accept();
}

void CDialogIrcColorDialog::on_toolButtonGray_clicked()
{
	m_oIrcColor.setNamedColor( "darkgray" );
	accept();
}
