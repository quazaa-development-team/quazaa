/*
** $Id$
**
** Copyright © Quazaa Development Team, 2009-2013.
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

#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#include <QDataStream>

// FileFragments code borrowed from Shareaza
// http://shareaza.sourceforge.net/

namespace Fragments
{
inline void SerializeOut(QDataStream& s, const Ranges::Range<quint64>& rhs)
{
	s << rhs.begin() << rhs.size();
}
inline Ranges::Range<quint64> SerializeIn(QDataStream& s)
{
	quint64 begin, length;
	s >> begin >> length;
	if(begin + length < begin)
	{
		s.setStatus(QDataStream::ReadCorruptData);
	}
	else
	{
		return Ranges::Range<quint64>(begin, begin + length);
	}

	throw std::exception();
}

inline void SerializeOut(QDataStream& s, const Ranges::List< Ranges::Range<quint64>, ListTraits >& rhs)
{
	quint64 nTotal = rhs.limit();
	quint64 nRemaining = rhs.length_sum();
	quint64 nFragments = rhs.size();

	s << nTotal << nRemaining << nFragments;

	for( Ranges::List< Ranges::Range<quint64>, ListTraits >::const_iterator i = rhs.begin(); i != rhs.end(); ++i )
	{
		SerializeOut(s, *i);
	}
}
inline void SerializeIn(QDataStream& s, Ranges::List< Ranges::Range<quint64>, ListTraits >& rhs)
{
	quint64 nTotal, nRemaining;
	quint64 nFragments;

	s >> nTotal >> nRemaining >> nFragments;

	{
		Ranges::List< Ranges::Range<quint64>, ListTraits > oNewRange(nTotal);
		rhs.swap(oNewRange);
	}

	for( ; nFragments--; )
	{
		const Ranges::Range<quint64>& fragment = SerializeIn(s);

		if( fragment.end() > nTotal )
		{
			s.setStatus(QDataStream::ReadCorruptData);
			break;
		}
		else
		{
			rhs.insert(rhs.end(), fragment);
		}
	}

	if( s.status() == QDataStream::Ok && rhs.length_sum() != nRemaining )
	{
		s.setStatus(QDataStream::ReadCorruptData);
	}
}

} // namespace Fragments

#endif // COMPATIBILITY_H
