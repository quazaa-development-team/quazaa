/*
** searchfilter.cpp
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

#include "searchfilter.h"
#include "searchtreemodel.h"

using namespace SearchFilter;

FilterControl::FilterControl() :
	m_sMatchString( "" ),
	m_bRegExp( false ),
	m_nMinSize( 0 ),
	m_nMaxSize( 18446744073709551615 ), // max value of 64 bit int
	m_nMinSources( 0 ),
	m_bBusy( true ),
	m_bFirewalled( true ),
	m_bUnstable( true ),
	m_bDRM( true ),
	m_bSuspicious( true ),
	m_bNonMatching( true ),
	m_bExistsInLibrary( true ),
	m_bBogus( true ),
	m_bAdult( true )
{
}

bool FilterControl::operator==(const FilterControl& rOther)
{
	return m_sMatchString     == rOther.m_sMatchString     &&
		   m_bRegExp          == rOther.m_bRegExp          &&
		   m_nMinSize         == rOther.m_nMinSize         &&
		   m_nMaxSize         == rOther.m_nMaxSize         &&
		   m_nMinSources      == rOther.m_nMinSources      &&
		   m_bBusy            == rOther.m_bBusy            &&
		   m_bFirewalled      == rOther.m_bFirewalled      &&
		   m_bUnstable        == rOther.m_bUnstable        &&
		   m_bDRM             == rOther.m_bDRM             &&
		   m_bSuspicious      == rOther.m_bSuspicious      &&
		   m_bNonMatching     == rOther.m_bNonMatching     &&
		   m_bExistsInLibrary == rOther.m_bExistsInLibrary &&
		   m_bBogus           == rOther.m_bBogus           &&
		   m_bAdult           == rOther.m_bAdult;
}

bool FilterControl::operator!=(const FilterControl& rOther)
{
	return !operator==( rOther );
}

// smaller amount of files
bool FilterControl::operator<(const FilterControl& rOther)
{
	return m_nMinSize         >  rOther.m_nMinSize         ||
		   m_nMaxSize         <  rOther.m_nMaxSize         ||
		   m_nMinSources      >  rOther.m_nMinSources      ||
		  !m_bBusy            && rOther.m_bBusy            ||
		  !m_bFirewalled      && rOther.m_bFirewalled      ||
		  !m_bUnstable        && rOther.m_bUnstable        ||
		  !m_bDRM             && rOther.m_bDRM             ||
		  !m_bSuspicious      && rOther.m_bSuspicious      ||
		  !m_bNonMatching     && rOther.m_bNonMatching     ||
		  !m_bExistsInLibrary && rOther.m_bExistsInLibrary ||
		  !m_bBogus           && rOther.m_bBogus           ||
		  !m_bAdult           && rOther.m_bAdult;
}

// bigger amount of files
bool FilterControl::operator>(const FilterControl& rOther)
{
	return m_nMinSize         <   rOther.m_nMinSize         ||
		   m_nMaxSize         >   rOther.m_nMaxSize         ||
		   m_nMinSources      <   rOther.m_nMinSources      ||
		   m_bBusy            && !rOther.m_bBusy            ||
		   m_bFirewalled      && !rOther.m_bFirewalled      ||
		   m_bUnstable        && !rOther.m_bUnstable        ||
		   m_bDRM             && !rOther.m_bDRM             ||
		   m_bSuspicious      && !rOther.m_bSuspicious      ||
		   m_bNonMatching     && !rOther.m_bNonMatching     ||
		   m_bExistsInLibrary && !rOther.m_bExistsInLibrary ||
		   m_bBogus           && !rOther.m_bBogus           ||
		   m_bAdult           && !rOther.m_bAdult;
}

FileFilterData::FileFilterData(SearchHit* pHit) :
	m_nSize( pHit->m_oHitData.pQueryHit->m_nObjectSize ),
	m_bExistsInLibrary( false )
{
	/*HitFilter* filter  = (HitFilter*)(&pHit->m_oFilter);

	m_bAdult           = filter->m_oHitFilterData.m_bAdult;
	m_bBogus           = filter->m_oHitFilterData.m_bBogus;
	m_bBusy            = filter->m_oHitFilterData.m_bBusy;
	m_bDRM             = filter->m_oHitFilterData.m_bDRM;
	m_bFirewalled      = filter->m_oHitFilterData.m_bFirewalled;
	m_bIncomplete      = filter->m_oHitFilterData.m_bIncomplete;
	m_bNonMatching     = filter->m_oHitFilterData.m_bNonMatching;
	m_bSuspicious      = filter->m_oHitFilterData.m_bSuspicious;
	m_bUnstable        = filter->m_oHitFilterData.m_bUnstable;*/
}

void FileFilterData::update(const HitFilterData& hitData)
{
	m_bAdult          |= hitData.m_bAdult;
	m_bBogus          |= hitData.m_bBogus;
	m_bBusy           &= hitData.m_bBusy;
	m_bDRM            |= hitData.m_bDRM;
	m_bFirewalled     &= hitData.m_bFirewalled;
	m_bIncomplete     &= hitData.m_bIncomplete;
	m_bNonMatching    &= hitData.m_bNonMatching;
	m_bSuspicious     |= hitData.m_bSuspicious;
	m_bUnstable       &= hitData.m_bUnstable;
}

HitFilterData::HitFilterData(QueryHit* pHit) :
	m_bAdult( false ),
	m_bBogus( false ),
	m_bBusy( false ),
	m_bDRM( false ),
	m_bFirewalled( false ),
	m_bIncomplete( pHit->m_bIsPartial ),
	m_bNonMatching( false ),
	m_bSuspicious( false ),
	m_bUnstable( false )

{
	// TODO: fill with useful data
	// TODO: catch new files in library and update filter
}

FileFilterState::FileFilterState() :
	// false: filtered out; true: visible in GUI
	m_bAdult( true ),
	m_bDRM( true ),
	m_bExistsInLibrary( true ),
	m_bIncomplete( true ),
	m_bSize( true ),
	m_bSourceCount( true )
{
}

HitFilterState::HitFilterState() :
	// false: filtered out; true: visible in GUI
	m_bBogus( true ),
	m_bBusy( true ),
	m_bFileName( true ),
	m_bFirewalled( true ),
	m_bNonMatching( true ),
	m_bSuspicious( true ),
	m_bUnstable( true )
{
}

Filter::Filter() :
	m_bVisible( true )
{
}

bool Filter::visible() const
{
	return m_bVisible;
}

FileFilter::FileFilter(FilterControl* pFilter, SearchHit* pHit) :
	m_pFilter( pFilter ),
	m_oFileFilterData( FileFilterData( pHit ) ),
	m_oFileFilterState( FileFilterState() )
{
}

HitFilter::HitFilter(FilterControl* pFilter, QueryHit* pHit) :
	m_pFilter( pFilter ),
	m_oHitFilterData( HitFilterData( pHit ) ),
	m_oHitFilterState( HitFilterState() )
{
}

