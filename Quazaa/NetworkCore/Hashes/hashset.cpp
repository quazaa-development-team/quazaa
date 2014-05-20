/*
** hashset.cpp
**
** Copyright © Quazaa Development Team, 2009-2014.
** This file is part of QUAZAA (quazaa.sourceforge.net)
**
** Quazaa is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Quazaa is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with Quazaa; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <vector>

#include "hashset.h"

/**
 * @brief Constructor. Creates an empty HashVector.
 */
HashSet::HashSet() :
	m_pHashes( new Hash*[Hash::NO_OF_TYPES] )
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		m_pHashes[i] = NULL;
	}
}

HashSet::HashSet( const HashSet& other ) :
	m_pHashes( new Hash*[Hash::NO_OF_TYPES] )
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( other.m_pHashes[i] )
		{
			m_pHashes[i] = new Hash( *other.m_pHashes[i] );
		}
		else
		{
			m_pHashes[i] = NULL;
		}
	}
}

/**
 * @brief Destructor.
 */
HashSet::~HashSet()
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			delete m_pHashes[i];
		}
	}

	delete[] m_pHashes;
}

/**
 * @brief HashVector::insert Adds a new CHash to the vector. If an equal CHash or a conflicting
 * CHash is present within the vector, no changes are made to the vector. Note: This takes ownership
 * of the hash. In case it is not added, it is deleted.
 * @param pHash The pointer to the hash.
 * @return true if the Hash was added or an equal Hash was already present; false if a conflicting
 * Hash is present in the vector.
 */
bool HashSet::insert( Hash* pHash )
{
	Q_ASSERT( pHash );

	Hash::Type t = pHash->type();

	if ( !m_pHashes[t] )
	{
		m_pHashes[t] = pHash;
		return true;
	}
	else if ( *m_pHashes[t] == *pHash )
	{
		delete pHash;
		return true;
	}
	else
	{
		delete pHash;
		return false;
	}
}

/**
 * @brief HashVector::insert Adds a new CHash to the vector. If an equal CHash or a conflicting
 * CHash is present within the vector, no changes are made to the vector.
 * @param rHash Reference to the CHash to insert.
 * @return true if the Hash was added or an equal Hash was already present; false if a conflicting
 * Hash is present in the vector.
 */
bool HashSet::insert( const Hash& rHash )
{
	Hash::Type t = rHash.type();

	if ( !m_pHashes[t] )
	{
		m_pHashes[t] = new Hash( rHash );
		return true;
	}
	else if ( *m_pHashes[t] == rHash )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief HashVector::insert Allows to insert the hashes managed by an other HashVector into this
 * one. If there is a conflict, neither HashVector is modified.
 * @param other The other HashVector.
 * @return true if the insertion was successful (e.g. no conflicts); false otherwise.
 */
bool HashSet::insert( const HashSet& other )
{
	std::vector<Hash*> vNewHashes;
	vNewHashes.reserve( other.hashCount() );

	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		// if there is a hash at position i
		if ( other.m_pHashes[i] )
		{
			if ( !conflicts( *other.m_pHashes[i] ) )
			{
				// if it doesn't clash with an existing hash
				// and we don't have a hash for that type yet
				if ( !m_pHashes[i] )
				{
					// remember the hash for later insertion
					vNewHashes.push_back( new Hash( *other.m_pHashes[i] ) );
				}
			}
			else
			{
				// conflict detected; clear remembered hashes
				for ( size_t i = 0, nMax = vNewHashes.size(); i < nMax; ++i )
				{
					delete vNewHashes[i];
				}
				return false;
			}
		}
	}

	// no conflicts detected; insert the hashes
	for ( size_t i = 0, nMax = vNewHashes.size(); i < nMax; ++i )
	{
		const quint8 type = vNewHashes[i]->type();
		m_pHashes[type] = vNewHashes[i];
	}

	return true;
}

/**
 * @brief HashVector::remove Allows to remove the specified hash from the vector.
 * @param rHash A hash.
 * @return true if the hash was found and removed; false if it was not present in the vector.
 */
bool HashSet::remove( const Hash& rHash )
{
	const quint8 type = rHash.type();

	if ( m_pHashes[type] && *m_pHashes[type] == rHash )
	{
		delete m_pHashes[type];
		m_pHashes[type] = NULL;
		return true;
	}

	return false;
}

/**
 * @brief HashVector::remove Allows to remove any hash with a matching Algorithm (can be either
 * zero or one) from the vector.
 * @param eAlgo The Algorithm to remove from the vector.
 * @return true if a hash with the specified Algorithm was present and removed; false otherwise.
 */
bool HashSet::remove( Hash::Algorithm eAlgo )
{
	const quint8 type = Hash::algoToType( eAlgo );

	if ( m_pHashes[type] )
	{
		delete m_pHashes[type];
		m_pHashes[type] = NULL;
		return true;
	}

	return false;
}

/**
 * @brief HashVector::get Allows to retrieve the stored CHash pertaining to the specified Algorithm.
 * @param eAlgo The Algorithm.
 * @return A pointer to the stored CHash.
 */
const Hash* const HashSet::get( Hash::Algorithm eAlgo ) const
{
	return m_pHashes[ Hash::algoToType( eAlgo ) ];
}

/**
 * @brief HashVector::getFirst Allows to retrieve the most important hash handled by this vector.
 * @return A pointer to the stored CHash.
 */
const Hash* const HashSet::mostImportant() const
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			return m_pHashes[i];
		}
	}

	Q_ASSERT( false );
	return NULL;
}

/**
 * @brief contains Allows to check whether the vector contains the specified hash.
 * @param rHash Const reference to the hash.
 * @return true if the vector contains the specified hash; false otherwise.
 */
bool HashSet::contains( const Hash& rHash ) const
{
	const quint8 type = rHash.type();

	if ( m_pHashes[type] && *m_pHashes[type] == rHash )
	{
		return true;
	}

	return false;
}

/**
 * @brief HashVector::conflicts Allows to check whether the specified hash conflicts with a hash
 * already present in the vector.
 * @param rHash
 * @return
 */
bool HashSet::conflicts( const Hash& rHash ) const
{
	const quint8 type = rHash.type();

	if ( m_pHashes[type] && *m_pHashes[type] != rHash )
	{
		return true;
	}

	return false;
}

/**
 * @brief HashVector::size Allows to access the vector size.
 * @return Always NO_OF_TYPES.
 */
quint8 HashSet::size() const
{
	return Hash::NO_OF_TYPES;
}

/**
 * @brief HashVector::hashCount Allows to find out the number of hashes stored within the vector.
 * @return The amount of non NULL hashes within the vector.
 */
quint8 HashSet::hashCount() const
{
	quint8 nCount = 0;
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			++nCount;
		}
	}

	return nCount;
}

/**
 * @brief HashVector::empty Allows to find out whether there haven't been any hashes set.
 * @return true if there are no hashes in the vector; false otherwise.
 */
bool HashSet::empty() const
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] )
		{
			return false;
		}
	}

	return true;
}

void HashSet::simplifyByHashPriority( quint8 nNumberOfHashes )
{
	while ( hashCount() > nNumberOfHashes )
	{
		quint8 i = Hash::NO_OF_TYPES;
		while ( i != 0 )
		{
			--i;

			if ( m_pHashes[i] )
			{
				delete m_pHashes[i];
				m_pHashes[i] = NULL;
				break;
			}
		}
	}
}

/**
 * @brief HashVector::operator [] Allows access based on position. Note that NULL may be returned.
 * @param nPos The position.
 * @return A pointer to the CHash at position nPos.
 */
const Hash* const & HashSet::operator[]( quint8 nPos ) const
{
#ifdef _DEBUG
	Q_ASSERT( nPos < Hash::NO_OF_TYPES );
#endif // _DEBUG

	return m_pHashes[nPos];
}

/**
 * @brief HashVector::operator == Allows to compare two HashVectors.
 * @param other The other HashVector.
 * @return true if all hashes present in both vectors are equal; false if a hash stored in one of
 * the vectors is missing or different in the other.
 */
bool HashSet::operator==(const HashSet& other) const
{
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] && other.m_pHashes[i] )
		{
			if ( *m_pHashes[i] != *other.m_pHashes[i] )
			{
				return false;
			}
		}
		else if ( m_pHashes[i] || other.m_pHashes[i] )
		{
			return false;
		}
	}

	return true;
}

bool HashSet::operator!=( const HashSet& other ) const
{
	return !( *this == other );
}

/**
 * @brief HashVector::matches Allows to compare two HashVectors.
 * @param other The other HashVector.
 * @return true if no conflicting hashes were found AND there was at least one pair of matched
 * hashes.
 */
bool HashSet::matches(const HashSet& other) const
{
	quint8 nCount = 0;
	for ( quint8 i = 0; i < Hash::NO_OF_TYPES; ++i )
	{
		if ( m_pHashes[i] && other.m_pHashes[i] )
		{
			if ( *m_pHashes[i] != *other.m_pHashes[i] )
			{
				return false;
			}
			else
			{
				++nCount;
			}
		}
	}

	return nCount;
}

QList<Hash>& operator<<( QList<Hash>& list, const HashSet& vHashes )
{
	for ( quint8 i = 0, nSize = vHashes.size(); i < nSize; ++i )
	{
		if ( vHashes[i] )
		{
			list.append( *vHashes[i] );
		}
	}

	return list;
}

QList<Hash>& operator>>( QList<Hash>& list, HashSet& vector )
{
	for ( int i = 0, nSize = list.size(); i < nSize; ++i )
	{
		vector.insert( list[i] );
	}

	return list;
}

HashSet& operator<<( HashSet& vector, const QList<Hash>& list )
{
	for ( int i = 0, nSize = list.size(); i < nSize; ++i )
	{
		vector.insert( list[i] );
	}

	return vector;
}

HashSet& operator>>( HashSet& vHashes, QList<Hash>& list )
{
	for ( quint8 i = 0, nSize = vHashes.size(); i < nSize; ++i )
	{
		if ( vHashes[i] )
		{
			list.append( *vHashes[i] );
		}
	}

	return vHashes;
}
