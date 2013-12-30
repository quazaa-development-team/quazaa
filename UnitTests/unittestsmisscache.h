#ifndef UNITTESTSMISSCACHE_H
#define UNITTESTSMISSCACHE_H

#include <QString>
#include <QtTest>

#include "misscache.h"

using namespace Security;

#define NO_OF_IP_CHECKS 61

class UnitTestsMissCache : public QObject
{
	Q_OBJECT

	typedef std::pair<QString, bool> DataPair;
	typedef std::vector< DataPair > DataVector;

private:
	MissCache* m_pCache;

	DataVector m_vTestData;

public:
	UnitTestsMissCache();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testCache();
	void testCache_data();

private:
	void prepareTestData();
	void populateRowsWithTestsForIPs();
};

#endif // UNITTESTSMISSCACHE_H
