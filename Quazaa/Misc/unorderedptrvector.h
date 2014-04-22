/*
** unorderedptrarraylist.h
**
** Copyright © Quazaa Development Team, 2014.
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


#ifndef UNORDEREDPTRARRAYLIST_H
#define UNORDEREDPTRARRAYLIST_H

#include <list>
#include <QtGlobal>

#ifdef _DEBUG
#define DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
#endif

template <typename T>
class UnorderedPtrVector
{
public:
	class const_iterator
	{
	private:
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
		UnorderedPtrVector<T>* m_pParent;
		T** m_pFirstValidPos;
		T** m_pLastValidPos;
		bool m_bValid;
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

		T** m_pPosition;

	private:
		const_iterator(const UnorderedPtrVector<T>* const pParent, T** const pPosition);

#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
		bool isValid() const;
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	public:
		virtual ~const_iterator();
		const_iterator(const const_iterator& it);

		const_iterator& operator++();   // pre
		const_iterator operator++(int); // post

		const_iterator& operator--();   // pre
		const_iterator operator--(int); // post

		const_iterator& operator=(const const_iterator& other);

		bool operator==(const const_iterator& other) const;
		bool operator!=(const const_iterator& other) const;

		T*& operator*();

		friend class UnorderedPtrVector<T>;
	};

	class iterator : public const_iterator
	{
	private:
		iterator(UnorderedPtrVector<T>* pParent, T** pPosition);

	public:
		~iterator();
		iterator(const iterator& it);

		iterator& operator=(const iterator& other);

		friend class UnorderedPtrVector<T>;
	};

private:
	T** m_pBuffer;
	T** m_pPastTheEnd;

#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	std::list<const_iterator*> m_lIterators;
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	quint32 m_nSize;
	quint32 m_nAllocatedSize;

public:
	UnorderedPtrVector(quint32 nReserve = 1023);

	T*& operator[](quint32 nPos);

	void reserve(const quint32 nReserve);
	quint32 capacity() const;

	void push(T* item);
	void push_back(T* item);
	void erase(quint32 nPos);

	quint32 count() const;

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

	iterator erase(iterator it);

private:
	void grow();
	void reallocate(quint32 nNewSize) throw(std::bad_alloc);

#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	void setIteratorValidity(T** pRangeStart, T** pRangeEnd);
	void registerIterator(const_iterator* pIterator);
	void unregisterIterator(const_iterator* pIterator);
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
};

template <typename T>
UnorderedPtrVector<T>::const_iterator::const_iterator(const UnorderedPtrVector<T>* const pParent,
													  T** const pPosition) :
	m_bValid( true ),
	m_pPosition( pPosition )
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	UnorderedPtrVector<T>* pModifiableParent = const_cast<UnorderedPtrVector<T>*>( pParent );
	m_pParent        = pModifiableParent;
	m_pFirstValidPos = pModifiableParent->m_pBuffer;
	m_pLastValidPos  = pModifiableParent->m_pPastTheEnd;
	m_pParent->registerIterator( this );
#else
	Q_UNUSED( pParent );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
}

#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
template <typename T>
bool UnorderedPtrVector<T>::const_iterator::isValid() const
{
	return m_pFirstValidPos == m_pParent->m_pBuffer &&
			m_pLastValidPos >= m_pParent->m_pBuffer &&
			m_pLastValidPos <= m_pParent->m_pPastTheEnd &&
			m_bValid;
}
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

template <typename T>
UnorderedPtrVector<T>::const_iterator::~const_iterator()
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	m_pParent->unregisterIterator( this );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
}

template <typename T>
UnorderedPtrVector<T>::const_iterator::const_iterator(
		const typename UnorderedPtrVector<T>::const_iterator& it) :
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	m_pParent( it.m_pParent ),
	m_pFirstValidPos( it.m_pFirstValidPos ),
	m_pLastValidPos( it.m_pLastValidPos ),
	m_bValid( true ),
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	m_pPosition( it.m_pPosition )
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	m_pParent->registerIterator( this );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
}

template <typename T>
typename UnorderedPtrVector<T>::const_iterator& UnorderedPtrVector<T>::const_iterator::operator++()
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	// tests for iterator validity
	Q_ASSERT( m_pFirstValidPos == m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  >= m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  <= m_pParent->m_pPastTheEnd );
	Q_ASSERT( m_pPosition      >= m_pFirstValidPos         );
	Q_ASSERT( m_pPosition      <  m_pLastValidPos          );
	Q_ASSERT( isValid() );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	++m_pPosition;
	return *this;
}

template <typename T>
typename UnorderedPtrVector<T>::const_iterator
	UnorderedPtrVector<T>::const_iterator::operator++(int)
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	// tests for iterator validity
	Q_ASSERT( m_pFirstValidPos == m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  >= m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  <= m_pParent->m_pPastTheEnd );
	Q_ASSERT( m_pPosition      >= m_pFirstValidPos         );
	Q_ASSERT( m_pPosition      <  m_pLastValidPos          );
	Q_ASSERT( isValid() );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	const_iterator copy = const_iterator( *this );
	++m_pPosition;
	return copy;
}

template <typename T>
typename UnorderedPtrVector<T>::const_iterator& UnorderedPtrVector<T>::const_iterator::operator--()
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	// tests for iterator validity
	Q_ASSERT( m_pFirstValidPos == m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  >= m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  <= m_pParent->m_pPastTheEnd );
	Q_ASSERT( m_pPosition      >  m_pFirstValidPos         );
	Q_ASSERT( m_pPosition      <= m_pLastValidPos          );
	Q_ASSERT( isValid() );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	--m_pPosition;
	return *this;
}

template <typename T>
typename UnorderedPtrVector<T>::const_iterator
	UnorderedPtrVector<T>::const_iterator::operator--(int)
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	// tests for iterator validity
	Q_ASSERT( m_pFirstValidPos == m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  >= m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  <= m_pParent->m_pPastTheEnd );
	Q_ASSERT( m_pPosition      >  m_pFirstValidPos         );
	Q_ASSERT( m_pPosition      <= m_pLastValidPos          );
	Q_ASSERT( isValid() );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	const_iterator copy = const_iterator( *this );
	--m_pPosition;
	return copy;
}

template <typename T>
typename UnorderedPtrVector<T>::const_iterator& UnorderedPtrVector<T>::const_iterator::operator=(
		const typename UnorderedPtrVector<T>::const_iterator& other)
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	m_pParent        = other.m_pParent;
	m_pFirstValidPos = other.m_pFirstValidPos;
	m_pLastValidPos  = other.m_pLastValidPos;
	m_bValid         = other.m_bValid;
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	m_pPosition      = other.m_pPosition;

	return *this;
}

template <typename T>
bool UnorderedPtrVector<T>::const_iterator::operator==(
		const typename UnorderedPtrVector<T>::const_iterator& other) const
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	// tests for iterator validity
	Q_ASSERT( m_pFirstValidPos == m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  >= m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  <= m_pParent->m_pPastTheEnd );
	Q_ASSERT( m_pPosition      >= m_pFirstValidPos         );
	Q_ASSERT( m_pPosition      <= m_pLastValidPos          );
	Q_ASSERT( isValid() );

	Q_ASSERT( m_pParent == other.m_pParent );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	return m_pPosition == other.m_pPosition;
}

template <typename T>
bool UnorderedPtrVector<T>::const_iterator::operator!=(
		const typename UnorderedPtrVector<T>::const_iterator& other) const
{
	return !( *this == other );
}

template <typename T>
T*& UnorderedPtrVector<T>::const_iterator::operator*()
{
#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
	// tests for iterator validity
	Q_ASSERT( m_pFirstValidPos == m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  >= m_pParent->m_pBuffer     );
	Q_ASSERT( m_pLastValidPos  <= m_pParent->m_pPastTheEnd );
	Q_ASSERT( m_pPosition      >= m_pFirstValidPos         );
	Q_ASSERT( m_pPosition      <= m_pLastValidPos          );
	Q_ASSERT( m_pPosition      <  m_pParent->m_pPastTheEnd );
	Q_ASSERT( isValid() );
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

	return *m_pPosition;
}

template <typename T>
UnorderedPtrVector<T>::iterator::iterator(UnorderedPtrVector<T>* pParent, T** pPosition) :
	const_iterator( pParent, pPosition )
{
}

template <typename T>
UnorderedPtrVector<T>::iterator::~iterator()
{
}

template <typename T>
UnorderedPtrVector<T>::iterator::iterator(const typename UnorderedPtrVector<T>::iterator& it) :
	const_iterator( it )
{
}

template <typename T>
typename UnorderedPtrVector<T>::iterator& UnorderedPtrVector<T>::iterator::operator=(
		const typename UnorderedPtrVector<T>::iterator& other)
{
	UnorderedPtrVector<T>::const_iterator::operator =( other );
	return *this;
}

template <typename T>
UnorderedPtrVector<T>::UnorderedPtrVector(quint32 nReserve) :
	m_pBuffer( NULL ),
	m_pPastTheEnd( NULL ),
	m_nSize( 0 ),
	m_nAllocatedSize( 0 )
{
	reserve( nReserve );
}

/**
 * @brief UnorderedPtrVector::operator [] Allows for index based item access. Undefined behavior for
 * nPos >= container size.
 * @param nPos The position to access.
 * @return Reference to the item pointer at position nPos.
 */
template <typename T>
T*& UnorderedPtrVector<T>::operator[](quint32 nPos)
{
#ifdef _DEBUG
	Q_ASSERT( nPos < m_nSize );
#endif
	return m_pBuffer[nPos];
}

/**
 * @brief UnorderedPtrVector::reserve increases the capacity to hold at least nReserve items.
 * Note: This does never shrink the container size.
 * @param nReserve The minimum amount of items the container can hold without having to grow.
 */
template <typename T>
void UnorderedPtrVector<T>::reserve(const quint32 nReserve)
{
	// we need one aditionnal space for past the end pointer
	if ( nReserve + 1 > m_nAllocatedSize )
		reallocate( nReserve + 1 );
}

/**
 * @brief UnorderedPtrVector::capacity Allows access to the container capacity.
 * @return The amount of items that can be pushed without the need for increasing the allocation
 * size.
 */
template <typename T>
quint32 UnorderedPtrVector<T>::capacity() const
{
	return m_nAllocatedSize - 1;
}

/**
 * @brief UnorderedPtrVector::push Adds an item to the container. Does not invalidate the current
 * iterator.
 * @param item The item to add.
 */
template <typename T>
void UnorderedPtrVector<T>::push_back(T* item)
{
	push( item );
}

template <typename T>
void UnorderedPtrVector<T>::push(T* item)
{
	m_pBuffer[m_nSize] = item;

	++m_nSize;

	if ( m_nSize == m_nAllocatedSize )
	{
		grow();
	}
	else
	{
		++m_pPastTheEnd;
	}
}

/**
 * @brief UnorderedPtrVector::erase Removes the item at position nPos from the container.
 * @param nPos The item posistion.
 */
template <typename T>
void UnorderedPtrVector<T>::erase(quint32 nPos)
{
#ifdef _DEBUG
	Q_ASSERT( nPos < m_nSize );
#endif

	m_pBuffer[nPos] = m_pBuffer[--m_nSize];

#ifdef _DEBUG
	m_pBuffer[m_nSize] = NULL;
#endif
}

/**
 * @brief UnorderedPtrVector::count
 * @return The container size.
 */
template <typename T>
quint32 UnorderedPtrVector<T>::count() const
{
	return m_nSize;
}

/**
 * @brief UnorderedPtrVector::begin Allows access to to the first item iterator.
 * @return The first container iterator.
 */
template <typename T>
typename UnorderedPtrVector<T>::iterator UnorderedPtrVector<T>::begin()
{
	return UnorderedPtrVector<T>::iterator( this, m_pBuffer );
}

/**
 * @brief UnorderedPtrVector::begin Allows access to to the first item const_iterator.
 * @return The first container iterator.
 */
template <typename T>
typename UnorderedPtrVector<T>::const_iterator UnorderedPtrVector<T>::begin() const
{
	return UnorderedPtrVector<T>::const_iterator( this, m_pBuffer );
}

/**
 * @brief UnorderedPtrVector::end Gives access to the past the end iterator.
 * @return The past the end iterator.
 */
template <typename T>
typename UnorderedPtrVector<T>::iterator UnorderedPtrVector<T>::end()
{
	return UnorderedPtrVector<T>::iterator( this, m_pPastTheEnd );
}

/**
 * @brief UnorderedPtrVector::end Gives access to the past the end const_iterator.
 * @return The past the end iterator.
 */
template <typename T>
typename UnorderedPtrVector<T>::const_iterator UnorderedPtrVector<T>::end() const
{
	return UnorderedPtrVector<T>::const_iterator( this, m_pPastTheEnd );
}

/**
 * @brief UnorderedPtrVector::erase removes the container item at the current iterator posistion.
 * Returns the iterator to the next element.
 */
template <typename T>
typename UnorderedPtrVector<T>::iterator UnorderedPtrVector<T>::erase(iterator it)
{
#ifdef _DEBUG
	Q_ASSERT( it.isValid() );
#endif

	*(it.m_pPosition) = m_pBuffer[--m_nSize];
	--m_pPastTheEnd;

#ifdef _DEBUG
	m_pBuffer[m_nSize] = NULL;
#endif

	return it;
}

/**
 * @brief UnorderedPtrVector::grow Increases the container capacity to be able to fit more items.
 */
template <typename T>
void UnorderedPtrVector<T>::grow()
{
	// round up to multiple of 1024 and add 1024
	quint32 nNewSize = ( m_nAllocatedSize / 1024 ) * 1024
					   + ( m_nAllocatedSize % 1024 ? 1024 : 0 ) + 1024;

	reallocate( nNewSize );
}

/**
 * @brief UnorderedPtrVector::reallocate
 * @param nNewSize
 */
template <typename T>
void UnorderedPtrVector<T>::reallocate(quint32 nNewSize) throw(std::bad_alloc)
{
	T** pBuffer = (T**)realloc( m_pBuffer, nNewSize * sizeof ( T* ) );

	if ( !pBuffer )
	{
		qWarning() << "Out of memory in UnorderedPtrArrayList::reallocate()";
		throw std::bad_alloc();
	}
	else
	{
		m_pBuffer        = pBuffer;
		m_pPastTheEnd    = pBuffer + m_nSize;
		m_nAllocatedSize = nNewSize;
	}
}

#ifdef DEBUG_UNORDERED_PTR_VECTOR_ITERATORS
template <typename T>
void UnorderedPtrVector<T>::setIteratorValidity(T** pRangeStart, T** pRangeEnd)
{
	foreach ( const_iterator<T>* it, m_lIterators )
	{
		if ( it->m_pPosition < pRangeStart || it->m_pPosition > pRangeEnd )
		{
			it->m_bValid         = false;
		}
		it->m_pFirstValidPos = pRangeStart;
		it->m_pLastValidPos  = pRangeEnd;
	}
}

template <typename T>
void UnorderedPtrVector<T>::registerIterator(const_iterator* pIterator)
{
	m_lIterators.push_back( pIterator );
}

template <typename T>
void UnorderedPtrVector<T>::unregisterIterator(const_iterator* pIterator)
{
	std::list<const_iterator*>::iterator it = m_lIterators.begin();
	while ( it != m_lIterators.end() )
	{
		if ( *it == pIterator )
		{
			m_lIterators.erase( it );
			return;
		}
		else
		{
			++it;
		}
	}

	// you may not unregister a non existing iterator...
	Q_ASSERT( false );
}
#endif // DEBUG_UNORDERED_PTR_VECTOR_ITERATORS

#endif // UNORDEREDPTRARRAYLIST_H
