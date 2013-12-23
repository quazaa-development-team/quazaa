#ifndef UNITTESTSMISSCACHE_H
#define UNITTESTSMISSCACHE_H

#include <QString>
#include <QtTest>

#include "misscache.h"

using namespace Security;

class UnitTestsMissCache : public QObject
{
	Q_OBJECT

	typedef std::pair<QString, bool> DataPair;
	typedef std::vector< DataPair > DataVector;

public:
	UnitTestsMissCache();

signals:

private Q_SLOTS:

};

#endif // UNITTESTSMISSCACHE_H
