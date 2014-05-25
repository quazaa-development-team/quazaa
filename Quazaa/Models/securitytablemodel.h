/*
** securitytablemodel.h
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

#ifndef SECURITYTABLEMODEL_H
#define SECURITYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

#include "securitymanager.h"

using namespace Security;

/**
 * @brief The SecurityTableModel class implements a table model for handling security rules.
 */
class SecurityTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	/**
	 * @brief The RuleData struct contains all GUI relevant data of a security rule.
	 */
	struct RuleData
	{
	private:
		// Object directly managed by security manager.
		Rule*           m_pRule;

	public:
		const ID        m_nID;
		const RuleType::Type m_nType;
		RuleAction::Action   m_nAction;
		quint32         m_nToday;
		quint32         m_nTotal;
		quint32         m_tExpire;  // UTC time in seconds
		QString         m_sExpire;
		quint32         m_tLastHit; // UTC time in seconds
		QString         m_sLastHit;
		QString         m_sContent;
		QString         m_sComment;
		const QIcon*    m_piAction;
		bool            m_bAutomatic;

		RuleData( Rule* pRule, const QIcon* pIcons[3] );
		~RuleData();

		/**
		 * @brief RuleData::update refreshes the data within RuleData if necessary.
		 * Locking: REQUIRES securityManager.m_oRWLock: R
		 * @param nRow : the row being refreshed
		 * @param nSortCol : the currently sorted column
		 * @param lToUpdate : the list of indexes that have changed
		 * @param pModel : the model
		 * @return true if an entry within the column col has been modified
		 */
		bool update( int nRow, int nSortCol, QModelIndexList& lToUpdate,
					 SecurityTableModel* pModel );
		QVariant data( int col ) const;

		/**
		 * @brief rule allows access to the rule pointer.
		 * @return the pointer to the Rule within the manager
		 */
		Rule* rule() const;

		bool lessThan( int col, bool bSortOrder , const RuleData* const pOther ) const;
		bool equals  ( int col, const RuleData* const pOther ) const;

		QString actionToString( RuleAction::Action nAction ) const;
		QString expiryToString( quint32 tExpire ) const;
		QString lastHitToString( quint32 tLastHit ) const;
	};

private:
	typedef std::vector<RuleData*>::size_type VectorPos;

	QWidget*        m_oContainer;
	Qt::SortOrder   m_nSortOrder;
	bool            m_bSecondarySortOrder;
	int             m_nSortColumn;
	static bool     m_bShutDown;
	quint32         m_nRuleInfo;

	std::vector<RuleData*> m_vNodes;

public:
	enum Column
	{
		CONTENT = 0,
		TYPE    = 1,
		ACTION  = 2,
		EXPIRES = 3,
		HITS    = 4,
		LASTHIT = 5,
		COMMENT = 6,
		_NO_OF_COLUMNS = 7
	};

	// icons used for the different rules
	const QIcon* m_pIcons[3];

public:
	explicit SecurityTableModel( QObject* parent = NULL, QWidget* container = NULL );
	~SecurityTableModel();

	int rowCount( const QModelIndex& parent = QModelIndex() ) const;
	int columnCount( const QModelIndex& parent ) const;
	QVariant data( const QModelIndex& index, int nRole ) const;
	QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
	QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;

	void sort( int column, Qt::SortOrder order , bool bUserInput = true );

	RuleData* dataFromRow( int nRow ) const;

	void triggerRuleRemoval( int nIndex );

	/**
	 * @brief clear removes all information from the GUI.
	 */
	void clear();

public slots:
	/**
	 * @brief completeRefresh does a complete refresh of all rules.
	 */
	void completeRefresh();

	/**
	 * @brief securityStartUpFinished initializes the GUI once Security has started.
	 */
	void securityStartUpFinished();

	/**
	 * @brief recieveRuleInfo
	 * @param pRule
	 */
	void recieveRuleInfo( Rule* pRule );

	/**
	 * @brief SecurityTableModel::addRule adds a rule to the GUI.
	 * @param pRule : the rule
	 */
	void addRule( Rule* pRule );

	/**
	 * @brief SecurityTableModel::removeRule removes a rule from the table model.
	 * This is to be triggered AFTER the rule has been removed from the manager by the manager.
	 * @param pRule : the rule
	 */
	void removeRule( SharedRulePtr pRule );

	/**
	 * @brief updateRule updates the GUI for a specified rule.
	 * @param nRuleID : the ID of the rule
	 */
	void updateRule( ID nRuleID );

	/**
	 * @brief updateAll updates all rules in the GUI.
	 */
	void updateAll();

	/**
	 * @brief onShutdown handles shutting down.
	 */
	void onShutdown();

private:

	VectorPos find( ID nRuleID ) const;

	/**
	 * @brief findInsertPos allows to determine the theoretical position (within [0; size]) of
	 * pData.
	 *
	 * @param pData  The RuleData to look for.
	 * @return the theoretical VectorPos
	 */
	VectorPos findInsertPos( const RuleData* const pData ) const;

	void insert( RuleData* pRule );

#ifdef _DEBUG
	void verifySorting() const;
#endif
};

#endif // SECURITYTABLEMODEL_H

