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

#include <QString>
#include <QtTest>

#include "securitymanager.h"

using namespace Security;

#define NO_OF_IPS 61
#define NO_OF_IP_BANS 42

#define NO_OF_IP_RANGES 24
#define NO_OF_IP_RANGE_RULES 20 // differs from previous value because of merged rules
#define NO_OF_IP_RANGE_TESTS 97

// TODO: tests for
//       * Miss Cache
//       * Countries
//       * Hashes
//       * User Agents
//       * Regular Expressions
//       * Other content rules

typedef std::pair<QString, bool> DataPair;
typedef std::vector< DataPair > DataVector;

class UnitTestsSecurity : public QObject
{
	Q_OBJECT

private:
	Manager* m_pManager;

	DataVector m_vSingleIPTestData; // test IPs for private and single IP testing
	DataVector m_vIPRangeTestData; // test IPs for range testing

	DataVector m_vRangeDefinitions;

	QString m_sCurrent;
	qreal   m_nCurrentValue;
	int     m_nCurrentIterations;

	qreal   m_nTotalValue;
	int     m_nTotalIterations;

public:
	UnitTestsSecurity();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testDeniedIPs();
	void testDeniedIPs_data();

	void benchmarkDeniedIPs();
	void benchmarkDeniedIPs_data();

	void testIPRanges();
	void testIPRanges_data();

	void benchmarkIPRanges();
	void benchmarkIPRanges_data();

	void testPrivateIPs();
	void testPrivateIPs_data();

#if SECURITY_DISABLE_IS_PRIVATE_OLD
	void testPrivateIPsOld();
	void testPrivateIPsOld_data();

	void testPrivateIPsNew();
	void testPrivateIPsNew_data();

	void benchmarkPrivateIPsOld();
	void benchmarkPrivateIPsOld_data();

	void benchmarkPrivateIPsNew();
	void benchmarkPrivateIPsNew_data();
#endif // SECURITY_DISABLE_IS_PRIVATE_OLD

private:
	void prepareTestData();

	void populateRowsWithTestsForIPs();
	void populateRowsWithTestsForIPRanges();
};

UnitTestsSecurity::UnitTestsSecurity() :
	m_nCurrentValue( 0 ),
	m_nCurrentIterations( 0 ),
	m_nTotalValue( 0 ),
	m_nTotalIterations( 0 )
{
}

void UnitTestsSecurity::initTestCase()
{
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
		CEndPoint oIP = CEndPoint( m_vSingleIPTestData[i].first );

		qDebug() << ( QString::number( i ) + " : " + oIP.toString() ).toLocal8Bit().data();
		if ( m_vSingleIPTestData[i].second )
		{
			m_pManager->ban( oIP, RuleTime::Forever, true,
							 QString( "Test " ) + QString::number( i )
							 + " - " + m_vSingleIPTestData[i].first, false );
		}
	}
	nCount += NO_OF_IP_BANS;
	QVERIFY2( m_pManager->count() == nCount, "Wrong number of IPs loaded!" );

	// Load IP range rules
	for ( ushort i = 0; i < m_vRangeDefinitions.size(); ++i )
	{
		QString sContent = m_vRangeDefinitions[i].first;
		bool    bDeny    = m_vRangeDefinitions[i].second;

		qDebug() << ( QString::number( i ) + " : " + sContent ).toLocal8Bit().data();

		IPRangeRule* pRule = new IPRangeRule();
		Q_ASSERT( pRule->parseContent( sContent ) );

		pRule->m_nAction = bDeny ? RuleAction::Deny : RuleAction::Accept;
		pRule->m_sComment = QString( "Test " ) + QString::number( i ) + " : " + sContent;

		m_pManager->add( pRule );
	}
	nCount += NO_OF_IP_RANGE_RULES;
	qDebug() << QString::number( nCount ).toLocal8Bit().data() << " IPs should have been loaded. "
			 << QString::number( m_pManager->count() ).toLocal8Bit().data() << "actually were.";
	QVERIFY2( m_pManager->count() == nCount, "Wrong number of IP ranges loaded!" );
}

void UnitTestsSecurity::cleanupTestCase()
{
	m_pManager->clear();
}


void UnitTestsSecurity::testDeniedIPs()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	CEndPoint oIP = CEndPoint( IP );

	QCOMPARE( m_pManager->isDenied( oIP ), isDenied );
}
void UnitTestsSecurity::testDeniedIPs_data()
{
	// Make sure the IPs are not filtered out because they are private IPs
	// but because there are actually rules for them.
	m_pManager->m_bIgnorePrivateIPs = false;

	populateRowsWithTestsForIPs();
}


void UnitTestsSecurity::benchmarkDeniedIPs()
{
	QFETCH( QString, IP );

	CEndPoint oIP = CEndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isDenied( oIP );
	}
}
void UnitTestsSecurity::benchmarkDeniedIPs_data()
{
	populateRowsWithTestsForIPs();
}


void UnitTestsSecurity::testIPRanges()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	CEndPoint oIP = CEndPoint( IP );

	QCOMPARE( m_pManager->isDenied( oIP ), isDenied );
}
void UnitTestsSecurity::testIPRanges_data()
{
	populateRowsWithTestsForIPRanges();
}


void UnitTestsSecurity::benchmarkIPRanges()
{
	QFETCH( QString, IP );

	CEndPoint oIP = CEndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isDenied( oIP );
	}
}
void UnitTestsSecurity::benchmarkIPRanges_data()
{
	populateRowsWithTestsForIPRanges();
}


void UnitTestsSecurity::testPrivateIPs()
{
	QFETCH( QString, IP );
	QFETCH( bool, isDenied );

	CEndPoint oIP = CEndPoint( IP );

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

	CEndPoint oIP = CEndPoint( IP );

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

	CEndPoint oIP = CEndPoint( IP );

	QCOMPARE( m_pManager->isPrivateNew( oIP ), isDenied );
}
void UnitTestsSecurity::testPrivateIPsNew_data()
{
	populateRowsWithTestIPs();
}


void UnitTestsSecurity::benchmarkPrivateIPsOld()
{
	QFETCH( QString, IP );

	CEndPoint oIP = CEndPoint( IP );

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

	CEndPoint oIP = CEndPoint( IP );

	QBENCHMARK
	{
		m_pManager->isPrivateNew( oIP );
	}
}
void UnitTestsSecurity::benchmarkPrivateIPsNew_data()
{
	populateRowsWithTestIPs();
}
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

		CEndPoint oTest( m_vSingleIPTestData[i].first );
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
	for ( ushort i = 0; i < NO_OF_IP_RANGE_TESTS; ++i )
	{
		m_vIPRangeTestData.push_back( std::make_pair( QString( pIPsForRangeTests[i] ),
													  pResultsForIPRangeTests[i] ) );

		CEndPoint oTest( m_vIPRangeTestData[i].first );
		QVERIFY( !oTest.isNull() );
		QVERIFY( oTest.toString() == m_vIPRangeTestData[i].first );
	}
	QVERIFY( m_vIPRangeTestData.size() == NO_OF_IP_RANGE_TESTS );
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

QTEST_APPLESS_MAIN(UnitTestsSecurity)

#include "tst_unittestssecurity.moc"
