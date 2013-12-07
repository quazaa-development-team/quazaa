#ifndef SECURITYTABLEMODEL_H
#define SECURITYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QIcon>

#include "securitymanager.h"

typedef Security::ID                    ID;
typedef Security::Rule                  Rule;
typedef Security::RuleType::Type        Type;
typedef Security::RuleAction::Action    Action;
typedef Security::SharedRulePtr         SharedRulePtr;

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
		const Type      m_nType;
		Action          m_nAction;
		quint32         m_nToday;
		quint32         m_nTotal;
		quint32         m_tExpire;    // UTC time in seconds
		QString         m_sContent;
		QString         m_sComment;
		bool            m_bContent;
		const QIcon*    m_piAction;
		bool            m_bRemoving;  // if set to true, don't access m_pRule any more
		bool            m_bAutomatic;

		RuleData(Rule* pRule, SecurityTableModel* pModel);
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
		bool update(int nRow, int nSortCol, QModelIndexList& lToUpdate, SecurityTableModel* pModel);
		QVariant data(int col) const;

		/**
		 * @brief rule allows access to the rule pointer.
		 * @return the pointer to the Rule within the manager; NULL if the rule is currently being
		 * removed.
		 */
		Rule* rule() const;

		bool lessThan(int col, const RuleData* const pOther) const;

		QString actionToString(Action nAction) const;
		QString expiryToString(quint32 tExpire) const;
	};

	typedef QSharedPointer<RuleData>    RuleDataPtr;

private:
	QWidget*        m_oContainer;
	Qt::SortOrder   m_nSortOrder;
	int             m_nSortColumn;
	bool            m_bNeedSorting;

protected:
	// TODO: use std::vector
	QList<RuleDataPtr> m_lNodes;

public:
	enum Column
	{
		CONTENT = 0,
		TYPE,
		ACTION,
		EXPIRES,
		HITS,
		COMMENT,
		_NO_OF_COLUMNS
	};

	// icons used for the different rules
	const QIcon* m_pIcons[3];

public:
	explicit SecurityTableModel(QObject* parent = 0, QWidget* container = 0);
	~SecurityTableModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent) const;
	QVariant data(const QModelIndex& index, int nRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	void sort(int column, Qt::SortOrder order);

	int find(ID nRuleID);

	RuleDataPtr dataFromRow(int nRow) const;
	//Rule* ruleFromIndex(const QModelIndex& index) const;

	void completeRefresh();

	void triggerRuleRemoval(int nIndex);

public slots:
	/**
	 * @brief SecurityTableModel::addRule adds a rule to the GUI.
	 * @param pRule : the rule
	 */
	void addRule(Rule* pRule);

	/**
	 * @brief SecurityTableModel::removeRule removes a rule from the table model.
	 * This is to be triggered AFTER the rule has been removed from the manager by the manager.
	 * @param pRule : the rule
	 */
	void removeRule(SharedRulePtr pRule);

	/**
	 * @brief updateRule updates the GUI for a specified rule.
	 * @param nRuleID : the ID of the rule
	 */
	void updateRule(ID nRuleID);

	/**
	 * @brief SecurityTableModel::updateAll updates all rules in the GUI.
	 */
	void updateAll();
};

typedef SecurityTableModel::RuleData    RuleData;
typedef SecurityTableModel::RuleDataPtr RuleDataPtr;

#endif // SECURITYTABLEMODEL_H

