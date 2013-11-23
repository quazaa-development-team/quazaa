#ifndef DIALOGIRCSETTINGS_H
#define DIALOGIRCSETTINGS_H

#include <QDialog>
#include <QWizardPage>

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace ShortcutColumns {
	enum Columns {
		Description,
		Shortcut
	};
}

namespace AliasColumns {
	enum Columns {
		Alias,
		Command
	};
}

namespace Ui {
	class CDialogIrcSettings;
}

class CDialogIrcSettings : public QDialog
{
	Q_OBJECT

public:
	explicit CDialogIrcSettings(QWidget *parent = 0);
	~CDialogIrcSettings();

private:
	Ui::CDialogIrcSettings *ui;

	QModelIndex addAliasRow(const QString& alias, const QString& command);

	QStandardItemModel* sourceModel;
	QSortFilterProxyModel* proxyModel;

	QColor m_ColorWhite;
	QColor m_ColorBlack;
	QColor m_ColorBlue;
	QColor m_ColorGreen;
	QColor m_ColorRed;
	QColor m_ColorBrown;
	QColor m_ColorPurple;
	QColor m_ColorOrange;
	QColor m_ColorYellow;
	QColor m_ColorLightGreen;
	QColor m_ColorCyan;
	QColor m_ColorLightCyan;
	QColor m_ColorLightBlue;
	QColor m_ColorPink;
	QColor m_ColorGray;
	QColor m_ColorLightGray;

	QColor m_ColorDefault;
	QColor m_ColorEvent;
	QColor m_ColorNotice;
	QColor m_ColorAction;
	QColor m_ColorInactive;
	QColor m_ColorAlert;
	QColor m_ColorHighlight;
	QColor m_ColorTimestamp;
	QColor m_ColorLink;

public slots:
	void saveSettings();
	void loadSettings();

private slots:
	void on_pushButtonOK_clicked();
	void enableApply();
	void addAlias();
	void onAliasClicked(const QModelIndex& index);
	void on_toolButtonChangeColorWhite_clicked();
	void on_toolButtonChangeColorBlack_clicked();
	void on_toolButtonChangeColorBlue_clicked();
	void on_toolButtonChangeColorGreen_clicked();
	void on_toolButtonChangeColorRed_clicked();
	void on_toolButtonChangeColorBrown_clicked();
	void on_toolButtonChangeColorPurple_clicked();
	void on_toolButtonChangeColorOrange_clicked();
	void on_toolButtonChangeColorYellow_clicked();
	void on_toolButtonChangeColorLightGreen_clicked();
	void on_toolButtonChangeColorCyan_clicked();
	void on_toolButtonChangeColorLightCyan_clicked();
	void on_toolButtonChangeColorLightBlue_clicked();
	void on_toolButtonChangeColorPink_clicked();
	void on_toolButtonChangeColorGray_clicked();
	void on_toolButtonChangeColorLightGray_clicked();
	void on_toolButtonChangeColorDefault_clicked();
	void on_toolButtonChangeColorEvent_clicked();
	void on_toolButtonChangeColorNotice_clicked();
	void on_toolButtonChangeColorAction_clicked();
	void on_toolButtonChangeColorInactive_clicked();
	void on_toolButtonChangeColorAlert_clicked();
	void on_toolButtonChangeColorHighlight_clicked();
	void on_toolButtonChangeColorTimestamp_clicked();
	void on_toolButtonChangeColorLink_clicked();
};

#endif // DIALOGIRCSETTINGS_H
