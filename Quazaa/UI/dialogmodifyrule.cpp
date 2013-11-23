/*
** dialogmodifyrule.cpp
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

#include "dialogmodifyrule.h"
#include "ui_dialogmodifyrule.h"
#include "skinsettings.h"

#include <QListView>
#include <QMessageBox>

#include "debug_new.h"

CDialogModifyRule::CDialogModifyRule(CWidgetSecurity* parent, RuleDataPtr pRule) :
	QDialog( parent ),
	ui( new Ui::CDialogModifyRule ),
	m_pParent( parent )
{
	m_pRule = pRule;

	ui->setupUi( this );
	ui->comboBoxAction->setView( new QListView() );
	ui->comboBoxExpire->setView( new QListView() );
	ui->comboBoxRuleType->setView( new QListView() );


	if ( !m_pRule.isNull() ) // We are modifying a rule.
	{
		ui->comboBoxRuleType->setEnabled( false );
		ui->lineEditComment->setText( m_pRule->m_sComment );

		switch ( m_pRule->m_nType )
		{
		case Type::IPAddress:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::IPAddress );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::IPAddress );

			ui->lineEditIP->setText( m_pRule->m_sContent );
			break;
		}

		case Type::IPAddressRange:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::IPAddressRange );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::IPAddressRange );

			QStringList lAddressRange = m_pRule->m_sContent.split( "-" );
			ui->lineEditStartIP->setText( lAddressRange.at( 0 ) );
			ui->lineEditEndIP->setText(   lAddressRange.at( 1 ) );
			break;
		}
			// TODO: country rules
		case Type::Hash:
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::Hash );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::Hash );
			//TODO: implement hashes
			break;

		case Type::Content:
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::Content );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::Content );

			ui->lineEditContent->setText( m_pRule->m_sContent );
			break;

		case Type::RegularExpression:
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::RegularExpression );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::RegularExpression );

			ui->lineEditRegularExpression->setText( m_pRule->m_sContent );
			break;

		case Type::UserAgent:
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::UserAgent );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::UserAgent );

			ui->lineEditUserAgent->setText( m_pRule->m_sContent );
			ui->checkBoxUserAgentRegularExpression->setChecked( m_pRule->m_bContent );
			break;

		default:
			Q_ASSERT( false );
		}

		switch ( m_pRule->m_nAction )
		{
			case Action::Accept:
				ui->comboBoxAction->setCurrentIndex( 1 );
				break;
			case Action::Deny:
				ui->comboBoxAction->setCurrentIndex( 2 );
				break;
			default:
				ui->comboBoxAction->setCurrentIndex( 0 );
				break;
		}

		quint32 tExpire = m_pRule->m_tExpire;
		switch ( tExpire )
		{
		case Security::RuleTime::Forever:
			ui->comboBoxExpire->setCurrentIndex( 0 );
			ui->lineEditMinutes->setEnabled( false );
			ui->lineEditHours->setEnabled( false );
			ui->lineEditDays->setEnabled( false );
			break;

		case Security::RuleTime::Session:
			ui->comboBoxExpire->setCurrentIndex( 1 );
			ui->lineEditMinutes->setEnabled( false );
			ui->lineEditHours->setEnabled( false );
			ui->lineEditDays->setEnabled( false );
			break;

		default:
			ui->comboBoxExpire->setCurrentIndex( 2 );
			ui->lineEditMinutes->setEnabled( true );
			ui->lineEditHours->setEnabled( true );
			ui->lineEditDays->setEnabled( true );

			tExpire -= common::getTNowUTC();

			tExpire /= 60; // minutes now
			ui->lineEditMinutes->setText( QString::number( tExpire % 60 ) );

			tExpire /= 60; // hours now
			ui->lineEditHours->setText(   QString::number( tExpire % 24 ) );
			ui->lineEditDays->setText(    QString::number( tExpire / 24 ) );
			break;
		}
	}
	else // We are adding a rule.
	{
		setWindowTitle( tr( "Add Security Rule" ) );
		setWindowIcon( QIcon( ":/Resource/Security/AddRule.png" ) );

		ui->comboBoxRuleType->setCurrentIndex( 0 );
		ui->stackedWidgetType->setCurrentIndex( 0 );
		ui->comboBoxAction->setCurrentIndex(0);
		ui->comboBoxExpire->setCurrentIndex( 0 );
	}

	setSkin();
}

CDialogModifyRule::~CDialogModifyRule()
{
	delete ui;
}

void CDialogModifyRule::changeEvent(QEvent* e)
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

// TODO: change user interface for IP ranges and hashes.
void CDialogModifyRule::on_pushButtonOK_clicked()
{
	Rule* pRule = NULL;

	// create a new rule from the dialogue
	switch ( ui->comboBoxRuleType->currentIndex() )
	{
	case RuleIndex::IPAddress:
		pRule = new IPRule();

		if ( !pRule->parseContent( ui->lineEditIP->text() ) )
		{
			QMessageBox::warning( this, tr( "IP Address Rule Invalid" ),
								  tr( "The ip address rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;

	case RuleIndex::IPAddressRange:
		pRule = new IPRangeRule();

		if ( !pRule->parseContent( ui->lineEditStartIP->text() + "-" +
									 ui->lineEditEndIP->text() ) )
		{
			QMessageBox::warning( this, tr( "IP Address Range Rule Invalid" ),
								  tr( "The ip address range rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;

		// TODO: country rule

	case RuleIndex::Hash:
		pRule = new HashRule();

		if ( !pRule->parseContent( ui->lineEditURI->text() ) )
		{
			QMessageBox::warning( this, tr( "Hash Rule Invalid" ),
								  tr( "The hash rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;

	case RuleIndex::Content:
		pRule = new ContentRule();

		if ( !pRule->parseContent( ui->lineEditContent->text() ) )
		{
			QMessageBox::warning( this, tr( "Content Rule Invalid" ),
								  tr( "The content rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		((ContentRule*)pRule)->setAll( ui->radioButtonMatchAll->isChecked() );
		break;

	case RuleIndex::RegularExpression:
		pRule = new RegExpRule();

		if ( !pRule->parseContent( ui->lineEditRegularExpression->text() ) )
		{
			QMessageBox::warning( this, tr( "Regular Expression Rule Invalid" ),
								  tr( "The regular expression rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;

	case RuleIndex::UserAgent:
		pRule = new UserAgentRule();

		if ( !pRule->parseContent( ui->lineEditUserAgent->text() ) )
		{
			QMessageBox::warning( this, tr( "User Agent Rule Invalid" ),
								  tr( "The user agent rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		((UserAgentRule*)pRule)->setRegExp( ui->checkBoxUserAgentRegularExpression->isChecked() );
		break;

	default:
		Q_ASSERT( false );
	}

	switch ( ui->comboBoxAction->currentIndex() )
	{
	case 1:  // Accept
		pRule->m_nAction = Security::RuleAction::Accept;
		break;

	case 2:  // Deny
		pRule->m_nAction = Security::RuleAction::Deny;
		break;

	default: // None
		pRule->m_nAction = Security::RuleAction::None;
		break;
	}

	quint32 tExpire = 0;
	switch ( ui->comboBoxExpire->currentIndex() )
	{
	case 0:  // Forever
		tExpire = Security::RuleTime::Forever;
		break;

	case 1:  // Session
		tExpire = Security::RuleTime::Session;
		break;

	default: // Set Time
	{
		tExpire += ui->lineEditMinutes->text().toUShort() * 60;
		tExpire += ui->lineEditHours->text().toUShort() * 3600;
		tExpire += ui->lineEditDays->text().toUShort() * 216000;
		tExpire += common::getTNowUTC();
		break;
	}
	}
	pRule->setExpiryTime( tExpire );

	pRule->m_sComment = ui->lineEditComment->text();
	pRule->m_bAutomatic = false;

	securityManager.m_oRWLock.lockForRead();
	Rule* pExistingRule = m_pRule.isNull() ? NULL : m_pRule.data()->rule();

	pRule->m_idUUID = pExistingRule->m_idUUID;

	bool bReplace = false;
	if ( pExistingRule )
	{
		bReplace = *pExistingRule != *pRule;
	}

	securityManager.m_oRWLock.unlock();

	if ( bReplace )
	{
		securityManager.remove( pExistingRule );
		securityManager.add( pRule );
	}
	else if ( pExistingRule ) // rule already exists and is no different from the existing rule
	{
		delete pRule;
	}
	else // no existing rule
	{
		securityManager.add( pRule );
	}

	accept();
}

void CDialogModifyRule::on_pushButtonCancel_clicked()
{
	reject();
}

void CDialogModifyRule::on_comboBoxExpire_currentIndexChanged(int index)
{
	if ( index == 2 )
	{
		ui->lineEditDays->setEnabled( true );
		ui->lineEditHours->setEnabled( true );
		ui->lineEditMinutes->setEnabled( true );
	}
	else
	{
		ui->lineEditDays->setEnabled( false );
		ui->lineEditHours->setEnabled( false );
		ui->lineEditMinutes->setEnabled( false );
	}
}

void CDialogModifyRule::setSkin()
{

}

void CDialogModifyRule::on_lineEditDays_editingFinished()
{
	quint64 nDays = ui->lineEditDays->text().toULong();

	ui->lineEditDays->setText( QString::number( nDays ) );
}

void CDialogModifyRule::on_lineEditHours_editingFinished()
{
	quint64 nHours = ui->lineEditHours->text().toULong();
	quint64 nDays = ui->lineEditDays->text().toULong();

	ui->lineEditHours->setText( QString::number( nHours % 24 ) );
	nDays += nHours / 24;
	ui->lineEditDays->setText( QString::number( nDays ) );
}

void CDialogModifyRule::on_lineEditMinutes_editingFinished()
{
	quint64 nMinutes = ui->lineEditMinutes->text().toULong();
	quint64 nHours = ui->lineEditHours->text().toULong();
	quint64 nDays = ui->lineEditDays->text().toULong();

	ui->lineEditMinutes->setText( QString::number( nMinutes % 60 ) );
	nHours += nMinutes / 60;
	ui->lineEditHours->setText( QString::number( nHours % 24 ) );
	nDays += nHours / 24;
	ui->lineEditDays->setText( QString::number( nDays ) );
}
