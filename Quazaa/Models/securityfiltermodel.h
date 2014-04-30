#ifndef SECURITYFILTERMODELAUTOMATIC_H
#define SECURITYFILTERMODELAUTOMATIC_H

#include <QSortFilterProxyModel>
#include "securitytablemodel.h"

/**
 * @brief The SecurityFilterModel class implements filtering and sorting rules based on whether they
 * are auto generated or not.
 */
class SecurityFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT

private:
	SecurityTableModel* m_lSecurity;
	bool                m_bShowAutomatic;

public:
	/**
	 * @brief SecurityFilterModel creates a new filter model for pTarget with bShowAutomatic.
	 * @param pTarget : the target model
	 * @param bShowAutomatic : whether to show auto generated rules or not
	 */
	SecurityFilterModel( SecurityTableModel* pTarget, bool bShowAutomatic );

	/**
	 * @brief sort sorts the underlying table model.
	 * @param nColumn : the column used as a basis for sorting
	 * @param eOrder : the sort order
	 */
	void sort( int nColumn, Qt::SortOrder eOrder );

protected:
	/**
	 * @brief filterAcceptsRow allows the caller to find out whether a row is filtered out or not.
	 * @param sourceRow : the row by source index number
	 * @param sourceParent : parent
	 * @return true if source row should be included in view; false otherwise
	 */
	bool filterAcceptsRow( int nSourceRow, const QModelIndex& sourceParent ) const;
};

#endif // SECURITYFILTERMODELAUTOMATIC_H
