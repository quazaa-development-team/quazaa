/*
** tst_unittestssecurity.cpp
**
** Copyright © Quazaa Development Team, 2013-2013.
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

#include "unittestssecurity.h"

UnitTestsSecurity::UnitTestsSecurity()
{
}

void UnitTestsSecurity::initTestCase()
{
	geoIP.loadGeoIP();
	prepareTestData();

	m_pManager = new Manager();
	m_pManager->loadPrivates();

	uint nCount = 0;

//	securitySettigs.m_bLogIPCheckHits = true;
//	m_pManager->settingsChanged();
//	QVERIFY2( m_pManager->m_bLogIPCheckHits, "Failed to set Manager to be more verboose." );

	// Load single IP ban rules
	for ( ushort i = 0; i < m_vSingleIPTestData.size(); ++i )
	{
		QString sContent = m_vSingleIPTestData[i].first;
		EndPoint oIP = EndPoint( sContent );

		//qDebug() << ( QString::number( i ) + " : " + oIP.toString() ).toLocal8Bit().data();
		if ( m_vSingleIPTestData[i].second )
		{
			m_pManager->ban( oIP, RuleTime::Forever, true,
							 QString( "Test " ) + QString::number( i )
							 + " - " + sContent, false );
		}
		/*else
		{
			IPRule* pIPRule = new IPRule();
			Q_ASSERT( pIPRule->parseContent( sContent ) );

			pIPRule->m_nAction  = RuleAction::Accept;
			pIPRule->m_sComment = QString( "Test " ) + QString::number( i ) + " : " + sContent;

			m_pManager->add( pRule );
		}*/
	}
	nCount += NO_OF_IP_BANS;
	QVERIFY2( m_pManager->count() == nCount, "Wrong number of IPs loaded!" );

	// Load IP range rules
	for ( ushort i = 0; i < m_vRangeDefinitions.size(); ++i )
	{
		QString sContent = m_vRangeDefinitions[i].first;
		bool    bDeny    = m_vRangeDefinitions[i].second;

		//qDebug() << ( QString::number( i ) + " : " + sContent ).toLocal8Bit().data();

		IPRangeRule* pRule = new IPRangeRule();
		Q_ASSERT( pRule->parseContent( sContent ) );

		pRule->m_nAction  = bDeny ? RuleAction::Deny : RuleAction::Accept;
		pRule->m_sComment = QString( "Test " ) + QString::number( i ) + " : " + sContent;

		m_pManager->add( pRule );
	}
	nCount += NO_OF_IP_RANGE_RULES;
	//qDebug() << QString::number( nCount ).toLocal8Bit().data() << " IPs should have been loaded. "
	//		 << QString::number( m_pManager->count() ).toLocal8Bit().data() << "actually were.";
	QVERIFY2( m_pManager->count() == nCount, "Wrong number of IP ranges loaded!" );

	// Add country rules
	for ( ushort i = 0; i < m_vCountryTestData.size(); ++i )
	{
		QString sIP      = m_vCountryTestData[i].first;
		QString sCountry = m_vCountryTestData[i].second;
		QString sComment = QString::number( i ) + " : " + sIP + " (" + sCountry + ")";

		// qDebug() << ( sComment ).toLocal8Bit().data();

		CountryRule* pRule = new CountryRule();
		Q_ASSERT( pRule->parseContent( sCountry ) );

		pRule->m_nAction  = RuleAction::Deny;
		pRule->m_sComment = sComment;

		m_pManager->add( pRule );
	}

	// Add hash rules
	uint nHashRuleCount = 0;
	for ( ushort i = 0; i < m_vHashData.size(); ++i )
	{
		if ( m_vHashData[i].second )
		{
			QString sComment = QString::number( i ) + " : " + m_vHashData[i].first;

			HashRule* pRule = new HashRule();
			Q_ASSERT( pRule->parseContent( m_vHashData[i].first ) );

			pRule->m_nAction  = RuleAction::Deny;
			pRule->m_sComment = sComment;

			m_pManager->add( pRule );
			nHashRuleCount += m_vHashData[i].first.count( "&" ) + 1;
		}
	}
	QCOMPARE( nHashRuleCount, m_pManager->m_lmmHashes.size() );

	// content rules
	for ( ushort i = 0; i < m_vContentDefinitions.size(); ++i )
	{
		QString sComment = QString::number( i ) + " - " +
						   ( m_vContentDefinitions[i].second ? "all" : "any" ) + ": " +
						   m_vContentDefinitions[i].first;

		ContentRule* pRule = new ContentRule();
		Q_ASSERT( pRule->parseContent( m_vContentDefinitions[i].first ) );
		pRule->setAll( m_vContentDefinitions[i].second );

		pRule->m_nAction  = RuleAction::Deny;
		pRule->m_sComment = sComment;

		m_pManager->add( pRule );
	}
	QCOMPARE( NO_OF_CONTENT_RULES, ( int )m_pManager->m_vContents.size() );

	// agent rules
	for ( ushort i = 0; i < m_vAgentDefinitions.size(); ++i )
	{
		QString sComment = QString::number( i ) + " : " +
						   ( m_vAgentDefinitions[i].second ? "true" : "false" ) + " - " +
						   m_vAgentDefinitions[i].first;

		UserAgentRule* pRule = new UserAgentRule();
		Q_ASSERT( pRule->parseContent( m_vAgentDefinitions[i].first ) );

		pRule->m_nAction  = m_vAgentDefinitions[i].second ? RuleAction::Deny : RuleAction::Accept;
		pRule->m_sComment = sComment;

		m_pManager->add( pRule );
	}
	QCOMPARE( NO_OF_AGENT_RULES, ( int )m_pManager->m_vUserAgents.size() );


	m_lQuery.append( "w0" );
	m_lQuery.append( "w1" );
	m_lQuery.append( "w2" );
	m_lQuery.append( "w3" );

	// reg exp rules
	for ( ushort i = 0; i < m_vRegExpDefinitions.size(); ++i )
	{
		QString sComment = QString::number( i ) + " : " +
						   ( m_vRegExpDefinitions[i].second ? "true" : "false" ) + " - " +
						   m_vRegExpDefinitions[i].first;

		RegularExpressionRule* pRule = new RegularExpressionRule();
		Q_ASSERT( pRule->parseContent( m_vRegExpDefinitions[i].first ) );

		pRule->m_nAction  = m_vRegExpDefinitions[i].second ? RuleAction::Deny : RuleAction::Accept;
		pRule->m_sComment = sComment;

		m_pManager->add( pRule );
	}
	QCOMPARE( NO_OF_REGEXP_RULES, ( int )m_pManager->m_vRegularExpressions.size() );
}

void UnitTestsSecurity::cleanupTestCase()
{
	m_pManager->clear();
}


void UnitTestsSecurity::testDeniedIPs()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	EndPoint oIP = EndPoint( IP );

	QCOMPARE( m_pManager->isDenied( oIP ), isDenied );
}
void UnitTestsSecurity::testDeniedIPs_data()
{
	// Make sure the IPs are not filtered out because they are private IPs
	// but because there are actually rules for them.
	m_pManager->m_bDenyPrivateIPs  = false;

	// Make sure the country rules are disabled.
	m_pManager->m_bEnableCountries = false;

	populateRowsWithTestsForIPs();
}


#if ENABLE_BENCHMARKS
void UnitTestsSecurity::benchmarkDeniedIPs()
{
	QFETCH( QString, IP );

	EndPoint oIP = EndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isDenied( oIP );
	}
}
void UnitTestsSecurity::benchmarkDeniedIPs_data()
{
	// Make sure the IPs are not filtered out because they are private IPs
	// but because there are actually rules for them.
	m_pManager->m_bDenyPrivateIPs  = false;

	// Make sure the country rules are disabled.
	m_pManager->m_bEnableCountries = false;

	populateRowsWithTestsForIPs();
}
#endif // ENABLE_BENCHMARKS


void UnitTestsSecurity::testIPRanges()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	EndPoint oIP = EndPoint( IP );

	QCOMPARE( m_pManager->isDenied( oIP ), isDenied );
}
void UnitTestsSecurity::testIPRanges_data()
{
	// Make sure the IPs are not filtered out because they are private IPs
	// but because there are actually rules for them.
	m_pManager->m_bDenyPrivateIPs  = false;

	// Make sure the country rules are disabled.
	m_pManager->m_bEnableCountries = false;

	populateRowsWithTestsForIPRanges();
}


#if ENABLE_BENCHMARKS
void UnitTestsSecurity::benchmarkIPRanges()
{
	QFETCH( QString, IP );

	EndPoint oIP = EndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isDenied( oIP );
	}
}
void UnitTestsSecurity::benchmarkIPRanges_data()
{
	// Make sure the IPs are not filtered out because they are private IPs
	// but because there are actually rules for them.
	m_pManager->m_bDenyPrivateIPs  = false;

	// Make sure the country rules are disabled.
	m_pManager->m_bEnableCountries = false;

	populateRowsWithTestsForIPRanges();
}
#endif // ENABLE_BENCHMARKS


void UnitTestsSecurity::testCountries()
{
	QFETCH( QString, IP );

	EndPoint oIP = EndPoint( IP );

	QVERIFY( m_pManager->isDenied( oIP ) );
}
void UnitTestsSecurity::testCountries_data()
{
	// Make sure the IPs are not filtered out because they are private IPs
	// but because there are actually rules for them.
	m_pManager->m_bDenyPrivateIPs  = false;

	// Make sure the country rules are disabled.
	m_pManager->m_bEnableCountries = true;

	populateRowsWithTestsForCountries();
}


void UnitTestsSecurity::testHashes()
{
	QFETCH( QueryHit, oHit );
	QFETCH( bool, isDenied );

	QCOMPARE( m_pManager->isDenied( &oHit ), isDenied );
}
void UnitTestsSecurity::testHashes_data()
{
	QTest::addColumn< QueryHit >( "oHit" );
	QTest::addColumn< bool >( "isDenied" );

	for ( ushort i = 0; i < NO_OF_TEST_HASHES; ++i )
	{
		QString sName = QString::number( i ) + " - " +
						( m_vHashData[i].second ? "true" : "false" ) + " - " +
						m_vHashData[i].first;

		QString sHitName = QString::number( i );
		QueryHit oHit = generateQueryHit( 1, sHitName, m_vHashData[i].first );

		QCOMPARE( oHit.m_sDescriptiveName, sHitName );

		QTest::newRow( sName.toLocal8Bit().data() ) << oHit
													<< m_vHashData[i].second;
	}
}


void UnitTestsSecurity::testContent()
{
	QFETCH( QueryHit, oHit );
	QFETCH( bool, isDenied );

	QCOMPARE( m_pManager->isDenied( &oHit ), isDenied );
}
void UnitTestsSecurity::testContent_data()
{
	QTest::addColumn< QueryHit >( "oHit" );
	QTest::addColumn< bool >( "isDenied" );

	// a hash not used within the hash match testing
	QString sHash( "urn:sha1:2KFOLWPNLCJX42TYOWMMK3RACKYHSGRE" );

	for ( ushort i = 0; i < m_vContentTestData.size(); ++i )
	{
		QString sName = QString::number( i ) + " - " +
						( m_vContentTestData[i].second ? "true" : "false" ) + " - " +
						m_vContentTestData[i].first.first + " (Size:" +
						QString::number( m_vContentTestData[i].first.second ) + ")";

		QString sHitName = m_vContentTestData[i].first.first;
		QueryHit oHit = generateQueryHit( m_vContentTestData[i].first.second, sHitName, sHash );
		QTest::newRow( sName.toLocal8Bit().data() ) << oHit
													<< m_vContentTestData[i].second;
	}
}


void UnitTestsSecurity::testAgent()
{
	QFETCH( QString, sAgent );
	QFETCH( bool, isDenied );

	QCOMPARE( m_pManager->isAgentDenied( sAgent ), isDenied );
}
void UnitTestsSecurity::testAgent_data()
{
	QTest::addColumn< QString >( "sAgent" );
	QTest::addColumn< bool >( "isDenied" );

	for ( ushort i = 0; i < m_vAgentTestData.size(); ++i )
	{
		QString sName = QString::number( i ) + " : " +
						( m_vAgentTestData[i].second ? "true" : "false" ) + " - " +
						m_vAgentTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vAgentTestData[i].first
													<< m_vAgentTestData[i].second;
	}
}


void UnitTestsSecurity::testRegExp()
{
	QFETCH( QString, sName );
	QFETCH( bool, isDenied );

	QCOMPARE( m_pManager->isDenied( m_lQuery, sName ), isDenied );
}
void UnitTestsSecurity::testRegExp_data()
{
	QTest::addColumn< QString >( "sName" );
	QTest::addColumn< bool >( "isDenied" );

	for ( ushort i = 0; i < m_vRegExpTestData.size(); ++i )
	{
		QString sName = QString::number( i ) + " : " +
						( m_vRegExpTestData[i].second ? "true" : "false" ) + " - " +
						m_vRegExpTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vRegExpTestData[i].first
													<< m_vRegExpTestData[i].second;
	}
}


void UnitTestsSecurity::testPrivateIPs()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	EndPoint oIP = EndPoint( IP );

	QCOMPARE( m_pManager->isPrivate( oIP ), isDenied );
}
void UnitTestsSecurity::testPrivateIPs_data()
{
	populateRowsWithTestsForIPs();
}


#if SECURITY_DISABLE_IS_PRIVATE_OLD
void UnitTestsSecurity::testPrivateIPsOld()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	EndPoint oIP = EndPoint( IP );

	QCOMPARE( m_pManager->isPrivateOld( oIP ), isDenied );
}
void UnitTestsSecurity::testPrivateIPsOld_data()
{
	populateRowsWithTestIPs();
}


void UnitTestsSecurity::testPrivateIPsNew()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	EndPoint oIP = EndPoint( IP );

	QCOMPARE( m_pManager->isPrivateNew( oIP ), isDenied );
}
void UnitTestsSecurity::testPrivateIPsNew_data()
{
	populateRowsWithTestIPs();
}


#if ENABLE_BENCHMARKS
void UnitTestsSecurity::benchmarkPrivateIPsOld()
{
	QFETCH( QString, IP );

	EndPoint oIP = EndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isPrivateOld( oIP );
	}
}
void UnitTestsSecurity::benchmarkPrivateIPsOld_data()
{
	populateRowsWithTestIPs();
}


void UnitTestsSecurity::benchmarkPrivateIPsNew()
{
	QFETCH( QString, IP );

	EndPoint oIP = EndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isPrivateNew( oIP );
	}
}
void UnitTestsSecurity::benchmarkPrivateIPsNew_data()
{
	populateRowsWithTestIPs();
}
#endif // ENABLE_BENCHMARKS
#endif // SECURITY_DISABLE_IS_PRIVATE_OLD

void UnitTestsSecurity::prepareTestData()
{
	const char* pIPs[NO_OF_IPS] =
	{
		"0.255.255.255",
		"0.4.6.1",
		"0.5.6.1",
		"1.1.100.1",
		"8.2.235.69",
		"10.0.0.0",
		"10.0.0.1",
		"10.0.1.0",
		"10.1.0.0",
		"10.23.0.5",
		"10.234.1.253",
		"10.255.254.255",
		"10.254.255.255",
		"10.255.255.255",
		"90.0.0.0",
		"90.90.90.90",
		"100.64.0.0",
		"100.64.0.1",
		"100.127.255.255",
		"127.255.255.255",
		"127.255.255.254",
		"127.2.2.5",
		"130.0.0.0",
		"145.90.94.101",
		"169.254.0.0",
		"169.254.0.1",
		"169.254.0.2",
		"169.254.7.0",
		"169.254.9.1",
		"169.254.34.2",
		"169.254.255.255",
		"169.255.255.255",
		"169.255.0.0",
		"169.255.89.34",
		"171.0.0.0",
		"172.16.0.0",
		"172.17.0.0",
		"172.25.181.53",
		"172.31.255.254",
		"172.31.255.255",
		"191.255.255.255",
		"191.255.255.254",
		"191.255.255.253",
		"172.32.0.0",
		"192.0.0.0",
		"192.0.2.255",
		"192.168.0.0",
		"192.168.255.255",
		"198.18.0.0",
		"198.19.255.255",
		"198.51.100.0",
		"198.51.100.255",
		"203.0.113.0",
		"203.0.113.255",
		"203.0.114.0",
		"203.0.114.255",
		"203.0.156.0",
		"230.0.113.0",
		"231.0.113.255",
		"240.0.0.0",
		"255.255.255.255"
	};

	// isDenied() => true/false
	const bool pIPBools[NO_OF_IPS] =
	{
		true,
		true,
		true,
		false,
		false,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		true,
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		false,
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		false,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		false,
		false,
		true,
		true
	};

	// Prepare vector containing single IP test data (used for private IP and IP testing)
	m_vSingleIPTestData.reserve( NO_OF_IPS );
	for ( ushort i = 0; i < NO_OF_IPS; ++i )
	{
		m_vSingleIPTestData.push_back( std::make_pair( QString( pIPs[i] ), pIPBools[i] ) );

		EndPoint oTest( m_vSingleIPTestData[i].first );
		QVERIFY( !oTest.isNull() );
		QVERIFY( oTest.toString() == m_vSingleIPTestData[i].first );
	}
	QVERIFY( m_vSingleIPTestData.size() == NO_OF_IPS );

	// uses 11.x.x.x - 12.x.x.x
	const char* pTestIPRanges[2 * NO_OF_IP_RANGES] =
	{
		/** tests for overlapping denied ranges **/
		// existing range contains new range start
		"11.0.0.0", "11.0.1.255",    // denied
		"11.0.1.0", "11.1.0.0",      // denied

		// existing range contains new range end
		"11.2.0.0", "11.3.0.0",      // denied
		"11.1.2.0", "11.2.9.0",      // denied

		// existing range contains new range
		"11.3.0.5", "11.6.0.0",      // denied
		"11.4.0.0", "11.5.0.0",      // denied

		// existing range contained within new range
		"11.8.0.0", "11.8.2.0",      // denied
		"11.7.0.0", "11.8.2.0",      // denied

		// new range spans accross a few existing ranges
		"11.8.3.0", "11.8.11.0",     // denied
		"11.8.11.20", "11.8.11.40",  // denied
		"11.8.11.41", "11.8.12.80",  // denied
		"11.8.8.0", "11.8.12.0",     // denied

		/** tests for overlapping mixed ranges **/
		// existing range contains new range start
		"12.0.0.0", "12.0.1.255",    // denied
		"12.0.1.0", "12.1.0.0",      // allowed

		// existing range contains new range end
		"12.2.0.0", "12.3.0.0",      // denied
		"12.1.2.0", "12.2.9.0",      // allowed

		// existing range contains new range
		"12.3.0.5", "12.6.0.0",      // denied
		"12.4.0.0", "12.5.0.0",      // allowed

		// existing range contained within new range
		"12.8.0.0", "12.8.2.0",      // denied
		"12.7.0.0", "12.8.2.0",      // allowed

		// new range spans accross a few existing ranges
		"12.8.3.0", "12.8.11.0",     // denied
		"12.8.11.20", "12.8.11.40",  // denied
		"12.8.11.41", "12.8.12.80",  // denied
		"12.8.8.0", "12.8.12.0"      // allowed
	};

	const bool pTestIPRangeBools[NO_OF_IP_RANGES] =
	{
		/** tests for overlapping denied ranges **/
		// existing range contains new range start
		true,  // denied
		true,  // denied

		// existing range contains new range end
		true,  // denied
		true,  // denied

		// existing range contains new range
		true,  // denied
		true,  // denied

		// existing range contained within new range
		true,  // denied
		true,  // denied

		// new range spans accross a few existing ranges
		true,  // denied
		true,  // denied
		true,  // denied
		true,  // denied

		/** tests for overlapping mixed ranges **/
		// existing range contains new range start
		true,  // denied
		false, // allowed

		// existing range contains new range end
		true,  // denied
		false, // allowed

		// existing range contains new range
		true,  // denied
		false, // allowed

		// existing range contained within new range
		true,  // denied
		false, // allowed

		// new range spans accross a few existing ranges
		true,  // denied
		true,  // denied
		true,  // denied
		false  // allowed
	};

	IPRangeRule oRule;
	QString sContent;

	// Prepare vector containing the IP range rules for IP range testing
	m_vRangeDefinitions.reserve( NO_OF_IP_RANGES );
	for ( ushort i = 0; i < NO_OF_IP_RANGES; ++i )
	{
		sContent = QString( pTestIPRanges[2 * i] ) + "-" + pTestIPRanges[2 * i + 1];
		m_vRangeDefinitions.push_back( std::make_pair( sContent, pTestIPRangeBools[i] ) );

		QVERIFY( oRule.parseContent( sContent ) );
	}
	QVERIFY( m_vRangeDefinitions.size() == NO_OF_IP_RANGES );

	/** test IPs for IP ranges; comments: isDenied() - default: true **/
	const char* pIPsForRangeTests[NO_OF_IP_RANGE_TESTS] =
	{
		"11.0.0.0",
		"11.0.0.102",
		"11.0.1.0",
		"11.0.1.255",
		"11.0.16.230",
		"11.1.0.0",

		"11.1.0.1",        // false
		"11.1.1.15",       // false
		"11.1.1.255",      // false

		"11.1.2.0",
		"11.1.78.9",
		"11.2.0.0",
		"11.2.9.0",
		"11.2.45.3",
		"11.3.0.0",

		"11.3.0.1",        // false
		"11.3.0.2",        // false
		"11.3.0.4",        // false

		"11.3.0.5",
		"11.3.0.6",
		"11.5.255.255",
		"11.6.0.0",
		"11.4.0.0",
		"11.4.100.56",
		"11.5.0.0",

		"11.6.0.1",        // false
		"11.6.23.22",      // false
		"11.6.255.255",    // false

		"11.7.0.0",
		"11.7.0.1",
		"11.7.255.255",
		"11.8.0.0",
		"11.8.1.7",
		"11.8.2.0",

		"11.8.2.1",        // false
		"11.8.2.20",       // false
		"11.8.2.255",      // false

		"11.8.3.0",
		"11.8.5.0",
		"11.8.8.0",
		"11.8.10.0",
		"11.8.11.0",
		"11.8.11.10",
		"11.8.11.20",
		"11.8.11.39",
		"11.8.11.40",
		"11.8.11.41",
		"11.8.11.255",
		"11.8.12.0",
		"11.8.12.1",
		"11.8.12.79",
		"11.8.12.80",

		"11.8.12.81",      // false
		"11.201.12.3",     // false
		"11.255.1.8",      // false
		"11.255.255.255",  // false

		"12.0.0.0",        // true
		"12.0.0.255",      // true
		"12.0.1.0",        // false
		"12.0.98.4",       // false
		"12.1.0.0",        // false

		"12.1.2.0",        // false
		"12.2.2.0",        // false
		"12.2.9.0",        // false
		"12.2.9.1",        // true
		"12.2.73.8",       // true
		"12.3.0.0",        // true

		"12.3.0.1",        // false
		"12.3.0.3",        // false
		"12.3.0.4",        // false

		"12.3.0.5",        // true
		"12.3.25.55",      // true
		"12.3.255.255",    // true
		"12.4.0.0",        // false
		"12.4.1.1",        // false
		"12.5.0.0",        // false
		"12.5.0.1",        // true
		"12.5.46.79",      // true
		"12.6.0.0",        // true
		"12.6.0.1",        // false

		"12.7.0.0",        // false
		"12.7.255.255",    // false
		"12.8.0.0",        // false
		"12.8.1.0",        // false
		"12.8.1.255",      // false
		"12.8.2.0",        // false

		"12.8.2.255",      // false
		"12.8.3.0",        // true
		"12.8.6.5",        // true
		"12.8.7.255",      // true
		"12.8.8.0",        // false
		"12.8.10.0",       // false
		"12.8.12.0",        // false
		"12.8.12.1",        // true
		"12.8.12.23",       // true
		"12.8.12.80",      // true
		"12.8.12.81",      // false
	};

	const bool pResultsForIPRangeTests[NO_OF_IP_RANGE_TESTS] =
	{
		true,
		true,
		true,
		true,
		true,
		true,

		false,
		false,
		false,

		true,
		true,
		true,
		true,
		true,
		true,

		false,
		false,
		false,

		true,
		true,
		true,
		true,
		true,
		true,
		true,

		false,
		false,
		false,

		true,
		true,
		true,
		true,
		true,
		true,

		false,
		false,
		false,

		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,
		true,

		false,
		false,
		false,
		false,

		/** tests for overlapping mixed ranges **/
		// existing range contains new range start
		true,
		true,
		false,
		false,
		false,

		false,
		false,
		false,
		true,
		true,
		true,

		false,
		false,
		false,

		true,
		true,
		true,
		false,
		false,
		false,
		true,
		true,
		true,
		false,

		false,
		false,
		false,
		false,
		false,
		false,

		false,
		true,
		true,
		true,
		false,
		false,
		false,
		true,
		true,
		true,
		false
	};

	// Prepare vector for IP range test results
	m_vIPRangeTestData.reserve( NO_OF_IP_RANGE_TESTS );
	for ( ushort i = 0; i < NO_OF_IP_RANGE_TESTS; ++i )
	{
		m_vIPRangeTestData.push_back( std::make_pair( QString( pIPsForRangeTests[i] ),
													  pResultsForIPRangeTests[i] ) );

		EndPoint oTest( m_vIPRangeTestData[i].first );
		QVERIFY( !oTest.isNull() );
		QVERIFY( oTest.toString() == m_vIPRangeTestData[i].first );
	}
	QVERIFY( m_vIPRangeTestData.size() == NO_OF_IP_RANGE_TESTS );


	uint nCount = 0;
	const quint32 nMin = QHostAddress("0.255.255.255").toIPv4Address();
	const quint32 nMax = QHostAddress("255.255.255.255").toIPv4Address();

	// make sure to get real random numbers
	qsrand( (uint)common::getTNowUTC() );

	// prepare vector for country tests
	m_vCountryTestData.reserve( NO_OF_IP_RANGE_TESTS );
	while ( nCount < NO_OF_COUNTRY_TEST_IPs )
	{
		quint32 nIP = common::getRandomNum( nMin, nMax );
		EndPoint oIP( nIP );
		QVERIFY( !oIP.isNull() );

		QString sCountry = geoIP.findCountryCode( oIP );
		Q_ASSERT( !sCountry.isEmpty() );

		/*qDebug() << QString::number( nCount ) << " - Country: "
				 << sCountry                  << " - "          << oIP.toString();*/

		if ( sCountry != "ZZ" )
		{
			m_vCountryTestData.push_back( std::make_pair( oIP.toString(), sCountry ) );
			++nCount;
		}
	}

	const char* pHashes[NO_OF_TEST_HASHES] =
	{
		"urn:sha1:3KFOLWPNLCJX42TYOWMMK3RACKYHSGRE",
		"urn:sha1:RMFOXPJQY7IKXVN35WDP7HPMYPCRJN3S",
		"urn:sha1:UCKJBM2QHP6D6VU5WQ7X2EYFXWZNHLEE",
		"urn:sha1:DH5J4DTZJHUAEYDN4J2AQ7BAHPDZLBK4",
		"urn:sha1:TYCTDPHH2SKV6LPUVJUQU3RBCUSUL6FJ",
		"urn:sha1:MLJEK2QTOVACRA373KDAOEHQBWMOSF24",
		"urn:sha1:LBEDSU74HA5N2VKIXWR5W2UWZHJY5LP3"
	};

	for ( uint i = 0; i < NO_OF_TEST_HASHES; ++i )
	{
		m_vHashData.push_back( std::make_pair( QString( pHashes[i] ), true ) );
	}


	/** Content rule content - comments document m_bMatchAll **/
	const char* pContents[NO_OF_CONTENT_RULES] =
	{
		"one two three",    // all
		"one four",         // all
		"Quazaa",           // all
		"seven eight nine", // any
		"ten",              // any
		"Shareaza",         // any
		"size:zip:1000",    // all
		"size:gzip:11",     // all
		" size:rar:12 ",    // any
		"size:lol: 42"      // any - Note: this should match hits including one of both key words(!)
	};

	/** Content rule content - contains m_bMatchAll **/
	const bool pMatchAll[NO_OF_CONTENT_RULES] =
	{
		true,
		true,
		true,
		false,
		false,
		false,
		true,
		true,
		false,
		false
	};

	/** Content rule test data (file names) - comments document isDenied() **/
	const char* pTestFileNames[NO_OF_CONTENT_MATCH_TESTS] =
	{
		"one.zip",
		"one lion.doc",
		"four dragons ead one tiny damsel in distress.avi",              // true
		"two three /t four.rar",
		"\ttwo\tthree one.goop",
		"One Quazaa to rule them all.epic",
		"One Four",                                                      // false (case sensitive)
		"eightteen Pastafari pray to the Flying Spaghetti Monster.god",
		"You might wonder about the ten incarnations of 42",
		"Sharing rules.belief",
		"Zippy.zip",
		"Zippy1001.zip",
		"Zappy.gzip",
		"11rar.rar",
		"12rar.rar",
		"13rar.rar",
		"lol.lol"
	};

	/** Content rule test data (file size) **/
	const quint64 pTestFileSize[NO_OF_CONTENT_MATCH_TESTS] =
	{
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1000,
		1001,
		8923,
		11,
		12,
		13,
		42,
	};

	/** Content rule test data (isDenied()) **/
	const bool pTestDenied[NO_OF_CONTENT_MATCH_TESTS] =
	{
		false,
		false,
		true,
		false,
		true,
		true,
		false,
		true,
		true,
		false,
		true,
		false,
		false,
		false,
		true,
		false,
		false
	};

	for ( uint i = 0; i < NO_OF_CONTENT_RULES; ++i )
	{
		m_vContentDefinitions.push_back( std::make_pair( QString( pContents[i] ), pMatchAll[i] ) );
	}

	for ( uint i = 0; i < NO_OF_CONTENT_MATCH_TESTS; ++i )
	{
		FileDataPair oData = std::make_pair( QString( pTestFileNames[i] ), pTestFileSize[i] );
		m_vContentTestData.push_back( std::make_pair( oData, pTestDenied[i] ) );
	}


	/** User agent rule content - comments document isDenied() **/
	const char* pAgentRulesData[NO_OF_AGENT_RULES] =
	{
		"TestAgent01",
		"TestAgent0",
		"TestAgent2",
		"TestAgent3",
		"TestAgent4",
		"TestAgent5"
	};

	/** User agent rule content - contains isDenied() **/
	const bool pAgentRulesDenied[NO_OF_AGENT_RULES] =
	{
		false,
		true,
		true,
		false,
		false,
		false,
	};

	/** User agent rule test data (file names) **/
	const char* pAgentRulesTestData[NO_OF_AGENT_MATCH_TESTS] =
	{
		"TestAgent0",
		"TestAgent01",
		"TestAgent0 next",
		"SmplifiedTestAgent0",
		"TestAgent15 v1.0.0.0",
		"TestAgent20",
		"Easy TestAgent2",
		"TestAgent30b CoolMod v1.0",
		"blub TestAgent4 blub",
		"TestAgent5"
	};

	/** User agent rule test data - contains isDenied() **/
	const bool pAgentRulesTestDenied[NO_OF_AGENT_MATCH_TESTS] =
	{
		true,
		false,
		true,
		true,
		false,
		true,
		true,
		false,
		false,
		false
	};

	for ( uint i = 0; i < NO_OF_AGENT_RULES; ++i )
	{
		m_vAgentDefinitions.push_back( std::make_pair( QString( pAgentRulesData[i] ),
													   pAgentRulesDenied[i] ) );
	}

	for ( uint i = 0; i < NO_OF_AGENT_MATCH_TESTS; ++i )
	{
		m_vAgentTestData.push_back( std::make_pair( QString( pAgentRulesTestData[i] ),
													pAgentRulesTestDenied[i] ) );
	}


	/** Regular expression rule content **/
	const char* pRegExpRulesData[NO_OF_REGEXP_RULES] =
	{
		"(c|C)ontent",
		"^[0-9]a",
		"\\.zip$",
		"^[0-9]+\\.number$",
		"^<1>\\s<2><3>\\s<4>$",
		"^<_>jibberish.talk$",
		"<>testtest<>testtest<>",
		"[a-f][k-n]$"
	};

	// test query is "w0 w1 w2 w3"
	/** Regular expression rule test data (file names) **/
	const char* pRegExpRulesTestData[NO_OF_REGEXP_MATCH_TESTS] =
	{
		"some blocked content",
		"Content that is also blocked",
		"5a blocked",
		" 5a allowed",
		"blocked archive.zip",
		"allowed archive.7z",
		"3945712345.number",
		"123451a.number",
		"w0 w1w2 w3",
		"w1 w2w3 ",
		"w0  w1w2 \tw3 jibberish.talk",
		"w0  w2w1 \tw3 jibberish.talk",
		"w0testtestw1\ttesttestw2 ",
		"1ck",
		"qufm",
		"hgn"
	};

	/** Regular expression rule test data - contains isDenied() **/
	const bool pRegExpRulesTestDenied[NO_OF_REGEXP_MATCH_TESTS] =
	{
		true,
		true,
		true,
		false,
		true,
		false,
		true,
		false,
		false,
		true,
		true,
		false,
		true,
		true,
		true,
		false
	};

	for ( uint i = 0; i < NO_OF_REGEXP_RULES; ++i )
	{
		m_vRegExpDefinitions.push_back( std::make_pair( QString( pRegExpRulesData[i] ), true ) );
	}

	for ( uint i = 0; i < NO_OF_REGEXP_MATCH_TESTS; ++i )
	{
		m_vRegExpTestData.push_back( std::make_pair( QString( pRegExpRulesTestData[i] ),
													 pRegExpRulesTestDenied[i] ) );
	}
}

void UnitTestsSecurity::populateRowsWithTestsForIPs()
{
	QTest::addColumn< QString >( "IP" );
	QTest::addColumn< bool    >( "isDenied" );

	for ( ushort i = 0; i < NO_OF_IPS; ++i )
	{
		QString sName = QString::number( i ) + " - " +
						( m_vSingleIPTestData[i].second ? "true" : "false" ) + " - " +
						m_vSingleIPTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vSingleIPTestData[i].first
													<< m_vSingleIPTestData[i].second;
	}
}

void UnitTestsSecurity::populateRowsWithTestsForIPRanges()
{
	QTest::addColumn< QString >( "IP" );
	QTest::addColumn< bool    >( "isDenied" );

	for ( ushort i = 0; i < NO_OF_IP_RANGE_TESTS; ++i )
	{
		QString sName = QString::number( i ) + " - " +
						( m_vIPRangeTestData[i].second ? "true" : "false" ) + " - " +
						m_vIPRangeTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vIPRangeTestData[i].first
													<< m_vIPRangeTestData[i].second;
	}
}

void UnitTestsSecurity::populateRowsWithTestsForCountries()
{
	QTest::addColumn< QString >( "IP" );

	for ( ushort i = 0; i < NO_OF_COUNTRY_TEST_IPs; ++i )
	{
		QString sName = QString::number( i ) + " - Country: " +
						m_vCountryTestData[i].second + " - " +
						m_vCountryTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vCountryTestData[i].first;
	}
}

QueryHit UnitTestsSecurity::generateQueryHit(quint64 nSize, QString sName, QString sHashes)
{
	QStringList prefixes;
	prefixes << "urn:sha1:"
			 << "urn:ed2k:"
			 << "urn:ed2khash:"
			 << "urn:tree:tiger:"
			 << "urn:btih:"
			 << "urn:bitprint:"
			 << "urn:md5:";

	HashSet vHashes;

	//qDebug() << "Hash String: " << sHashes;
	for ( int i = 0; i < prefixes.size(); ++i )
	{
		QString tmp, sHash;
		int pos1, pos2;

		pos1 = sHashes.indexOf( prefixes.at(i) );
		if ( pos1 != -1 )
		{
			tmp  = sHashes.mid( pos1 );
			int length = Hash::lengthForUrn( prefixes.at(i) ) + prefixes.at(i).length();
			pos2 = tmp.indexOf( "&" );

			//qDebug() << "Expected hash length:" << length;
			//qDebug() << "Actual hash length:" << pos2;

			if ( pos2 == length )
			{
				//qDebug() << "Hash:" << tmp.left( pos2 );
				//qDebug() << "Hash found for hash rule: %1";
				sHash = tmp.left( pos2 );
			}
			else if ( pos2 == -1 && tmp.length() == length )
			{
				//qDebug() << "Hash found for hash rule at end of string: %1";
				sHash = tmp;
			}
			else
			{
				qDebug() << "Error extracting hash: %1";
				continue;
			}

			Hash* pHash = Hash::fromURN( sHash );
			if ( pHash )
			{
				vHashes.insert( pHash );
			}
			else
			{
				qDebug() << "Unit Tests: Hash type not recognised.";
			}
		}
		else
		{
			//qDebug() << "No match found for: " << prefixes.at(i);
		}
	}

	QueryHit oHit = QueryHit();

	if ( !vHashes.empty() )
	{
		oHit.m_vHashes = vHashes;
		oHit.m_nObjectSize = nSize;
		oHit.m_sDescriptiveName = sName;
	}
	else
	{
		qDebug() << "Failed to generate Query Hit.";
	}

	return oHit;
}
