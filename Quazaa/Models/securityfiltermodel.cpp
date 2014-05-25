#include "securityfiltermodel.h"

/**
 * @brief SecurityFilterModel creates a new filter model for pTarget with bShowAutomatic.
 * @param pTarget : the target model
 * @param bShowAutomatic : whether to show auto generated rules or not
 */
SecurityFilterModel::SecurityFilterModel( SecurityTableModel* pTarget, bool bShowAutomatic )
{
	m_lSecurity = pTarget;
	setSourceModel( m_lSecurity );
	m_bShowAutomatic = bShowAutomatic;
	invalidateFilter();
}

/**
 * @brief sort sorts the underlying table model.
 * @param nColumn : the column used as a basis for sorting
 * @param eOrder : the sort order
 */
void SecurityFilterModel::sort( int nColumn, Qt::SortOrder eOrder )
{
	m_lSecurity->sort( nColumn, eOrder );
}

/**
 * @brief filterAcceptsRow allows the caller to find out whether a row is filtered out or not.
 * @param sourceRow : the row by source index number
 * @param sourceParent : parent
 * @return true if source row should be included in view; false otherwise
 */
bool SecurityFilterModel::filterAcceptsRow( int nSourceRow, const QModelIndex& sourceParent ) const
{
	if ( sourceParent.isValid() )
	{
		return false;
	}

	Q_ASSERT( nSourceRow >= 0 && nSourceRow < m_lSecurity->rowCount() );

	SecurityTableModel::RuleData* pData = m_lSecurity->dataFromRow( nSourceRow );

	Q_ASSERT( pData );

	return m_bShowAutomatic == pData->m_bAutomatic;
}
