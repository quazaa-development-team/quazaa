/*
** securitytablemodel.cpp
**** Copyright © Quazaa Development Team, 2009-2013.
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

#include <QAbstractItemView>

#include "securitytablemodel.h"

#include "debug_new.h"

RuleData::RuleData(Rule* pRule, SecurityTableModel* pModel) :
	m_pRule(      pRule                     ),
	m_nID(        pRule->m_nGUIID           ),
	m_nType(      pRule->type()             ),
	m_nAction(    pRule->m_nAction          ),
	m_nToday(     pRule->getTodayCount()    ),
	m_nTotal(     pRule->getTotalCount()    ),
	m_tExpire(    pRule->getExpiryTime()    ),
	m_sContent(   pRule->getContentString() ),
	m_sComment(   pRule->m_sComment         ),
	m_bRemoving(  false                     ),
	m_bAutomatic( pRule->m_bAutomatic       )
{
	switch( m_nAction )
	{
	case Security::RuleAction::None:
		m_piAction = pModel->m_pIcons[0];
		break;

	case Security::RuleAction::Accept:
		m_piAction = pModel->m_pIcons[1];
		break;

	case Security::RuleAction::Deny:
		m_piAction = pModel->m_pIcons[2];
		break;

	default:
		Q_ASSERT( false );
	}

	if ( m_nType == Type::RegularExpression )
		m_bContent = ((Security::UserAgentRule*)pRule)->getRegExp();
}

RuleData::~RuleData()
{
}

/**
 * @brief RuleData::update refreshes the data within RuleData if necessary.
 * Locking: REQUIRES securityManager.m_oRWLock: R
 * @param nRow : the row being refreshed
 * @param nSortCol : the currently sorted column
 * @param lToUpdate : the list of indexes that have changed
 * @param pModel : the model
 * @return true if an entry within the column col has been modified
 */
bool RuleData::update(int nRow, int nSortCol, QModelIndexList& lToUpdate,
					  SecurityTableModel* pModel)
{
	if ( m_bRemoving )
		return false;

	Q_ASSERT( m_pRule );

	bool bReturn = false;

	// type and ID never change

	if ( m_nAction != m_pRule->m_nAction )
	{
		lToUpdate.append( pModel->index( nRow, ACTION ) );
		m_nAction = m_pRule->m_nAction;

		switch( m_nAction )
		{
		case Security::RuleAction::None:
			m_piAction = pModel->m_pIcons[0];
			break;

		case Security::RuleAction::Accept:
			m_piAction = pModel->m_pIcons[1];
			break;

		case Security::RuleAction::Deny:
			m_piAction = pModel->m_pIcons[2];
			break;

		default:
			Q_ASSERT( false );
		}

		if ( nSortCol == ACTION )
			bReturn = true;
	}

	if ( m_nToday != m_pRule->getTodayCount() )
	{
		lToUpdate.append( pModel->index( nRow, HITS ) );
		m_nToday = m_pRule->getTodayCount();
		m_nTotal = m_pRule->getTotalCount();

		if ( nSortCol == HITS )
			bReturn = true;
	}

	if ( m_tExpire != m_pRule->getExpiryTime() )
	{
		lToUpdate.append( pModel->index( nRow, EXPIRES ) );
		m_tExpire = m_pRule->getExpiryTime();

		if ( nSortCol == EXPIRES )
			bReturn = true;
	}

	if ( m_sContent != m_pRule->getContentString() )
	{
		lToUpdate.append( pModel->index( nRow, CONTENT ) );
		m_sContent = m_pRule->getContentString();

		if ( nSortCol == CONTENT )
			bReturn = true;
	}

	if ( m_sComment != m_pRule->m_sComment )
	{
		lToUpdate.append( pModel->index( nRow, COMMENT ) );
		m_sComment = m_pRule->m_sComment;

		if ( nSortCol == COMMENT )
			bReturn = true;
	}

	if ( m_nType == Type::RegularExpression )
		m_bContent = ((Security::UserAgentRule*)m_pRule)->getRegExp();

	m_bAutomatic = m_pRule->m_bAutomatic;

	return bReturn;
}

/**
 * @brief RuleData::data
 * @param col
 * @return
 */
QVariant RuleData::data(int col) const
{
	switch ( col )
	{
	case CONTENT:
		return m_sContent;

	case TYPE:
		switch ( m_nType )
		{
		case Type::IPAddress:
			return tr( "IP Address" );

		case Type::IPAddressRange:
			return tr( "IP Address Range" );

		case Type::Country:
			return tr( "Country" );

		case Type::Hash:
			return tr( "File Filter" );

		case Type::RegularExpression:
			return tr( "Regular Expression" );

		case Type::UserAgent:
			return tr( "User Agent" );

		case Type::Content:
			return tr( "Content Filter" );

		default:
			Q_ASSERT( false );
			return tr( "Unknown" );
		}

	case ACTION:
		return actionToString( m_nAction );

	case EXPIRES:
		return expiryToString( m_tExpire );

	case HITS:
		return QString( "%1 (%2)" ).arg( QString::number( m_nToday ),
										 QString::number( m_nTotal ) );

	case COMMENT:
		return m_sComment;

	default:
		return QVariant();
	}
}

Rule* RuleData::rule() const
{
	return m_bRemoving ? NULL : m_pRule;
}

bool RuleData::lessThan(int col, const SecurityTableModel::RuleData* const pOther) const
{
	if ( !pOther )
		return false;

	switch ( col )
	{
	case CONTENT:
		return m_sContent < pOther->m_sContent;

	case TYPE:
		return m_nType < pOther->m_nType;

	case ACTION:
		return m_nAction  < pOther->m_nAction;

	case EXPIRES:
		if ( m_tExpire && !pOther->m_tExpire)
			return true;

		if ( m_tExpire > 1 && pOther->m_tExpire <= 1 )
			return true;

		return m_tExpire  < pOther->m_tExpire;

	case HITS:
		// TODO: sort by m_nToday for second click
		if ( m_nTotal == pOther->m_nTotal )
			return m_nToday < pOther->m_nToday;
		return m_nTotal < pOther->m_nTotal;

	case COMMENT:
		return m_sComment < pOther->m_sComment;

	default:
		return false;
	}

}

QString RuleData::actionToString(Action nAction) const
{
	switch( nAction )
	{
	case Action::None:
		return tr( "None" );

	case Action::Accept:
		return tr( "Allow" );

	case Action::Deny:
		return tr( "Deny" );

	default:
		return QString();
	}
}

QString RuleData::expiryToString(quint32 tExpire) const
{
	switch( tExpire )
	{
	case Security::RuleTime::Forever:
		return tr( "Forever" );

	case Security::RuleTime::Session:
		return tr( "Session" );

	default:
		return QDateTime::fromTime_t( tExpire ).toLocalTime().toString();
	}
}

SecurityTableModel::SecurityTableModel(QObject* parent, QWidget* container) :
	QAbstractTableModel( parent ),
	m_oContainer( container ),
	m_nSortColumn( -1 ),
	m_bNeedSorting( false )
{
	m_pIcons[0] = new QIcon( ":/Resource/Security/Null.ico" );
	m_pIcons[1] = new QIcon( ":/Resource/Security/Accept.ico" );
	m_pIcons[2] = new QIcon( ":/Resource/Security/Deny.ico" );

	// register necessary meta types before using them
	securityManager.registerMetaTypes();

	connect( &securityManager, SIGNAL( ruleAdded( Rule* ) ), this,
			 SLOT( addRule( Rule* ) ), Qt::QueuedConnection );

	connect( &securityManager, SIGNAL( ruleRemoved( SharedRulePtr ) ), this,
			 SLOT( removeRule( SharedRulePtr ) ), Qt::QueuedConnection );

	// This handles GUI updates on rule changes.
	connect( &securityManager, SIGNAL( ruleUpdated( ID ) ), this,
			 SLOT( updateRule( ID ) ), Qt::QueuedConnection );

	// This needs to be called to make sure that all rules added to the securityManager before this
	// part of the GUI is loaded are properly added to the model.
	completeRefresh();
}

SecurityTableModel::~SecurityTableModel()
{
	m_lNodes.clear();

	delete m_pIcons[0];
	delete m_pIcons[1];
	delete m_pIcons[2];
}

int SecurityTableModel::rowCount(const QModelIndex& parent) const
{
	if ( parent.isValid() )
	{
		return 0;
	}
	else
	{
		return m_lNodes.count();
	}
}

int SecurityTableModel::columnCount(const QModelIndex& parent) const
{
	if ( parent.isValid() )
	{
		return 0;
	}
	else
	{
		return _NO_OF_COLUMNS;
	}
}

QVariant SecurityTableModel::data(const QModelIndex& index, int nRole) const
{
	if ( !index.isValid() || index.row() > m_lNodes.size() || index.row() < 0 )
	{
		return QVariant();
	}

	const RuleDataPtr pData = m_lNodes.at( index.row() );

	switch ( nRole )
	{
	case Qt::DisplayRole:
		return pData->data( index.column() );

	case Qt::DecorationRole:
		if ( index.column() == ACTION )
		{
			return *pData->m_piAction;
		}
		break;

	// TODO: Reimplement formatting options in models.
	/*case Qt::ForegroundRole:
		switch ( nbr->nState )
		{
		case nsConnected:
			//return skinSettings.listsColorSpecial;
			break;
		case nsConnecting:
			//return skinSettings.listsColorActive;
			break;
		default:
			//return skinSettings.listsColorNormal;
			break;
		}
		return QVariant();*/

	/*case Qt::FontRole:
		QFont font = qApp->font( m_oContainer );
		switch ( nbr->nState )
		{
		case nsConnected:
			//font.setWeight(skinSettings.listsWeightSpecial);
			return font;
			break;
		case nsConnecting:
			//font.setWeight(skinSettings.listsWeightActive);
			return font;
			break;
		default:
			//font.setWeight(skinSettings.listsWeightNormal);
			return font;
			break;
		}*/

	default:
		break;
	}

	return QVariant();
}

QVariant SecurityTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ( orientation != Qt::Horizontal )
		return QVariant();

	if ( role == Qt::DisplayRole )
	{
		switch ( section )
		{
		case CONTENT:
			return tr( "Content" );

		case TYPE:
			return tr( "Type" );

		case ACTION:
			return tr( "Action" );

		case EXPIRES:
			return tr( "Expires" );

		case HITS:
			return tr( "Hits" );

		case COMMENT:
			return tr( "Comment" );
		}
	}
	else if ( role == Qt::ToolTipRole )
	{
		switch(section)
		{
		case CONTENT:
			return tr( "The content of the Security Manager rule" );

		case TYPE:
			return tr( "The rule type." );

		case ACTION:
			return tr( "Whether a rule blocks or allows content" );

		case EXPIRES:
			return tr( "When the Security Manager rule expires" );

		case HITS:
			return tr( "How often the rule has been hit today (since its creation date)" );

		case COMMENT:
			return tr( "Comment" );
		}
	}

	return QVariant();
}

QModelIndex SecurityTableModel::index(int row, int column, const QModelIndex& parent) const
{
	if ( parent.isValid() || row < 0 || row >= m_lNodes.count() )
		return QModelIndex();
	else
		return createIndex( row, column );
}

class SecurityTableModelCmp
{
public:
	SecurityTableModelCmp( int col, Qt::SortOrder o ) :
		column( col ),
		order( o )
	{
	}

	bool operator()( RuleDataPtr a, RuleDataPtr b )
	{
		if ( order == Qt::AscendingOrder )
		{
			return a->lessThan( column, b.data() );
		}
		else
		{
			return b->lessThan( column, a.data() );
		}
	}

	int column;
	Qt::SortOrder order;
};

void SecurityTableModel::sort(int column, Qt::SortOrder order)
{
	m_nSortColumn = column;
	m_nSortOrder  = order;

	emit layoutAboutToBeChanged();

	// I hope this is the correct way to convince Qt...
	QModelIndexList oldIdx = persistentIndexList();
	QModelIndexList newIdx = oldIdx;

	qStableSort( m_lNodes.begin(), m_lNodes.end(), SecurityTableModelCmp( column, order ) );

	for ( int i = 0; i < oldIdx.size(); ++i ) // For each persistent index
	{
		int oldRow = oldIdx.at(i).row();

		// if oldRow is outside range
		if ( oldRow > m_lNodes.size()
				// or the index points to another item
				|| oldIdx.at(i).internalPointer() != m_lNodes.at(oldRow) )
		{
			// find the correct item and update persistent index
			for ( int j = 0; j < m_lNodes.size(); ++j )
			{
				if ( oldIdx.at(i).internalPointer() == m_lNodes.at(j) )
				{
					newIdx[i] = createIndex( j, oldIdx.at(i).column(),
											 oldIdx.at(i).internalPointer() );
					break;
				}
			}
		}
	}

	changePersistentIndexList( oldIdx, newIdx );
	emit layoutChanged();
}

int SecurityTableModel::find(ID nRuleID)
{
	int nSize = m_lNodes.size();

	for ( int nPos = 0; nPos < nSize; ++nPos )
	{
		if ( m_lNodes[nPos]->m_nID == nRuleID )
			return nPos;
	}

	return -1;
}

RuleDataPtr SecurityTableModel::dataFromRow(int nRow) const
{
	if ( nRow < m_lNodes.count() && nRow >= 0 )
		return m_lNodes[nRow];
	else
		return RuleDataPtr();
}

/*Rule* SecurityTableModel::ruleFromIndex(const QModelIndex &index) const
{
	if ( index.isValid() && index.row() < m_lNodes.count() && index.row() >= 0 )
		return m_lNodes[ index.row() ]->m_pRule;
	else
		return NULL;
}*/

void SecurityTableModel::completeRefresh()
{
	// Remove all rules.
	if ( m_lNodes.size() )
	{
		beginRemoveRows( QModelIndex(), 0, m_lNodes.size() - 1 );
		m_lNodes.clear();
		endRemoveRows();
	}

	// Note that this slot is automatically disconnected once all rules have been recieved once.
	connect( &securityManager, SIGNAL( ruleInfo( Rule* ) ), this,
			 SLOT( recieveRuleInfo( Rule* ) ), Qt::QueuedConnection );

	// Request getting them back from the Security Manager.
	m_nRuleInfo = securityManager.requestRuleInfo();
}

/**
 * @brief SecurityTableModel::triggerRuleRemoval
 * @param nIndex
 */
void SecurityTableModel::triggerRuleRemoval(int nIndex)
{
	Q_ASSERT( nIndex >= 0 && nIndex < m_lNodes.size() );

	securityManager.remove( m_lNodes[nIndex]->rule() );
}

void SecurityTableModel::recieveRuleInfo(Rule* pRule)
{
	--m_nRuleInfo;

	// This handles disconnecting the ruleInfo signal after a completeRefresh() has been finished.
	if ( !m_nRuleInfo )
	{
		disconnect( &securityManager, SIGNAL( ruleInfo( Rule* ) ),
					this, SLOT( recieveRuleInfo( Rule* ) ) );
	}

	addRule( pRule );
}

/**
 * @brief SecurityTableModel::addRule adds a rule to the GUI.
 * @param pRule : the rule
 */
void SecurityTableModel::addRule(Rule* pRule)
{
	Q_ASSERT( pRule );
	Q_ASSERT( securityManager.check( pRule ) );

	securityManager.m_oRWLock.lockForRead();

	beginInsertRows( QModelIndex(), m_lNodes.size(), m_lNodes.size() );

	m_lNodes.append( RuleDataPtr( new RuleData( pRule, this ) ) );
	m_bNeedSorting = true; // triggers list to be resorted

	endInsertRows();

	securityManager.m_oRWLock.unlock();
}

/**
 * @brief SecurityTableModel::removeRule removes a rule from the table model.
 * This is to be triggered AFTER the rule has been removed from the manager by the manager.
 * @param pRule : the rule
 */
void SecurityTableModel::removeRule(SharedRulePtr pRule)
{
	//qDebug() << "SecurityTableModel::removeRule()";

	// TODO: remove assert before beta1
	Q_ASSERT( pRule );

	//qDebug() << "SecurityTableModel::removeRule() Rule appears to be valid.";

	for ( int i = 0; i < m_lNodes.size(); ++i )
	{
		if ( m_lNodes[i]->rule() == pRule.data() )
		{
			beginRemoveRows( QModelIndex(), i, i );

			//qDebug() << "SecurityTableModel::removeRule() Found rule row. Removing now.";

			m_lNodes[i]->m_bRemoving = true;// make sure m_pRule is not accessed anymore
			m_lNodes.removeAt( i );         // clears shared rule data pointer

			endRemoveRows();

			// m_bNeedSorting needs not to be set to true here as sorting is not required on removal
			break;
		}
	}

	//qDebug() << "SecurityTableModel::removeRule() finished";
}

/**
 * @brief updateRule updates the GUI for a specified rule.
 * @param nRuleID : the ID of the rule
 */
void SecurityTableModel::updateRule(ID nRuleID)
{
	QModelIndexList uplist;
	bool bSort = m_bNeedSorting;

	const int nRuleRowPos = find( nRuleID );

	Q_ASSERT( nRuleRowPos != -1 );

	securityManager.m_oRWLock.lockForRead();
	if ( m_lNodes[nRuleRowPos]->update( nRuleRowPos, m_nSortColumn, uplist, this ) )
	{
		bSort = true;
	}
	securityManager.m_oRWLock.unlock();

	// if necessary adjust container order (also updates view)
	if ( bSort )
	{
		sort( m_nSortColumn, m_nSortOrder );
		m_bNeedSorting = false;
	}
	// update view for all changed model indexes
	else if ( !uplist.isEmpty() )
	{
		QAbstractItemView* pView = qobject_cast< QAbstractItemView* >( m_oContainer );

		if ( pView )
		{
			foreach ( QModelIndex index, uplist )
			{
				pView->update( index );
			}
		}
	}
}

/**
 * @brief SecurityTableModel::updateAll updates all rules in the GUI.
 */
void SecurityTableModel::updateAll()
{
	QModelIndexList uplist;
	bool bSort = m_bNeedSorting;

	securityManager.m_oRWLock.lockForRead();
	for ( int i = 0, max = m_lNodes.count(); i < max; ++i )
	{
		if ( m_lNodes[i]->update( i, m_nSortColumn, uplist, this ) )
			bSort = true;
	}
	securityManager.m_oRWLock.unlock();

	if ( bSort )
	{
		sort( m_nSortColumn, m_nSortOrder );
		m_bNeedSorting = false;
	}
	else
	{
		if ( !uplist.isEmpty() )
		{
			QAbstractItemView* pView = qobject_cast< QAbstractItemView* >( m_oContainer );

			if( pView )
			{
				foreach ( QModelIndex index, uplist )
				{
					pView->update( index );
				}
			}
		}
	}
}
