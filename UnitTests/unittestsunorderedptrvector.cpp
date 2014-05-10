/*
** unittestsunorderedptrvector.cpp
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

#include "unittestsunorderedptrvector.h"

#include "g2hostcachehost.h"

Q_DECLARE_METATYPE( QString* )

UnitTestsUnorderedPtrVector::UnitTestsUnorderedPtrVector() :
	QObject()
{
}

void UnitTestsUnorderedPtrVector::initTestCase()
{
	m_lUnordered = UnorderedPtrVector<QString>( 1 );

	prepareTestData();
}
void UnitTestsUnorderedPtrVector::cleanupTestCase()
{
	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		delete (*it).first;
	}

	m_vTestData.clear();
}

void UnitTestsUnorderedPtrVector::test0Capacity()
{
	static quint32 nCapacity = 1;

	QFETCH( quint32, capacity );

	if ( capacity > nCapacity )
	{
		nCapacity = capacity;
	}

	m_lUnordered.reserve( capacity );
	QCOMPARE( m_lUnordered.capacity(), nCapacity );
}
void UnitTestsUnorderedPtrVector::test0Capacity_data()
{
	QTest::addColumn< quint32 >( "capacity" );

	const quint32 pCapacities[15] =
	{
		5,
		10,
		8,
		5,
		12,
		13,
		22,
		20,
		21,
		1,
		25,
		26,
		67,
		80,
		100
	};

	for ( quint32 i = 0; i < 15; ++i )
	{
		QString sName = QString::number( i ) + " - " + "Capacity: " +
						QString::number( pCapacities[i] );

		QTest::newRow( sName.toLocal8Bit().data() ) << pCapacities[i];
	}
}

void UnitTestsUnorderedPtrVector::test1AddAll()
{
	QFETCH( QString*, string );
	QFETCH( bool, parameter );

	Q_UNUSED( parameter );

	bool bFound = false;
	for( quint32 i = 0; i < m_lUnordered.count(); ++i )
	{
		if ( m_lUnordered[i] == string )
			bFound = true;
	}

	QVERIFY2( bFound, (QString( "Could not find: " ) + *string).toLocal8Bit().data() );
}

void UnitTestsUnorderedPtrVector::test1AddAll_data()
{
	populateRowsWithTestIPs();

	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		m_lUnordered.push( (*it).first );
	}
}

void UnitTestsUnorderedPtrVector::test2RemoveTrue()
{
	m_lVerificationSet.clear();

	UnorderedPtrVector<QString>::iterator it = m_lUnordered.begin();
	while ( it != m_lUnordered.end() )
	{
		bool bRemove = false;
		bool bFound = false;
		for ( DataVector::const_iterator it2 = m_vTestData.begin(); it2 != m_vTestData.end(); ++it2 )
		{
			const DataPair& pair = *it2;
			if ( pair.first == *it )
			{
				bRemove = pair.second;
				bFound = true;
				break;
			}
		}

		QVERIFY( bFound );

		if ( bRemove )
		{
			it = m_lUnordered.erase( it );
		}
		else
		{
			m_lVerificationSet.insert( *it );
			++it;
		}
	}

	QCOMPARE( m_lUnordered.count(), m_lVerificationSet.size() );

	for ( quint32 i = 0; i < m_lUnordered.count(); ++i )
	{
		QVERIFY( m_lVerificationSet.count( m_lUnordered[i] ) == 1 );
	}
}

void UnitTestsUnorderedPtrVector::test3AddTrue()
{
	m_lVerificationSet.clear();
	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		const DataPair& pair = *it;
		m_lVerificationSet.insert( pair.first );

		if ( pair.second )
		{
			m_lUnordered.push( pair.first );
		}
	}

	QCOMPARE( m_lUnordered.count(), m_lVerificationSet.size() );

	for ( quint32 i = 0; i < m_lUnordered.count(); ++i )
	{
		QVERIFY( m_lVerificationSet.count( m_lUnordered[i] ) == 1 );
	}
}

void UnitTestsUnorderedPtrVector::test4RemoveFalseTrueIterator()
{
	m_lVerificationSet.clear();
	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		if ( !(*it).second )
		{
			m_lVerificationSet.insert( (*it).first );
		}
	}

	UnorderedPtrVector<QString>::iterator it = m_lUnordered.begin();
	while ( it != m_lUnordered.end() )
	{
		if ( m_lVerificationSet.count( *it ) == 1 )
		{
			it = m_lUnordered.erase( it );
		}
		else
		{
			++it;
		}
	}

	m_lVerificationSet.clear();
	for ( DataVector::const_iterator itData = m_vTestData.begin();
		  itData != m_vTestData.end(); ++itData )
	{
		if ( (*itData).second )
		{
			m_lVerificationSet.insert( (*itData).first );
		}
	}

	it = m_lUnordered.begin();
	while ( it != m_lUnordered.end() )
	{
		if ( m_lVerificationSet.count( *it ) == 1 )
		{
			it = m_lUnordered.erase( it );
		}
		else
		{
			++it;
		}
	}

	QCOMPARE( m_lUnordered.count(), (quint32)0 );
}

void UnitTestsUnorderedPtrVector::test5RemovePosition()
{
	m_lVerificationSet.clear();
	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		m_lUnordered.push( (*it).first );
		m_lVerificationSet.insert( (*it).first );
	}

	int count = 0;
	for ( quint32 i = m_lUnordered.count() - 1; i > 0; --i )
	{
		if ( i % 2 )
		{
			++count;
			m_lUnordered.erase( i );
		}
	}

	QCOMPARE( m_vTestData.size(), count + m_lUnordered.count() );

	quint32 count2 = 0;
	for( quint32 i = 0; i < m_lUnordered.count(); ++i )
	{
		QVERIFY( m_lVerificationSet.count( m_lUnordered[i] ) == 1 );
		++count2;
	}

	QCOMPARE( m_lUnordered.count(), count2 );

	while ( m_lUnordered.count() )
	{
		m_lUnordered.erase( 0 );
		--count2;
	}

	QCOMPARE( m_lUnordered.count(), (quint32)0 );
	QCOMPARE( count2, (quint32)0 );
}

void UnitTestsUnorderedPtrVector::test6Clear()
{
	m_lUnordered.clear();

	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		m_lUnordered.push( (*it).first );
	}

	m_lUnordered.clear();

	QCOMPARE( m_lUnordered.count(), (quint32)0 );
}

void UnitTestsUnorderedPtrVector::test7Grow()
{
	m_lUnordered = UnorderedPtrVector<QString>( 10 );

	QCOMPARE( m_lUnordered.count(),    (quint32)0 );
	QCOMPARE( m_lUnordered.capacity(), (quint32)10 );

	quint32 nSize = 0;
	for ( DataVector::const_iterator it = m_vTestData.begin(); it != m_vTestData.end(); ++it )
	{
		QVERIFY( m_lUnordered.capacity() >= nSize );

		m_lUnordered.push( (*it).first );
		++nSize;

		QVERIFY( m_lUnordered.capacity() >= nSize );
	}

	QCOMPARE( m_lUnordered.count(),    (quint32)m_vTestData.size() );

}

void UnitTestsUnorderedPtrVector::prepareTestData()
{
	/** test Strings - default: true **/
	const char* pStrings[NO_OF_VECTOR_STRING_TESTS] =
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

	const bool pResults[NO_OF_VECTOR_STRING_TESTS] =
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

	m_vTestData.reserve( NO_OF_VECTOR_STRING_TESTS );
	for ( ushort i = 0; i < NO_OF_VECTOR_STRING_TESTS; ++i )
	{
		m_vTestData.push_back( std::make_pair( new QString( pStrings[i] ), pResults[i] ) );
	}
	QVERIFY( m_vTestData.size() == NO_OF_VECTOR_STRING_TESTS );
}
void UnitTestsUnorderedPtrVector::populateRowsWithTestIPs()
{
	QTest::addColumn< QString* >( "string" );
	QTest::addColumn< bool     >( "parameter" );

	for ( quint32 i = 0; i < NO_OF_VECTOR_STRING_TESTS; ++i )
	{
		QString sName = QString::number( i ) + " - " +
						( m_vTestData[i].second ? "true " : "false" ) + " - " +
						*m_vTestData[i].first;

		QTest::newRow( sName.toLocal8Bit().data() ) << m_vTestData[i].first
													<< m_vTestData[i].second;
	}
}
