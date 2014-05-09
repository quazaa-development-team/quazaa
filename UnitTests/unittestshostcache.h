#ifndef UNITTESTSHOSTCACHE_H
#define UNITTESTSHOSTCACHE_H

#include <QString>
#include <QtTest>

#include "g2hostcache.h"

#define UNIT_TEST_PORT 1
#define NO_OF_HOST_CACHE_IP_TESTS 97

class UnitTestsHostCache : public QObject
{
	Q_OBJECT

	typedef std::pair<QString, bool> DataPair;
	typedef std::vector< DataPair > DataVector;

private:
	HostManagement::G2HostCache* m_pCache;
	DataVector m_vTestData;

public:
	UnitTestsHostCache();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testCache();
	void testCache_data();

private:
	void prepareTestData();
	void populateRowsWithTestIPs();
};

#endif // UNITTESTSHOSTCACHE_H
