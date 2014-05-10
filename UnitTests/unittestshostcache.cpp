#include "unittestshostcache.h"

#include "g2hostcachehost.h"

UnitTestsHostCache::UnitTestsHostCache() :
	QObject()
{
}

void UnitTestsHostCache::initTestCase()
{
	m_pCache = new HostManagement::G2HostCache();

	m_pCache->start();
	m_pCache->clear();

	prepareTestData();

	for ( uint i = 0; i < NO_OF_HOST_CACHE_IP_TESTS; ++i )
	{
		if ( m_vTestData[i].second )
		{
			EndPoint oIP = EndPoint( m_vTestData[i].first, UNIT_TEST_PORT );

			// Use i as a timestamp to be able to test timestamps easily.
			m_pCache->add( oIP, i );
		}
	}
}
void UnitTestsHostCache::cleanupTestCase()
{
	m_pCache->clear();
}

void UnitTestsHostCache::testCache()
{
	//qDebug() << "starting test";

	QFETCH( QString, IP );
	QFETCH( bool, isCached );
	QFETCH( quint32, timestamp );

	EndPoint oIP = EndPoint( IP, UNIT_TEST_PORT );

	m_pCache->lock();
	// a NULL pointer is returned if the IP cannot be found in the cache
	SharedG2HostPtr pHost = m_pCache->get( oIP );
	m_pCache->unlock();

	QCOMPARE( !(pHost.isNull()), isCached );

	if ( pHost )
	{
		QCOMPARE( pHost->address(), oIP );
		QCOMPARE( pHost->timestamp(), timestamp);
	}
}
void UnitTestsHostCache::testCache_data()
{
	populateRowsWithTestIPs();

	// give the ascnchronous Host Cache some time to finish working
	qDebug() << "started waiting";
	QTest::qSleep( 1000 );
	qDebug() << "finished waiting";
}

void UnitTestsHostCache::prepareTestData()
{
	/** test IPs for IP ranges; comments: isCached() - default: true **/
	const char* pIPs[NO_OF_HOST_CACHE_IP_TESTS] =
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

	const bool pResults[NO_OF_HOST_CACHE_IP_TESTS] =
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
	m_vTestData.reserve( NO_OF_HOST_CACHE_IP_TESTS );
	for ( ushort i = 0; i < NO_OF_HOST_CACHE_IP_TESTS; ++i )
	{
		m_vTestData.push_back( std::make_pair( QString( pIPs[i] ), pResults[i] ) );

		EndPoint oTest( m_vTestData[i].first );
		QVERIFY( !oTest.isNull() );
		QVERIFY( oTest.toString() == m_vTestData[i].first );
	}
	QVERIFY( m_vTestData.size() == NO_OF_HOST_CACHE_IP_TESTS );
}
void UnitTestsHostCache::populateRowsWithTestIPs()
{
	QTest::addColumn< QString >( "IP" );
	QTest::addColumn< bool    >( "isCached" );
	QTest::addColumn< quint32 >( "timestamp" );

	for ( quint32 i = 0; i < NO_OF_HOST_CACHE_IP_TESTS; ++i )
	{
		QString sName = QString::number( i ) + " - " +
						( m_vTestData[i].second ? "true" : "false" ) + " - " + m_vTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vTestData[i].first
													<< m_vTestData[i].second
													<< i;
	}
}
