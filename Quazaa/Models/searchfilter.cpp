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

#include "quazaaglobals.h"

#include <QRegularExpression>

using namespace SearchFilter;

FilterControlData::FilterControlData( SavedFilters* pFilters ) :
	m_sMatchString( "" ),
	m_bRegExp( false ),

	m_nMinSize( 0 ),
	m_nMaxSize( 18446744073709551615 ), // max value of 64 bit int
	m_nMinSources( 0 ),

	m_bAdultAllowed(           true ),
	m_bBogusAllowed(           true ),
	m_bBusyAllowed(            true ),
	m_bDRMAllowed(             true ),
	m_bExistsInLibraryAllowed( true ),
	m_bFirewalledAllowed(      true ),
	m_bIncompleteAllowed(      true ),
	m_bNonMatchingAllowed(     true ),
	m_bSuspiciousAllowed(      true ),
	m_bUnstableAllowed(        true ),

	m_pFilters( pFilters ),
	m_sFilterName( "" )
{
}

FilterControlData::FilterControlData( const FilterControlData& rOther ) :
	m_sMatchString(   rOther.m_sMatchString     ),
	m_bRegExp(        rOther.m_bRegExp          ),

	m_nMinSize(       rOther.m_nMinSize         ),
	m_nMaxSize(       rOther.m_nMaxSize         ),
	m_nMinSources(    rOther.m_nMinSources      ),

	m_bAdultAllowed(           rOther.m_bAdultAllowed           ),
	m_bBogusAllowed(           rOther.m_bBogusAllowed           ),
	m_bBusyAllowed(            rOther.m_bBusyAllowed            ),
	m_bDRMAllowed(             rOther.m_bDRMAllowed             ),
	m_bExistsInLibraryAllowed( rOther.m_bExistsInLibraryAllowed ),
	m_bFirewalledAllowed(      rOther.m_bFirewalledAllowed      ),
	m_bIncompleteAllowed(      rOther.m_bIncompleteAllowed      ),
	m_bNonMatchingAllowed(     rOther.m_bNonMatchingAllowed     ),
	m_bSuspiciousAllowed(      rOther.m_bSuspiciousAllowed      ),
	m_bUnstableAllowed(        rOther.m_bUnstableAllowed        ),

	m_pFilters(    rOther.m_pFilters    ),
	m_sFilterName( rOther.m_sFilterName )
{
}

void FilterControlData::operator=(  const FilterControlData& rOther )
{
	Q_ASSERT( m_pFilters == rOther.m_pFilters );

	m_sMatchString   = rOther.m_sMatchString;
	m_bRegExp        = rOther.m_bRegExp;

	m_nMinSize       = rOther.m_nMinSize;
	m_nMaxSize       = rOther.m_nMaxSize;
	m_nMinSources    = rOther.m_nMinSources;

	m_bAdultAllowed           = rOther.m_bAdultAllowed;
	m_bBogusAllowed           = rOther.m_bBogusAllowed;
	m_bBusyAllowed            = rOther.m_bBusyAllowed;
	m_bDRMAllowed             = rOther.m_bDRMAllowed;
	m_bExistsInLibraryAllowed = rOther.m_bExistsInLibraryAllowed;
	m_bFirewalledAllowed      = rOther.m_bFirewalledAllowed;
	m_bIncompleteAllowed      = rOther.m_bIncompleteAllowed;
	m_bNonMatchingAllowed     = rOther.m_bNonMatchingAllowed;
	m_bSuspiciousAllowed      = rOther.m_bSuspiciousAllowed;
	m_bUnstableAllowed        = rOther.m_bUnstableAllowed;

	m_sFilterName    = rOther.m_sFilterName;
}

// ignores an eventual filter name
bool FilterControlData::operator==( const FilterControlData& rOther )
{
	Q_ASSERT( m_pFilters == rOther.m_pFilters );

	return m_sMatchString   == rOther.m_sMatchString     &&
		   m_bRegExp        == rOther.m_bRegExp          &&

		   m_nMinSize       == rOther.m_nMinSize         &&
		   m_nMaxSize       == rOther.m_nMaxSize         &&
		   m_nMinSources    == rOther.m_nMinSources      &&

		   m_bAdultAllowed           == rOther.m_bAdultAllowed           &&
		   m_bBogusAllowed           == rOther.m_bBogusAllowed           &&
		   m_bBusyAllowed            == rOther.m_bBusyAllowed            &&
		   m_bDRMAllowed             == rOther.m_bDRMAllowed             &&
		   m_bExistsInLibraryAllowed == rOther.m_bExistsInLibraryAllowed &&
		   m_bFirewalledAllowed      == rOther.m_bFirewalledAllowed      &&
		   m_bIncompleteAllowed      == rOther.m_bIncompleteAllowed      &&
		   m_bNonMatchingAllowed     == rOther.m_bNonMatchingAllowed     &&
		   m_bSuspiciousAllowed      == rOther.m_bSuspiciousAllowed      &&
		   m_bUnstableAllowed        == rOther.m_bUnstableAllowed;
}

bool FilterControlData::operator!=( const FilterControlData& rOther )
{
	return !operator==( rOther );
}

void FilterControlData::load( FilterControlData* pFilterData, QDataStream& fsFile,
							  quint16 /*nVersion*/ )
{
	fsFile >> pFilterData->m_sMatchString;
	fsFile >> pFilterData->m_bRegExp;

	fsFile >> pFilterData->m_nMinSize;
	fsFile >> pFilterData->m_nMaxSize;
	fsFile >> pFilterData->m_nMinSources;

	fsFile >> pFilterData->m_bAdultAllowed;
	fsFile >> pFilterData->m_bBogusAllowed;
	fsFile >> pFilterData->m_bBusyAllowed;
	fsFile >> pFilterData->m_bDRMAllowed;
	fsFile >> pFilterData->m_bExistsInLibraryAllowed;
	fsFile >> pFilterData->m_bFirewalledAllowed;
	fsFile >> pFilterData->m_bIncompleteAllowed;
	fsFile >> pFilterData->m_bNonMatchingAllowed;
	fsFile >> pFilterData->m_bSuspiciousAllowed;
	fsFile >> pFilterData->m_bUnstableAllowed;

	fsFile >> pFilterData->m_sFilterName;
}

void FilterControlData::save( const FilterControlData* const pFilterData, QDataStream& fsFile )
{
	fsFile << pFilterData->m_sMatchString;
	fsFile << pFilterData->m_bRegExp;

	fsFile << pFilterData->m_nMinSize;
	fsFile << pFilterData->m_nMaxSize;
	fsFile << pFilterData->m_nMinSources;

	fsFile << pFilterData->m_bAdultAllowed;
	fsFile << pFilterData->m_bBogusAllowed;
	fsFile << pFilterData->m_bBusyAllowed;
	fsFile << pFilterData->m_bDRMAllowed;
	fsFile << pFilterData->m_bExistsInLibraryAllowed;
	fsFile << pFilterData->m_bFirewalledAllowed;
	fsFile << pFilterData->m_bIncompleteAllowed;
	fsFile << pFilterData->m_bNonMatchingAllowed;
	fsFile << pFilterData->m_bSuspiciousAllowed;
	fsFile << pFilterData->m_bUnstableAllowed;

	fsFile << pFilterData->m_sFilterName;
}

SavedFilters::SavedFilters() :
	m_mFilters( FilterMap() ),
	m_sDefaultName( "" )
{
}

SavedFilters::~SavedFilters()
{
	clear();
}

quint32 SavedFilters::load()
{
	QString sPath = QuazaaGlobals::DATA_PATH() + "searchfilters.dat";

	if ( load( sPath ) )
	{
		systemLog.postLog( LogSeverity::Debug, Component::Network,
						   QObject::tr( "Loaded %1 search filter(s) from file:"
										).arg( QString::number( count() ) ) + " " + sPath );
	}
	else // Unable to load default file. Switch to backup one instead.
	{
		sPath = QuazaaGlobals::DATA_PATH() + "searchfilters_backup.dat";

		systemLog.postLog( LogSeverity::Warning, Component::Network,
			QObject::tr( "Failed to load search filters from primary file. Switching to backup:" )
			+ " " + sPath );

		if ( load( sPath ) )
		{
			systemLog.postLog( LogSeverity::Debug, Component::Network,
							   QObject::tr( "Loaded %1 search filter(s) from backup file: "
											).arg( QString::number( count() ) ) + sPath );
		}
		else
		{
			systemLog.postLog( LogSeverity::Warning, Component::Network,
							   QObject::tr( "Failed secondary attempt on loading filters!" ) );
		}
	}

	if ( m_mFilters.empty() )
	{
		// insert empty default filter
		insert( FilterControlData( this ) );
		m_sDefaultName = "";
	}

	return count();
}

bool SavedFilters::load( QString sPath )
{
	QFile oFile( sPath );

	if ( !oFile.open( QIODevice::ReadOnly ) )
	{
		return false;
	}

	FilterControlData* pFilterData = NULL;

	try
	{
		Q_ASSERT( !count() );

		QDataStream fsFile( &oFile );

		quint16 nVersion;
		quint32 nCount;

		fsFile >> nVersion;
		fsFile >> nCount;
		fsFile >> m_sDefaultName;

		if ( nVersion == FILTERCONTROLDATA_CODE_VERSION )
		{
			for ( quint32 i = 0; i < nCount; ++i )
			{
				pFilterData = new FilterControlData( this );
				FilterControlData::load( pFilterData, fsFile, nVersion );
				m_mFilters[pFilterData->m_sFilterName] = pFilterData;
				pFilterData = NULL;
			}
		}
	}
	catch ( ... )
	{
		if ( pFilterData )
		{
			delete pFilterData;
		}

		clear();
		oFile.close();

		systemLog.postLog( LogSeverity::Error, Component::Network,
						   QObject::tr( "Caught an exception while loading filters from file!" ) );

		return false;
	}
	oFile.close();

	return true;
}

void SavedFilters::clear()
{
	for ( ConstIterator it = m_mFilters.begin(); it != m_mFilters.end(); ++it )
	{
		delete (*it).second;
	}

	m_mFilters.clear();
}

void SavedFilters::save()
{
	const quint32 nCount = common::securedSaveFile( QuazaaGlobals::DATA_PATH(), "searchfilters.dat",
													Component::Network, this,
													&SavedFilters::writeToFile );

	systemLog.postLog( LogSeverity::Debug, Component::Network,
					   QObject::tr( "%1 search filter(s) saved." ).arg( nCount ) );
}

quint32 SavedFilters::count() const
{
	return (quint32)m_mFilters.size();
}

void SavedFilters::insert( const FilterControlData& rData )
{
	ConstIterator it = m_mFilters.find( rData.m_sFilterName );
	if ( it != m_mFilters.end() )
	{
		m_mFilters[rData.m_sFilterName]->operator=( rData );
	}
	else
	{
		m_mFilters[rData.m_sFilterName] = new FilterControlData( rData );
	}
}

void SavedFilters::remove(const QString& rName )
{
	ConstIterator it = m_mFilters.find( rName );
	if ( it != m_mFilters.end() )
	{
		delete (*it).second;
		m_mFilters.erase( it );

		if ( m_sDefaultName == rName )
		{
			m_sDefaultName = "";
		}
	}
}

void SavedFilters::select( const QString& sName, FilterControlData& rDestination )
{
	rDestination.operator=( *m_mFilters[sName] );
}

void SavedFilters::rename( const QString& sOldName, const QString& sNewName )
{
	ConstIterator it = m_mFilters.find( sNewName );
	if ( it != m_mFilters.end() )
	{
		delete (*it).second;
		m_mFilters.erase( it );
	}

	it = m_mFilters.find( sOldName );
	Q_ASSERT( it != m_mFilters.end() );

	FilterControlData* pData = (*it).second;
	m_mFilters.erase( it );

	pData->m_sFilterName = sNewName;
	m_mFilters[sNewName] = pData;

	if ( m_sDefaultName == sOldName )
	{
		m_sDefaultName = sNewName;
	}
}

void SavedFilters::setDefault(const QString& sName)
{
	ConstIterator it = m_mFilters.find( sName );
	if ( it != m_mFilters.end() )
	{
		m_sDefaultName = sName;
	}
}

const FilterControlData& SavedFilters::defaultData() const
{
	// This is typically the first method of SavedFilters that is called, so we use it for loading
	// as we cannot load within the constructor because loading relies on the systemLog being
	// initialized.

	// I know this is ugly, so sue me :D
	SavedFilters* pThis = const_cast<SavedFilters*>( this );
	// make sure to load only once
	static quint32 foo = pThis->load();
	Q_UNUSED( foo );

	ConstIterator it = m_mFilters.find( m_sDefaultName );
	Q_ASSERT( it != m_mFilters.end() );
	return *(*it).second;
}

const QString&SavedFilters::defaultName() const
{
	return m_sDefaultName;
}

void SavedFilters::repopulate( QComboBox& rBox , const QString& expectedSecletion )
{
	Q_ASSERT( !m_mFilters.empty() );   // at least one item has been inserted

	rBox.clear();

	int i = 0;
	for ( ConstIterator it = m_mFilters.begin(); it != m_mFilters.end(); ++it )
	{
		rBox.addItem( (*it).first );

		if ( (*it).first == expectedSecletion )
		{
			rBox.setCurrentIndex( i );
		}
		else
		{
			++i;
		}

	}

	Q_ASSERT( i < m_mFilters.size() ); // expectedSecletion has been found and set
}

quint32 SavedFilters::writeToFile( const void* const pManager, QFile& oFile )
{
#if ENABLE_DISCOVERY_DEBUGGING
	postLog( LogSeverity::Debug, QString( "Manager::writeToFile()" ) ), true );
#endif

	SavedFilters* pSavedFilters = ( SavedFilters* )pManager;

	const quint16 nVersion = FILTERCONTROLDATA_CODE_VERSION;
	const quint32 nCount   = pSavedFilters->count();

	try
	{
		QDataStream fsFile( &oFile );

		fsFile << nVersion;
		fsFile << nCount;
		fsFile << pSavedFilters->m_sDefaultName;

		FilterControlData* pFilterData;

		// write services to stream
		for ( ConstIterator it = pSavedFilters->m_mFilters.begin();
			  it != pSavedFilters->m_mFilters.end(); ++it )
		{
			const MapPair& pair = *it;
			pFilterData = pair.second;
			FilterControlData::save( pFilterData, fsFile );
		}
	}
	catch ( ... )
	{
		systemLog.postLog( LogSeverity::Error, Component::Network,
			QObject::tr( "Unspecified problem while writing search filters to disk." ) );

		return 0;
	}

	return nCount;
}

SavedFilters FilterControl::m_oSavedFilters = SavedFilters();

FilterControl::FilterControl() :
	m_lVisibleHits( HitList( 4095 ) ),
	m_lFilteredHits( HitList( 4095 ) ),
	m_lVisibleFiles( FileList( 2047 ) ),
	m_lFilteredFiles( FileList( 2047 ) ),
	m_oFilterControlData( m_oSavedFilters.defaultData() ),
	m_bStringChanged( false ),
	m_bSizeChanged( false ),
	m_bMinSourcesChanged( false ),
	m_bHitBoolsChanged( false ),
	m_bFileBoolsChanged( false ),
	m_bStringFilterInvisibleHitsInvalidated( false )
{
}

FilterControl::~FilterControl()
{
	m_oSavedFilters.save();
}

/*
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

void FilterControl::add( SearchTreeItem* pItem )
{
	Q_ASSERT( pItem->m_pFilter->dataInitialized() );

	if ( pItem->type() == SearchTreeItem::SearchHitType )
	{
		SearchHit* pHitItem   = ( SearchHit* )pItem;
		HitFilter* pHitFilter = ( HitFilter* )pItem->m_pFilter;

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
		SearchFile* pFileItem   = ( SearchFile* )pItem;
		FileFilter* pFileFilter = ( FileFilter* )pItem->m_pFilter;

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

void FilterControl::remove( SearchTreeItem* pItem )
{
	// REMOVE for beta 1
	Q_ASSERT( pItem->m_pFilter->dataInitialized() );

	if ( pItem->type() == SearchTreeItem::SearchHitType )
	{
		SearchHit* pHitItem   = ( SearchHit* )pItem;
		HitFilter* pHitFilter = ( HitFilter* )pItem->m_pFilter;

		if ( pHitFilter->visible() )
		{
			bool bFound = false;
			for ( quint32 i = 0, count = m_lVisibleHits.count(); i < count; ++i )
			{
				if ( m_lVisibleHits[i] == pHitItem )
				{
					m_lVisibleHits.erase( i );
					bFound = true;
					break;
				}
			}
			Q_ASSERT( bFound );

			pHitItem->parent()->removeVisibleChild();

			// TODO: handle file becoming visible!
		}
		else
		{
			bool bFound = false;
			for ( quint32 i = 0, count = m_lFilteredHits.count(); i < count; ++i )
			{
				if ( m_lFilteredHits[i] == pHitItem )
				{
					m_lFilteredHits.erase( i );
					bFound = true;
					break;
				}
			}
			Q_ASSERT( bFound );
		}
	}
	else if ( pItem->type() == SearchTreeItem::SearchFileType )
	{
		SearchFile* pFileItem   = ( SearchFile* )pItem;
		FileFilter* pFileFilter = ( FileFilter* )pItem->m_pFilter;

		if ( pFileFilter->visible() )
		{
			bool bFound = false;
			for ( quint32 i = 0, count = m_lVisibleFiles.count(); i < count; ++i )
			{
				if ( m_lVisibleFiles[i] == pFileItem )
				{
					m_lVisibleFiles.erase( i );
					bFound = true;
					break;
				}
			}
			Q_ASSERT( bFound );
		}
		else
		{
			bool bFound = false;
			for ( quint32 i = 0, count = m_lFilteredFiles.count(); i < count; ++i )
			{
				if ( m_lFilteredFiles[i] == pFileItem )
				{
					m_lFilteredFiles.erase( i );
					bFound = true;
					break;
				}
			}
			Q_ASSERT( bFound );
		}
	}
	else
	{
		Q_ASSERT( false );
	}
}

/**
 * @brief FilterControl::clear removes all managed SearchTreeItems from the filter.
 */
void FilterControl::clear()
{
	m_lVisibleHits.clear();         // contains currently visible hits
	m_lFilteredHits.clear();        // contains currently hidden hits

	m_lVisibleFiles.clear();       // contains currently visible files
	m_lFilteredFiles.clear();      // contains currently hidden files

	m_bStringChanged     = false;
	m_bSizeChanged       = false;
	m_bMinSourcesChanged = false;
	m_bHitBoolsChanged   = false;
	m_bFileBoolsChanged  = false;

	m_bStringFilterInvisibleHitsInvalidated = false;
}

#define MOVED_FROM_INVISIBLE_TO_VISIBLE 1
#define MOVED_FROM_VISIBLE_TO_INVISIBLE 2

void FilterControl::update( const FilterControlData& rControlData )
{
	m_bStringChanged      = m_oFilterControlData.m_bRegExp          != rControlData.m_bRegExp  ||
							m_oFilterControlData.m_sMatchString     != rControlData.m_sMatchString;
	m_bSizeChanged        = m_oFilterControlData.m_nMinSize         != rControlData.m_nMinSize ||
							m_oFilterControlData.m_nMaxSize         != rControlData.m_nMaxSize;
	m_bMinSourcesChanged  = m_oFilterControlData.m_nMinSources      != rControlData.m_nMinSources;
	m_bHitBoolsChanged    = m_oFilterControlData.m_bBogusAllowed           != rControlData.m_bBogusAllowed           ||
							m_oFilterControlData.m_bBusyAllowed            != rControlData.m_bBusyAllowed            ||
							m_oFilterControlData.m_bFirewalledAllowed      != rControlData.m_bFirewalledAllowed      ||
							m_oFilterControlData.m_bNonMatchingAllowed     != rControlData.m_bNonMatchingAllowed     ||
							m_oFilterControlData.m_bSuspiciousAllowed      != rControlData.m_bSuspiciousAllowed      ||
							m_oFilterControlData.m_bUnstableAllowed        != rControlData.m_bUnstableAllowed;
	m_bFileBoolsChanged =   m_oFilterControlData.m_bAdultAllowed           != rControlData.m_bAdultAllowed           ||
							m_oFilterControlData.m_bDRMAllowed             != rControlData.m_bDRMAllowed             ||
							m_oFilterControlData.m_bExistsInLibraryAllowed != rControlData.m_bExistsInLibraryAllowed ||
							m_oFilterControlData.m_bIncompleteAllowed      != rControlData.m_bIncompleteAllowed;

	bool bHitFilterChanges = m_bStringChanged || m_bHitBoolsChanged;
	if ( bHitFilterChanges || m_bSizeChanged || m_bMinSourcesChanged || m_bFileBoolsChanged )
	{
		if ( bHitFilterChanges )
		{
			filterHits( rControlData );
		}

		filterFiles( rControlData );
	}

	Q_ASSERT( m_oFilterControlData == rControlData );
}

FilterControlData* FilterControl::getDataCopy() const
{
	return new FilterControlData( m_oFilterControlData );
}

/**
 * @brief FilterControl::updateHitFilterStatus
 * @param rControlData
 */
void FilterControl::filterHits( const FilterControlData& rControlData )
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

	// contains hits moved from visible list on filter change
	HitList lNewlyFilteredHits = HitList( m_lVisibleHits.count() );

	int nNewlyVisible = 0, nNewlyFiltered = 0;

	// Updating all bools no matter their change state should be faster than using an if to check
	// for the respective change state each time.
	const quint32 nCount1 = m_lVisibleHits.count();
	// after i == 0, it overflows to MAX_VALUE, which is bigger than nCount1
	for ( quint32 i = nCount1 - 1; i < nCount1; --i )
	{
		if ( !( ( HitFilter* )m_lVisibleHits[i]->m_pFilter )->updateBoolState( rControlData ) )
		{
			m_lVisibleHits[i]->parent()->removeVisibleChild();

			// if hit invisible after filtering, move it to temp list
			lNewlyFilteredHits.push_back( m_lVisibleHits[i] );
			m_lVisibleHits.erase( i );

			++nNewlyFiltered;
		}
	}

	const quint32 nCount2 = m_lFilteredHits.count();
	for ( quint32 i = nCount2 - 1; i < nCount2; --i )
	{
		if ( ( ( HitFilter* )m_lFilteredHits[i]->m_pFilter )->updateBoolState( rControlData ) )
		{
			m_lFilteredHits[i]->parent()->addVisibleChild();

			// if hit visible after filtering, move it to visible list
			m_lVisibleHits.push_back( m_lFilteredHits[i] );
			m_lFilteredHits.erase( i );

			++nNewlyVisible;
		}
	}

	for ( quint32 i = 0, count = lNewlyFilteredHits.count(); i < count; ++i )
	{
		m_lFilteredHits.push_back( lNewlyFilteredHits[i] );
	}

	// make sure the m_oFilterControlData state is updated properly
	if ( m_bHitBoolsChanged )
	{
		m_oFilterControlData.m_bBogusAllowed           = rControlData.m_bBogusAllowed;
		m_oFilterControlData.m_bBusyAllowed            = rControlData.m_bBusyAllowed;
		m_oFilterControlData.m_bFirewalledAllowed      = rControlData.m_bFirewalledAllowed;
		m_oFilterControlData.m_bNonMatchingAllowed     = rControlData.m_bNonMatchingAllowed;
		m_oFilterControlData.m_bSuspiciousAllowed      = rControlData.m_bSuspiciousAllowed;
		m_oFilterControlData.m_bUnstableAllowed        = rControlData.m_bUnstableAllowed;
	}
}

/**
 * @brief FilterControl::analyseFilter takes a new filter string and compares it with the current
 * filter string.
 * @param sNewMatchString represents the new match filter (list of words, NOT RegExp)
 * @param lNewWords contains all words added in the new filter (not present in old filter)
 * @param lRemovedWords contains all words removed from the filter (compared to the old filter)
 */
void FilterControl::analyseFilter( const QString& sNewMatchString,
								   QStringList& lNewWords, QStringList& lRemovedWords ) const
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
void FilterControl::separateFilter( const QStringList& lWords, QStringList& lMustHaveWords,
									QStringList& lMustNotHaveWords ) const
{
	foreach ( const QString & s, lWords )
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
void FilterControl::applyStringFilter( HitList& lHits, const QStringList& lMustHaveWords,
									   const QStringList& lMustNotHaveWords ) const
{
	for ( quint32 i = 0, count = lHits.count(); i < count; ++i )
	{
		( ( HitFilter* )lHits[i]->m_pFilter )->m_oHitFilterState.m_bFileName =
			matchStringFilter( lHits[i], lMustHaveWords, lMustNotHaveWords );
	}
}

/**
 * @brief FilterControl::matchStringFilter
 * @param pHit
 * @param lMustHaveWords
 * @param lMustNotHaveWords
 * @return
 */
bool FilterControl::matchStringFilter( SearchHit* pHit, const QStringList& lMustHaveWords,
									   const QStringList& lMustNotHaveWords ) const
{
	foreach ( const QString & sMust, lMustHaveWords )
	{
		if ( !pHit->m_oHitData.pQueryHit->m_sDescriptiveName.contains( sMust ) )
		{
			return false;
		}
	}
	foreach ( const QString & sMustNot, lMustNotHaveWords )
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
void FilterControl::applyRegExpFilter( const QString& sRegExp )
{
	QRegularExpression oRegExp( sRegExp );

	for ( quint32 i = 0, count = m_lVisibleHits.count(); i < count; ++i )
	{
		// false: filtered out; true: visible in GUI
		( ( HitFilter* )m_lVisibleHits[i]->m_pFilter )->m_oHitFilterState.m_bFileName =
			oRegExp.match( m_lVisibleHits[i]->m_oHitData.pQueryHit->m_sDescriptiveName
						 ).hasMatch();
	}

	for ( quint32 i = 0, count = m_lFilteredHits.count(); i < count; ++i )
	{
		// false: filtered out; true: visible in GUI
		( ( HitFilter* )m_lFilteredHits[i]->m_pFilter )->m_oHitFilterState.m_bFileName =
			oRegExp.match( m_lFilteredHits[i]->m_oHitData.pQueryHit->m_sDescriptiveName
						 ).hasMatch();
	}
}

/**
 * @brief FilterControl::filterFiles filters all file entries according to the new control data.
 * Note: this requires the hit filtering to have been applied already.
 * @param rControlData
 */
void FilterControl::filterFiles( const FilterControlData& rControlData )
{
	// Update all of these state bools only if there has actually been a change.
	if ( m_bSizeChanged || m_bMinSourcesChanged || m_bFileBoolsChanged )
	{
		for ( quint32 i = 0, count = m_lVisibleFiles.count(); i < count; ++i )
		{
			FileFilter* pFilter = ( FileFilter* )m_lVisibleFiles[i]->m_pFilter;
			pFilter->updateBoolState( rControlData );
			pFilter->m_oFileFilterState.m_bEnoughHits =
				m_lVisibleFiles[i]->childCount() >= rControlData.m_nMinSources;
		}
		for ( quint32 i = 0, count = m_lFilteredFiles.count(); i < count; ++i )
		{
			FileFilter* pFilter = ( FileFilter* )m_lFilteredFiles[i]->m_pFilter;
			pFilter->updateBoolState( rControlData );
			pFilter->m_oFileFilterState.m_bEnoughHits =
				m_lFilteredFiles[i]->childCount() >= rControlData.m_nMinSources;
		}

		m_oFilterControlData.m_nMinSize    = rControlData.m_nMinSize;
		m_oFilterControlData.m_nMaxSize    = rControlData.m_nMaxSize;
		m_oFilterControlData.m_nMinSources = rControlData.m_nMinSources;

		m_oFilterControlData.m_bAdultAllowed           = rControlData.m_bAdultAllowed;
		m_oFilterControlData.m_bDRMAllowed             = rControlData.m_bDRMAllowed;
		m_oFilterControlData.m_bExistsInLibraryAllowed = rControlData.m_bExistsInLibraryAllowed;
		m_oFilterControlData.m_bIncompleteAllowed      = rControlData.m_bIncompleteAllowed;
	}

	// contains files moved from visible list on filter change
	FileList lNewlyFilteredFiles = FileList( m_lVisibleFiles.count() );
	int nNewlyVisible = 0, nNewlyFiltered = 0;

	FileList::iterator it = m_lVisibleFiles.begin();
	while ( it != m_lVisibleFiles.end() )
	{
		FileFilter* pFilter = ( FileFilter* )( *it )->m_pFilter;

		// m_bVisibleHits always changes because of the applied hit filter, so it must be updated.
		pFilter->m_oFileFilterState.m_bVisibleHits = ( *it )->visibleChildCount();
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
		FileFilter* pFilter = ( FileFilter* )( *it )->m_pFilter;

		// m_bVisibleHits always changes because of the applied hit filter, so it must be updated.
		pFilter->m_oFileFilterState.m_bVisibleHits = ( *it )->visibleChildCount();
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

	for ( quint32 i = 0, count = lNewlyFilteredFiles.count(); i < count; ++i )
	{
		m_lFilteredFiles.push_back( lNewlyFilteredFiles[i] );
	}
}

FileFilterData::FileFilterData( const SearchHit* const pHit )
{
	initialize( pHit );
}

void FileFilterData::initialize( const SearchHit* const pHit )
{
	m_nSize = pHit->m_oHitData.pQueryHit->m_nObjectSize;
	m_bExistsInLibrary = false;

	const HitFilter* const pHitFilter  = ( HitFilter* )pHit->getFilter();

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
void FileFilterData::addDataSet( const HitFilterData& hitData )
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
void FileFilterData::refresh( const SearchFile* const pThisFile )
{
	Q_ASSERT( pThisFile->childCount() > 0 );

	initialize( ( SearchHit* )pThisFile->child( 0 ) );

	for ( int i = 1; i < pThisFile->childCount(); ++i )
	{
		addDataSet( ( ( HitFilter* )pThisFile->child( i )->getFilter() )->m_oHitFilterData );
	}
}

HitFilterData::HitFilterData( const QueryHit* const pHit ) :
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

FileFilter::FileFilter( SearchHit* pHit ) :
	m_oFileFilterData( FileFilterData( pHit ) ),
	m_oFileFilterState( FileFilterState() )
{
	m_bInitialized = true;
}

void FileFilter::initializeFilterState( const FilterControl& rControl )
{
	updateBoolState( rControl.m_oFilterControlData );
	updateVisible();
}

/**
 * @brief FileFilter::updateBoolState updates all filter state booleans with the exception of
 * m_bEnoughHits and m_bVisibleHits. Does not update m_bVisible.
 * @param rControlData
 * @param pFile
 */
void FileFilter::updateBoolState( const FilterControlData& rControlData )
{
	// false: filtered out; true: visible in GUI
	m_oFileFilterState.m_bAdult           = m_oFileFilterData.m_bAdult ?
											rControlData.m_bAdultAllowed           : true;
	m_oFileFilterState.m_bDRM             = m_oFileFilterData.m_bDRM   ?
											rControlData.m_bDRMAllowed             : true;
	m_oFileFilterState.m_bExistsInLibrary = m_oFileFilterData.m_bExistsInLibrary ?
											rControlData.m_bExistsInLibraryAllowed : true;
	m_oFileFilterState.m_bIncomplete      = m_oFileFilterData.m_bIncomplete ?
											rControlData.m_bIncompleteAllowed      : true;
	m_oFileFilterState.m_bSize            = m_oFileFilterData.m_nSize >= rControlData.m_nMinSize &&
											m_oFileFilterData.m_nSize <= rControlData.m_nMaxSize;
}

bool FileFilter::updateVisible()
{
	m_bVisible = m_oFileFilterState.m_bAdult           &&
				 m_oFileFilterState.m_bDRM             &&
				 m_oFileFilterState.m_bEnoughHits      &&
				 m_oFileFilterState.m_bExistsInLibrary &&
				 m_oFileFilterState.m_bIncomplete      &&
				 m_oFileFilterState.m_bSize            &&
				 m_oFileFilterState.m_bVisibleHits;

	return m_bVisible;
}

HitFilter::HitFilter( const QueryHit* const pHit ) :
	m_oHitFilterData( HitFilterData( pHit ) ),
	m_oHitFilterState( HitFilterState() )
{
	m_bInitialized = true;
}

void HitFilter::initializeFilterState( const FilterControl& rControl )
{
	updateBoolState( rControl.m_oFilterControlData );
}

/**
 * @brief HitFilter::updateBoolState updates all bool states (except m_bFileName which is handled
 * dircetly by the string filter) using the provided control data.
 * @param rControlData
 * @return true if hit is visible after update; false otherwise
 */
bool HitFilter::updateBoolState( const FilterControlData& rControlData )
{
	// false: filtered out; true: visible in GUI
	m_oHitFilterState.m_bBogus       = m_oHitFilterData.m_bBogus ?
									   rControlData.m_bBogusAllowed       : true;
	m_oHitFilterState.m_bBusy        = m_oHitFilterData.m_bBusy ?
									   rControlData.m_bBusyAllowed        : true;
	m_oHitFilterState.m_bFirewalled  = m_oHitFilterData.m_bFirewalled ?
									   rControlData.m_bFirewalledAllowed  : true;
	m_oHitFilterState.m_bNonMatching = m_oHitFilterData.m_bNonMatching ?
									   rControlData.m_bNonMatchingAllowed : true;
	m_oHitFilterState.m_bSuspicious  = m_oHitFilterData.m_bSuspicious ?
									   rControlData.m_bSuspiciousAllowed  : true;
	m_oHitFilterState.m_bUnstable    = m_oHitFilterData.m_bUnstable ?
									   rControlData.m_bUnstableAllowed    : true;

	m_bVisible = m_oHitFilterState.m_bBogus       &&
				 m_oHitFilterState.m_bBusy        &&
				 m_oHitFilterState.m_bFileName    &&
				 m_oHitFilterState.m_bFirewalled  &&
				 m_oHitFilterState.m_bNonMatching &&
				 m_oHitFilterState.m_bSuspicious  &&
				 m_oHitFilterState.m_bUnstable;

	return m_bVisible;
}
