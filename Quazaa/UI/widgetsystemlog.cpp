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

CWidgetSystemLog::CWidgetSystemLog(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::CWidgetSystemLog)
{
	ui->setupUi(this);
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	ui->textEditSystemLog->setFont(font);
	ui->textEditSystemLog->document()->setMaximumBlockCount(100);
	quazaaSettings.loadLogSettings();
	ui->actionPauseLogDisplay->setChecked(quazaaSettings.Logging.IsPaused);
	logMenu = new QMenu(ui->textEditSystemLog);
	logMenu->addAction(ui->actionCopy);
	logMenu->addSeparator();
	logMenu->addAction(ui->actionShowInformation);
	ui->actionShowInformation->setChecked(quazaaSettings.Logging.ShowInformation);
	logMenu->addAction(ui->actionShowSecurity);
	ui->actionShowSecurity->setChecked(quazaaSettings.Logging.ShowSecurity);
	logMenu->addAction(ui->actionShowNotice);
	ui->actionShowNotice->setChecked(quazaaSettings.Logging.ShowNotice);
	logMenu->addAction(ui->actionShowDebug);
	ui->actionShowDebug->setChecked(quazaaSettings.Logging.ShowDebug);
	logMenu->addAction(ui->actionShowWarnings);
	ui->actionShowWarnings->setChecked(quazaaSettings.Logging.ShowWarnings);
	logMenu->addAction(ui->actionShowError);
	ui->actionShowError->setChecked(quazaaSettings.Logging.ShowError);
	logMenu->addAction(ui->actionShowCritical);
	ui->actionShowCritical->setChecked(quazaaSettings.Logging.ShowCritical);
	restoreState(quazaaSettings.WinMain.SystemLogToolbar);
	ui->actionToggleTimestamp->setChecked(quazaaSettings.Logging.LogShowTimestamp);

	connect( &systemLog, &CSystemLog::logPosted, this, &CWidgetSystemLog::appendLog );

	setSkin();
}

CWidgetSystemLog::~CWidgetSystemLog()
{
	delete ui;
}

void CWidgetSystemLog::changeEvent(QEvent* e)
{
	QMainWindow::changeEvent(e);
	switch(e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void CWidgetSystemLog::appendLog(QString message, LogSeverity::Severity severity)
{
	if(!ui->actionPauseLogDisplay->isChecked())
	{
		QStringList sLines = message.split(QRegExp("\r\n|\n|\r"));

		foreach ( const QString& sLine, sLines )
		{
			if(ui->actionToggleTimestamp->isChecked())
			{
				timeStamp = QTime::currentTime();
				switch(severity)
				{
					case LogSeverity::Information:
						if(ui->actionShowInformation->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:normal;").arg(QColor(qRgb(0, 0, 0)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
					case LogSeverity::Security:
						if(ui->actionShowSecurity->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:600;").arg(QColor(qRgb(170, 170, 0)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
					case LogSeverity::Notice:
						if(ui->actionShowNotice->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:600;").arg(QColor(qRgb(0, 170, 0)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
					case LogSeverity::Debug:
						if(ui->actionShowDebug->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:normal;").arg(QColor(qRgb(117, 117, 117)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
					case LogSeverity::Warning:
						if(ui->actionShowWarnings->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:normal;").arg(QColor(qRgb(255, 0, 0)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
					case LogSeverity::Error:
						if(ui->actionShowError->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:600;").arg(QColor(qRgb(170, 0, 0)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
					case LogSeverity::Critical:
						if(ui->actionShowCritical->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3:%4</span>").arg("font-weight:600;").arg(QColor(qRgb(255, 0, 0)).name()).arg(timeStamp.toString("hh:mm:ss.zzz")).arg(sLine));
						}
						break;
				}
			}
			else
			{
				switch(severity)
				{
					case LogSeverity::Information:
						if(ui->actionShowInformation->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:normal;").arg(QColor(qRgb(0, 0, 0)).name()).arg(sLine));
						}
						break;
					case LogSeverity::Security:
						if(ui->actionShowSecurity->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:600;").arg(QColor(qRgb(170, 170, 0)).name()).arg(sLine));
						}
						break;
					case LogSeverity::Notice:
						if(ui->actionShowNotice->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:600;").arg(QColor(qRgb(0, 170, 0)).name()).arg(sLine));
						}
						break;
					case LogSeverity::Debug:
						if(ui->actionShowDebug->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:normal;").arg(QColor(qRgb(117, 117, 117)).name()).arg(sLine));
						}
						break;
					case LogSeverity::Warning:
						if(ui->actionShowWarnings->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:normal;").arg(QColor(qRgb(255, 0, 0)).name()).arg(sLine));
						}
						break;
					case LogSeverity::Error:
						if(ui->actionShowError->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:600;").arg(QColor(qRgb(170, 0, 0)).name()).arg(sLine));
						}
						break;
					case LogSeverity::Critical:
						if(ui->actionShowCritical->isChecked())
						{
							ui->textEditSystemLog->append(QString("<span style=\" font-size:8pt; %1 color:%2;\">%3</span>").arg("font-weight:600;").arg(QColor(qRgb(255, 0, 0)).name()).arg(sLine));
						}
						break;
				}
			}
		}
	}
}

void CWidgetSystemLog::saveWidget()
{
	quazaaSettings.WinMain.SystemLogToolbar = saveState();
	quazaaSettings.Logging.LogShowTimestamp = ui->actionToggleTimestamp->isChecked();
	quazaaSettings.Logging.ShowWarnings = ui->actionShowWarnings->isChecked();
	quazaaSettings.Logging.ShowInformation = ui->actionShowInformation->isChecked();
	quazaaSettings.Logging.ShowSecurity = ui->actionShowSecurity->isChecked();
	quazaaSettings.Logging.ShowNotice = ui->actionShowNotice->isChecked();
	quazaaSettings.Logging.ShowDebug = ui->actionShowDebug->isChecked();
	quazaaSettings.Logging.ShowError = ui->actionShowError->isChecked();
	quazaaSettings.Logging.ShowCritical = ui->actionShowCritical->isChecked();
	quazaaSettings.Logging.IsPaused = ui->actionPauseLogDisplay->isChecked();
	quazaaSettings.saveLogSettings();
}

void CWidgetSystemLog::on_actionClearBuffer_triggered()
{
	ui->textEditSystemLog->clear();
}

void CWidgetSystemLog::on_textEditSystemLog_customContextMenuRequested(QPoint pos)
{
	Q_UNUSED(pos);
	logMenu->exec(QCursor::pos());
}

void CWidgetSystemLog::on_actionCopy_triggered()
{
	ui->textEditSystemLog->copy();
}

void CWidgetSystemLog::setSkin()
{

}
