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

#include "widgetsystemlog.h"
#include "ui_widgetsystemlog.h"

#include "quazaasettings.h"
#include "skinsettings.h"

#include <QMenu>

#include "debug_new.h"

WidgetSystemLog::WidgetSystemLog( QWidget* parent ) :
	QMainWindow( parent ),
	ui( new Ui::WidgetSystemLog )
{
	ui->setupUi( this );
	QFont font( "Monospace" );
	font.setStyleHint( QFont::TypeWriter );
	ui->textEditSystemLog->setFont( font );
	ui->textEditSystemLog->document()->setMaximumBlockCount( 100 );
	quazaaSettings.loadLogSettings();
	ui->actionPauseLogDisplay->setChecked( quazaaSettings.Logging.IsPaused );

	m_pLogMenu = new QMenu( ui->textEditSystemLog );
	m_pLogMenu->addAction( ui->actionCopy            );
	m_pLogMenu->addSeparator();
	m_pLogMenu->addAction( ui->actionShowInformation );
	m_pLogMenu->addAction( ui->actionShowSecurity    );
	m_pLogMenu->addAction( ui->actionShowNotice      );
	m_pLogMenu->addAction( ui->actionShowDebug       );
	m_pLogMenu->addAction( ui->actionShowWarnings    );
	m_pLogMenu->addAction( ui->actionShowError       );
	m_pLogMenu->addAction( ui->actionShowCritical    );

	ui->actionShowInformation->setChecked( quazaaSettings.Logging.ShowInformation  );
	ui->actionShowSecurity   ->setChecked( quazaaSettings.Logging.ShowSecurity     );
	ui->actionShowNotice     ->setChecked( quazaaSettings.Logging.ShowNotice       );
	ui->actionShowDebug      ->setChecked( quazaaSettings.Logging.ShowDebug        );
	ui->actionShowWarnings   ->setChecked( quazaaSettings.Logging.ShowWarnings     );
	ui->actionShowError      ->setChecked( quazaaSettings.Logging.ShowError        );
	ui->actionShowCritical   ->setChecked( quazaaSettings.Logging.ShowCritical     );
	ui->actionToggleTimestamp->setChecked( quazaaSettings.Logging.LogShowTimestamp );

	restoreState( quazaaSettings.WinMain.SystemLogToolbar );

	connect( &systemLog, &SystemLog::logPosted, this, &WidgetSystemLog::appendLog );

	setSkin();
}

WidgetSystemLog::~WidgetSystemLog()
{
	delete ui;
}

void WidgetSystemLog::saveWidget()
{
	quazaaSettings.WinMain.SystemLogToolbar = saveState();
	quazaaSettings.Logging.LogShowTimestamp = ui->actionToggleTimestamp->isChecked();
	quazaaSettings.Logging.ShowWarnings     = ui->actionShowWarnings   ->isChecked();
	quazaaSettings.Logging.ShowInformation  = ui->actionShowInformation->isChecked();
	quazaaSettings.Logging.ShowSecurity     = ui->actionShowSecurity   ->isChecked();
	quazaaSettings.Logging.ShowNotice       = ui->actionShowNotice     ->isChecked();
	quazaaSettings.Logging.ShowDebug        = ui->actionShowDebug      ->isChecked();
	quazaaSettings.Logging.ShowError        = ui->actionShowError      ->isChecked();
	quazaaSettings.Logging.ShowCritical     = ui->actionShowCritical   ->isChecked();
	quazaaSettings.Logging.IsPaused         = ui->actionPauseLogDisplay->isChecked();
	quazaaSettings.saveLogSettings();
}

void WidgetSystemLog::changeEvent( QEvent* e )
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

void WidgetSystemLog::on_actionCopy_triggered()
{
	ui->textEditSystemLog->copy();
}

void WidgetSystemLog::on_textEditSystemLog_customContextMenuRequested( QPoint pos )
{
	Q_UNUSED( pos );
	m_pLogMenu->exec( QCursor::pos() );
}

void WidgetSystemLog::on_actionClearBuffer_triggered()
{
	ui->textEditSystemLog->clear();
}

void WidgetSystemLog::appendLog( QString message, LogSeverity severity )
{
	if ( !ui->actionPauseLogDisplay->isChecked() )
	{
		QStringList sLines = message.split( QRegularExpression( "\r\n|\n|\r" ) );

		foreach ( const QString & sLine, sLines )
		{
			writeLine( sLine, severity );
		}
	}
}

void WidgetSystemLog::writeLine( const QString& sLine, LogSeverity severity )
{
	bool bTimeStamp = ui->actionToggleTimestamp->isChecked();
	m_oTimeStamp    = QTime::currentTime();

	switch ( severity )
	{
	case LogSeverity::Information:
		if ( ui->actionShowInformation->isChecked() )
		{
			appendText( QString( "font-weight:normal;" ),
						QColor( qRgb( 0, 0, 0 ) ), sLine, bTimeStamp );
		}
		break;
	case LogSeverity::Security:
		if ( ui->actionShowSecurity->isChecked() )
		{
			appendText( QString( "font-weight:600;" ),
						QColor( qRgb( 170, 170, 0 ) ), sLine, bTimeStamp );
		}
		break;
	case LogSeverity::Notice:
		if ( ui->actionShowNotice->isChecked() )
		{
			appendText( QString( "font-weight:600;" ),
						QColor( qRgb( 0, 170, 0 ) ), sLine, bTimeStamp );
		}
		break;
	case LogSeverity::Debug:
		if ( ui->actionShowDebug->isChecked() )
		{
			appendText( QString( "font-weight:normal;" ),
						QColor( qRgb( 117, 117, 117 ) ), sLine, bTimeStamp );
		}
		break;
	case LogSeverity::Warning:
		if ( ui->actionShowWarnings->isChecked() )
		{
			appendText( QString( "font-weight:normal;" ),
						QColor( qRgb( 255, 0, 0 ) ), sLine, bTimeStamp );
		}
		break;
	case LogSeverity::Error:
		if ( ui->actionShowError->isChecked() )
		{
			appendText( QString( "font-weight:600;" ),
						QColor( qRgb( 170, 0, 0 ) ), sLine, bTimeStamp );
		}
		break;
	case LogSeverity::Critical:
		if ( ui->actionShowCritical->isChecked() )
		{
			appendText( QString( "font-weight:600;" ),
						QColor( qRgb( 255, 0, 0 ) ), sLine, bTimeStamp );
		}
		break;
	}
}

void WidgetSystemLog::appendText( const QString& sFontArgs, const QColor& color,
								  const QString& sLine, bool bAddTimeStamp )
{
	QString sTimestamp = bAddTimeStamp ? m_oTimeStamp.toString( "hh:mm:ss.zzz" ) + ": " : QString();
	QString sMessage   = QString( "<span style=\" font-size:8pt; %1 color:%2;\">%3</span>" );
	ui->textEditSystemLog->append( sMessage.arg( sFontArgs, color.name(), sTimestamp + sLine ) );
}

void WidgetSystemLog::setSkin()
{
}
