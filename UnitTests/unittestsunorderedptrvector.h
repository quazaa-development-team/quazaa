/*
** unittestsunorderedptrvector.h
**
** Copyright © Quazaa Development Team, 2014-2014.
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

#ifndef UNITTESTSUNORDEREDPTRVECTOR_H
#define UNITTESTSUNORDEREDPTRVECTOR_H

#include <set>
#include <QString>
#include <QtTest>

#include "Misc/unorderedptrvector.h"

#define NO_OF_VECTOR_STRING_TESTS 97

class UnitTestsUnorderedPtrVector : public QObject
{
	Q_OBJECT

	typedef std::pair<QString*, bool> DataPair;
	typedef std::vector< DataPair > DataVector;

private:
	UnorderedPtrVector<QString> m_lUnordered;

	std::set<QString*> m_lVerificationSet;

	DataVector m_vTestData;

public:
	UnitTestsUnorderedPtrVector();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void test0Capacity();
	void test0Capacity_data();

	void test1AddAll();
	void test1AddAll_data();

	void test2RemoveTrue();

	void test3AddTrue();

	void test4RemoveFalseTrueIterator();

	void test5RemovePosition();

	void test6Clear();

	void test7Grow();

private:
	void prepareTestData();
	void populateRowsWithTestIPs();
};

#endif // UNITTESTSUNORDEREDPTRVECTOR_H
