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

FilterControlData::FilterControlData() :
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

FilterControl::FilterControl() :
	m_oFilterControlData( FilterControlData() )
{
}

/*bool FilterControl::operator==(const FilterControl& rOther)
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
}*/

void FilterControl::add(SearchTreeItem* pItem)
{
	Q_ASSERT( pItem->m_oFilter.dataInitialized() );

	if ( pItem->type() == SearchTreeItem::Type::SearchHitType )
	{
		SearchHit* pHitItem   = (SearchHit*)pItem;
		HitFilter* pHitFilter = (HitFilter*)&pItem->m_oFilter;

		pHitFilter->initializeFilterState( this );

		if ( pHitFilter->visible() )
		{
			m_lVisibleHits.push_back( pHitItem );
		}
		else
		{
			m_lFilteredHits.push_back( pHitItem );
		}
	}
	else if ( pItem->type() == SearchTreeItem::Type::SearchFileType )
	{
		SearchFile* pFileItem   = (SearchFile*)pItem;
		FileFilter* pFileFilter = (FileFilter*)&pItem->m_oFilter;

		pFileFilter->initializeFilterState( this );

		if ( pFileFilter->visible() )
		{
			m_lVisibleFiles.push_back( pFileItem );
		}
		else
		{
			m_lFilteredFiles.push_back( pFileItem );
		}
	}
	else
	{
		Q_ASSERT( false );
	}
}

void FilterControl::remove(SearchTreeItem* pItem)
{
	// TODO: do something
}

void FilterControl::update(const FilterControlData& rControlData)
{
	m_oFilterControlData = rControlData;
	// TODO: do actual filtering
}

FileFilterData::FileFilterData(const SearchHit* const pHit)
{
	initialize( pHit );
}

void FileFilterData::initialize(const SearchHit* const pHit)
{
	m_nSize = pHit->m_oHitData.pQueryHit->m_nObjectSize;
	m_bExistsInLibrary = false;

	const HitFilter* const pHitFilter  = (HitFilter*)(pHit->getFilter());

	m_bAdult           = pHitFilter->m_oHitFilterData.m_bAdult;
	m_bBogus           = pHitFilter->m_oHitFilterData.m_bBogus;
	m_bBusy            = pHitFilter->m_oHitFilterData.m_bBusy;
	m_bDRM             = pHitFilter->m_oHitFilterData.m_bDRM;
	m_bFirewalled      = pHitFilter->m_oHitFilterData.m_bFirewalled;
	m_bIncomplete      = pHitFilter->m_oHitFilterData.m_bIncomplete;
	m_bNonMatching     = pHitFilter->m_oHitFilterData.m_bNonMatching;
	m_bSuspicious      = pHitFilter->m_oHitFilterData.m_bSuspicious;
	m_bUnstable        = pHitFilter->m_oHitFilterData.m_bUnstable;
}

/**
 * @brief FileFilterData::update updates the filter data after a new hit has been added to the file.
 * @param hitData
 */
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

/**
 * @brief FileFilterData::refresh completely refreshes the filter data fer example after a hit has
 * been removed from the file.
 * @param pThisFile
 */
void FileFilterData::refresh(const SearchFile* const pThisFile)
{
	Q_ASSERT( pThisFile->childCount() > 0 );

	initialize( (SearchHit*)pThisFile->child( 0 ) );

	for ( int i = 1; i < pThisFile->childCount(); ++i )
	{
		update( ((HitFilter*)pThisFile->child( i )->getFilter())->m_oHitFilterData );
	}
}

HitFilterData::HitFilterData(const QueryHit* const pHit) :
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
	m_bVisible( true ),
	m_bInitialized( false )
{
}

bool Filter::visible() const
{
	return m_bVisible;
}

/**
 * @brief Filter::initialized allows to find out whether the filter has been properly initialized.
 * @return true if the filter has been properly initialized as either a HitFilter or a FileFilter;
 * false otherwise.
 */
bool Filter::dataInitialized() const
{
	return m_bInitialized;
}

FileFilter::FileFilter(SearchHit* pHit) :
	m_oFileFilterData( FileFilterData( pHit ) ),
	m_oFileFilterState( FileFilterState() )
{
	m_bInitialized = true;
}

void FileFilter::initializeFilterState(FilterControl* pControl)
{
	// TODO: do something
}

HitFilter::HitFilter(const QueryHit* const pHit) :
	m_oHitFilterData( HitFilterData( pHit ) ),
	m_oHitFilterState( HitFilterState() )
{
	m_bInitialized = true;
}

void HitFilter::initializeFilterState(FilterControl* pControl)
{
	// TODO: do something
}
