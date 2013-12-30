#include "unittestsmisscache.h"
#include "commonfunctions.h"

UnitTestsMissCache::UnitTestsMissCache()
{
}

void UnitTestsMissCache::initTestCase()
{
	prepareTestData();

	m_pCache = new MissCache();
	m_pCache->m_bUseMissCache  = true;
	m_pCache->m_nMaxIPsInCache = 100;

	const quint32 tNow = common::getTNowUTC();

	for ( ushort i = 0; i < m_vTestData.size(); ++i )
	{
		QString sContent = m_vTestData[i].first;
		QHostAddress oIP = QHostAddress( sContent );

		qDebug() << ( QString::number( i ) + " : " + oIP.toString() ).toLocal8Bit().data();
		QVERIFY( !oIP.isNull() );
		if ( m_vTestData[i].second )
		{
			m_pCache->insert( oIP, tNow );
		}
	}
}
void UnitTestsMissCache::cleanupTestCase()
{
	m_pCache->clear();
}


void UnitTestsMissCache::testCache()
{
	QFETCH( QString, IP );
	QFETCH( bool, isCached );

	QHostAddress oIP = QHostAddress( IP );

	QCOMPARE( m_pCache->check( oIP ), isCached );
}
void UnitTestsMissCache::testCache_data()
{
	populateRowsWithTestsForIPs();
}


void UnitTestsMissCache::prepareTestData()
{
	const char* pIPs[NO_OF_IP_CHECKS] =
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
	const bool pIPBools[NO_OF_IP_CHECKS] =
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
	m_vTestData.reserve( NO_OF_IP_CHECKS );
	for ( ushort i = 0; i < NO_OF_IP_CHECKS; ++i )
	{
		m_vTestData.push_back( std::make_pair( QString( pIPs[i] ), pIPBools[i] ) );

		QHostAddress oTest( m_vTestData[i].first );
		QVERIFY( !oTest.isNull() );
		QVERIFY( oTest.toString() == m_vTestData[i].first );
	}
	QVERIFY( m_vTestData.size() == NO_OF_IP_CHECKS );
}

void UnitTestsMissCache::populateRowsWithTestsForIPs()
{
	QTest::addColumn< QString >( "IP" );
	QTest::addColumn< bool >( "isCached" );

	for ( ushort i = 0; i < NO_OF_IP_CHECKS; ++i )
	{
		QString sName = QString::number( i ) + " - " + ( m_vTestData[i].second ? "true" : "false" )
						+ " - " + m_vTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vTestData[i].first
													<< m_vTestData[i].second;
	}
}
