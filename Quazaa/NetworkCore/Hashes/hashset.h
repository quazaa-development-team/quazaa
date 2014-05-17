/*
** hashset.h
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

#ifndef HASHSET_H
#define HASHSET_H

#include "hash.h"

class HashSet
{
private:
	Hash** m_pHashes;

public:
	HashSet();
	HashSet( const HashSet& other );
	~HashSet();

	bool insert( Hash* pHash );
	bool insert( const Hash& rHash );
	bool insert( const HashSet& other );

	bool remove( const Hash& rHash );
	bool remove( Hash::Algorithm eAlgo );

	const Hash* const mostImportant() const;
	const Hash* const get( Hash::Algorithm eAlgo ) const;

	bool contains( const Hash& rHash ) const;
	bool conflicts( const Hash& rHash ) const;

	quint8 size() const;
	quint8 hashCount() const;
	bool empty() const;

	void simplifyByHashPriority( quint8 nNumberOfHashes );

public:
	const Hash* const & operator[]( quint8 pos ) const;
	bool operator==( const HashSet& other ) const;
	bool operator!=( const HashSet& other ) const;
	bool matches( const HashSet& other ) const;

	friend class Hash;
};

QList<Hash>& operator<<(       QList<Hash>& list, const HashSet& vector );
QList<Hash>& operator>>( const QList<Hash>& list,       HashSet& vector );
HashSet&   operator<<(       HashSet& vector, const QList<Hash>& list );
HashSet&   operator>>( const HashSet& vector,       QList<Hash>& list );

#endif // HASHSET_H