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

class UnitTestsSecurity : public QObject
{
	Q_OBJECT

private:
	Manager* m_pManager;

public:
	UnitTestsSecurity();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void privateIPs();
	void privateIPs_data();

	void privateIPsOld();
	void privateIPsOld_data();

	void privateIPsNew();
	void privateIPsNew_data();

private:
	void populateWithTestIPs();
};

UnitTestsSecurity::UnitTestsSecurity()
{
}

void UnitTestsSecurity::initTestCase()
{
	m_pManager = new Manager();
	m_pManager->loadPrivates();
}

void UnitTestsSecurity::cleanupTestCase()
{
}

void UnitTestsSecurity::privateIPs()
{
	QFETCH( QString, IP );
	QFETCH( bool, isPrivate );

	QCOMPARE( m_pManager->isPrivate( CEndPoint( IP ) ), isPrivate );
}
void UnitTestsSecurity::privateIPs_data()
{
	populateWithTestIPs();
}


void UnitTestsSecurity::privateIPsOld()
{
	QFETCH( QString, IP );
	QFETCH( bool, isPrivate );

	QCOMPARE( m_pManager->isPrivateOld( CEndPoint( IP ) ), isPrivate );
}
void UnitTestsSecurity::privateIPsOld_data()
{
	populateWithTestIPs();
}


void UnitTestsSecurity::privateIPsNew()
{
	QFETCH( QString, IP );
	QFETCH( bool, isPrivate );

	QCOMPARE( m_pManager->isPrivateNew( CEndPoint( IP ) ), isPrivate );
}
void UnitTestsSecurity::privateIPsNew_data()
{
	populateWithTestIPs();
}

void UnitTestsSecurity::populateWithTestIPs()
{
	QTest::addColumn< QString >( "IP" );
	QTest::addColumn< bool    >( "isPrivate" );

	QTest::newRow( "0" ) << "0.255.255.255"   << true;
	QTest::newRow( "1" ) << "0.4.6.1"         << true;
	QTest::newRow( "2" ) << "0.5.6.1"         << true;

	QTest::newRow( "3" ) << "1.1.1.1"         << false;
	QTest::newRow( "4" ) << "2.2.2.2"         << false;

	QTest::newRow( "5" ) << "10.0.0.0"        << true;
	QTest::newRow( "6" ) << "10.0.0.1"        << true;
	QTest::newRow( "7" ) << "10.0.1.0"        << true;
	QTest::newRow( "8" ) << "10.1.0.0"        << true;
	QTest::newRow( "9" ) << "10.255.255.254"  << true;
	QTest::newRow( "10" ) << "10.255.254.255"  << true;
	QTest::newRow( "11" ) << "10.254.255.255"  << true;
	QTest::newRow( "12" ) << "10.255.255.255"  << true;

	QTest::newRow( "13" ) << "90.0.0.0"        << false;
	QTest::newRow( "14" ) << "90.90.90.90"     << false;

	QTest::newRow( "15" ) << "100.64.0.0"      << true;
	QTest::newRow( "16" ) << "100.64.0.1"      << true;
	QTest::newRow( "17" ) << "100.127.255.255" << true;

	QTest::newRow( "18" ) << "127.255.255.255" << true;
	QTest::newRow( "19" ) << "127.255.255.254" << true;
	QTest::newRow( "20" ) << "127.2.2.5"       << true;

	QTest::newRow( "21" ) << "130.0.0.0"       << false;
	QTest::newRow( "22" ) << "145.90.94.101"   << false;

	QTest::newRow( "23" ) << "169.254.0.0"     << true;
	QTest::newRow( "24" ) << "169.254.0.1"     << true;
	QTest::newRow( "25" ) << "169.254.0.2"     << true;
	QTest::newRow( "26" ) << "169.254.7.0"     << true;
	QTest::newRow( "27" ) << "169.254.9.1"     << true;
	QTest::newRow( "28" ) << "169.254.34.2"    << true;
	QTest::newRow( "29" ) << "169.254.255.255" << true;

	QTest::newRow( "30" ) << "169.255.255.255" << false;
	QTest::newRow( "31" ) << "169.255.0.0"     << false;
	QTest::newRow( "32" ) << "169.255.89.34"   << false;
	QTest::newRow( "33" ) << "171.0.0.0"       << false;

	QTest::newRow( "34" ) << "172.16.0.0"      << true;
	QTest::newRow( "35" ) << "172.17.0.0"      << true;
	QTest::newRow( "36" ) << "127.2.2.5"       << true;
	QTest::newRow( "37" ) << "172.31.255.254"  << true;
	QTest::newRow( "38" ) << "172.31.255.255"  << true;

	QTest::newRow( "39" ) << "191.255.255.255" << false;
	QTest::newRow( "40" ) << "191.255.255.254" << false;
	QTest::newRow( "41" ) << "191.255.255.253" << false;
	QTest::newRow( "42" ) << "172.32.0.0"      << false;

	QTest::newRow( "43" ) << "192.0.0.0"       << true;
	QTest::newRow( "44" ) << "192.0.2.255"     << true;

	QTest::newRow( "45" ) << "192.168.0.0"     << true;
	QTest::newRow( "46" ) << "192.168.255.255" << true;

	QTest::newRow( "47" ) << "198.18.0.0"      << true;
	QTest::newRow( "48" ) << "198.19.255.255"  << true;

	QTest::newRow( "49" ) << "198.51.100.0"    << true;
	QTest::newRow( "50" ) << "198.51.100.255"  << true;

	QTest::newRow( "51" ) << "203.0.113.0"     << true;
	QTest::newRow( "52" ) << "203.0.113.255"   << true;

	QTest::newRow( "53" ) << "203.0.114.0"     << false;
	QTest::newRow( "54" ) << "203.0.114.255"   << false;
	QTest::newRow( "55" ) << "203.0.156.0"     << false;
	QTest::newRow( "56" ) << "230.0.113.0"     << false;
	QTest::newRow( "57" ) << "231.0.113.255"   << false;
	QTest::newRow( "58" ) << "238.0.113.20"    << false;

	QTest::newRow( "59" ) << "240.0.0.0"       << true;
	QTest::newRow( "60" ) << "255.255.255.255" << true;
}

QTEST_APPLESS_MAIN(UnitTestsSecurity)

#include "tst_unittestssecurity.moc"
