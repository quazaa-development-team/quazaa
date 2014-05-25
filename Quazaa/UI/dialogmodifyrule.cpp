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

DialogModifyRule::DialogModifyRule( CWidgetSecurity* parent, Rule* pRule ) :
	QDialog( parent ),
	m_pParent( parent ),
	ui( new Ui::DialogModifyRule )
{
	m_pRule = pRule;

	ui->setupUi( this );

#ifdef _DEBUG
	ui->comboBoxRuleType->addItem( "Country Filter" );
#endif // _DEBUG


	if ( m_pRule ) // We are modifying a Rule.
	{
		ui->comboBoxRuleType->setEnabled( false );
		ui->lineEditComment->setText( m_pRule->m_sComment );

		switch ( m_pRule->type() )
		{
		case RuleType::IPAddress:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::IPAddress );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::IPAddress );

			ui->lineEditIP->setText( m_pRule->contentString() );
			break;
		}

		case RuleType::IPAddressRange:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::IPAddressRange );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::IPAddressRange );

			QStringList lAddressRange = m_pRule->contentString().split( "-" );
			ui->lineEditStartIP->setText( lAddressRange.at( 0 ) );
			ui->lineEditEndIP->setText(   lAddressRange.at( 1 ) );
			break;
		}
#if SECURITY_ENABLE_GEOIP
		case RuleType::Country:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::Country );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::Country );

			ui->lineEditCountry->setText( m_pRule->contentString() );
			break;
		}
#endif // SECURITY_ENABLE_GEOIP
		case RuleType::Hash:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::Hash );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::Hash );

			ui->lineEditContent->setText( m_pRule->contentString() );
			break;
		}

		case RuleType::Content:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::Content );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::Content );

			ui->lineEditContent->setText( m_pRule->contentString() );
			break;
		}

		case RuleType::RegularExpression:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::RegularExpression );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::RegularExpression );

			ui->lineEditRegularExpression->setText( m_pRule->contentString() );
			break;
		}

		case RuleType::UserAgent:
		{
			ui->comboBoxRuleType->setCurrentIndex( RuleIndex::UserAgent );
			ui->stackedWidgetType->setCurrentIndex( RuleIndex::UserAgent );

			ui->lineEditUserAgent->setText( m_pRule->contentString() );
			ui->checkBoxUserAgentRegularExpression->setChecked(
						( ( UserAgentRule* )m_pRule )->isRegExp() );
			break;
		}

		default:
			Q_ASSERT( false );
		}

		switch ( m_pRule->m_nAction )
		{
		case RuleAction::Accept:
			ui->comboBoxAction->setCurrentIndex( 1 );
			break;
		case RuleAction::Deny:
			ui->comboBoxAction->setCurrentIndex( 2 );
			break;
		default:
			ui->comboBoxAction->setCurrentIndex( 0 );
			break;
		}

		quint32 tExpire = m_pRule->expiryTime();
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
		ui->comboBoxAction->setCurrentIndex( 0 );
		ui->comboBoxExpire->setCurrentIndex( 0 );
	}

	setSkin();
}

DialogModifyRule::~DialogModifyRule()
{
	delete ui;

	if ( m_pRule )
		delete m_pRule;
}

void DialogModifyRule::changeEvent( QEvent* e )
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

void DialogModifyRule::on_pushButtonOK_clicked()
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
								  tr( "The IP address rule is invalid. Please see the example for correct usage." ),
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
			QMessageBox::warning( this, tr( "IP Range Rule Invalid" ),
								  tr( "The IP range rule is invalid. Please see the example for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;

#ifdef SECURITY_ENABLE_GEOIP
	case RuleIndex::Country:
		pRule = new CountryRule();

		if ( !pRule->parseContent( ui->lineEditCountry->text() ) )
		{
			QMessageBox::warning( this, tr( "Country Rule Invalid" ),
								  tr( "The country rule is invalid. Please see the examples for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;
#endif // SECURITY_ENABLE_GEOIP

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
								  tr( "The content rule is invalid. Content rules may not be created without content." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		( ( ContentRule* )pRule )->setAll( ui->radioButtonMatchAll->isChecked() );
		break;

	case RuleIndex::RegularExpression:
		pRule = new RegularExpressionRule();

		if ( !pRule->parseContent( ui->lineEditRegularExpression->text() ) )
		{
			QMessageBox::warning( this, tr( "Regular Expression Rule Invalid" ),
								  tr( "The regular expression rule is invalid. Please specify a valid regular expression." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
		break;

	case RuleIndex::UserAgent:
		pRule = new UserAgentRule();

		// by setting this first, the content parsing detects regular expression errors
		( ( UserAgentRule* )pRule )->setRegExp( ui->checkBoxUserAgentRegularExpression->isChecked() );

		pRule->parseContent( ui->lineEditUserAgent->text() );
		{
			QMessageBox::warning( this, tr( "User Agent Rule Invalid" ),
								  tr( "The user agent rule is invalid. Please see the examples for correct usage." ),
								  QMessageBox::Ok, QMessageBox::Ok );
			delete pRule;
			return;
		}
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

	bool bReplace = false;
	if ( m_pRule )
	{
		pRule->m_idUUID = m_pRule->m_idUUID;
		bReplace = *m_pRule != *pRule;
	}

	securityManager.m_oRWLock.unlock();

	if ( bReplace )
	{
		securityManager.remove( m_pRule );
		securityManager.add( pRule );
	}
	else if ( m_pRule ) // rule already exists and is no different from the existing rule
	{
		delete pRule;
	}
	else // no existing rule
	{
		securityManager.add( pRule );
	}

	accept();
}

void DialogModifyRule::on_pushButtonCancel_clicked()
{
	reject();
}

void DialogModifyRule::on_comboBoxExpire_currentIndexChanged( int index )
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

void DialogModifyRule::setSkin()
{
}

void DialogModifyRule::on_lineEditDays_editingFinished()
{
	quint64 nDays = ui->lineEditDays->text().toULong();

	ui->lineEditDays->setText( QString::number( nDays ) );
}

void DialogModifyRule::on_lineEditHours_editingFinished()
{
	quint64 nHours = ui->lineEditHours->text().toULong();
	quint64 nDays = ui->lineEditDays->text().toULong();

	ui->lineEditHours->setText( QString::number( nHours % 24 ) );
	nDays += nHours / 24;
	ui->lineEditDays->setText( QString::number( nDays ) );
}

void DialogModifyRule::on_lineEditMinutes_editingFinished()
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
