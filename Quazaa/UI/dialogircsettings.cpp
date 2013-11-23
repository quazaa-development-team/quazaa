#include "dialogircsettings.h"
#include "ui_dialogircsettings.h"

#include "quazaasettings.h"

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QComboBox>
#include <QDebug>
#include <QColorDialog>

class ShortcutItemDelegate : public QStyledItemDelegate
{
public:
	ShortcutItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) { }
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& opt, const QModelIndex& index) const {
		QWidget* editor = 0;
		if (index.column() == ShortcutColumns::Shortcut)
			editor = QStyledItemDelegate::createEditor(parent, opt, index);
		return editor;
	}
};

class SortFilterProxyModel : public QSortFilterProxyModel
{
public:
	SortFilterProxyModel(QObject* parent = 0) : QSortFilterProxyModel(parent)
	{
	}

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
	{
		QModelIndex aliasIndex = sourceModel()->index(sourceRow, AliasColumns::Alias, sourceParent);
		QModelIndex commandIndex = sourceModel()->index(sourceRow, AliasColumns::Command, sourceParent);
		QString alias, command;
		if (!aliasIndex.isValid() || !commandIndex.isValid()) // the column may not exist
			alias = aliasIndex.data(filterRole()).toString();
		command = commandIndex.data(filterRole()).toString();

		return alias.contains(filterRegExp()) || command.contains(filterRegExp());
		return false;
	}
};

class CloseDelegate : public QStyledItemDelegate
{
public:
	CloseDelegate(QObject* parent = 0) : QStyledItemDelegate(parent)
	{
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const_cast<QStyleOptionViewItem&>(option).decorationPosition = QStyleOptionViewItem::Right;
		QStyledItemDelegate::paint(painter, option, index);
	}
};

CDialogIrcSettings::CDialogIrcSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CDialogIrcSettings)
{
	ui->setupUi(this);

	setWindowFilePath(tr("Settings"));

//TODO:	ui->treeWidgetColors->setItemDelegate(new ColorItemDelegate(ui->treeWidgetColors));
	ui->treeWidgetShortcuts->setItemDelegate(new ShortcutItemDelegate(ui->treeWidgetShortcuts));
//TODO:		ui->treeWidgetColors->header()->setSectionResizeMode(ColorColumns::Message, QHeaderView::ResizeToContents);
	ui->treeWidgetShortcuts->header()->setSectionResizeMode(ShortcutColumns::Description, QHeaderView::ResizeToContents);
	ui->treeWidgetShortcuts->expandItem(ui->treeWidgetShortcuts->topLevelItem(0));
	ui->treeWidgetShortcuts->expandItem(ui->treeWidgetShortcuts->topLevelItem(1));
	proxyModel = new SortFilterProxyModel(ui->treeViewAliases);
	sourceModel = new QStandardItemModel(proxyModel);
	proxyModel->setSourceModel(sourceModel);
	ui->treeViewAliases->setModel(proxyModel);
	ui->treeViewAliases->setItemDelegateForColumn(2, new CloseDelegate(ui->treeViewAliases));

	loadSettings();

	connect(ui->checkBoxConnectOnStartup, SIGNAL(toggled(bool)), this, SLOT(enableApply()));
	connect(ui->fontComboBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(enableApply()));
	connect(ui->spinBoxFontSize, SIGNAL(valueChanged(int)), this, SLOT(enableApply()));
	connect(ui->checkBoxStripNicks, SIGNAL(toggled(bool)), this, SLOT(enableApply()));
	connect(ui->checkBoxHighlightSounds, SIGNAL(toggled(bool)), this, SLOT(enableApply()));
	connect(ui->checkBoxTimeStamp, SIGNAL(toggled(bool)), this, SLOT(enableApply()));
	connect(ui->checkBoxDarkTheme, SIGNAL(toggled(bool)), this, SLOT(enableApply()));
	connect(ui->spinBoxBlockCount, SIGNAL(valueChanged(int)), this, SLOT(enableApply()));
//TODO:		connect(ui->treeWidgetColors, SIGNAL(clicked(QModelIndex)), this, SLOT(enableApply()));
	connect(ui->treeWidgetShortcuts, SIGNAL(clicked(QModelIndex)), this, SLOT(enableApply()));
	connect(ui->toolButtonAddAliases, SIGNAL(clicked()), this, SLOT(addAlias()));
	connect(ui->treeViewAliases, SIGNAL(clicked(QModelIndex)), this, SLOT(onAliasClicked(QModelIndex)));
	connect(ui->treeViewAliases, SIGNAL(activated(QModelIndex)), this, SLOT(enableApply()));
	connect(ui->lineEditAliasesFilter, SIGNAL(textEdited(QString)), proxyModel, SLOT(setFilterWildcard(QString)));

	connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(saveSettings()));
	ui->pushButtonApply->setEnabled(false);
}

CDialogIrcSettings::~CDialogIrcSettings()
{
	delete ui;
}

QModelIndex CDialogIrcSettings::addAliasRow(const QString &alias, const QString &command)
{
	QStandardItem* aliasItem = new QStandardItem(alias);
	QStandardItem* commandItem = new QStandardItem(command);
	QStandardItem* closeItem = new QStandardItem(QPixmap(":/Resource/Generic/Exit.png"), QString());
	closeItem->setFlags(closeItem->flags() & ~Qt::ItemIsEditable);
	sourceModel->appendRow(QList<QStandardItem*>() << aliasItem << commandItem << closeItem);
	return proxyModel->mapFromSource(aliasItem->index());
}

void CDialogIrcSettings::saveSettings()
{
	quazaaSettings.Chat.Font = ui->fontComboBox->currentFont();
	quazaaSettings.Chat.Font.setPointSize(ui->spinBoxFontSize->value());
	quazaaSettings.Chat.ConnectOnStartup = ui->checkBoxConnectOnStartup->isChecked();
	quazaaSettings.Chat.Scrollback = ui->spinBoxBlockCount->value();
	quazaaSettings.Chat.ShowTimestamp = ui->checkBoxTimeStamp->isChecked();
	quazaaSettings.Chat.HighlightSounds = ui->checkBoxHighlightSounds->isChecked();
	quazaaSettings.Chat.TimestampFormat = ui->lineEditTimestampFormat->text();

	quazaaSettings.Chat.DarkTheme = ui->checkBoxDarkTheme->isChecked();
	quazaaSettings.Chat.Colors[IrcColorType::White] = m_ColorWhite.name();
	quazaaSettings.Chat.Colors[IrcColorType::Black] = m_ColorBlack.name();
	quazaaSettings.Chat.Colors[IrcColorType::Blue] = m_ColorBlue.name();
	quazaaSettings.Chat.Colors[IrcColorType::Green] = m_ColorGreen.name();
	quazaaSettings.Chat.Colors[IrcColorType::Red] = m_ColorRed.name();
	quazaaSettings.Chat.Colors[IrcColorType::Brown] = m_ColorBrown.name();
	quazaaSettings.Chat.Colors[IrcColorType::Purple] = m_ColorPurple.name();
	quazaaSettings.Chat.Colors[IrcColorType::Orange] = m_ColorOrange.name();
	quazaaSettings.Chat.Colors[IrcColorType::Yellow] = m_ColorYellow.name();
	quazaaSettings.Chat.Colors[IrcColorType::LightGreen] = m_ColorLightGreen.name();
	quazaaSettings.Chat.Colors[IrcColorType::Cyan] = m_ColorCyan.name();
	quazaaSettings.Chat.Colors[IrcColorType::LightCyan] = m_ColorLightCyan.name();
	quazaaSettings.Chat.Colors[IrcColorType::LightBlue] = m_ColorLightBlue.name();
	quazaaSettings.Chat.Colors[IrcColorType::Pink] = m_ColorPink.name();
	quazaaSettings.Chat.Colors[IrcColorType::Gray] = m_ColorGray.name();
	quazaaSettings.Chat.Colors[IrcColorType::LightGray] = m_ColorLightGray.name();

	quazaaSettings.Chat.Colors[IrcColorType::Default] = m_ColorDefault.name();
	quazaaSettings.Chat.Colors[IrcColorType::Event] = m_ColorEvent.name();
	quazaaSettings.Chat.Colors[IrcColorType::Notice] = m_ColorNotice.name();
	quazaaSettings.Chat.Colors[IrcColorType::Action] = m_ColorAction.name();
	quazaaSettings.Chat.Colors[IrcColorType::Inactive] = m_ColorInactive.name();
	quazaaSettings.Chat.Colors[IrcColorType::Alert] = m_ColorAlert.name();
	quazaaSettings.Chat.Colors[IrcColorType::Highlight] = m_ColorHighlight.name();
	quazaaSettings.Chat.Colors[IrcColorType::TimeStamp] = m_ColorTimestamp.name();
	quazaaSettings.Chat.Colors[IrcColorType::Link] = m_ColorLink.name();

	for (int i = IrcShortcutType::NextView; i <= IrcShortcutType::MostActiveView; ++i) {
		QTreeWidgetItem* item = ui->treeWidgetShortcuts->topLevelItem(i);
		quazaaSettings.Chat.Shortcuts.insert(i, item->text(ShortcutColumns::Shortcut));
	}

	QMap<QString, QString> aliases;
	for (int i = 0; i < sourceModel->rowCount(); ++i)
	{
		QString alias = sourceModel->item(i, AliasColumns::Alias)->text();
		QString command = sourceModel->item(i, AliasColumns::Command)->text();
		if (!alias.isEmpty() && !command.isEmpty())
			aliases[alias] = command;
	}
	quazaaSettings.Chat.Aliases = aliases;

	quazaaSettings.saveChat();

	ui->pushButtonApply->setEnabled(false);
}

void CDialogIrcSettings::loadSettings()
{
	ui->fontComboBox->setCurrentFont(quazaaSettings.Chat.Font);
	ui->spinBoxFontSize->setValue(quazaaSettings.Chat.Font.pointSize());
	ui->checkBoxConnectOnStartup->setChecked(quazaaSettings.Chat.ConnectOnStartup);
	ui->spinBoxBlockCount->setValue(quazaaSettings.Chat.Scrollback);
	ui->checkBoxTimeStamp->setChecked(quazaaSettings.Chat.ShowTimestamp);
	ui->checkBoxHighlightSounds->setChecked(quazaaSettings.Chat.HighlightSounds);
	ui->lineEditTimestampFormat->setText(quazaaSettings.Chat.TimestampFormat);

	ui->checkBoxDarkTheme->setChecked(quazaaSettings.Chat.DarkTheme);

	m_ColorWhite = quazaaSettings.Chat.Colors[IrcColorType::White];
	ui->labelColorWhite->setStyleSheet(QString("background-color: %1;").arg(m_ColorWhite.name()));
	m_ColorBlack = quazaaSettings.Chat.Colors[IrcColorType::Black];
	ui->labelColorBlack->setStyleSheet(QString("background-color: %1;").arg(m_ColorBlack.name()));
	m_ColorBlue = quazaaSettings.Chat.Colors[IrcColorType::Blue];
	ui->labelColorBlue->setStyleSheet(QString("background-color: %1;").arg(m_ColorBlue.name()));
	m_ColorGreen = quazaaSettings.Chat.Colors[IrcColorType::Green];
	ui->labelColorGreen->setStyleSheet(QString("background-color: %1;").arg(m_ColorGreen.name()));
	m_ColorRed = quazaaSettings.Chat.Colors[IrcColorType::Red];
	ui->labelColorRed->setStyleSheet(QString("background-color: %1;").arg(m_ColorRed.name()));
	m_ColorBrown = quazaaSettings.Chat.Colors[IrcColorType::Brown];
	ui->labelColorBrown->setStyleSheet(QString("background-color: %1;").arg(m_ColorBrown.name()));
	m_ColorPurple = quazaaSettings.Chat.Colors[IrcColorType::Purple];
	ui->labelColorPurple->setStyleSheet(QString("background-color: %1;").arg(m_ColorPurple.name()));
	m_ColorOrange = quazaaSettings.Chat.Colors[IrcColorType::Orange];
	ui->labelColorOrange->setStyleSheet(QString("background-color: %1;").arg(m_ColorOrange.name()));
	m_ColorYellow = quazaaSettings.Chat.Colors[IrcColorType::Yellow];
	ui->labelColorYellow->setStyleSheet(QString("background-color: %1;").arg(m_ColorYellow.name()));
	m_ColorLightGreen = quazaaSettings.Chat.Colors[IrcColorType::LightGreen];
	ui->labelColorLightGreen->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightGreen.name()));
	m_ColorCyan = quazaaSettings.Chat.Colors[IrcColorType::Cyan];
	ui->labelColorCyan->setStyleSheet(QString("background-color: %1;").arg(m_ColorCyan.name()));
	m_ColorLightCyan = quazaaSettings.Chat.Colors[IrcColorType::LightCyan];
	ui->labelColorLightCyan->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightCyan.name()));
	m_ColorLightBlue = quazaaSettings.Chat.Colors[IrcColorType::LightBlue];
	ui->labelColorLightBlue->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightBlue.name()));
	m_ColorPink = quazaaSettings.Chat.Colors[IrcColorType::Pink];
	ui->labelColorPink->setStyleSheet(QString("background-color: %1;").arg(m_ColorPink.name()));
	m_ColorGray = quazaaSettings.Chat.Colors[IrcColorType::Gray];
	ui->labelColorGray->setStyleSheet(QString("background-color: %1;").arg(m_ColorGray.name()));
	m_ColorLightGray = quazaaSettings.Chat.Colors[IrcColorType::LightGray];
	ui->labelColorLightGray->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightGray.name()));

	m_ColorDefault = quazaaSettings.Chat.Colors[IrcColorType::Default];
	ui->labelColorDefault->setStyleSheet(QString("background-color: %1;").arg(m_ColorDefault.name()));
	m_ColorEvent = quazaaSettings.Chat.Colors[IrcColorType::Event];
	ui->labelColorEvent->setStyleSheet(QString("background-color: %1;").arg(m_ColorEvent.name()));
	m_ColorNotice = quazaaSettings.Chat.Colors[IrcColorType::Notice];
	ui->labelColorNotice->setStyleSheet(QString("background-color: %1;").arg(m_ColorNotice.name()));
	m_ColorAction = quazaaSettings.Chat.Colors[IrcColorType::Action];
	ui->labelColorAction->setStyleSheet(QString("background-color: %1;").arg(m_ColorAction.name()));
	m_ColorInactive = quazaaSettings.Chat.Colors[IrcColorType::Inactive];
	ui->labelColorInactive->setStyleSheet(QString("background-color: %1;").arg(m_ColorInactive.name()));
	m_ColorAlert = quazaaSettings.Chat.Colors[IrcColorType::Alert];
	ui->labelColorAlert->setStyleSheet(QString("background-color: %1;").arg(m_ColorAlert.name()));
	m_ColorHighlight = quazaaSettings.Chat.Colors[IrcColorType::Highlight];
	ui->labelColorHighlight->setStyleSheet(QString("background-color: %1;").arg(m_ColorHighlight.name()));
	m_ColorTimestamp = quazaaSettings.Chat.Colors[IrcColorType::TimeStamp];
	ui->labelColorTimestamp->setStyleSheet(QString("background-color: %1;").arg(m_ColorTimestamp.name()));
	m_ColorLink = quazaaSettings.Chat.Colors[IrcColorType::Link];
	ui->labelColorLink->setStyleSheet(QString("background-color: %1;").arg(m_ColorLink.name()));

	for (int i = IrcShortcutType::NextView; i <= IrcShortcutType::MostActiveView; ++i) {
		QTreeWidgetItem* item = ui->treeWidgetShortcuts->topLevelItem(i);
		item->setText(ShortcutColumns::Shortcut, quazaaSettings.Chat.Shortcuts.value(i));
	}

	sourceModel->clear();
	sourceModel->setHorizontalHeaderLabels(QStringList() << tr("Alias") << tr("Command") << QString());

	QMapIterator<QString, QString> aliasIt(quazaaSettings.Chat.Aliases);
	while (aliasIt.hasNext())
	{
		aliasIt.next();
		QString alias = aliasIt.key();
		QString command = aliasIt.value();
		//qDebug() << "Alias: " << alias << " Command: " << command;
		if (!alias.isEmpty() && !command.isEmpty())
			addAliasRow(alias, command);
	}

	ui->treeViewAliases->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui->treeViewAliases->header()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui->treeViewAliases->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

void CDialogIrcSettings::on_pushButtonOK_clicked()
{
	if(ui->pushButtonApply->isEnabled())
		saveSettings();
	accept();
}

void CDialogIrcSettings::enableApply()
{
	ui->pushButtonApply->setEnabled(true);
}

void CDialogIrcSettings::addAlias()
{
	ui->lineEditAliasesFilter->clear();
	QModelIndex index = addAliasRow(QString(), QString());
	ui->treeViewAliases->setCurrentIndex(index);
	ui->treeViewAliases->edit(index);
	enableApply();
}

void CDialogIrcSettings::onAliasClicked(const QModelIndex &index)
{
	if (index.column() == 2) {
		qDeleteAll(sourceModel->takeRow(proxyModel->mapToSource(index).row()));
		enableApply();
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorWhite_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorWhite);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorWhite = dlgColor->currentColor();
		ui->labelColorWhite->setStyleSheet(QString("background-color: %1;").arg(m_ColorWhite.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorBlack_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorBlack);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorBlack = dlgColor->currentColor();
		ui->labelColorBlack->setStyleSheet(QString("background-color: %1;").arg(m_ColorBlack.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorBlue_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorBlue);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorBlue = dlgColor->currentColor();
		ui->labelColorBlue->setStyleSheet(QString("background-color: %1;").arg(m_ColorBlue.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorGreen_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorGreen);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorGreen = dlgColor->currentColor();
		ui->labelColorGreen->setStyleSheet(QString("background-color: %1;").arg(m_ColorGreen.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorRed_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorRed);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorRed = dlgColor->currentColor();
		ui->labelColorRed->setStyleSheet(QString("background-color: %1;").arg(m_ColorRed.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorBrown_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorBrown);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorBrown = dlgColor->currentColor();
		ui->labelColorBrown->setStyleSheet(QString("background-color: %1;").arg(m_ColorBrown.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorPurple_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorPurple);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorPurple = dlgColor->currentColor();
		ui->labelColorPurple->setStyleSheet(QString("background-color: %1;").arg(m_ColorPurple.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorOrange_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorOrange);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorOrange = dlgColor->currentColor();
		ui->labelColorOrange->setStyleSheet(QString("background-color: %1;").arg(m_ColorOrange.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorYellow_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorYellow);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorYellow = dlgColor->currentColor();
		ui->labelColorYellow->setStyleSheet(QString("background-color: %1;").arg(m_ColorYellow.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorLightGreen_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorLightGreen);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorLightGreen = dlgColor->currentColor();
		ui->labelColorLightGreen->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightGreen.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorCyan_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorCyan);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorCyan = dlgColor->currentColor();
		ui->labelColorCyan->setStyleSheet(QString("background-color: %1;").arg(m_ColorCyan.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorLightCyan_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorLightCyan);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorLightCyan = dlgColor->currentColor();
		ui->labelColorLightCyan->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightCyan.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorLightBlue_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorLightBlue);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorLightBlue = dlgColor->currentColor();
		ui->labelColorLightBlue->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightBlue.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorPink_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorPink);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorPink = dlgColor->currentColor();
		ui->labelColorPink->setStyleSheet(QString("background-color: %1;").arg(m_ColorPink.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorGray_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorGray);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorGray = dlgColor->currentColor();
		ui->labelColorGray->setStyleSheet(QString("background-color: %1;").arg(m_ColorGray.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorLightGray_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorLightGray);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorLightGray = dlgColor->currentColor();
		ui->labelColorLightGray->setStyleSheet(QString("background-color: %1;").arg(m_ColorLightGray.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorDefault_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorDefault);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorDefault = dlgColor->currentColor();
		ui->labelColorDefault->setStyleSheet(QString("background-color: %1;").arg(m_ColorDefault.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorEvent_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorEvent);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorEvent = dlgColor->currentColor();
		ui->labelColorEvent->setStyleSheet(QString("background-color: %1;").arg(m_ColorEvent.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorNotice_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorNotice);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorNotice = dlgColor->currentColor();
		ui->labelColorNotice->setStyleSheet(QString("background-color: %1;").arg(m_ColorNotice.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorAction_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorAction);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorAction = dlgColor->currentColor();
		ui->labelColorAction->setStyleSheet(QString("background-color: %1;").arg(m_ColorAction.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorInactive_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorInactive);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorInactive = dlgColor->currentColor();
		ui->labelColorInactive->setStyleSheet(QString("background-color: %1;").arg(m_ColorInactive.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorAlert_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorAlert);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorAlert = dlgColor->currentColor();
		ui->labelColorAlert->setStyleSheet(QString("background-color: %1;").arg(m_ColorAlert.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorHighlight_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorHighlight);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorHighlight = dlgColor->currentColor();
		ui->labelColorHighlight->setStyleSheet(QString("background-color: %1;").arg(m_ColorHighlight.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorTimestamp_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorTimestamp);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorTimestamp = dlgColor->currentColor();
		ui->labelColorTimestamp->setStyleSheet(QString("background-color: %1;").arg(m_ColorTimestamp.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}

void CDialogIrcSettings::on_toolButtonChangeColorLink_clicked()
{
	QColorDialog* dlgColor = new QColorDialog(this);
	dlgColor->setCurrentColor(m_ColorLink);

	int result = dlgColor->exec();

	if(result == QDialog::Accepted) {
		m_ColorLink = dlgColor->currentColor();
		ui->labelColorLink->setStyleSheet(QString("background-color: %1;").arg(m_ColorLink.name()));

		ui->pushButtonApply->setEnabled(true);
	}
}
