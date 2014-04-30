/*
** idprovider.h
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

#ifndef IDPROVIDER_H
#define IDPROVIDER_H

#include <list>

#include <QtGlobal>
#include <QMutex>

template <typename T>
class IDProvider
{
private:
	std::list<T> m_lIDs;
	QMutex       m_oSection;

public:
	IDProvider();

	T aquire();
	void release( T nID );
};

template <typename T>
IDProvider<T>::IDProvider()
{
	// 0 equals an invalid ID
	m_lIDs.push_back( ( T )1 );
}

template <typename T>
T IDProvider<T>::aquire()
{
	m_oSection.lock();

	// get the next ID to return
	T nReturn = m_lIDs.front();
	m_lIDs.pop_front();

	if ( !m_lIDs.size() )
	{
		// The last element in the list will always be
		// bigger than any ID that has ever been assigned.
		m_lIDs.push_back( nReturn + 1 );
	}
#ifdef _DEBUG
	else
	{
		const T nBiggest = m_lIDs.back();

		typename std::list<T>::iterator it    = m_lIDs.begin();
		typename std::list<T>::iterator itEnd = m_lIDs.end();

		while ( *it < nBiggest )
		{
			++it;
		}

		// last element must always be biggest element in list
		Q_ASSERT( it == --itEnd );
	}
#endif

	m_oSection.unlock();

	return nReturn;
}

template <typename T>
void IDProvider<T>::release( T nID )
{
	m_oSection.lock();

	typename std::list<T>::iterator it = m_lIDs.end();
	--it; // is always a valid operation as there is always at least 1 ID in the list

	// insert it so that it can be reused
	it = m_lIDs.insert( it, nID );

#ifdef _DEBUG
	typename std::list<T>::iterator it2 = m_lIDs.end();
	--it2;

	// last element in the list is always bigger than any existing ID
	Q_ASSERT( *it < *it2 );
#endif

	m_oSection.unlock();
}

#endif // IDPROVIDER_H

