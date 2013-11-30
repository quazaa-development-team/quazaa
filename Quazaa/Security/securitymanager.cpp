/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2013.
** This file is part of the Quazaa Security Library (quazaa.sourceforge.net)
**
** The Quazaa Security Library is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** The Quazaa Security Library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with the Quazaa Security Library; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>

#include <QDir>
#include <QDateTime>
#include <QMetaType>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "securitymanager.h"

#include "quazaaglobals.h"
#include "quazaasettings.h"
#include "timedsignalqueue.h"

#include "debug_new.h"

CSecurity securityManager;

bool IPRangeLessThan(const CIPRangeRule *rule1, const CIPRangeRule *rule2)
{
	return rule1->startIP() < rule2->startIP();
}

bool IPLessThan(const CIPRule *rule1, const CIPRule *rule2)
{
	return rule1->IP() < rule2->IP();
}

/**
  * Constructor. Variable initializations.
  * Locking: /
  */
// QApplication hasn't been started when the global definition creates this object, so
// no qt specific calls (for example connect() or emit signal) may be used over here.
// See initialize() for that kind of initializations.
CSecurity::CSecurity() :
	m_pSection(QMutex::Recursive),
	m_bIsLoading( false ),
	m_bLogIPCheckHits( false ),
	m_bUseMissCache( false ),
	m_bNewRulesLoaded( false ),
	m_nPendingOperations( 0 ),
	m_nMaxUnsavedRules( 100 ),
	m_nUnsaved( 0 ),
	m_bDenyPolicy( false )
{
}

/**
  * Destructor.
  * Locking: RW
  */
CSecurity::~CSecurity()
{
}

/**
  * Sets the deny policy to a given value.
  * Locking: RW
  */
void CSecurity::setDenyPolicy(bool bDenyPolicy)
{
	m_bDenyPolicy = bDenyPolicy;
}

/**
  * Checks whether a rule with the same UUID exists within the security database.
  * Locking: R
  */
bool CSecurity::check(const CSecureRule* const pRule) const
{
	return pRule && getUUID( pRule->m_oUUID );
}

//////////////////////////////////////////////////////////////////////
// CSecurity rule modification
/**
  * Adds a rule to the security database. This makes no copy of the rule, so don't delete it after
  * adding.
  * Locking: RW
  */
bool CSecurity::add(CSecureRule* pRule)
{
	QMutexLocker locker(&m_pSection);
	if ( !pRule ) return false;

	// check for invalid rules
	Q_ASSERT( pRule->type() > 0 && pRule->type() < 8 && pRule->m_nAction < 3 );
	Q_ASSERT( !pRule->m_oUUID.isNull() );

	RuleType::Type type = pRule->type();
	CSecureRule* pExRule = NULL;

	bool bNewAddress = false;
	bool bNewHit	 = false;

	// Special treatment for the different types of rules
	switch ( type )
	{
	case RuleType::IPAddress:
	{
		CIPRule* pNewRule = ((CIPRule*)pRule);
		CIPRule* pOldRule = isInAddressRules( pNewRule->IP() );

		if ( pOldRule ) // there is a potentially conflicting rule in our map
		{
			if ( pOldRule->m_oUUID   != pNewRule->m_oUUID   ||
				 pOldRule->m_nAction != pNewRule->m_nAction ||
				 pOldRule->isForever() != pNewRule->isForever() ||
				 pOldRule->getExpiryTime() != pNewRule->getExpiryTime() )
			{
				// remove conflicting rule if one of the important attributes
				// differs from the rule we'd like to add
				remove( pOldRule );
			}
			else
			{
				// the rule does not need to be added.
				delete pRule;
				pRule = NULL;
				return false;
			}
		}

		m_lIPs.prepend(pNewRule);

		bNewAddress = true;

		if ( !m_bUseMissCache )
			evaluateCacheUsage();
	}
	break;

	case RuleType::IPAddressRange:
	{
		CIPRangeRule* pNewRule = ((CIPRangeRule*)pRule);
		CIPRangeRule* pOldRule = isInAddressRangeRules(pNewRule->startIP());

		if(!pOldRule)
			 pOldRule = isInAddressRangeRules(pNewRule->endIP());

		if(pOldRule)
		{
			// fix range conflicts with old rules
			if((pNewRule->m_nAction == RuleAction::Deny && pOldRule->m_nAction == RuleAction::Deny) ||
			   (pNewRule->m_nAction == RuleAction::Accept && pOldRule->m_nAction == RuleAction::Accept)) {
				if( pNewRule->startIP() == pOldRule->startIP() && pNewRule->endIP() == pOldRule->endIP()  )
				{
					systemLog.postLog(LogSeverity::Security, QString("New IP range rule is the same as old rule %3-%4, skipping.")
										.arg(pOldRule->startIP().toString())
										.arg(pOldRule->endIP().toString()) );

					delete pRule;
					pRule = NULL;
					return false;
				}
				else if( pOldRule->startIP() < pNewRule->startIP() && pOldRule->endIP() > pNewRule->endIP()  )
				{
					systemLog.postLog(LogSeverity::Security, QString("Old IP range rule %1-%2 encompasses new rule %3-%4, skipping.")
										.arg(pNewRule->startIP().toString())
										.arg(pNewRule->endIP().toString())
										.arg(pOldRule->startIP().toString())
										.arg(pOldRule->endIP().toString()) );

					delete pRule;
					pRule = NULL;
					return false;
				}
				else if( pNewRule->startIP() < pOldRule->startIP() && pNewRule->endIP() > pOldRule->endIP()  )
				{
					systemLog.postLog(LogSeverity::Security, QString("New IP range rule %1-%2 encompasses old rule %3-%4, replacing.")
										.arg(pNewRule->startIP().toString())
										.arg(pNewRule->endIP().toString())
										.arg(pOldRule->startIP().toString())
										.arg(pOldRule->endIP().toString()) );

					remove( pOldRule );
				} else {
					bool bMatch = false;
					if( pNewRule->contains( pOldRule->startIP() ) )
					{
						systemLog.postLog(LogSeverity::Security, QString("Old IP range rule start IP %1 is within new rule %2-%3, merging.")
											.arg(pOldRule->startIP().toString())
											.arg(pNewRule->startIP().toString())
											.arg(pNewRule->endIP().toString()) );

						bMatch = true;
						pNewRule->parseContent(QString("%1-%2").arg(pNewRule->startIP().toString()).arg(pOldRule->endIP().toString()));
					}
					if( pNewRule->contains( pOldRule->endIP() ) )
					{
						systemLog.postLog(LogSeverity::Security, QString("Old IP range rule end IP %1 is within new rule %2-%3, merging.")
											.arg(pOldRule->endIP().toString())
											.arg(pNewRule->startIP().toString())
											.arg(pNewRule->endIP().toString()) );

						bMatch = true;
						pNewRule->parseContent(QString("%1-%2").arg(pOldRule->startIP().toString()).arg(pNewRule->endIP().toString()));
					}


					if(bMatch)
						remove( pOldRule );
				}
			}
		}

		m_lIPRanges.prepend( pNewRule );

		bNewAddress = true;

		if ( !m_bUseMissCache )
			evaluateCacheUsage();
	}
	break;
	case RuleType::Hash:
	{
		CHashRule* pHashRule = (CHashRule*)pRule;

		QList<CHash> oHashes = pHashRule->getHashes();

		if ( oHashes.isEmpty() )
		{
			// There is no point in adding an invalid rule...
			delete pRule;
			pRule = NULL;
			return false;
		}

		CHashRule* pExHashRule = getHash( oHashes );

		if ( pExHashRule )
		{
			if ( pHashRule->hashEquals( pExHashRule ) )
			{
				if ( pExHashRule->m_oUUID   != pRule->m_oUUID   ||
					 pExHashRule->m_nAction != pRule->m_nAction ||
					 pExHashRule->isForever() != pRule->isForever() ||
					 pExHashRule->getExpiryTime() != pRule->getExpiryTime() )
				{
					// remove conflicting rule if one of the important attributes
					// differs from the rule we'd like to add
					remove( pExHashRule );
				}
				else
				{
					// there is no point on adding a rule for the same content twice,
					// as that content is already blocked.
					delete pRule;
					pRule = NULL;
					return false;
				}
			}
		}

		// If there isn't a rule for this content or there is a rule for
		// similar but not 100% identical content, add hashes to map.
		foreach ( CHash oHash, oHashes )
		{
			m_lmmHashes.insert( qHash( oHash.rawValue() ), pHashRule );
		}

		bNewHit	= true;
	}
	break;

	case RuleType::RegularExpression:
	{
		int i = 0;

		while ( i < m_lRegularExpressions.size() )
		{
			CRegularExpressionRule* pOldRule = m_lRegularExpressions.at(i);
			if ( m_lRegularExpressions.at(i)->m_oUUID == pRule->m_oUUID )
			{
				if ( pOldRule->m_nAction != pRule->m_nAction ||
					 pOldRule->isForever() != pRule->isForever() ||
					 pOldRule->getExpiryTime() != pRule->getExpiryTime() ||
					 pOldRule->getContentString() != ((CRegularExpressionRule*)pRule)->getContentString() )
				{
					// remove conflicting rule if one of the important attributes
					// differs from the rule we'd like to add
					remove( pOldRule );
				}
				else
				{
					// the rule does not need to be added.
					delete pRule;
					pRule = NULL;
					return false;
				}
			}
			++i;
		}

		m_lRegularExpressions.prepend( (CRegularExpressionRule*)pRule );

		bNewHit	= true;
	}
	break;

	case RuleType::Content:
	{
		int i = 0;

		while ( i < m_lContents.size() )
		{
			CContentRule* pOldRule = m_lContents.at(i);
			if ( pOldRule->m_oUUID == pRule->m_oUUID )
			{
				if ( pOldRule->m_nAction != pRule->m_nAction ||
					 pOldRule->isForever() != pRule->isForever() ||
					 pOldRule->getExpiryTime() != pRule->getExpiryTime() ||
					 pOldRule->getContentString() != ((CRegularExpressionRule*)pRule)->getContentString() )
				{
					// remove conflicting rule if one of the important attributes
					// differs from the rule we'd like to add
					remove( pOldRule );
				}
				else
				{
					// the rule does not need to be added.
					delete pRule;
					pRule = NULL;
					return false;
				}
			}
			++i;
		}

		m_lContents.prepend( (CContentRule*)pRule );

		bNewHit	= true;
	}
	break;

	case RuleType::UserAgent:
	{
		QString agent = ((CUserAgentRule*)pRule)->getContentString();
		QMap<QString, CUserAgentRule*>::iterator it = m_lmUserAgents.find( agent );

		if ( it != m_lmUserAgents.end() ) // there is a conflicting rule in our map
		{
			pExRule = it.value();
			if ( pExRule->m_oUUID   != pRule->m_oUUID   ||
				 pExRule->m_nAction != pRule->m_nAction ||
				 pExRule->isForever() != pRule->isForever() ||
				 pExRule->getExpiryTime() != pRule->getExpiryTime() )
			{
				// remove conflicting rule if one of the important attributes
				// differs from the rule we'd like to add
				remove( pExRule );
			}
			else
			{
				// the rule does not need to be added.
				delete pRule;
				pRule = NULL;
				return false;
			}

			pExRule = NULL;
		}

		m_lmUserAgents[ agent ] = (CUserAgentRule*)pRule;

		bNewHit	= true;
	}
	break;

	default:
		Q_ASSERT( false );
	}

	// Add rule to list of all rules
	pExRule = getUUID( pRule->m_oUUID );
	if ( pExRule ) // we do not allow 2 rules by the same UUID
		remove( pExRule );
	m_lRules.append( pRule );

	// If an address rule is added, the miss cache is cleared either in whole or just the relevant
	// address.
	if ( type == RuleType::IPAddress )
	{
		m_lsCache.remove( qHash( ((CIPRule*)pRule)->IP() ) );

		if ( !m_bUseMissCache )
			evaluateCacheUsage();
	}
	else if ( type == RuleType::IPAddressRange )
	{
		missCacheClear();

		if ( !m_bUseMissCache )
			evaluateCacheUsage();
	}

	if ( bNewAddress )	// only add IP, IP range and country rules to the queue
	{
		m_lqNewAddressRules.enqueue( pRule->getCopy() );
	}
	else if ( bNewHit )		// only add rules related to hit filtering to the queue
	{
		m_lqNewHitRules.enqueue( pRule->getCopy() );
	}

	const quint32 tNow = common::getTNowUTC();
	// if ( indefinite or longer than 1,5h ) increase unsaved rules counter
	if ( !pRule->isForever() || pRule->getExpiryTime() - tNow > 60 * 90 )
	{
		m_nUnsaved.fetchAndAddRelaxed( 1 );
	}

	// Inform CSecurityTableModel about new rule and update the GUI.
	emit ruleAdded( pRule );

	// If we're not loading, check all lists for newly denied hosts.
	if ( !m_bIsLoading )
	{
		if(pRule->type() == RuleType::IPAddress)
			qSort(m_lIPs.begin(), m_lIPs.end(), IPLessThan);
		if(pRule->type() == RuleType::IPAddressRange)
			qSort(m_lIPRanges.begin(), m_lIPRanges.end(), IPRangeLessThan);
		sanityCheck();
		save();
	}

	return true;
}

/**
  * Frees all memory and storing containers. Removes all rules.
  * Locking: RW
  */
void CSecurity::clear()
{
	m_lIPs.clear();
	m_lIPRanges.clear();
	m_lmmHashes.clear();
	m_lRegularExpressions.clear();
	m_lContents.clear();
	m_lmUserAgents.clear();

	qDeleteAll( m_lRules );
	m_lRules.clear();

	qDeleteAll( m_lLoadedAddressRules );
	m_lLoadedAddressRules.clear();

	qDeleteAll( m_lLoadedHitRules );
	m_lLoadedHitRules.clear();

	CSecureRule* pRule = NULL;
	while( m_lqNewAddressRules.size() )
	{
		pRule = m_lqNewAddressRules.dequeue();
		delete pRule;
	}

	while ( m_lqNewHitRules.size() )
	{
		pRule = m_lqNewHitRules.dequeue();
		delete pRule;
	}
	missCacheClear();

	m_nUnsaved.fetchAndStoreRelaxed( 0 );
}

//////////////////////////////////////////////////////////////////////
// CSecurity ban
/**
  * Bans a given IP for a specified amount of time and adds strComment as comment to the security
  * rule. If bMessage is set to true, a notification is send to the system log.
  * Locking: RW
  */
void CSecurity::ban(const CEndPoint& oAddress, quint32 nRuleTime, bool bMessage,
					const QString& sComment, bool bAutomatic, bool bForever)
{
	QMutexLocker locker(&m_pSection);
	if ( oAddress.isNull() )
	{
		Q_ASSERT( false ); // if this happens, make sure to fix the caller... :)
		return;
	}
	const quint32 tNow = common::getTNowUTC();

	CIPRule* pRule = isInAddressRules(oAddress);
	if ( pRule ) // If rule exists, add time on to ban
	{
		pRule->m_bAutomatic = bAutomatic;

		if ( pRule->m_nAction == RuleAction::Deny )
		{
			if ( !( sComment.isEmpty() ) )
				pRule->m_sComment = sComment;

			switch( nRuleTime )
			{
			case RuleTime::Special:
				pRule->setForever(bForever);
				pRule->setExpiryTime(RuleTime::Special);
				break;

			case RuleTime::FiveMinutes:
				pRule->setExpiryTime(tNow + RuleTime::FiveMinutes);
				break;

			case RuleTime::ThirtyMinutes:
				pRule->setExpiryTime(tNow + RuleTime::ThirtyMinutes);
				break;

			case RuleTime::TwoHours:
				pRule->setExpiryTime(tNow + RuleTime::TwoHours);
				break;

			case RuleTime::SixHours:
				pRule->setExpiryTime(tNow + RuleTime::SixHours);
				break;

			case RuleTime::TwelveHours:
				pRule->setExpiryTime(tNow + RuleTime::TwelveHours);
				break;

			case RuleTime::Day:
				pRule->setExpiryTime(tNow + RuleTime::Day);
				break;

			case RuleTime::Week:
				pRule->setExpiryTime(tNow + RuleTime::Week);
				break;

			case RuleTime::Month:
				pRule->setExpiryTime(tNow + RuleTime::Month);
				break;

			default:
				pRule->setExpiryTime(pRule->getExpiryTime() + nRuleTime);
				break;
			}

			if ( bMessage )
			{
				if( nRuleTime == RuleTime::Special ) {
					if( bForever ) {
						systemLog.postLog( LogSeverity::Security,
								 Components::Security,
								 tr( "Adjusted ban expiry time of %1 to forever."
									 ).arg( oAddress.toString() ) );
					} else {
						systemLog.postLog( LogSeverity::Security,
								 Components::Security,
								 tr( "Adjusted ban expiry time of %1 to end of session."
									 ).arg( oAddress.toString() ) );
					}
				} else {
					systemLog.postLog( LogSeverity::Security,
							 Components::Security,
							 tr( "Adjusted ban expiry time of %1 to %2."
								 ).arg( oAddress.toString(),
										QDateTime::fromTime_t( pRule->getExpiryTime() ).toString() ) );
				}
			}

			hit(pRule);

			return;
		}
		else
		{
			remove( pRule );
		}
	}

	m_bIsLoading = true;

	pRule = new CIPRule();

	pRule->setIP( oAddress );

	pRule->m_bAutomatic = bAutomatic;

	switch( nRuleTime )
	{
	case RuleTime::Special:
		pRule->setForever(bForever);
		pRule->setExpiryTime(RuleTime::Special);
		break;

	case RuleTime::FiveMinutes:
		pRule->setExpiryTime(tNow + RuleTime::FiveMinutes);
		pRule->m_sComment = tr( "Temp Ignore (5 min)" );
		break;

	case RuleTime::ThirtyMinutes:
		pRule->setExpiryTime(tNow + RuleTime::ThirtyMinutes);
		pRule->m_sComment = tr( "Temp Ignore (30 min)" );
		break;

	case RuleTime::TwoHours:
		pRule->setExpiryTime(tNow + RuleTime::TwoHours);
		pRule->m_sComment = tr( "Temp Ignore (2 h)" );
		break;

	case RuleTime::SixHours:
		pRule->setExpiryTime(tNow + RuleTime::SixHours);
		pRule->m_sComment = tr( "Temp Ignore (2 h)" );
		break;

	case RuleTime::TwelveHours:
		pRule->setExpiryTime(tNow + RuleTime::TwelveHours);
		pRule->m_sComment = tr( "Temp Ignore (2 h)" );
		break;

	case RuleTime::Day:
		pRule->setExpiryTime(tNow + RuleTime::Day);
		pRule->m_sComment = tr( "Temp Ignore (1 d)" );
		break;

	case RuleTime::Week:
		pRule->setExpiryTime(tNow + RuleTime::Week);
		pRule->m_sComment = tr( "Client Block (1 week)" );
		break;

	case RuleTime::Month:
		pRule->setExpiryTime(tNow + RuleTime::Month);
		pRule->m_sComment = tr( "Quick IP Block (1 month)" );
		break;

	default:
		pRule->setExpiryTime(tNow + nRuleTime);
		break;
	}

	if ( !( sComment.isEmpty() ) )
		pRule->m_sComment = sComment;

	if ( bMessage )
	{
		if( nRuleTime == RuleTime::Special ) {
			if( bForever ) {
				systemLog.postLog( LogSeverity::Security,
						 Components::Security,
						 tr( "Banned %1 forever."
							 ).arg( oAddress.toString() ) );
			} else {
				systemLog.postLog( LogSeverity::Security,
						 Components::Security,
						 tr( "Banned %1 until the end of the session."
							 ).arg( oAddress.toString() ) );
			}
		} else {
			systemLog.postLog( LogSeverity::Security,
					 Components::Security,
					 tr( "Banned %1 until %2."
						 ).arg( oAddress.toString(),
								QDateTime::fromTime_t( pRule->getExpiryTime() ).toString() ) );
		}
	}

	hit(pRule);

	bool result = add( pRule );
	m_bIsLoading = false;

	if(result) {
		qSort(m_lIPs.begin(), m_lIPs.end(), IPLessThan);
		sanityCheck();
	}
}

//////////////////////////////////////////////////////////////////////
// public CSecurity access checks
/**
  * Checks an IP against the list of loaded new security rules.
  * Locking: R
  */
bool CSecurity::isNewlyDenied(const CEndPoint& oAddress)
{
	if ( oAddress.isNull() )
		return false;

	CSecureRule* pRule = NULL;

	// This should only be called if new rules have been loaded previously.
	Q_ASSERT( m_bNewRulesLoaded );

	if ( m_lLoadedAddressRules.empty() )
		return false;

	int i = 0;

	while ( i < m_lLoadedAddressRules.size() )
	{
		pRule = m_lLoadedAddressRules.at(i);

		if ( pRule->match( oAddress ) )
		{
			// the rules are new, so we don't need to check whether they are expired or not

			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}

		++i;
	}

	return false;
}

/**
  * Checks a hit against the list of loaded new security rules.
  * Locking: R
  */
bool CSecurity::isNewlyDenied(const CQueryHit* pHit, const QList<QString>& lQuery)
{
	if ( !pHit )
		return false;

	CSecureRule* pRule = NULL;

	// This should only be called if new rules have been loaded previously.
	Q_ASSERT( m_bNewRulesLoaded );

	if ( m_lLoadedHitRules.empty() )
		return false;

	int i = 0;

	while ( i < m_lLoadedHitRules.size() )
	{
		pRule = m_lLoadedHitRules.at(i);

		if ( pRule->match( pHit ) || pRule->match( pHit->m_sDescriptiveName ) ||
			 pRule->match( lQuery, pHit->m_sDescriptiveName ) )
		{
			// The rules are new, so we don't need to check whether they are expired or not.

			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}

		++i;
	}

	return false;
}

/**
  * Checks an IP against the security database. Writes a message to the system log if LogIPCheckHits
  * is true.
  * Locking: R (+ RW while/if new IP is added to miss cache)
  */
bool CSecurity::isDenied(const CEndPoint &oAddress)
{
	QMutexLocker locker(&m_pSection);
	if ( oAddress.isNull() )
		return true;

	const quint32 tNow = common::getTNowUTC();

	// First, check the miss cache if the IP is not included in the list of rules.
	// If the address is in cache, it is a miss and no further lookup is needed.
	if ( m_bUseMissCache )
	{
		if(m_lsCache.contains( qHash( oAddress ) ))
		{
			if ( m_bLogIPCheckHits )
			{
				systemLog.postLog( LogSeverity::Security,
						 Components::Security,
						 tr( "Skipped repeat IP security check for %s (%i IPs cached)")
						.arg( oAddress.toString(), m_lsCache.size() ) );
			}

			return m_bDenyPolicy;
		}
	}
	else if ( m_bLogIPCheckHits )
	{
		systemLog.postLog( LogSeverity::Security,
				 Components::Security,
				 tr( "Called first-time IP security check for %s"
					 ).arg( oAddress.toString() )
//					+ tr( " ( Call source: %s)" ).arg( source )
				 );
	}

	// Second, if quazaa local/private blocking is turned on, check if the IP is local/private
	if( quazaaSettings.Security.IgnorePrivateIP )
	{
		if(isPrivate( oAddress ))
		{
			systemLog.postLog( LogSeverity::Security,
					 Components::Security,
					 tr( "Local/Private IP denied: %s" ).arg( oAddress.toString() ) );
			return true;
		}
	}

	// Third, check whether the IP is contained within one of the IP range rules
	CIPRangeRule* pIPRangeRule = isInAddressRangeRules( oAddress );

	if(pIPRangeRule)
	{
		hit( pIPRangeRule );

		if ( pIPRangeRule->m_nAction == RuleAction::Accept )
			return false;
		else if ( pIPRangeRule->m_nAction == RuleAction::Deny )
			return true;
		else
			Q_ASSERT( pIPRangeRule->m_nAction == RuleAction::None );
	}

	// Fourth, check whether the IP is contained within one of the single IP rules
	CIPRule* pIPRule = isInAddressRules( oAddress );

	if ( pIPRule )
	{
		if(pIPRule->m_bAutomatic) {
			if(pIPRule->getExpiryTime() != RuleTime::Special) // If rule isn't forever or session
				ban(pIPRule->IP(), 10, false, pIPRule->m_sComment); // Add 30 seconds to the rule time for every hit.
		} else {
			if ( !pIPRule->isExpired( tNow ) && pIPRule->match( oAddress ) )
			{
				hit( pIPRule );

				if ( pIPRule->m_nAction == RuleAction::Accept )
					return false;
				else if ( pIPRule->m_nAction == RuleAction::Deny )
					return true;
				else
					Q_ASSERT( pIPRule->m_nAction == RuleAction::None );
			}
		}
	}

	// If the IP is not within the rules (and we're using the cache),
	// add the IP to the miss cache.
	missCacheAdd( qHash( oAddress ) );

	// In this case, return our default policy
	return m_bDenyPolicy;
}

/**
  * Checks a hit against the security database. Expects a list of all search keywords in the same
  * order they have been entered in the edit box of the GUI. Note that if no keywords are passed or
  * the list is in the wrong order, this beaks RegEx compatibility.
  * Locking: R (3x)
  */
bool CSecurity::isDenied(const CQueryHit* const pHit, const QList<QString> &lQuery)
{
	QMutexLocker locker(&m_pSection);
	return ( isDenied( pHit ) ||                             // test hashes, file size and extension
			 isDenied( pHit->m_sDescriptiveName ) ||         // test file name
			 isDenied( lQuery, pHit->m_sDescriptiveName ) ); // test regex
}

bool CSecurity::isPrivate(const CEndPoint &oAddress)
{
	if( oAddress <= CEndPoint("0.255.255.255") )
		return true;

	if( oAddress >= CEndPoint("10.0.0.0") &&
	  oAddress <= CEndPoint("10.255.255.255") )
		return true;

	if( oAddress >= CEndPoint("100.64.0.0") &&
	  oAddress <= CEndPoint("100.127.255.255") )
		return true;

	if( oAddress >= CEndPoint("127.0.0.0") &&
	  oAddress <= CEndPoint("127.255.255.255") )
		return true;

	if( oAddress >= CEndPoint("169.254.0.0") &&
	  oAddress <= CEndPoint("169.254.255.255") )
		return true;

	if( oAddress >= CEndPoint("172.16.0.0") &&
	  oAddress <= CEndPoint("172.31.255.255") )
		return true;

	if( oAddress >= CEndPoint("192.0.0.0") &&
	  oAddress <= CEndPoint("192.0.2.255") )
		return true;

	if( oAddress >= CEndPoint("192.0.2.0") &&
	  oAddress <= CEndPoint("10.255.255.255") )
		return true;

	if( oAddress >= CEndPoint("192.168.0.0") &&
	  oAddress <= CEndPoint("192.168.255.255") )
		return true;

	if( oAddress >= CEndPoint("198.18.0.0") &&
	  oAddress <= CEndPoint("198.19.255.255") )
		return true;

	if( oAddress >= CEndPoint("198.51.100.0") &&
	  oAddress <= CEndPoint("198.51.100.255") )
		return true;

	if( oAddress >= CEndPoint("203.0.113.0") &&
	  oAddress <= CEndPoint("203.0.113.255") )
		return true;

	if( oAddress >= CEndPoint("240.0.0.0") &&
	  oAddress <= CEndPoint("255.255.255.255") )
		return true;

	return false;
}

CIPRule *CSecurity::isInAddressRules(const CEndPoint nIp)
{
	if ( m_lIPs.isEmpty() )
	{
		return NULL;
	}

	int nMiddle;
	int nBegin = 0;
	int nEnd = m_lIPs.size();

	int n = nEnd - nBegin;

	int nHalf;

	while (n > 0)
	{
		nHalf = n >> 1;

		nMiddle = nBegin + nHalf;

		if ( nIp < m_lIPs.at(nMiddle)->IP() )
		{
			n = nHalf;
		}
		else
		{
			if( nIp <= m_lIPs.at(nMiddle)->IP() )
			{
				return m_lIPs.at(nMiddle);
			}
			nBegin = nMiddle + 1;
			n -= nHalf + 1;
		}
	}

	return NULL;
}

CIPRangeRule* CSecurity::isInAddressRangeRules(const CEndPoint nIp)
{
	if ( m_lIPRanges.isEmpty() )
	{
		return NULL;
	}

	int nMiddle;
	int nBegin = 0;
	int nEnd = m_lIPRanges.size();

	int n = nEnd - nBegin;

	int nHalf;

	while (n > 0)
	{
		nHalf = n >> 1;

		nMiddle = nBegin + nHalf;

		if ( nIp < m_lIPRanges.at(nMiddle)->startIP() )
		{
			n = nHalf;
		}
		else
		{
			if( nIp <= m_lIPRanges.at(nMiddle)->endIP() )
			{
				return m_lIPRanges.at(nMiddle);
			}
			nBegin = nMiddle + 1;
			n -= nHalf + 1;
		}
	}

	return NULL;
}

/**
  * Returns true if the remote computer is running a client that is breaking GPL, causing problems,
  * etc. We don't actually ban these clients, but we don't accept them as a leaf. Can still upload,
  * though.
  * Locking: /
  */
bool CSecurity::isClientBad(const QString& sUserAgent) const
{
	// No user agent- assume bad - They allowed to connect but no searches were performed
	if ( sUserAgent.isEmpty() )                 return true;

	QString sSubStr;

	// Bad/unapproved versions of Shareaza
	// Really obsolete versions of Shareaza should be blocked. (they may have bad settings)
	if ( sUserAgent.startsWith( "shareaza", Qt::CaseInsensitive ) )
	{
		sSubStr = sUserAgent.mid( 8 );
		if ( sSubStr.startsWith( " 0."  ) )     return true;
		// There can be some 1.x versions of the real Shareaza but most are fakes
		if ( sSubStr.startsWith( " 1."  ) )     return true;
		// There is also a Shareaza rip-off that identifies as Shareaza 2.0.0.0. Also, the real
		if ( sSubStr.startsWith( " 2.0" ) )     return true; // Shareaza 2.0.0.0 is old and bad.
		if ( sSubStr.startsWith( " 2.1" ) )     return true; // Old version
		if ( sSubStr.startsWith( " 2.2" ) )     return true; // Old version
		if ( sSubStr.startsWith( " 2.3" ) )     return true; // Old version
		if ( sSubStr.startsWith( " 2.4" ) )     return true; // Old version
		if ( sSubStr.startsWith( " 2.5.0") )	return true; // Old version
		if ( sSubStr.startsWith( " 3.0" ) )     return true;
		if ( sSubStr.startsWith( " 3.1" ) )     return true;
		if ( sSubStr.startsWith( " 3.2" ) )     return true;
		if ( sSubStr.startsWith( " 3.3" ) )     return true;
		if ( sSubStr.startsWith( " 3.4" ) )     return true;
		if ( sSubStr.startsWith( " 6."  ) )     return true;
		if ( sSubStr.startsWith( " 7."  ) )     return true;
		if ( sSubStr.startsWith( " Pro" ) )     return true;

		return false;
	}

	// Dianlei: Shareaza rip-off
	// add only based on alpha code, need verification for others
	if ( sUserAgent.startsWith( "Dianlei", Qt::CaseInsensitive ) )
	{
		sSubStr = sUserAgent.mid( 7 );
		if ( sSubStr.startsWith( " 1." ) )      return true;
		if ( sSubStr.startsWith( " 0." ) )      return true;

		return false;
	}

	// BearShare
	if ( sUserAgent.startsWith( "BearShare", Qt::CaseInsensitive ) )
	{
		sSubStr = sUserAgent.mid( 9 );
		if ( sSubStr.startsWith( " Lite"  ) )   return true;
		if ( sSubStr.startsWith( " Pro"   ) )   return true;
		if ( sSubStr.startsWith( " MP3"   ) )   return true;	// GPL breaker
		if ( sSubStr.startsWith( " Music" ) )   return true;	// GPL breaker
		if ( sSubStr.startsWith( " 6."    ) )   return true;	// iMesh

		return false;
	}

	// Fastload.TV
	if ( sUserAgent.startsWith( "Fastload.TV", Qt::CaseInsensitive ) )            return true;

	// Fildelarprogram
	if ( sUserAgent.startsWith( "Fildelarprogram", Qt::CaseInsensitive ) )        return true;

	// Gnutella Turbo (Look into this client some more)
	if ( sUserAgent.startsWith( "Gnutella Turbo", Qt::CaseInsensitive ) )         return true;

	// Identified Shareaza Leecher Mod
	if ( sUserAgent.startsWith( "eMule mod (4)", Qt::CaseInsensitive ) )          return true;

	// iMesh
	if ( sUserAgent.startsWith( "iMesh", Qt::CaseInsensitive ) )                  return true;

	// Mastermax File Sharing
	if ( sUserAgent.startsWith( "Mastermax File Sharing", Qt::CaseInsensitive ) ) return true;

	// Trilix
	if ( sUserAgent.startsWith( "Trilix", Qt::CaseInsensitive ) )                 return true;

	// Wru (bad GuncDNA based client)
	if ( sUserAgent.startsWith( "Wru", Qt::CaseInsensitive ) )                    return true;

	// GPL breakers- Clients violating the GPL
	// See http://www.gnu.org/copyleft/gpl.html
	// Some other breakers outside the list

	if ( sUserAgent.startsWith( "C -3.0.1", Qt::CaseInsensitive ) )               return true;

	// outdated rip-off
	if ( sUserAgent.startsWith( "eTomi", Qt::CaseInsensitive ) )                  return true;

	// Shareaza rip-off / GPL violator
	if ( sUserAgent.startsWith( "FreeTorrentViewer", Qt::CaseInsensitive ) )      return true;

	// Is it bad?
	if ( sUserAgent.startsWith( "K-Lite", Qt::CaseInsensitive ) )                 return true;

	// Leechers, do not allow to connect
	if ( sUserAgent.startsWith( "mxie", Qt::CaseInsensitive ) )                   return true;

	// ShareZilla (bad Shareaza clone)
	if ( sUserAgent.startsWith( "ShareZilla", Qt::CaseInsensitive ) )             return true;

	// Rip-off with bad tweaks
	if ( sUserAgent.startsWith( "SlingerX", Qt::CaseInsensitive ) )               return true;

	// Not clear why it's bad
	if ( sUserAgent.startsWith( "vagaa", Qt::CaseInsensitive ) )                  return true;

	// Unknown- Assume OK
	return false;
}

/**
  * Returns true for especially bad / leecher clients, as well as user defined agent blocks.
  * Locking: R
  */
bool CSecurity::isAgentBlocked(const QString& sUserAgent)
{
	// The remote computer didn't send a "User-Agent", or it sent whitespace
	// We don't like those.
	if ( sUserAgent.isEmpty() )                                         return true;

	// i2hub - leecher client. (Tested, does not upload)
	if ( sUserAgent.startsWith( "i2hub 2.0", Qt::CaseInsensitive ) )    return true;

	// foxy - leecher client. (Tested, does not upload)
	// having something like Authentication which is not defined on specification
	if ( sUserAgent.startsWith( "foxy", Qt::CaseInsensitive ) )         return true;

	// Check by content filter
	return isAgentDenied( sUserAgent );
}

/**
  * Returns true for blocked vendors.
  * Locking: /
  */
bool CSecurity::isVendorBlocked(const QString& sVendor) const
{
	// foxy - leecher client. (Tested, does not upload)
	// having something like Authentication which is not defined on specification
	if ( sVendor.startsWith( "foxy", Qt::CaseInsensitive ) ) return true;

	// Allow it
	return false;
}

//////////////////////////////////////////////////////////////////////
// CSecurity load and save
/**
  * Initializes signal/slot connections, pulls settings and sets up cleanup interval counters.
  * Locking: RW
  */
bool CSecurity::start()
{
	// Register QSharedPointer<CSecureRule> to allow using this type with queued signal/slot
	// connections.
	qRegisterMetaType<CSecureRule>( "CSecureRule" );

	// Set up interval timed cleanup operations.
	m_tMaintenance = new QTimer(this);
	connect(m_tMaintenance, SIGNAL(timeout()), SLOT(expire()));
	connect(m_tMaintenance, SIGNAL(timeout()), SLOT(missCacheClear()));
	m_tMaintenance->start(1000);

	connect( &quazaaSettings, SIGNAL( securitySettingsChanged() ), SLOT( settingsChanged() ) );

	// Pull settings from global database to local copy.
	settingsChanged();

	return load(); // Load security rules from HDD.
}

/**
  * Saves the rules to disk, disonnects signal/slot connections, frees memory
  * and clears up storage containers.
  * Call this to make the security manager ready for destruction
  * Locking: RW
  */
bool CSecurity::stop()
{
	disconnect( &quazaaSettings, SIGNAL( securitySettingsChanged() ),
				this, SLOT( settingsChanged() ) );

	bool bSaved = save( true ); // Save security rules to disk.
	clear();                    // Release memory and free containers.

	signalQueue.pop( this );    // Remove all cleanup intervall timers from the queue.

	return bSaved;
}

/**
  * Loads the rule database from the HDD.
  * Locking: RW
  */
bool CSecurity::load()
{
	QString sPath = CQuazaaGlobals::DATA_PATH() + "security.dat";

	if ( load( sPath ) )
	{
		return true;
	}
	else
	{
		sPath = QDir::toNativeSeparators(QString("%1/DefaultSecurity.dat").arg(qApp->applicationDirPath()));
		qDebug() << "Default security file path: " << sPath;
		return load(sPath);
	}
}

/**
  * Private helper method for load()
  * Locking: RW
  */
bool CSecurity::load( QString sPath )
{
	QFile oFile( sPath );

	if ( ! oFile.open( QIODevice::ReadOnly ) )
		return false;

	CSecureRule* pRule = NULL;

	try
	{
		clear();

		QDataStream fsFile( &oFile );

		quint16 nVersion;
		fsFile >> nVersion;

		bool bDenyPolicy;
		fsFile >> bDenyPolicy;

		quint32 nCount;
		fsFile >> nCount;

		const quint32 tNow = common::getTNowUTC();

		m_bDenyPolicy = bDenyPolicy;
		m_bIsLoading = true; // Prevent sanity check from being executed at each add() operation.
		int nSuccessCount = 0;

		while ( nCount > 0 )
		{
			CSecureRule::load( pRule, fsFile, nVersion );

			if ( pRule->isExpired( tNow, true ) )
			{
				delete pRule;
			}
			else
			{
				add( pRule );
				++nSuccessCount;
			}

			pRule = NULL;

			nCount--;
			qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 50);
		}

		if(nSuccessCount > 0) {
			systemLog.postLog(LogSeverity::Debug, Components::Security, tr("Loaded security rules from file: %1").arg(sPath));
			systemLog.postLog(LogSeverity::Debug, Components::Security, tr("Loaded %1 rules.").arg(nSuccessCount));
		}

		// If necessary perform sanity check after loading.
		qSort(m_lIPs.begin(), m_lIPs.end(), IPLessThan);
		qSort(m_lIPRanges.begin(), m_lIPRanges.end(), IPRangeLessThan);
		sanityCheck();

		m_bIsLoading = false;
	}
	catch ( ... )
	{
		if ( pRule )
			delete pRule;

		clear();
		oFile.close();

		m_bIsLoading = false;

		return false;
	}
	oFile.close();

	return true;
}

/**
  * Helper method for save()
  * Requires Locking: R
  */
quint32 CSecurity::writeToFile(const void * const pManager, QFile& oFile)
{
	quint16 nVersion = SECURITY_CODE_VERSION;

	QDataStream oStream( &oFile );
	CSecurity* pSManager = (CSecurity*)pManager;

	oStream << nVersion;
	oStream << pSManager->m_bDenyPolicy;
	oStream << pSManager->getCount();

	for ( int i = 0; i != pSManager->m_lRules.size(); ++i )
	{
		const CSecureRule* pRule = pSManager->m_lRules.at(i);
		CSecureRule::save( pRule, oStream );
	}

	return pSManager->getCount();
}

/**
  * Saves the security rules to HDD. Skips saving if there haven't
  * been any important changes and bForceSaving is not set to true.
  * Locking: R
  */
bool CSecurity::save(bool bForceSaving) const
{
	if ( m_nUnsaved.loadAcquire() < m_nMaxUnsavedRules && !bForceSaving )
	{
		return true;		// Saving not required ATM.
	}

	bool bReturn;

	if ( !common::securedSaveFile( CQuazaaGlobals::DATA_PATH(), "security.dat", "[Security] ",
									this, &CSecurity::writeToFile ) )
	{
		bReturn = false;
	}
	else
	{
		m_nUnsaved.fetchAndStoreOrdered( 0 );
		bReturn = true;
	}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////
// CSecurity XML
/**
  * Security xml file schema specification.
  */
const QString CSecurity::xmlns = "http://www.shareaza.com/schemas/Security.xsd";

/**
  * Exports all rules to an XML file.
  * Locking: R
  */
bool CSecurity::toXML(const QString& sPath) const
{
	QFile oFile( sPath );
	if( !oFile.open( QIODevice::ReadWrite ) )
		return false;

	QXmlStreamWriter xmlDocument( &oFile );

	xmlDocument.writeStartElement( xmlns, "security" );
	xmlDocument.writeAttribute( "version", "2.0" );

	for ( int i = 0; i < m_lRules.size() ; ++i )
	{
		m_lRules.at(i)->toXML( xmlDocument );
	}

	xmlDocument.writeEndElement();

	return true;
}

/**
  * Imports rules from an XML file.
  * Locking: RW
  */
bool CSecurity::fromXML(const QString& sPath)
{
	QFile oFile( sPath );
	if ( !oFile.open( QIODevice::ReadOnly ) )
		return false;

	QXmlStreamReader xmlDocument( &oFile );

	if ( xmlDocument.atEnd() ||
		 !xmlDocument.readNextStartElement() ||
		 xmlDocument.name().toString().compare( "security", Qt::CaseInsensitive ) )
		return false;

	systemLog.postLog( LogSeverity::Security,
			 Components::Security, tr( "Importing security rules from file: " ) + sPath );

	float nVersion;

	QXmlStreamAttributes attributes = xmlDocument.attributes();
	{
		bool bOK;
		QString sVersion;

		// attributes.value() returns an empty StringRef if the attribute "version" is not present.
		// In that case the conversion to float fails and version is set to 1.0.
		sVersion = attributes.value( "version" ).toString();
		nVersion = sVersion.toFloat( &bOK );
		if ( !bOK )
		{
			systemLog.postLog( LogSeverity::Security,
					 Components::Security,
					 tr( "Failed to read the Security XML version number from file." ) );
			nVersion = 1.0;
		}
	}

	const quint32 tNow = common::getTNowUTC();

	m_bIsLoading = true;

	CSecureRule* pRule = NULL;
	unsigned int nRuleCount = 0;

	// For all rules do:
	while ( !xmlDocument.atEnd() )
	{
		// Go forward until the beginning of the next rule
		xmlDocument.readNextStartElement();

		// Verify whether it's a rule
		if ( xmlDocument.name().toString().compare( "rule", Qt::CaseInsensitive ) )
		{
			// Parse it
			pRule = CSecureRule::fromXML( xmlDocument, nVersion );

			if ( pRule )
			{
				if ( !pRule->isExpired( tNow ) )
				{
					add( pRule );
				}
				else
				{
					delete pRule;
				}
				pRule = NULL;
				++nRuleCount;
			}
			else
			{
				systemLog.postLog( LogSeverity::Security,
						 Components::Security, tr( "Failed to read a Security Rule from XML." ) );
			}
		}
		else
		{
			systemLog.postLog( LogSeverity::Security,
					 Components::Security,
					 tr( "Unrecognized entry in XML file with name: " ) +
					 xmlDocument.name().toString() );
		}
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 50);
	}

	m_bIsLoading = false;

	qSort(m_lIPs.begin(), m_lIPs.end(), IPLessThan);
	qSort(m_lIPRanges.begin(), m_lIPRanges.end(), IPRangeLessThan);
	sanityCheck();
	save();

	systemLog.postLog( LogSeverity::Security,
			 Components::Security, QString::number( nRuleCount ) + tr( " Rules imported." ) );

	return nRuleCount != 0;
}

bool CSecurity::fromP2P(const QString &sFile)
{
	QFile file(sFile);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	emit updateLoadMax(file.size());

	m_bIsLoading = true;

	int iGuiThrottle = 0;
	QTextStream import(&file);
	while (!import.atEnd()) {
		QString line = import.readLine();
		emit updateLoadProgress(import.pos());

		if(!line.isEmpty() && !line.startsWith("#") && line.contains(":"))
		{
			QStringList arguments = line.split(":");
			QString comment = arguments.at(0);
			QString rule = arguments.at(1);
			CSecureRule* pRule;

			QStringList addresses = rule.split("-");

			if(addresses.at(0) == addresses.at(1)) {
				rule = addresses.at(0);
				pRule = new CIPRule();
			} else {
				pRule = new CIPRangeRule();
			}

			if( !pRule->parseContent(rule) )
				break;

			pRule->m_sComment = comment;
			pRule->m_nAction = RuleAction::Deny;
			pRule->setForever(true);
			pRule->m_bAutomatic = false;

			add( pRule );
		}
		++iGuiThrottle;
		if(iGuiThrottle == 50) {
			qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 50);
			iGuiThrottle = 0;
		}
	}
	m_bIsLoading = false;

	qSort(m_lIPs.begin(), m_lIPs.end(), IPLessThan);
	qSort(m_lIPRanges.begin(), m_lIPRanges.end(), IPRangeLessThan);
	sanityCheck();
	save();

	return true;
}

const char* CSecurity::ruleInfoSignal = SIGNAL( ruleInfo( CSecureRule* ) );

/**
  * Returns the number of listeners to a given signal of the Security Manager.
  * Note that this is a method that violates the modularity principle.
  * Plz don't use it if you've got a problem with this (for example because of religious reasons).
  * Locking: /
  */
int CSecurity::receivers(const char* signal) const
{
	return QObject::receivers( signal );
}

//////////////////////////////////////////////////////////////////////
// Qt slots
/**
  * Qt slot. Triggers the Security Manager to emit all rules using the ruleInfo() signal.
  * Locking: R
  */
void CSecurity::requestRuleList()
{
	for ( int i = 0; i < m_lRules.size(); ++i )
	{
		emit ruleInfo( m_lRules.at(i) );
	}
}

//////////////////////////////////////////////////////////////////////
// Sanity checking slots
/**
  * Qt slot. Triggers a system wide sanity check.
  * The sanity check is delayed by 5s, if a write lock couldn't be aquired after 500ms.
  * The sanity check is aborted if it takes longer than 2min to finish.
  * Locking: RW
  */
void CSecurity::sanityCheck()
{
	// This indicates that an error happend previously.
	Q_ASSERT( !m_bNewRulesLoaded || !m_lLoadedAddressRules.empty() || !m_lLoadedHitRules.empty());

	// Check whether there are new rules to deal with.
	bool bNewRules = m_lqNewAddressRules.size() || m_lqNewHitRules.size();

	if ( bNewRules )
	{
		if ( !m_bNewRulesLoaded )
		{
			loadNewRules();

			// Count how many "OK"s we need to get back.
			m_nPendingOperations = receivers( SIGNAL( performSanityCheck() ) );

			// if there is anyone listening, start the sanity check
			if ( m_nPendingOperations )
			{
#ifdef _DEBUG
				// Failsafe mechanism in case there are massive problems somewhere else.
				m_idForceEoSC = signalQueue.push( this, "forceEndOfSanityCheck", 120 );
#endif

				// Inform all other modules about the necessity of a sanity check.
				emit performSanityCheck();
			}
			else
			{
				clearNewRules();
			}
		}
		else // other sanity check still in progress
		{
			// try again later
			signalQueue.push( this, "sanityCheck", 5 );
		}
	}
	else // We didn't get a write lock in a timely manner.
	{
		// try again later
		signalQueue.push( this, "sanityCheck", 5 );
	}
}

/**
  * Qt slot. Must be notified by all listeners to the signal performSanityCheck().
  * Locking: RW
  */
void CSecurity::sanityCheckPerformed()
{
	Q_ASSERT( m_bNewRulesLoaded );        // TODO: remove after testing
	Q_ASSERT( m_nPendingOperations > 0 );

	if ( --m_nPendingOperations == 0 )
	{
		systemLog.postLog( LogSeverity::Security,
				 Components::Security, QString( "Sanity Check finished successfully. " ) +
				 QString( "Starting cleanup now." ) );

		clearNewRules();
	}
	else
	{
		systemLog.postLog( LogSeverity::Security,
				 Components::Security, QString( "A component finished with sanity checking. " ) +
				 QString( "Still waiting for %s other components to finish."
						  ).arg( m_nPendingOperations ) );
	}
}

/**
  * Qt slot. Aborts the currently running sanity check by clearing its rule lists.
  * Locking: RW
  */
void CSecurity::forceEndOfSanityCheck()
{
#ifdef _DEBUG
	if ( m_nPendingOperations )
	{
		QString sTmp = QString( "Sanity check aborted. Most probable reason: It took some " ) +
					   QString( "component longer than 2min to call sanityCheckPerformed() " ) +
					   QString( "after having recieved the signal performSanityCheck()." );
		systemLog.postLog( LogSeverity::Security,
				 Components::Security, sTmp );
		Q_ASSERT( false );
	}
#endif //_DEBUG

	clearNewRules();
}

/**
  * Qt slot. Checks the security database for expired rules.
  * Locking: RW
  */
void CSecurity::expire()
{
	QMutexLocker locker(&m_pSection);
	const quint32 tNow = common::getTNowUTC();
	quint16 nCount = 0;

	int j, i = 0;
	while (  i < m_lRules.size() )
	{
		if ( m_lRules.at(i)->isExpired( tNow ) )
		{
			j = i;
			++j; // Make sure we have a valid iterator after removal.
			remove( m_lRules.at(i) );
			i = j;
			++nCount;
		}
		else
		{
			++i;
		}
	}

	if(nCount > 0)
		systemLog.postLog( LogSeverity::Security,
				 Components::Security, QString::number( nCount ) + " rules expired." );
}

/**
  * Qt slot. Clears the miss cache.
  * Locking: RW
  */
void CSecurity::missCacheClear()
{
	QMutexLocker locker(&m_pSection);
	m_lsCache.clear();

	if ( !m_bUseMissCache )
		evaluateCacheUsage();
}

/**
  * Qt slot. Pulls all relevant settings from quazaaSettings.Security
  * and refreshes all components depending on them.
  * Locking: RW
  */
void CSecurity::settingsChanged()
{
	m_bLogIPCheckHits			= quazaaSettings.Security.LogIPCheckHits;

	// TODO: load from settings.
	m_nMaxUnsavedRules = 100;
}

//////////////////////////////////////////////////////////////////////
// Private method definitions

void CSecurity::loadNewRules()
{
	// should both be empty
	Q_ASSERT( !( m_lLoadedAddressRules.size() || m_lLoadedHitRules.size() ) );

	// there should be at least 1 new rule
	Q_ASSERT( m_lqNewAddressRules.size() || m_lqNewHitRules.size() );

	CSecureRule* pRule = NULL;

	while ( m_lqNewAddressRules.size() )
	{
		pRule = m_lqNewAddressRules.dequeue();

		// Only IP, IP range and coutry rules are allowed.
		Q_ASSERT( pRule->type() != 0 && pRule->type() < 4 );

		m_lLoadedAddressRules.push_back( pRule );

		pRule = NULL;
	}

	while ( m_lqNewHitRules.size() )
	{
		pRule = m_lqNewHitRules.dequeue();

		// Only hit related rules are allowed.
		Q_ASSERT( pRule->type() > 3 );

		m_lLoadedHitRules.push_back( pRule );

		pRule = NULL;
	}

	m_bNewRulesLoaded = true;
}

void CSecurity::clearNewRules()
{
	Q_ASSERT( m_bNewRulesLoaded );

	// There should at least be one rule.
	Q_ASSERT( m_lLoadedAddressRules.size() || m_lLoadedHitRules.size() );

	CSecureRule* pRule = NULL;

	while ( m_lLoadedAddressRules.size() )
	{
		pRule = m_lLoadedAddressRules.front();
		m_lLoadedAddressRules.pop_front();

		// only IP, IP range and coutry rules allowed
		Q_ASSERT( pRule->type() > 0 && pRule->type() < 4 );

		delete pRule;
		pRule = NULL;
	}

	while ( m_lLoadedHitRules.size() )
	{
		pRule = m_lLoadedHitRules.front();
		m_lLoadedHitRules.pop_front();

		// Only hit related rules are allowed
		Q_ASSERT( pRule->type() > 3 );

		delete pRule;
		pRule = NULL;
	}

	m_bNewRulesLoaded = false;
}

CHashRule* CSecurity::getHash(const QList<CHash>& hashes) const
{
	// We are not searching for any hash. :)
	if ( hashes.isEmpty() )
		return NULL;

	// For each hash that has been given to the function:
	QList<CHashRule*> lHashesCheck;
	foreach ( CHash oHash, hashes )
	{
		uint iHash = qHash( oHash.rawValue() );
		lHashesCheck = m_lmmHashes.values( iHash );

		foreach( CHashRule* pHashRuleCheck, lHashesCheck )
		{
			if ( pHashRuleCheck->match( hashes ) )
				return ((CHashRule*)getUUID( pHashRuleCheck->m_oUUID ));
		}
	}

	return NULL;
}

CSecureRule* CSecurity::getUUID(const QUuid& oUUID) const
{
	for ( int i = 0; i < m_lRules.size(); i++ )
		if ( m_lRules.at(i)->m_oUUID == oUUID ) return m_lRules.at(i);

	return NULL;
}

void CSecurity::remove(CSecureRule* pRule)
{
	QMutexLocker locker(&m_pSection);
	if(!pRule->isLockedForModify()) {
		pRule->beingRemoved(true);

		// Removing the rule from special containers for fast access.
		switch ( pRule->type() )
		{
		case RuleType::IPAddress:
		{
			for (int i = 0; i < m_lIPs.size(); ++i) {
				if ( m_lIPs.at(i)->m_oUUID == pRule->m_oUUID ) {
					m_lIPs.removeAt(i);
					break;
				}
			}
		}
		break;

		case RuleType::IPAddressRange:
		{
			for (int i = 0; i < m_lIPRanges.size(); ++i) {
			if ( m_lIPRanges.at(i)->m_oUUID == pRule->m_oUUID ) {
					m_lIPRanges.removeAt(i);
					break;
				}
			}

			if ( m_bUseMissCache )
				evaluateCacheUsage();
		}
		break;

		case RuleType::Hash:
		{
			CHashRule* pHashRule = (CHashRule*)pRule;

			QList<CHash> lHashes = pHashRule->getHashes();

			QList<CHashRule*> lHashesCheck;
			foreach ( CHash oHash, lHashes )
			{
				uint iHash = qHash( oHash.rawValue() );
				lHashesCheck = m_lmmHashes.values( iHash );

				foreach( CHashRule* pHashRuleCheck, lHashesCheck )
				{
					if ( pHashRuleCheck->m_oUUID == pHashRule->m_oUUID )
					{
						m_lmmHashes.remove( iHash, pHashRule );
					}
				}
			}
		}
		break;

		case RuleType::Content:
		{
			int i = 0;

			while ( i < m_lContents.size() )
			{
				if ( m_lContents.at(i)->m_oUUID == pRule->m_oUUID )
				{
					m_lContents.removeAt(i);
					break;
				}

				++i;
			}
		}
		break;

		case RuleType::RegularExpression:
		{
			int i = 0;

			while ( i < m_lRegularExpressions.size() )
			{
				if ( m_lRegularExpressions.at(i)->m_oUUID == pRule->m_oUUID )
				{
					m_lRegularExpressions.removeAt( i );
					break;
				}

				++i;
			}
		}
		break;

		case RuleType::UserAgent:
		{
			QMap<QString, CUserAgentRule*>::iterator it = m_lmUserAgents.begin();

			while ( it != m_lmUserAgents.end() )
			{
				if ( it.value()->m_oUUID == pRule->m_oUUID )
				{
					m_lmUserAgents.erase( it );
					break;
				}

				++it;
			}
		}
		break;

		default:
			Q_ASSERT( false );
		}

		m_nUnsaved.fetchAndAddRelaxed( 1 );

		// Remove rule entry from list of all rules
		m_lRules.removeOne(pRule);

		emit ruleRemoved( pRule );
	}
}

bool CSecurity::isAgentDenied(const QString& sUserAgent)
{
	if ( sUserAgent.isEmpty() )
		return false;

	const quint32 tNow = common::getTNowUTC();

	QMap<QString, CUserAgentRule*>::iterator it = m_lmUserAgents.find( sUserAgent );
	if ( it != m_lmUserAgents.end() )
	{
		CUserAgentRule* pRule = it.value();

		if ( !pRule->isExpired( tNow ) && pRule->match( sUserAgent ) )
		{
			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}
	}

	return false;
}

void CSecurity::missCacheAdd(const uint &nIP)
{
	QMutexLocker locker(&m_pSection);
	if ( m_bUseMissCache )
	{
		m_lsCache.insert( nIP );
		evaluateCacheUsage();
	}
}

void CSecurity::evaluateCacheUsage()
{
	double nCache		= m_lsCache.size();
	double nIPMap		= m_lIPs.size();

	static const double log2	= log( 2.0 );

	static double s_nCache		= 0;
	static double s_nLogCache	= 0;

	static double s_nIPMap		= -1;
	static double s_nLogMult	= 0;

	// Only do the heavy log operations if necessary.
	if ( s_nCache != nCache )
	{
		s_nCache    = nCache;

		if ( !nCache )
			nCache = 1;

		s_nLogCache = log( nCache );
	}

	// Only do the heavy log operations if necessary.
	if ( s_nIPMap != nIPMap
		 )
	{
		s_nIPMap		= nIPMap;

		if ( !nIPMap )
			nIPMap = 1;

		s_nLogMult	= log( nIPMap );
	}

	m_bUseMissCache = ( s_nLogCache < s_nLogMult + m_lIPRanges.size() * log2 );
}

bool CSecurity::isDenied(const QString& sContent)
{
	if ( sContent.isEmpty() )
		return false;

	const quint32 tNow = common::getTNowUTC();

	int i = 0;
	while ( i < m_lContents.size() )
	{
		CContentRule* pRule = m_lContents.at(i);
		++i;

		if ( pRule->isExpired( tNow ) )
		{
			continue; // let the expire() method handle expiries
		}

		if ( pRule->match( sContent ) )
		{
			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}
	}

	return false;
}

bool CSecurity::isDenied(const CQueryHit* const pHit)
{
	if ( !pHit )
		return false;

	const QList<CHash>& lHashes = pHit->m_lHashes;

	const quint32 tNow = common::getTNowUTC();

	// Search for a rule matching these hashes
	CHashRule* pExRule = getHash( lHashes );

	// If this rule matches the file, return the specified action.
	if ( pExRule )
	{
		CHashRule* pRule = ((CHashRule*)pExRule);
		if ( pRule->match( lHashes ) && !pRule->isExpired( tNow ) )
		{
			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}
	}

	// Else check other content rules.
	int i = 0;
	while ( i < m_lContents.size() )
	{
		CContentRule* pRule = m_lContents.at(i);
		++i;

		if ( pRule->isExpired( tNow ) )
		{
			continue; // let the expire() method handle expiries
		}

		if ( pRule->match( pHit ) )
		{
			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}
	}

	return false;
}

bool CSecurity::isDenied(const QList<QString>& lQuery, const QString& sContent)
{
	if ( lQuery.isEmpty() || sContent.isEmpty() )
		return false;

	const quint32 tNow = common::getTNowUTC();

	int i = 0;
	while ( i < m_lRegularExpressions.size() )
	{
		CRegularExpressionRule* pRule = m_lRegularExpressions.at(i);
		++i;

		if ( pRule->isExpired( tNow ) )
		{
			continue; // let the expire() method handle expiries
		}

		if ( pRule->match( lQuery, sContent ) )
		{
			hit( pRule );

			if ( pRule->m_nAction == RuleAction::Accept )
				return false;
			else if ( pRule->m_nAction == RuleAction::Deny )
				return true;
		}
	}

	return false;
}

