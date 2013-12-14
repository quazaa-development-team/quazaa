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
//#include "../Quazaa/Security/securitymanager.h"

using namespace Security;

typedef std::pair<QString, bool> DataPair;
typedef std::vector< DataPair > DataVector;

class UnitTestsSecurity : public QObject
{
	Q_OBJECT

private:
	Manager* m_pManager;
	DataVector m_vData;

public:
	UnitTestsSecurity();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testPrivateIPs();
	void testPrivateIPs_data();

	void testPrivateIPsOld();
	void testPrivateIPsOld_data();

	void testPrivateIPsNew();
	void testPrivateIPsNew_data();

	void benchmarkPrivateIPsOld();
	void benchmarkPrivateIPsOld_data();

	void benchmarkPrivateIPsNew();
	void benchmarkPrivateIPsNew_data();

private:
	void prepareTestData();
	void populateRowsWithTestIPs();
};

UnitTestsSecurity::UnitTestsSecurity()
{
}

void UnitTestsSecurity::initTestCase()
{
	prepareTestData();

	m_pManager = new Manager();
	m_pManager->loadPrivates();

	/*for ( ushort i = 0; i < m_vData.size(); ++i )
	{

	}*/
}

void UnitTestsSecurity::cleanupTestCase()
{
}

void UnitTestsSecurity::testPrivateIPs()
{
	QFETCH( QString, IP );
	QFETCH( bool, isPrivate );

	QCOMPARE( m_pManager->isPrivate( CEndPoint( IP ) ), isPrivate );
}
void UnitTestsSecurity::testPrivateIPs_data()
{
	populateRowsWithTestIPs();
}


void UnitTestsSecurity::testPrivateIPsOld()
{
	QFETCH( QString, IP );
	QFETCH( bool, isPrivate );

	CEndPoint oIP = CEndPoint( IP );

	QCOMPARE( m_pManager->isPrivateOld( oIP ), isPrivate );
}
void UnitTestsSecurity::testPrivateIPsOld_data()
{
	populateRowsWithTestIPs();
}


void UnitTestsSecurity::testPrivateIPsNew()
{
	QFETCH( QString, IP );
	QFETCH( bool, isPrivate );

	CEndPoint oIP = CEndPoint( IP );

	QCOMPARE( m_pManager->isPrivateNew( oIP ), isPrivate );
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


void UnitTestsSecurity::prepareTestData()
{
	m_vData.reserve( 61 );

	const char* pIPs[] =
	{
		"0.255.255.255",
		"0.4.6.1",
		"0.5.6.1",
		"1.1.1.1",
		"2.2.2.2",
		"10.0.0.0",
		"10.0.0.1",
		"10.0.1.0",
		"10.1.0.0",
		"10.1.0.0",
		"10.1.0.0",
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
		"127.2.2.5",
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

	const bool pBools[] =
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

	for ( ushort i = 0; i < 61; ++i )
	{
		m_vData.push_back( std::make_pair( QString( pIPs[i] ), pBools[i] ) );
	}
}

void UnitTestsSecurity::populateRowsWithTestIPs()
{
	QTest::addColumn< QString >( "IP" );
	QTest::addColumn< bool    >( "isPrivate" );

	for ( ushort i = 0; i < 61; ++i )
	{
		QTest::newRow( QString::number( i ).append( m_vData[i].first ).toLocal8Bit().data() )
				<< m_vData[i].first << m_vData[i].second;
	}
}

QTEST_APPLESS_MAIN(UnitTestsSecurity)

#include "tst_unittestssecurity.moc"
