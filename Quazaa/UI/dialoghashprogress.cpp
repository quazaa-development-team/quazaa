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

#include "dialoghashprogress.h"
#include <QProgressBar>
#include <QDesktopWidget>
#include "commonfunctions.h"
#include "ui_dialoghashprogress.h"
#include "skinsettings.h"

#include <QDebug>

#include "debug_new.h"

CDialogHashProgress::CDialogHashProgress( QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::CDialogHashProgress )
{
	setWindowFlags( Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowStaysOnTopHint );
	ui->setupUi( this );
	setWindowOpacity( 0.8 );
	setSkin();
}

CDialogHashProgress::~CDialogHashProgress()
{
	delete ui;
}

void CDialogHashProgress::changeEvent( QEvent* e )
{
	QDialog::changeEvent( e );
	switch ( e->type() )
	{
	case QEvent::LanguageChange:
		ui->retranslateUi( this );
		break;
	default:
		break;
	}
}

void CDialogHashProgress::onHasherStarted( int nId )
{
	if ( m_lProgress.contains( nId ) )
	{
		return;
	}

	QLabel* label = new QLabel();
	QProgressBar* pb = new QProgressBar();
	ui->verticalLayout->addWidget( label );
	ui->verticalLayout->addWidget( pb );

	m_lProgress[nId] = qMakePair<QWidget*, QWidget*>( label, pb );
}

void CDialogHashProgress::onHasherFinished( int nId )
{
	if ( !m_lProgress.contains( nId ) )
	{
		return;
	}

	QPair<QWidget*, QWidget*> pair = m_lProgress.take( nId );
	delete pair.first;
	delete pair.second;
	if ( m_lProgress.isEmpty() )
	{
		hide();
	}
}

void CDialogHashProgress::onHashingProgress( int nId, QString sFilename, double nPercent, int nRate )
{
	if ( !m_lProgress.contains( nId ) )
	{
		return;
	}

	QString strText = sFilename + " [" + common::formatBytes( nRate ) + "/s]";
	( ( QLabel* )m_lProgress[nId].first )->setText( strText );
	( ( QProgressBar* )m_lProgress[nId].second )->setValue( nPercent );
}

void CDialogHashProgress::resizeEvent( QResizeEvent* e )
{
	QDialog::resizeEvent( e );

	int x = 0, y = 0;

	QRect rectDesktop = QApplication::desktop()->availableGeometry( this );

	x = rectDesktop.width() - width();
	y = rectDesktop.height() - height();

	setGeometry( x, y, width(), height() );
	update();
}

void CDialogHashProgress::mousePressEvent( QMouseEvent* e )
{
	hide();
	QDialog::mousePressEvent( e );
}

void CDialogHashProgress::onRemainingFilesChanged( qint32 nRemaining )
{
	QString strText( tr( "Quazaa is creating hashes. Remaining files: %1" ) );
	strText = strText.arg( nRemaining );

	ui->labelStatus->setText( strText );
}

void CDialogHashProgress::setSkin()
{

}
