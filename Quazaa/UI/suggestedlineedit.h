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

#ifndef SUGGESTEDLINEEDIT_H
#define SUGGESTEDLINEEDIT_H

#include <QObject>
#include <QStringList>

class QLineEdit;
class QStandardItemModel;
class QNetworkAccessManager;
class QModelIndex;

class CSuggestedLineEdit : public QObject
{
	Q_OBJECT

protected:
	QLineEdit* m_pLineEdit;
	QStandardItemModel* m_pModel;
	QStringList	m_lRecent;
	QStringList m_lSuggested;
	int m_nMaxRecent;
	bool m_bEnableExternal;
	QNetworkAccessManager* m_pNetworkAccessManager;
public:
	CSuggestedLineEdit( QLineEdit* lineEdit, QObject* parent = 0 );
	~CSuggestedLineEdit();

	void load();
	void save();

	void setNetworkAccessManager( QNetworkAccessManager* pManager );

	void setEnableExternal( bool bEnable );
	bool enableExternal();
	void setMaxRecentItems( uint nMax );
	int  maxRecentItems();

signals:

public slots:
	void updateRecent();
	void getSuggestions();
	void setupCompleter();
	void finished();
	void clearRecent();

private slots:
	void onTextChanged( const QString& text );
	void onItemActivated( const QModelIndex& index );
	void setSkin();
};


#endif // SUGGESTEDLINEEDIT_H
