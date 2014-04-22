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

#include <QRegularExpression>

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

FilterControlData::FilterControlData(const FilterControlData& other) :
	m_sMatchString(     other.m_sMatchString     ),
	m_bRegExp(          other.m_bRegExp          ),
	m_nMinSize(         other.m_nMinSize         ),
	m_nMaxSize(         other.m_nMaxSize         ),
	m_nMinSources(      other.m_nMinSources      ),
	m_bBusy(            other.m_bBusy            ),
	m_bFirewalled(      other.m_bFirewalled      ),
	m_bUnstable(        other.m_bUnstable        ),
	m_bDRM(             other.m_bDRM             ),
	m_bSuspicious(      other.m_bSuspicious      ),
	m_bNonMatching(     other.m_bNonMatching     ),
	m_bExistsInLibrary( other.m_bExistsInLibrary ),
	m_bBogus(           other.m_bBogus           ),
	m_bAdult(           other.m_bAdult           )
{
}

bool FilterControlData::operator==(const FilterControlData& rOther)
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

bool FilterControlData::operator!=(const FilterControlData& rOther)
{
	return !operator==( rOther );
}

FilterControl::FilterControl() :
	m_oFilterControlData( FilterControlData() ),
	m_bStringChanged( false ),
	m_bSizeChanged( false ),
	m_bMinSourcesChanged( false ),
	m_bHitBoolsChanged( false ),
	m_bFileBoolsChanged( false ),
	m_bStringFilterInvisibleHitsInvalidated( false )
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

	if ( pItem->type() == SearchTreeItem::SearchHitType )
	{
		SearchHit* pHitItem   = (SearchHit*)pItem;
		HitFilter* pHitFilter = (HitFilter*)&pItem->m_oFilter;

		pHitFilter->initializeFilterState( *this );

		if ( pHitFilter->visible() )
		{
			m_lVisibleHits.push_back( pHitItem );
			pHitItem->parent()->addVisibleChild();

			// TODO: handle file becoming visible!
		}
		else
		{
			m_lFilteredHits.push_back( pHitItem );
		}
	}
	else if ( pItem->type() == SearchTreeItem::SearchFileType )
	{
		SearchFile* pFileItem   = (SearchFile*)pItem;
		FileFilter* pFileFilter = (FileFilter*)&pItem->m_oFilter;

		pFileFilter->initializeFilterState( *this );

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


	//pHitItem->parent()->removeVisibleChild();
}

#define MOVED_FROM_INVISIBLE_TO_VISIBLE 1
#define MOVED_FROM_VISIBLE_TO_INVISIBLE 2

void FilterControl::update(const FilterControlData& rControlData)
{
	m_bStringChanged      = m_oFilterControlData.m_bRegExp          != rControlData.m_bRegExp  ||
							m_oFilterControlData.m_sMatchString     != rControlData.m_sMatchString;
	m_bSizeChanged        = m_oFilterControlData.m_nMinSize         != rControlData.m_nMinSize ||
							m_oFilterControlData.m_nMaxSize         != rControlData.m_nMaxSize;
	m_bMinSourcesChanged  = m_oFilterControlData.m_nMinSources      != rControlData.m_nMinSources;
	m_bHitBoolsChanged    = m_oFilterControlData.m_bBogus           != rControlData.m_bBogus           ||
							m_oFilterControlData.m_bBusy            != rControlData.m_bBusy            ||
							m_oFilterControlData.m_bFirewalled      != rControlData.m_bFirewalled      ||
							m_oFilterControlData.m_bNonMatching     != rControlData.m_bNonMatching     ||
							m_oFilterControlData.m_bSuspicious      != rControlData.m_bSuspicious      ||
							m_oFilterControlData.m_bUnstable        != rControlData.m_bUnstable;
	m_bFileBoolsChanged =   m_oFilterControlData.m_bDRM             != rControlData.m_bDRM             ||
							m_oFilterControlData.m_bExistsInLibrary != rControlData.m_bExistsInLibrary ||
							m_oFilterControlData.m_bAdult           != rControlData.m_bAdult;

	if ( m_bStringChanged || m_bSizeChanged || m_bMinSourcesChanged ||
		 m_bHitBoolsChanged || m_bFileBoolsChanged )
	{
		char c1 = updateHitFilterStatus( rControlData );
		char c2 = filterFiles( rControlData );

		// TODO: do I need these return values?
	}

	// TODO: continue here

	Q_ASSERT( m_oFilterControlData == rControlData );
}

FilterControlData* FilterControl::getDataCopy() const
{
	return new FilterControlData( m_oFilterControlData );
}

/**
 * @brief FilterControl::updateHitFilterStatus
 * @param rControlData
 * @return (0, 1, 2, 3): hits moved to (no, visible, invisible, both) hit lists
 */
char FilterControl::updateHitFilterStatus(const FilterControlData& rControlData)
{
	if ( m_bStringChanged )
	{
		if ( rControlData.m_bRegExp )
		{
			// complete regexp refresh
			applyRegExpFilter( rControlData.m_sMatchString );

			// all hits have been refreshed, so none are currently invalidated
			m_bStringFilterInvisibleHitsInvalidated = false;
		}
		else
		{
			// switch from RegExp to non RegExp => complete stringfilter refresh required
			if ( m_oFilterControlData.m_bRegExp )
			{
				// build filter word lists
				QStringList lWords, lMustHave, lMustNotHave;
				lWords = rControlData.m_sMatchString.split( QRegularExpression( "\\s+" ) );
				separateFilter( lWords, lMustHave, lMustNotHave );

				// apply filter to both hit lists
				applyStringFilter( m_lVisibleHits, lMustHave, lMustNotHave );
				applyStringFilter( m_lFilteredHits, lMustHave, lMustNotHave );

				// all hit states have been refreshed, so none are currently invalidated
				m_bStringFilterInvisibleHitsInvalidated = false;
			}
			else // stringfilter word change
			{
				QStringList lNewWords, lRemovedWords;
				analyseFilter( rControlData.m_sMatchString, lNewWords, lRemovedWords );

				Q_ASSERT( lNewWords.size() || lRemovedWords.size() ); // because of m_bStringChanged

				// Note: Each time a new word is added, the filter is only applied to the visible
				// hits. This means we have to invalidate the String filter state for all invisible
				// hits as we don't know whether they have been filtered out by the string filter
				// or one of the other applied filters (lazy evaluation).

				// apply filter to visible hits only
				if ( lNewWords.size() )
				{
					// build filter word lists
					QStringList lNewMustHave, lNewMustNotHave;
					separateFilter( lNewWords, lNewMustHave, lNewMustNotHave );

					// apply filter to visible hits
					applyStringFilter( m_lVisibleHits, lNewMustHave, lNewMustNotHave );

					// filter only applied to visible hits => filter state of invisible hits unknown
					m_bStringFilterInvisibleHitsInvalidated = true;
				}

				// apply filter to invisible hits only - visible hits stay valid implicidly
				if ( lRemovedWords.size() )
				{
					// build filter word lists
					// Note: we need to apply the complete filter here (!)
					QStringList lWords, lMustHave, lMustNotHave;
					lWords = rControlData.m_sMatchString.split( QRegularExpression( "\\s+" ) );
					separateFilter( lWords, lMustHave, lMustNotHave );

					// apply filter to invisible hits only - visible hits stay valid implicidly
					applyStringFilter( m_lFilteredHits, lMustHave, lMustNotHave );

					// filter state for invisible hits has been refreshed
					m_bStringFilterInvisibleHitsInvalidated = false;
				}
			}
		}

		m_oFilterControlData.m_bRegExp      = rControlData.m_bRegExp;
		m_oFilterControlData.m_sMatchString = rControlData.m_sMatchString;
	}

	if ( m_bStringFilterInvisibleHitsInvalidated && m_bHitBoolsChanged )
	{
		// refresh string filter for invisible hits

		// build filter word lists
		// Note: we need to apply the complete filter here (!)
		QStringList lWords, lMustHave, lMustNotHave;
		lWords = rControlData.m_sMatchString.split( QRegularExpression( "\\s+" ) );
		separateFilter( lWords, lMustHave, lMustNotHave );

		// apply filter to invisible hits only - visible hits stay valid implicidly
		applyStringFilter( m_lFilteredHits, lMustHave, lMustNotHave );

		// filter state for invisible hits has been refreshed
		m_bStringFilterInvisibleHitsInvalidated = false;
	}

	HitList lNewlyFilteredHits;   // contains hits moved from visible list on filter change

	int nNewlyVisible = 0, nNewlyFiltered = 0;

	// Updating all bools no matter their change state should be faster than using an if to check
	// for the respective change state each time.
	HitList::iterator it = m_lVisibleHits.begin();
	while ( it != m_lVisibleHits.end() )
	{
		if ( ((HitFilter*)&(*it)->m_oFilter)->updateBoolState( rControlData ) )
		{
			++it;
		}
		else
		{
			(*it)->parent()->removeVisibleChild();

			// if hit invisible after filtering, move it to temp list
			lNewlyFilteredHits.push_back( *it );
			it = m_lVisibleHits.erase( it );

			++nNewlyFiltered;
		}
	}

	it = m_lFilteredHits.begin();
	while ( it != m_lFilteredHits.end() )
	{
		if ( ((HitFilter*)&(*it)->m_oFilter)->updateBoolState( rControlData ) )
		{
			(*it)->parent()->addVisibleChild();

			// if hit visible after filtering, move it to visible list
			m_lVisibleHits.push_back( *it );
			it = m_lFilteredHits.erase( it );

			++nNewlyVisible;
		}
		else
		{
			++it;
		}
	}

	foreach ( SearchHit* pHit, lNewlyFilteredHits )
	{
		m_lFilteredHits.push_back( pHit );
	}

	char nReturn = 0;
	if ( nNewlyVisible )
	{
		nReturn |= MOVED_FROM_INVISIBLE_TO_VISIBLE;
	}
	if ( nNewlyFiltered )
	{
		nReturn |= MOVED_FROM_VISIBLE_TO_INVISIBLE;
	}
	return nReturn;
}

/**
 * @brief FilterControl::analyseFilter takes a new filter string and compares it with the current
 * filter string.
 * @param sNewMatchString represents the new match filter (list of words, NOT RegExp)
 * @param lNewWords contains all words added in the new filter (not present in old filter)
 * @param lRemovedWords contains all words removed from the filter (compared to the old filter)
 */
void FilterControl::analyseFilter(const QString& sNewMatchString,
								  QStringList& lNewWords, QStringList& lRemovedWords) const
{
	lRemovedWords = m_oFilterControlData.m_sMatchString.split( QRegularExpression( "\\s+" ) );
	lNewWords     = sNewMatchString.split( QRegularExpression( "\\s+" ) );

	lRemovedWords.removeDuplicates();
	lNewWords.removeDuplicates();

	// Remove all entries shared by both lists. The items remaining in the list created from the
	// current filter are the ones removed in the new filter and the items remaining in the list
	// created from the new filter are the ones added in that new filter.
	QStringList::iterator it = lRemovedWords.begin();
	while ( it != lRemovedWords.end() )
	{
		if ( lNewWords.contains( *it ) )
		{
			lNewWords.removeAt( lNewWords.indexOf( *it ) );
			it = lRemovedWords.erase( it );
		}
		else
		{
			++it;
		}
	}
}

/**
 * @brief FilterControl::separateFilter separates filter words into two lists by whether they are
 * preceeded by a '-' or not. Removes the first preceding '-' of a word in the process.
 * @param lWords Input list containing all words.
 * @param lMustHaveWords Output list for all words without preceeding '-'.
 * @param lMustNotHaveWords Output list for all words preceedded by '-'.
 */
void FilterControl::separateFilter(const QStringList& lWords, QStringList& lMustHaveWords,
								   QStringList& lMustNotHaveWords) const
{
	foreach ( QString s, lWords )
	{
		if ( s.startsWith( "-" ) )
		{
			lMustNotHaveWords.push_back( s.mid( 1 ) );
		}
		else
		{
			lMustHaveWords.push_back( s );
		}
	}
}

/**
 * @brief FilterControl::applyStringFilter refreshes m_oFilter)->m_oHitFilterState.m_bFileName for
 * all hits in lHits.
 * @param lHits
 * @param lMustHaveWords
 * @param lMustNotHaveWords
 */
void FilterControl::applyStringFilter(HitList& lHits, const QStringList& lMustHaveWords,
									  const QStringList& lMustNotHaveWords) const
{
	foreach ( SearchHit* pHit, lHits )
	{
		((HitFilter*)&pHit->m_oFilter)->m_oHitFilterState.m_bFileName =
				matchStringFilter( pHit, lMustHaveWords, lMustNotHaveWords );
	}
}

/**
 * @brief FilterControl::matchStringFilter
 * @param pHit
 * @param lMustHaveWords
 * @param lMustNotHaveWords
 * @return
 */
bool FilterControl::matchStringFilter(SearchHit* pHit, const QStringList& lMustHaveWords,
									  const QStringList& lMustNotHaveWords) const
{
	foreach ( QString sMust, lMustHaveWords )
	{
		if ( !pHit->m_oHitData.pQueryHit->m_sDescriptiveName.contains( sMust ) )
		{
			return false;
		}
	}
	foreach ( QString sMustNot, lMustNotHaveWords )
	{
		if ( pHit->m_oHitData.pQueryHit->m_sDescriptiveName.contains( sMustNot ) )
		{
			return false;
		}
	}

	return true;
}

/**
 * @brief FilterControl::applyRegExpFilter refreshes the string filter state for both hit lists.
 * @param rRegExp String representation of the regular expression to be used for matching.
 */
void FilterControl::applyRegExpFilter(const QString& sRegExp)
{
	QRegularExpression oRegExp( sRegExp );

	foreach ( SearchHit* pHit, m_lVisibleHits )
	{
		// false: filtered out; true: visible in GUI
		((HitFilter*)&pHit->m_oFilter)->m_oHitFilterState.m_bFileName =
				oRegExp.match( pHit->m_oHitData.pQueryHit->m_sDescriptiveName ).hasMatch();
	}
	foreach ( SearchHit* pHit, m_lFilteredHits )
	{
		// false: filtered out; true: visible in GUI
		((HitFilter*)&pHit->m_oFilter)->m_oHitFilterState.m_bFileName =
				oRegExp.match( pHit->m_oHitData.pQueryHit->m_sDescriptiveName ).hasMatch();
	}
}

/**
 * @brief FilterControl::filterFiles filters all file entries according to the new control data.
 * Note: this requires the hit filtering to have been applied already.
 * @param rControlData
 * @return (0, 1, 2, 3): files moved to (no, visible, invisible, both) file lists
 */
char FilterControl::filterFiles(const FilterControlData& rControlData)
{
	// Update all of these state bools only if there has actually been a change.
	if ( m_bSizeChanged || m_bMinSourcesChanged || m_bFileBoolsChanged )
	{
		foreach ( SearchFile* pFile, m_lVisibleFiles )
		{
			FileFilter* pFilter = (FileFilter*)&pFile->m_oFilter;
			pFilter->updateBoolState( rControlData );
			pFilter->m_oFileFilterState.m_bEnoughHits =
					pFile->childCount() >= rControlData.m_nMinSources;
		}
		foreach ( SearchFile* pFile, m_lFilteredFiles )
		{
			FileFilter* pFilter = (FileFilter*)&pFile->m_oFilter;
			pFilter->updateBoolState( rControlData );
			pFilter->m_oFileFilterState.m_bEnoughHits =
					pFile->childCount() >= rControlData.m_nMinSources;
		}

		m_oFilterControlData.m_nMinSize    = rControlData.m_nMinSize;
		m_oFilterControlData.m_nMaxSize    = rControlData.m_nMaxSize;
		m_oFilterControlData.m_nMinSources = rControlData.m_nMinSources;
	}

	FileList lNewlyFilteredFiles; // contains files moved from visible list on filter change
	int nNewlyVisible = 0, nNewlyFiltered = 0;

	FileList::iterator it = m_lVisibleFiles.begin();
	while ( it != m_lVisibleFiles.end() )
	{
		FileFilter* pFilter = (FileFilter*)&(*it)->m_oFilter;

		// m_bVisibleHits always changes because of the applied hit filter, so it must be updated.
		pFilter->m_oFileFilterState.m_bVisibleHits = (*it)->visibleChildCount();
		if ( pFilter->updateVisible() )
		{
			++it;
		}
		else
		{
			// if file invisible after filtering, move it to temp list
			lNewlyFilteredFiles.push_back( *it );
			it = m_lVisibleFiles.erase( it );

			++nNewlyFiltered;
		}
	}

	it = m_lFilteredFiles.begin();
	while ( it != m_lFilteredFiles.end() )
	{
		FileFilter* pFilter = (FileFilter*)&(*it)->m_oFilter;

		// m_bVisibleHits always changes because of the applied hit filter, so it must be updated.
		pFilter->m_oFileFilterState.m_bVisibleHits = (*it)->visibleChildCount();
		if ( pFilter->updateVisible() )
		{
			// if hit visible after filtering, move it to visible list
			m_lVisibleFiles.push_back( *it );
			it = m_lFilteredFiles.erase( it );

			++nNewlyVisible;
		}
		else
		{
			++it;
		}
	}

	foreach ( SearchFile* pFile, lNewlyFilteredFiles )
	{
		m_lFilteredFiles.push_back( pFile );
	}

	char nReturn = 0;
	if ( nNewlyVisible )
	{
		nReturn |= MOVED_FROM_INVISIBLE_TO_VISIBLE;
	}
	if ( nNewlyFiltered )
	{
		nReturn |= MOVED_FROM_VISIBLE_TO_INVISIBLE;
	}
	return nReturn;
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
	m_bEnoughHits( true ),
	m_bVisibleHits( true )
{
}

void FileFilterState::initialize(const FilterControl& rControl)
{
	// TODO: do something
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

void HitFilterState::initialize(const FilterControl& rControl)
{
	// TODO: do something
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

void FileFilter::initializeFilterState(const FilterControl& rControl)
{
	m_oFileFilterState.initialize( rControl );
}

/**
 * @brief FileFilter::updateBoolState updates all filter state booleans with the exception of
 * m_bEnoughHits and m_bVisibleHits. Does not update m_bVisible.
 * @param rControlData
 * @param pFile
 */
void FileFilter::updateBoolState(const FilterControlData& rControlData)
{
	// TODO: do something
}

bool FileFilter::updateVisible()
{
	m_bVisible = m_oFileFilterState.m_bAdult &&
				 m_oFileFilterState.m_bDRM &&
				 m_oFileFilterState.m_bEnoughHits &&
				 m_oFileFilterState.m_bExistsInLibrary &&
				 m_oFileFilterState.m_bIncomplete &&
				 m_oFileFilterState.m_bSize &&
				 m_oFileFilterState.m_bVisibleHits;

	return m_bVisible;
}

HitFilter::HitFilter(const QueryHit* const pHit) :
	m_oHitFilterData( HitFilterData( pHit ) ),
	m_oHitFilterState( HitFilterState() )
{
	m_bInitialized = true;
}

void HitFilter::initializeFilterState(const FilterControl& rControl)
{
	m_oHitFilterState.initialize( rControl );
}

/**
 * @brief HitFilter::updateBoolState
 * @param rControlData
 * @return true if hit is visible after update; false otherwise
 */
bool HitFilter::updateBoolState(const FilterControlData& rControlData)
{

	// TODO: do something




	return true;
}
