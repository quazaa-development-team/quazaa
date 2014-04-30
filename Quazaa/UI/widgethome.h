/*
** widgethome.h
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

#ifndef WIDGETHOME_H
#define WIDGETHOME_H

#include <QWidget>

namespace Ui
{
class CWidgetHome;
}

class CWidgetHome : public QWidget
{
	Q_OBJECT
public:
	CWidgetHome( QWidget* parent = 0 );
	~CWidgetHome();
	void saveWidget();

signals:
	void triggerLibrary();
	void triggerTransfers();
	void triggerSecurity();
	void requestSearch( QString* searchString );

protected:
	void changeEvent( QEvent* e );

private:
	Ui::CWidgetHome* ui;

private slots:
	void on_toolButtonWelcomeSearch_clicked();
	void on_labelWelcomeUserGuideLink_linkActivated( QString link );
	void on_labelWelcomeWizardLink_linkActivated( QString link );
	void on_labelWelcomeSkinLink_linkActivated( QString link );
	void on_labelWelcomeOpenTorrentLink_linkActivated( QString link );
	void on_labelWelcomeURLDownloadLink_linkActivated( QString link );
	void setSkin();
};

#endif // WIDGETHOME_H
