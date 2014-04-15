/*
** searchfilter.h
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

#ifndef SEARCHFILTER_H
#define SEARCHFILTER_H

#include <QtGlobal>

#include "NetworkCore/queryhit.h"

class SearchHit;
class SearchFile;

#include "searchtreemodel.h"

namespace SearchFilter
{
class FilterControl
{
public:
	QString m_sMatchString;
	bool m_bRegExp;

	quint64 m_nMinSize;
	quint64 m_nMaxSize;
	quint16 m_nMinSources;

	// bools: state allowed
	bool m_bBusy;
	bool m_bFirewalled;
	bool m_bUnstable;
	bool m_bDRM;
	bool m_bSuspicious;
	bool m_bNonMatching;
	bool m_bExistsInLibrary;
	bool m_bBogus;
	bool m_bAdult;

public:
	FilterControl();
	bool operator==(const FilterControl& rOther);
	bool operator!=(const FilterControl& rOther);
	bool operator<(const FilterControl& rOther);
	bool operator>(const FilterControl& rOther);
};

struct HitFilterData;
struct FileFilterData
{
	quint64 m_nSize         : 64;

	bool m_bExistsInLibrary : 1; // File exists in library

	bool m_bAdult           : 1; // There is an adult flagged hit
	bool m_bBogus           : 1; // File might be bogus (at least one bogus hit)
	bool m_bBusy            : 1; // All sources are busy
	bool m_bDRM             : 1; // One hit reports the file as DRM
	bool m_bFirewalled      : 1; // All sources are firewalled
	bool m_bIncomplete      : 1; // All sources are incomplete
	bool m_bNonMatching     : 1; // No hit matches search query
	bool m_bSuspicious      : 1; // There is a suspicious hit
	bool m_bUnstable        : 1; // All sources are unstable

	FileFilterData(const SearchHit* const pHit);
	void update(const HitFilterData& hitData);
};

struct HitFilterData
{
	bool m_bAdult           : 1; // Hit is flagged by adult filter (Security)
	bool m_bBogus           : 1; // Hit is flagged bogus
	bool m_bBusy            : 1; // Source is busy
	bool m_bDRM             : 1; // Hit reports the file as DRM
	bool m_bFirewalled      : 1; // Source is firewalled
	bool m_bIncomplete      : 1; // All sources are incomplete
	bool m_bNonMatching     : 1; // Hit doesn't matche search query
	bool m_bSuspicious      : 1; // Hit is flagged as being suspicious
	bool m_bUnstable        : 1; // Source is unstable

	HitFilterData(const QueryHit* const pHit);
};

struct FileFilterState
{
	// false: filtered out; true: visible in GUI
	bool m_bAdult           : 1;
	bool m_bDRM             : 1;
	bool m_bExistsInLibrary : 1;
	bool m_bIncomplete      : 1;
	bool m_bSize            : 1;
	bool m_bSourceCount     : 1;

	FileFilterState();
};

struct HitFilterState
{
	// false: filtered out; true: visible in GUI
	bool m_bBogus           : 1;
	bool m_bBusy            : 1;
	bool m_bFileName        : 1;
	bool m_bFirewalled      : 1;
	bool m_bNonMatching     : 1;
	bool m_bSuspicious      : 1;
	bool m_bUnstable        : 1;

	HitFilterState();
};

class Filter
{
protected:
	bool m_bVisible;     // Is the item visible in GUI?

public:
	Filter();
	bool visible() const;
};

class FileFilter : public Filter
{
private:
	FilterControl*   m_pFilter;

public:
	FileFilterData  m_oFileFilterData;
	FileFilterState m_oFileFilterState;

	FileFilter(FilterControl* pFilter, SearchHit* pHit);
};

class HitFilter : public Filter
{
private:
	FilterControl*   m_pFilter;

public:
	HitFilterData   m_oHitFilterData;
	HitFilterState  m_oHitFilterState;

	HitFilter(FilterControl* pFilter, const QueryHit* const pHit);
};

}
#endif // SEARCHFILTER_H