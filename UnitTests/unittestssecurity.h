/*
** unittestssecurity.h
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

#ifndef UNITTESTSECURITY_H
#define UNITTESTSECURITY_H

#include <QString>
#include <QtTest>

#include "securitymanager.h"

using namespace Security;

#define ENABLE_BENCHMARKS 0

#define NO_OF_IPS 61
#define NO_OF_IP_BANS 42

#define NO_OF_IP_RANGES 24
#define NO_OF_IP_RANGE_RULES 20 // differs from previous value because of merged rules
#define NO_OF_IP_RANGE_TESTS 97

#define NO_OF_COUNTRY_TEST_IPs 50

#define NO_OF_TEST_HASHES 7

#define NO_OF_CONTENT_RULES 10
#define NO_OF_CONTENT_MATCH_TESTS 17

#define NO_OF_AGENT_RULES 6
#define NO_OF_AGENT_MATCH_TESTS 10

#define NO_OF_REGEXP_RULES 8
#define NO_OF_REGEXP_MATCH_TESTS 16

class UnitTestsSecurity : public QObject
{
	Q_OBJECT

	typedef std::pair<QString, bool> DataPair;
	typedef std::vector< DataPair > DataVector;

	typedef std::pair<QString, QString> CountryDataPair;
	typedef std::vector< CountryDataPair > CountryDataVector;

	typedef std::pair<QString, quint64> FileDataPair;
	typedef std::pair<FileDataPair, bool> ContentTestPair;
	typedef std::vector< ContentTestPair > ContentDataVector;

private:
	Manager* m_pManager;

	DataVector m_vSingleIPTestData; // test IPs for private and single IP testing
	DataVector m_vIPRangeTestData;  // test IPs for range testing

	DataVector m_vRangeDefinitions;

	CountryDataVector m_vCountryTestData;

	DataVector m_vHashData;

	DataVector m_vContentDefinitions;
	ContentDataVector m_vContentTestData;

	DataVector m_vAgentDefinitions;
	DataVector m_vAgentTestData;

	QList<QString> m_lQuery;
	DataVector m_vRegExpDefinitions;
	DataVector m_vRegExpTestData;

public:
	UnitTestsSecurity();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testDeniedIPs();
	void testDeniedIPs_data();

#if ENABLE_BENCHMARKS
	void benchmarkDeniedIPs();
	void benchmarkDeniedIPs_data();
#endif // ENABLE_BENCHMARKS

	void testIPRanges();
	void testIPRanges_data();

#if ENABLE_BENCHMARKS
	void benchmarkIPRanges();
	void benchmarkIPRanges_data();
#endif // ENABLE_BENCHMARKS

	void testCountries();
	void testCountries_data();

	void testHashes();
	void testHashes_data();

	void testContent();
	void testContent_data();

	void testAgent();
	void testAgent_data();

	void testRegExp();
	void testRegExp_data();

	void testPrivateIPs();
	void testPrivateIPs_data();

#if SECURITY_DISABLE_IS_PRIVATE_OLD
	void testPrivateIPsOld();
	void testPrivateIPsOld_data();

	void testPrivateIPsNew();
	void testPrivateIPsNew_data();

#if ENABLE_BENCHMARKS
	void benchmarkPrivateIPsOld();
	void benchmarkPrivateIPsOld_data();

	void benchmarkPrivateIPsNew();
	void benchmarkPrivateIPsNew_data();
#endif // ENABLE_BENCHMARKS
#endif // SECURITY_DISABLE_IS_PRIVATE_OLD

private:
	void prepareTestData();

	void populateRowsWithTestsForIPs();
	void populateRowsWithTestsForIPRanges();
	void populateRowsWithTestsForCountries();

	QueryHit generateQueryHit(quint64 nSize, QString sName, QString sHashes);
};

#endif // UNITTESTSECURITY_H
