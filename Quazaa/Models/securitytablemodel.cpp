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

#include <limits>

#include "winmain.h"
#include "securitytablemodel.h"

#include "debug_new.h"

SecurityTableModel::RuleData::RuleData( Rule* pRule, const QIcon* pIcons[3] ) :
	m_pRule(      pRule                       ),
	m_nID(        pRule->m_nGUIID             ),
	m_nType(      pRule->type()               ),
	m_nAction(    pRule->m_nAction            ),
	m_nToday(     pRule->todayCount()         ),
	m_nTotal(     pRule->totalCount()         ),
	m_tExpire(    pRule->expiryTime()         ),
	m_sExpire(  expiryToString ( m_tExpire  ) ),
	m_tLastHit(   pRule->lastHit()            ),
	m_sLastHit( lastHitToString( m_tLastHit ) ),
	m_sContent(   pRule->contentString()   ),
	m_sComment(   pRule->m_sComment           ),
	m_bAutomatic( pRule->m_bAutomatic         )
{
	switch ( m_nAction )
	{
	case Security::RuleAction::None:
		m_piAction = pIcons[0];
		break;

	case Security::RuleAction::Accept:
		m_piAction = pIcons[1];
		break;

	case Security::RuleAction::Deny:
		m_piAction = pIcons[2];
		break;

	default:
		Q_ASSERT( false );
	}
}

SecurityTableModel::RuleData::~RuleData()
{
	// m_pRule     is handled by SecurityManager
	// m_piAction  is handled by SecurityTableModel
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
bool SecurityTableModel::RuleData::update( int nRow, int nSortCol, QModelIndexList& lToUpdate,
										   SecurityTableModel* pModel )
{
	if ( m_bShutDown )
	{
		return false;
	}

	Q_ASSERT( m_pRule );

	bool bReturn = false;

	// type and ID never change
	Q_ASSERT( m_nType == m_pRule->type()   );
	Q_ASSERT( m_nID   == m_pRule->m_nGUIID );

	if ( m_nAction != m_pRule->m_nAction )
	{
		lToUpdate.append( pModel->index( nRow, ACTION ) );
		m_nAction = m_pRule->m_nAction;

		switch ( m_nAction )
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
		{
			bReturn = true;
		}
	}

	if ( m_nToday != m_pRule->todayCount() )
	{
		lToUpdate.append( pModel->index( nRow, HITS ) );
		m_nToday = m_pRule->todayCount();
		m_nTotal = m_pRule->totalCount();

		lToUpdate.append( pModel->index( nRow, LASTHIT ) );
		m_tLastHit = m_pRule->lastHit();
		m_sLastHit = lastHitToString( m_tLastHit );

		if ( nSortCol == HITS || nSortCol == LASTHIT )
		{
			bReturn = true;
		}
	}

	if ( m_tExpire != m_pRule->expiryTime() )
	{
		lToUpdate.append( pModel->index( nRow, EXPIRES ) );
		m_tExpire = m_pRule->expiryTime();
		m_sExpire = expiryToString( m_tExpire );

		if ( nSortCol == EXPIRES )
		{
			bReturn = true;
		}
	}

	if ( m_sContent != m_pRule->contentString() )
	{
		lToUpdate.append( pModel->index( nRow, CONTENT ) );
		m_sContent = m_pRule->contentString();

		if ( nSortCol == CONTENT )
		{
			bReturn = true;
		}
	}

	if ( m_sComment != m_pRule->m_sComment )
	{
		lToUpdate.append( pModel->index( nRow, COMMENT ) );
		m_sComment = m_pRule->m_sComment;

		if ( nSortCol == COMMENT )
		{
			bReturn = true;
		}
	}

	m_bAutomatic = m_pRule->m_bAutomatic;

	return bReturn;
}

/**
 * @brief RuleData::data
 * @param col
 * @return
 */
QVariant SecurityTableModel::RuleData::data( int col ) const
{
	switch ( col )
	{
	case CONTENT:
		return m_sContent;

	case TYPE:
		switch ( m_nType )
		{
		case RuleType::IPAddress:
			return tr( "IP Address" );

		case RuleType::IPAddressRange:
			return tr( "IP Address Range" );

		case RuleType::Country:
			return tr( "Country" );

		case RuleType::Hash:
			return tr( "File Filter" );

		case RuleType::RegularExpression:
			return tr( "Regular Expression" );

		case RuleType::UserAgent:
			return tr( "User Agent" );

		case RuleType::Content:
			return tr( "Content Filter" );

		default:
			Q_ASSERT( false );
			return tr( "Unknown" );
		}

	case ACTION:
		return actionToString( m_nAction );

	case EXPIRES:
		return m_sExpire;

	case HITS:
		return QString( "%1 (%2)" ).arg( QString::number( m_nTotal ),
										 QString::number( m_nToday ) );

	case LASTHIT:
		return m_sLastHit;

	case COMMENT:
		return m_sComment;

	default:
		return QVariant();
	}
}

Rule* SecurityTableModel::RuleData::rule() const
{
	return m_bShutDown ? NULL : m_pRule;
}

bool SecurityTableModel::RuleData::lessThan( int col, bool bSortOrder,
											 const SecurityTableModel::RuleData* const pOther ) const
{
	Q_ASSERT( pOther );

	switch ( col )
	{
	case CONTENT:
		return m_sContent < pOther->m_sContent;

	case TYPE:
		return m_nType    < pOther->m_nType;

	case ACTION:
		return m_nAction  < pOther->m_nAction;

	case EXPIRES:
		return ( m_tExpire < 2 ? std::numeric_limits<quint32>::max() - m_tExpire : m_tExpire ) <
			   ( pOther->m_tExpire < 2 ?
					 std::numeric_limits<quint32>::max() - pOther->m_tExpire : pOther->m_tExpire );

	case HITS:
		if ( bSortOrder )
		{
			if ( m_nTotal == pOther->m_nTotal )
			{
				return m_nToday < pOther->m_nToday;
			}
			else
			{
				return m_nTotal < pOther->m_nTotal;
			}
		}
		else
		{
			if ( m_nToday == pOther->m_nToday )
			{
				return m_nTotal < pOther->m_nTotal;
			}
			else
			{
				return m_nToday < pOther->m_nToday;
			}
		}

	case LASTHIT:
		return m_tLastHit < pOther->m_tLastHit;

	case COMMENT:
		return m_sComment < pOther->m_sComment;

	default:
		Q_ASSERT( false ); // a column comparison implementation is missing
		return false;
	}
}

bool SecurityTableModel::RuleData::equals( int col,
										   const SecurityTableModel::RuleData* const pOther ) const
{
	Q_ASSERT( pOther );

	switch ( col )
	{
	case CONTENT:
		return m_sContent == pOther->m_sContent;

	case TYPE:
		return m_nType    == pOther->m_nType;

	case ACTION:
		return m_nAction  == pOther->m_nAction;

	case EXPIRES:
		return m_tExpire  == pOther->m_tExpire;

	case HITS:
		return m_nTotal   == pOther->m_nTotal && m_nToday == pOther->m_nToday;

	case LASTHIT:
		return m_tLastHit == pOther->m_tLastHit;

	case COMMENT:
		return m_sComment == pOther->m_sComment;

	default:
		Q_ASSERT( false ); // a column comparison implementation is missing
		return false;
	}
}

QString SecurityTableModel::RuleData::actionToString( RuleAction::Action nAction ) const
{
	switch ( nAction )
	{
	case RuleAction::None:
		return tr( "None" );

	case RuleAction::Accept:
		return tr( "Allow" );

	case RuleAction::Deny:
		return tr( "Deny" );

	default:
		return QString();
	}
}

QString SecurityTableModel::RuleData::expiryToString( quint32 tExpire ) const
{
	switch ( tExpire )
	{
	case Security::RuleTime::Forever:
		return tr( "Forever" );

	case Security::RuleTime::Session:
		return tr( "Session" );

	default:
		return QDateTime::fromTime_t( tExpire ).toLocalTime().toString();
	}
}

QString SecurityTableModel::RuleData::lastHitToString( quint32 tLastHit ) const
{
	if ( !tLastHit )
	{
		return tr( "Never" );
	}
	else
	{
		return QDateTime::fromTime_t( tLastHit ).toLocalTime().toString();
	}
}

bool SecurityTableModel::m_bShutDown = false;

SecurityTableModel::SecurityTableModel( QObject* parent, QWidget* container ) :
	QAbstractTableModel( parent ),
	m_oContainer( container ),
	m_nSortOrder( Qt::AscendingOrder ),
	m_nSortColumn( -1 ),
	m_bSecondarySortOrder( true ),
	m_nRuleInfo( 0 ),
	m_vNodes( std::vector<RuleData*>() )
{
	m_pIcons[0] = new QIcon( ":/Resource/Security/Null.ico"   );
	m_pIcons[1] = new QIcon( ":/Resource/Security/Accept.ico" );
	m_pIcons[2] = new QIcon( ":/Resource/Security/Deny.ico"   );

	// This connects the GUI to the Security Manager and the Main Window.
	connect( &securityManager, &Security::Manager::startUpFinished,
			 this, &SecurityTableModel::securityStartUpFinished );
}

SecurityTableModel::~SecurityTableModel()
{
	m_vNodes.clear();

	delete m_pIcons[0];
	delete m_pIcons[1];
	delete m_pIcons[2];
}

int SecurityTableModel::rowCount( const QModelIndex& parent ) const
{
	if ( parent.isValid() )
	{
		return 0;
	}
	else
	{
		return ( int )m_vNodes.size();
	}
}

int SecurityTableModel::columnCount( const QModelIndex& parent ) const
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

QVariant SecurityTableModel::data( const QModelIndex& index, int nRole ) const
{
	if ( !index.isValid() || index.row() >= m_vNodes.size() || index.row() < 0 )
	{
		return QVariant();
	}

	const RuleData* pData = m_vNodes[index.row()];

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

QVariant SecurityTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if ( orientation != Qt::Horizontal )
	{
		return QVariant();
	}

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

		case LASTHIT:
			return tr( "Last Hit" );

		case COMMENT:
			return tr( "Comment" );
		}
	}
	else if ( role == Qt::ToolTipRole )
	{
		switch ( section )
		{
		case CONTENT:
			return tr( "The content of the Security Manager rule" );

		case TYPE:
			return tr( "The rule type" );

		case ACTION:
			return tr( "Whether a rule blocks or allows content" );

		case EXPIRES:
			return tr( "When the Security Manager rule expires" );

		case HITS:
			return tr( "How often the rule has been hit since its creation (in this session)" );

		case LASTHIT:
			return tr( "When the rule was hit last" );

		case COMMENT:
			return tr( "Comment" );
		}
	}

	return QVariant();
}

QModelIndex SecurityTableModel::index( int row, int column, const QModelIndex& parent ) const
{
	if ( parent.isValid() || row < 0 || row >= m_vNodes.size() )
	{
		return QModelIndex();
	}
	else
	{
		return createIndex( row, column );
	}
}

class SecurityTableModelCmp
{
private:
	int             m_nColumn;
	Qt::SortOrder   m_nOrder;
	bool            m_bSecondarySortOrder;

public:
	SecurityTableModelCmp( int col, Qt::SortOrder o, bool bOrder ) :
		m_nColumn( col ),
		m_nOrder( o ),
		m_bSecondarySortOrder( bOrder )
	{
	}

	bool operator()( const SecurityTableModel::RuleData* const a,
					 const SecurityTableModel::RuleData* const b )
	{
		if ( m_nOrder == Qt::AscendingOrder )
		{
			return a->lessThan( m_nColumn, m_bSecondarySortOrder, b );
		}
		else
		{
			return b->lessThan( m_nColumn, m_bSecondarySortOrder, a );
		}
	}
};

void SecurityTableModel::sort( int column, Qt::SortOrder order, bool bUserInput )
{
	m_nSortColumn = column;
	m_nSortOrder  = order;

	if ( bUserInput )
	{
		if ( m_nSortColumn == HITS )
		{
			if ( m_nSortOrder == Qt::DescendingOrder )
			{
				m_bSecondarySortOrder = !m_bSecondarySortOrder;
			}
		}
		else
		{
			m_bSecondarySortOrder = false;
		}
	}

	emit layoutAboutToBeChanged();

	// I hope this is the correct way to convince Qt...
	QModelIndexList oldIdx = persistentIndexList();
	QModelIndexList newIdx = oldIdx;

	// make sure there is enough memory for sorting efficiently
	const VectorPos nSize = m_vNodes.size();
	m_vNodes.reserve( nSize + nSize / 2 );

	std::stable_sort( m_vNodes.begin(), m_vNodes.end(),
					  SecurityTableModelCmp( column, order, m_bSecondarySortOrder ) );

	for ( int i = 0, nSize = oldIdx.size(); i < nSize; ++i ) // For each persistent index
	{
		int oldRow = oldIdx.at( i ).row();

		// if oldRow is outside range
		if ( oldRow > nSize ||
			 // or the index points to another item
			 oldIdx.at( i ).internalPointer() != m_vNodes[oldRow] )
		{
			// find the correct item and update persistent index
			for ( int j = 0; j < nSize; ++j )
			{
				if ( oldIdx.at( i ).internalPointer() == m_vNodes[j] )
				{
					newIdx[i] = createIndex( j, oldIdx.at( i ).column(),
											 oldIdx.at( i ).internalPointer() );
					break;
				}
			}
		}
	}

	changePersistentIndexList( oldIdx, newIdx );
	emit layoutChanged();
}

SecurityTableModel::RuleData* SecurityTableModel::dataFromRow( int nRow ) const
{
	if ( nRow < m_vNodes.size() && nRow >= 0 )
	{
		return m_vNodes[nRow];
	}
	else
	{
		return NULL;
	}
}

/**
 * @brief SecurityTableModel::triggerRuleRemoval
 * @param nIndex
 */
void SecurityTableModel::triggerRuleRemoval( int nIndex )
{
	Q_ASSERT( nIndex >= 0 && nIndex < m_vNodes.size() );

	securityManager.remove( m_vNodes[nIndex]->rule() );
}

/**
 * @brief clear removes all information from the GUI.
 */
void SecurityTableModel::clear()
{
	if ( m_vNodes.size() )
	{
		beginRemoveRows( QModelIndex(), 0, ( int )m_vNodes.size() - 1 );

		RuleData** pData = m_vNodes.data();
		for ( VectorPos n = 0, nSize = m_vNodes.size(); n < nSize; ++n )
		{
			delete pData[n];
		}

		m_vNodes.clear();

		endRemoveRows();
	}
}

/**
 * @brief completeRefresh does a complete refresh of all rules.
 */
void SecurityTableModel::completeRefresh()
{
	if ( m_bShutDown )
	{
		return;
	}

	if ( m_nRuleInfo )
	{
		// prevent two refreshes from mixing
		QMetaObject::invokeMethod( this, "completeRefresh", Qt::QueuedConnection );
		return;
	}

	// Remove all rules.
	clear();

	// This slot is automatically disconnected once all rules have been recieved once.
	connect( &securityManager, &Security::Manager::ruleInfo, this,
			 &SecurityTableModel::recieveRuleInfo, Qt::QueuedConnection );

	// Request getting the rules back from the Security Manager.
	m_nRuleInfo = securityManager.requestRuleInfo();

	// prevent unnecessary reallocations
	m_vNodes.reserve( 2 * m_nRuleInfo );

	if ( !m_nRuleInfo )
	{
		// In case we don't need to recieve any rules after all.
		disconnect( &securityManager, &Security::Manager::ruleInfo,
					this, &SecurityTableModel::recieveRuleInfo );
	}
}

/**
 * @brief securityStartUpFinished initializes the GUI once Security has started.
 */
void SecurityTableModel::securityStartUpFinished()
{
	// register necessary meta types before using them
	securityManager.registerMetaTypes();

	connect( &securityManager, &Security::Manager::ruleAdded, this,
			 &SecurityTableModel::addRule, Qt::QueuedConnection );

	connect( &securityManager, &Security::Manager::ruleRemoved, this,
			 &SecurityTableModel::removeRule, Qt::QueuedConnection );

	// This handles GUI updates on rule changes.
	connect( &securityManager, &Security::Manager::ruleUpdated, this,
			 &SecurityTableModel::updateRule, Qt::QueuedConnection );

	// Prepare GUI for closing
	connect( mainWindow, &CWinMain::shutDown, this, &SecurityTableModel::onShutdown );

	// This needs to be called to make sure that all rules added to the securityManager before this
	// part of the GUI is loaded are properly added to the model.
	completeRefresh();
}

void SecurityTableModel::recieveRuleInfo( Rule* pRule )
{
	--m_nRuleInfo;

	// This handles disconnecting the ruleInfo signal after a completeRefresh() has been finished.
	if ( !m_nRuleInfo )
	{
		disconnect( &securityManager, &Security::Manager::ruleInfo,
					this, &SecurityTableModel::recieveRuleInfo );
	}

	addRule( pRule );
}

/**
 * @brief SecurityTableModel::addRule adds a rule to the GUI.
 * @param pRule : the rule
 */
void SecurityTableModel::addRule( Rule* pRule )
{
	if ( m_bShutDown )
	{
		return;
	}

#ifdef _DEBUG
	Q_ASSERT( pRule );
	Q_ASSERT( securityManager.check( pRule ) );
	verifySorting();
#endif // _DEBUG

	insert( new RuleData( pRule, m_pIcons ) );

#ifdef _DEBUG
	verifySorting();
#endif
}

/**
 * @brief SecurityTableModel::removeRule removes a rule from the table model.
 * This is to be triggered AFTER the rule has been removed from the manager by the manager.
 * @param pRule : the rule
 */
void SecurityTableModel::removeRule( SharedRulePtr pRule )
{
	if ( m_bShutDown )
	{
		return;
	}

	const VectorPos nSize = m_vNodes.size();

	Q_ASSERT( pRule );
	Q_ASSERT( nSize );

	const VectorPos nMax  = nSize - 1;

	RuleData  tmpData = RuleData( pRule.data(), m_pIcons );
	RuleData** pArray = m_vNodes.data();
	VectorPos    nPos = findInsertPos( &tmpData );

	VectorPos nTmp = nPos;

	// first look prior to the returned item
	while ( nTmp > 0 && pArray[--nTmp]->equals( m_nSortColumn, &tmpData ) )
	{
		if ( pArray[nTmp]->m_nID == tmpData.m_nID )
		{
			break;
		}
	}

	if ( pArray[nTmp]->m_nID == tmpData.m_nID )
	{
		// if the desired item could be found, fine
		nPos = nTmp;
	}
	else
	{
		// else if the item could not be found before, look behind
		while ( nPos < nSize && pArray[nPos]->equals( m_nSortColumn, &tmpData ) )
		{
			if ( pArray[nPos]->m_nID == tmpData.m_nID )
			{
				break;
			}
			++nPos;
		}
	}

	Q_ASSERT( nPos != nSize );

	Rule* pArrayRule  = pArray[nPos]->rule();
	Rule* pSharedRule = pRule.data();

	Q_ASSERT( *pArrayRule == *pSharedRule );

	beginRemoveRows( QModelIndex(), ( int )nPos, ( int )nPos );
	delete pArray[nPos];
	pArray[nPos] = NULL;

	// move all rules starting from position nPos one spot to the right
	memmove( pArray + nPos, pArray + nPos + 1, ( nMax - nPos ) * sizeof( RuleData* ) );
	m_vNodes.pop_back();
	endRemoveRows();
}

/**
 * @brief updateRule updates the GUI for a specified rule.
 * @param nRuleID : the ID of the rule
 */
void SecurityTableModel::updateRule( ID nRuleID )
{
	if ( m_bShutDown )
	{
		return;
	}

	QModelIndexList uplist;

	const VectorPos nRuleRowPos = find( nRuleID );

	Q_ASSERT( nRuleRowPos != m_vNodes.size() );

	bool bSort = false;
	securityManager.m_oRWLock.lockForRead();
	if ( m_vNodes[nRuleRowPos]->update( ( int )nRuleRowPos, m_nSortColumn, uplist, this ) )
	{
		bSort = true;
	}
	securityManager.m_oRWLock.unlock();

	// if necessary adjust container order (also updates view)
	if ( bSort )
	{
		sort( m_nSortColumn, m_nSortOrder, false );
	}
	// update view for all changed model indexes
	else if ( !uplist.isEmpty() )
	{
		QAbstractItemView* pView = qobject_cast< QAbstractItemView* >( m_oContainer );

		if ( pView )
		{
			foreach ( const QModelIndex & index, uplist )
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
	if ( m_bShutDown )
	{
		return;
	}

	QModelIndexList uplist;

	bool bSort = false;
	securityManager.m_oRWLock.lockForRead();
	for ( VectorPos n = 0, nMax = m_vNodes.size(); n < nMax; ++n )
	{
		if ( m_vNodes[n]->update( ( int )n, m_nSortColumn, uplist, this ) )
		{
			bSort = true;
		}
	}
	securityManager.m_oRWLock.unlock();

	if ( bSort )
	{
		sort( m_nSortColumn, m_nSortOrder, false );
	}
	else
	{
		if ( !uplist.isEmpty() )
		{
			QAbstractItemView* pView = qobject_cast< QAbstractItemView* >( m_oContainer );

			if ( pView )
			{
				foreach ( const QModelIndex & index, uplist )
				{
					pView->update( index );
				}
			}
		}
	}
}

/**
 * @brief onShutdown handles shutting down.
 */
void SecurityTableModel::onShutdown()
{
	m_bShutDown = true;
	clear();
}

SecurityTableModel::VectorPos SecurityTableModel::find( ID nRuleID ) const
{
	const VectorPos nSize = m_vNodes.size();
	const RuleData* const * const pArray = m_vNodes.data();

	for ( VectorPos nPos = 0; nPos < nSize; ++nPos )
	{
		if ( pArray[nPos]->m_nID == nRuleID )
		{
			return nPos;
		}
	}

	return nSize;
}

SecurityTableModel::VectorPos SecurityTableModel::findInsertPos( const RuleData* const pData ) const
{
	Q_ASSERT( pData );

	if ( m_vNodes.empty() )
	{
		return 0;
	}

#ifdef _DEBUG
	verifySorting();
#endif // _DEBUG

	SecurityTableModelCmp oCmp = SecurityTableModelCmp( m_nSortColumn, m_nSortOrder,
														m_bSecondarySortOrder );

	const VectorPos nSize = m_vNodes.size();
	const RuleData* const * const pArray = m_vNodes.data();

#ifdef _DEBUG
	VectorPos i = 0;
	// find precise position
	while ( i < nSize && pArray[i]->m_nID != pData->m_nID )
	{
		++i;
	}

	// find theoretical position
	if ( i == nSize )
	{
		i = 0;
		while ( i < nSize && oCmp( pArray[i], pData ) )
		{
			++i;
		}
	}

	qDebug() << "Size: " << nSize << " i: " << i;
#endif // _DEBUG

	VectorPos nMiddle, nHalf, nBegin = 0;
	VectorPos n = nSize;

	// Note: In the comments nPos is the theoretical position pRule.
	while ( n > 0 )
	{
		nHalf = n >> 1;

		nMiddle = nBegin + nHalf;

		if ( oCmp( pData, pArray[nMiddle] ) )
		{
			// at this point: nPos >= nBegin && nPos < nMiddle
			n = nHalf;
			// at this point: nPos >= nBegin && nPos < nBegin + n
		}
		else
		{
			// (!(a < b) && !(b < a)) == (a == b)
			if ( !oCmp( pArray[nMiddle], pData ) )
			{
				// We have found an element whose sorting value is identical to the one we're
				// looking for. Note that in case an identical element exists within the vector,
				// this return value needs not be the location of that element.


				// We return the position right of the element with an identical sorting key, thus
				// reducing the number of items that have to be moved to the right on insertions.
				return ++nMiddle;
			}
			// at this point: nPos > nMiddle && nPos <= nBegin + n

			nBegin = nMiddle + 1;
			n -= nHalf + 1;

			// at this point: nPos >= nBegin && nPos <= nBegin + n
		}

#ifdef _DEBUG
		if ( i < nBegin || i > nBegin + n )
		{
			qDebug() << "nBegin: " << nBegin << " nBegin + n: " << nBegin + n;

			Q_ASSERT( i >= nBegin );
			Q_ASSERT( i <= nBegin + n );
		}
#endif // _DEBUG

		// at this point: nPos >= nBegin && nPos <= nBegin + n
	}

	// REMOVE for Quazaa 1.0
#ifdef _DEBUG
	if ( nBegin != nSize )
	{
		bool bNBeginBigger = oCmp( pData, pArray[nBegin] );

		if ( !bNBeginBigger )
		{
			Q_ASSERT( bNBeginBigger );
		}
	}

	if ( nBegin )
	{
		bool bLeftSmaller  = oCmp( pArray[nBegin - 1], pData );

		if ( !bLeftSmaller )
		{
			Q_ASSERT( bLeftSmaller );
		}
	}
#endif // _DEBUG

#ifdef _DEBUG
		Q_ASSERT( i == nBegin );
#endif // _DEBUG

	return nBegin;
}

void SecurityTableModel::insert( RuleData* pRule )
{
	if ( m_vNodes.empty() )
	{
		beginInsertRows( QModelIndex(), 0, 0 );
		m_vNodes.push_back( pRule );
		endInsertRows();
	}
	else
	{
		const VectorPos nMax = m_vNodes.size();
		const VectorPos nPos = findInsertPos( pRule );

		Q_ASSERT( nPos >= 0 && nPos <= nMax );

		beginInsertRows( QModelIndex(), ( int )nPos, ( int )nPos );
		m_vNodes.push_back( NULL );

		RuleData** pArray = m_vNodes.data();
		// move all rules starting from position nPos one spot to the right
		memmove( pArray + nPos + 1, pArray + nPos, ( nMax - nPos ) * sizeof( RuleData* ) );

		pArray[nPos] = pRule;
		endInsertRows();
	}

#ifdef _DEBUG
		verifySorting();
#endif // _DEBUG
}

#ifdef _DEBUG
void SecurityTableModel::verifySorting() const
{
	SecurityTableModelCmp oCmp = SecurityTableModelCmp( m_nSortColumn, m_nSortOrder,
														m_bSecondarySortOrder );

	const RuleData* const * const pArray = m_vNodes.data();
	for ( VectorPos n = 0, nSize = m_vNodes.size(); n + 1 < nSize; ++n )
	{
		// pArray[n+1] may not be smaller than pArray[n]
		Q_ASSERT( !oCmp( pArray[n+1], pArray[n] ) );
	}
}
#endif
