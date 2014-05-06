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
#include <QtWidgets/QComboBox>

#include "NetworkCore/queryhit.h"
#include "Misc/unorderedptrvector.h"

class SearchHit;
class SearchFile;
class SearchTreeItem;

typedef UnorderedPtrVector<SearchHit>  HitList;
typedef UnorderedPtrVector<SearchFile> FileList;

namespace SearchFilter
{
class SavedFilters;

#define FILTERCONTROLDATA_CODE_VERSION	1
struct FilterControlData
{
	// filter attributes
	QString m_sMatchString;         // applied to hit names
	bool m_bRegExp;

	quint64 m_nMinSize;             // file size
	quint64 m_nMaxSize;
	quint16 m_nMinSources;          // minimum number of hits per file

	// bools: state allowed
	bool m_bAdultAllowed;           // adult files
	bool m_bBogusAllowed;           // bogus hits
	bool m_bBusyAllowed;            // busy hits/sources
	bool m_bDRMAllowed;             // DRM files
	bool m_bExistsInLibraryAllowed; // existing files
	bool m_bFirewalledAllowed;      // firewalled hits
	bool m_bIncompleteAllowed;      // incomplete files
	bool m_bNonMatchingAllowed;     // non matching hits
	bool m_bSuspiciousAllowed;      // suspicious hits
	bool m_bUnstableAllowed;        // unstable hits/sources

	// saving support
	SavedFilters* m_pFilters;
	QString       m_sFilterName;

private:
	FilterControlData( SavedFilters* pFilters );
public:
	FilterControlData( const FilterControlData& rOther );

	void operator=(  const FilterControlData& rOther );

	bool operator==( const FilterControlData& rOther );
	bool operator!=( const FilterControlData& rOther );

	static void load( FilterControlData* pFilterData, QDataStream& fsFile, quint16 );
	static void save( const FilterControlData* const pFilterData, QDataStream& fsFile );

	friend class SavedFilters;
};

class SavedFilters
{
private:
	typedef std::pair< QString, FilterControlData* > MapPair;
	typedef std::map<  QString, FilterControlData* > FilterMap;
	typedef FilterMap::const_iterator ConstIterator;
	typedef FilterMap::iterator Iterator;

	FilterMap m_mFilters;
	QString m_sDefaultName;

public:
	SavedFilters();
	~SavedFilters();

	quint32 load();

private:
	bool load( QString sPath );
	void clear();

public:
	void save();

	quint32 count() const;

	void insert( const FilterControlData& rData );
	void remove( const QString& rName );
	void select( const QString& sName, FilterControlData& rDestination );
	void rename( const QString& sOldName, const QString& sNewName );

	void setDefault( const QString& sName );
	const FilterControlData& defaultData() const;
	const QString& defaultName() const;

	void repopulate( QComboBox& rBox, const QString& expectedSecletion );

private:
	static quint32 writeToFile(const void* const pManager, QFile& oFile );
};

class FilterControl
{
private:
	// lists used to keep track of items for filtering purposes
	HitList m_lVisibleHits;         // contains currently visible hits
	HitList m_lFilteredHits;        // contains currently hidden hits

	FileList m_lVisibleFiles;       // contains currently visible files
	FileList m_lFilteredFiles;      // contains currently hidden files

	FilterControlData m_oFilterControlData;

	bool m_bStringChanged;
	bool m_bSizeChanged;
	bool m_bMinSourcesChanged;
	bool m_bHitBoolsChanged;
	bool m_bFileBoolsChanged;

	// allows lazy evaluation of string filter in case of an additionnal filter word
	bool m_bStringFilterInvisibleHitsInvalidated;

	// saving/loading support for advanced filter dialog
	static SavedFilters m_oSavedFilters;

public:
	FilterControl();
	~FilterControl();

	/*bool operator==(const FilterControl& rOther);
	bool operator!=(const FilterControl& rOther);
	bool operator<(const FilterControl& rOther);
	bool operator>(const FilterControl& rOther);*/

	void add( SearchTreeItem* pItem );
	void remove( SearchTreeItem* pItem );

	void clear();

	void update( const FilterControlData& rControlData );

	FilterControlData* getDataCopy() const;

private:
	void filterHits( const FilterControlData& rControlData );

	void analyseFilter( const QString& sNewMatchString,
						QStringList& lNewWords, QStringList& lRemovedWords ) const;
	void separateFilter( const QStringList& lWords, QStringList& lMustHaveWords,
						 QStringList& lMustNotHaveWords ) const;

	void applyStringFilter( HitList& lHits, const QStringList& lMustHaveWords,
							const QStringList& lMustNotHaveWords ) const;

	bool matchStringFilter( SearchHit* pHit, const QStringList& lMustHaveWords,
							const QStringList& lMustNotHaveWords ) const;

	void applyRegExpFilter( const QString& sRegExp );

	void filterFiles( const FilterControlData& rControlData );

	friend class HitFilter;
	friend class FileFilter;
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

	FileFilterData( const SearchHit* const pHit );
	void initialize( const SearchHit* const pHit );
	void addDataSet( const HitFilterData& hitData );
	void refresh( const SearchFile* const pThisFile );
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

	HitFilterData( const QueryHit* const pHit );
};

struct FileFilterState
{
	// false: filtered out; true: visible in GUI
	bool m_bAdult           : 1;
	bool m_bDRM             : 1;
	bool m_bExistsInLibrary : 1;
	bool m_bIncomplete      : 1;
	bool m_bSize            : 1;
	bool m_bEnoughHits      : 1;
	bool m_bVisibleHits     : 1;

	FileFilterState();
};

struct HitFilterState
{
	// false: filtered out; true: visible in GUI
	bool m_bBogus           : 1;
	bool m_bBusy            : 1;
	bool m_bFileName        : 1;    // used by string filter
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
	bool m_bInitialized;

public:
	Filter();
	bool visible() const;
	bool dataInitialized() const;
};

class FileFilter : public Filter
{
private:

public:
	FileFilterData  m_oFileFilterData;
	FileFilterState m_oFileFilterState;

	FileFilter( SearchHit* pHit );
	void initializeFilterState( const FilterControl& rControl );

	void updateBoolState( const FilterControlData& rControlData );

	bool updateVisible();
};

class HitFilter : public Filter
{
private:

public:
	HitFilterData   m_oHitFilterData;
	HitFilterState  m_oHitFilterState;

	HitFilter( const QueryHit* const pHit );
	void initializeFilterState( const FilterControl& rControl );

	bool updateBoolState( const FilterControlData& rControlData );
};

}
#endif // SEARCHFILTER_H
