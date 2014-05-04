#ifndef SKINSETTINGS_H
#define SKINSETTINGS_H

#include <QObject>
#include <QColor>

class SkinSettings : public QObject
{
	Q_OBJECT
public:
	explicit SkinSettings( QObject* parent = 0 );

	// Standard Items
	QString standardItems;

	// List Views
	QString listViews;

	// Sidebar
	QString sidebarBackground;
	QString sidebarTaskBackground;
	QString sidebarTaskHeader;
	QString addSearchButton;

	// Toolbars
	QString toolbars;
	QString navigationToolbar;

	// Headers
	QString genericHeader;
	QString dialogHeader;
	QString taskHeader;

	// Media
	QString seekSlider;
	QString volumeSlider;
	QString mediaToolbar;

	// Specialised Tab Widgets
	QString libraryNavigator;
	QString tabSearches;

	// Colors
	QColor logColorInformation;
	QString logWeightInformation;
	QColor logColorSecurity;
	QString logWeightSecurity;
	QColor logColorNotice;
	QString logWeightNotice;
	QColor logColorDebug;
	QString logWeightDebug;
	QColor	logColorWarning;
	QString logWeightWarning;
	QColor logColorError;
	QString logWeightError;
	QColor logColorCritical;
	QString logWeightCritical;
	QColor listsColorNormal;
	int listsWeightNormal;
	QColor listsColorActive;
	int listsWeightActive;
	QColor listsColorSpecial;
	int listsWeightSpecial;
	QColor listsColorHighlighted;
	int listsWeightHighlighted;
signals:

public slots:
	void setGenericSkin();
	void setAeroSkin();
	void setLunaskin();
};

extern SkinSettings skinSettings;
#endif // SKINSETTINGS_H
