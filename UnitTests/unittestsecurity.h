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

#endif // UNITTESTSECURITY_H
