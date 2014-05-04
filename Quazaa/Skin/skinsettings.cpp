#include "skinsettings.h"
#include "quazaasysinfo.h"

#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QPalette>

SkinSettings skinSettings;

SkinSettings::SkinSettings( QObject* parent ) :
	QObject( parent )
{
	QuazaaSysInfo* sysInfo = new QuazaaSysInfo();
	switch ( sysInfo->osVersion() )
	{
	case OSVersion::Linux:
		setGenericSkin();
		break;
	case OSVersion::BSD:
		setGenericSkin();
		break;
	case OSVersion::Unix:
		setGenericSkin();
		break;
	case OSVersion::MacCheetah:
		break;
	case OSVersion::MacPuma:
		break;
	case OSVersion::MacJaguar:
		break;
	case OSVersion::MacPanther:
		break;
	case OSVersion::MacTiger:
		break;
	case OSVersion::MacLeopard:
		break;
	case OSVersion::MacSnowLeopard:
		break;
	case OSVersion::Win2000:
		setGenericSkin();
		break;
	case OSVersion::WinXP:
		setLunaskin();
		break;
	case OSVersion::Win2003:
		setLunaskin();
		break;
	case OSVersion::WinVista:
		setAeroSkin();
		break;
	case OSVersion::Win7:
		setAeroSkin();
		break;
	case OSVersion::Win8:
		setAeroSkin();
		break;
	}
}

void SkinSettings::setGenericSkin()
{
	// Standard Items
	standardItems = "";

	// ListViews
	listViews =
		"QAbstractItemView { border: 1px solid rgb(217, 212, 218); color: black; background-color: white; selection-color: black; selection-background-color: white; } QAbstractItemView::item:selected { /* when user selects item using mouse or keyboard */ border: 3px solid transparent; border-image: url(:/Resource/Aero/itemHoverSelected.png); } QHeaderView::section, QHeaderView::section:checked, QHeaderView::section:selected { border-image: url(:/Resource/Aero/headerSection.png); color: black; padding-right: 15px; border: 3px solid transparent; } QHeaderView::section:hover { border-image: url(:/Resource/Aero/headerSectionHover.png); } QHeaderView::section:pressed { border-image: url(:/Resource/Aero/headerSectionPressed.png); } /* style the sort indicator */ QHeaderView::down-arrow {  image: url(:/Resource/scrollDownArrow.png); width: 13px; height: 12px; subcontrol-position: center right; } QHeaderView::up-arrow {  image: url(:/Resource/scrollUpArrow.png); width: 13px; height: 12px; subcontrol-position: center right; }";

	// Sidebar
	sidebarBackground = ".QFrame { background-color: #0C1A2B; } ";
	sidebarBackground += "QTabWidget::pane { border: 2px transparent; } ";
	sidebarBackground +=
		"QTabBar::tab { color: white; background-color: transparent; border: 1px solid transparent; border-radius: 3px; margin: 2px; min-width: 8ex; padding: 3px; font-weight: bold; } ";
	sidebarBackground += "QTabBar::tab:selected, QTabBar::tab:hover { background-color: #6E99C6; }";

	sidebarTaskBackground =
		"QFrame { border-image: none; border: 0px solid transparent; background-color: white; color: rgb(16,21,59); }";

	sidebarTaskHeader =
		"QToolButton { border-image: none; background-color: #6E99C6; border: 0px solid transparent; border-radius: 3px; border-bottom-left-radius: 0px; border-bottom-right-radius: 0px; color: white; font-size: 16px; font-weight: bold; } ";
	sidebarTaskHeader += "QToolButton:!checked { border-bottom-left-radius: 3px; border-bottom-right-radius: 3px; } ";
	sidebarTaskHeader += "QToolButton:hover { color: black; background-color: #666666; }";

	addSearchButton = "";

	// Toolbars
	toolbars =
		"QToolBar { border-image: url(:/Resource/Aero/toolbar.png); border: 2px; border-bottom: 3px; } QToolBar::handle:horizontal { border-image: url(:/Resource/Aero/toolbarGrip.png); border-left: 3px; border-right: 3px; width: 2px; } QToolBar::handle:vertical { border-image: url(:/Resource/Aero/toolBarGripVertical.png); border-top: 3px; border-bottom: 3px; height: 2px; } QToolButton { border: 3px solid transparent; border-image: none; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Aero/toolButtonDisabled.png); } QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */  padding-right: 20px; /* make way for the popup button */ } /* the subcontrols below are used only in the MenuButtonPopup mode */ QToolButton::menu-button {  border: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-button:hover {  border: 1px solid transparent; border-left-color: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-arrow:open {  top: 1px; left: 1px; /* shift it a bit */ } ";
	navigationToolbar =
		"QToolButton { border: 3px solid transparent; border-image: none; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Aero/toolButtonDisabled.png); } QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */      padding-right: 20px; /* make way for the popup button */  } /* the subcontrols below are used only in the MenuButtonPopup mode */ QToolButton::menu-button {      border: 1px solid transparent;      /* 16px width + 4px for border = 20px allocated above */      width: 16px;  } QToolButton::menu-button:hover {     border: 1px solid transparent; border-left-color: 1px solid transparent;      /* 16px width + 4px for border = 20px allocated above */      width: 16px;  } QToolButton::menu-arrow:open {      top: 1px; left: 1px; /* shift it a bit */  } /*I was using the text property but that property gets changed during translations.  What's This help buttons are someting I do not plan to implement into Quazaa so this  is a way to use the property constructively.*/ QToolButton:hover[whatsThis=\"Home\"] { border-image: url(:/Resource/Aero/homeToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Home\"] { border-image: url(:/Resource/Aero/homeToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Library\"] { border-image: url(:/Resource/Aero/libraryToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Library\"] { border-image: url(:/Resource/Aero/libraryToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Media\"] { border-image: url(:/Resource/Aero/mediaToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Media\"] { border-image: url(:/Resource/Aero/mediaToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Search\"] { border-image: url(:/Resource/Aero/searchToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Search\"] { border-image: url(:/Resource/Aero/searchToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Transfers\"] { border-image: url(:/Resource/Aero/transfersToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Transfers\"] { border-image: url(:/Resource/Aero/transfersToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Security\"] { border-image: url(:/Resource/Aero/securityToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Security\"] { border-image: url(:/Resource/Aero/securityToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Activity\"] { border-image: url(:/Resource/Aero/activityToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Activity\"] { border-image: url(:/Resource/Aero/activityToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Chat\"] { border-image: url(:/Resource/Aero/chatToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Chat\"] { border-image: url(:/Resource/Aero/chatToolButtonPressed.png); }";

	genericHeader =
		".QFrame { border-image: url(:/Resource/Aero/header.png) stretch; border: 2px solid transparent; } QLabel { color: white; font-size: 15px; font-weight: bold; }";
	dialogHeader =
		".QFrame { border-image: url(:/Resource/Aero/dialogHeader.png) stretch; border: 2px solid transparent; } QLabel { color: black; font-size: 15px; font-weight: bold; }";
	taskHeader =
		"QToolButton { border-image: url(:/Resource/Aero/taskHeader.png); border: 2px solid transparent; background-color: transparent; font-weight: 100; color: white; }";

	// Media
	seekSlider =
		"QSlider::groove:horizontal { border: 1px solid rgb(82, 111, 174); border-image: none; height: 16px; background: black; margin: 3px 0; } QSlider::handle:horizontal { background: qlineargradient(spread:pad, x1:0.510526, y1:0, x2:0.511, y2:1, stop:0 rgba(206, 215, 255, 255), stop:0.184211 rgba(82, 107, 192, 255), stop:0.342105 rgba(55, 80, 167, 255), stop:0.484211 rgba(17, 26, 148, 255), stop:0.636842 rgba(0, 0, 0, 255), stop:0.8 rgba(24, 46, 171, 255), stop:0.984211 rgba(142, 142, 255, 255)); border: 1px solid rgb(82, 111, 174); border-image: none; border-radius: 0px; width: 4px; margin: 1px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ }";
	volumeSlider =
		"QSlider::groove:horizontal { border: 1px solid rgb(0, 61, 89); border-image: none; height: 3px; background: black; margin: 2px 0; } QSlider::handle:horizontal { background: qlineargradient(spread:pad, x1:0.510526, y1:0, x2:0.511, y2:1, stop:0 rgba(206, 215, 255, 255), stop:0.184211 rgba(82, 107, 192, 255), stop:0.342105 rgba(55, 80, 167, 255), stop:0.484211 rgba(17, 26, 148, 255), stop:0.636842 rgba(0, 0, 0, 255), stop:0.8 rgba(24, 46, 171, 255), stop:0.984211 rgba(142, 142, 255, 255)); border: 1px solid rgb(82, 111, 174); border-image: none; width: 6px; margin: -4px 0; } ";
	mediaToolbar = "QToolBar { color: white; background: black; }";

	// Specialised Tab Widgets
	libraryNavigator =
		"QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid transparent; } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } /* Style the tab using the tab sub-control. Note that  it reads QTabBar _not_ QTabWidget */ QTabBar::tab { border-image: none;  border: 2px solid transparent;  padding: 2px; color: white; } QTabBar::tab:selected, QTabBar::tab:hover { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); color: black; }";
	tabSearches =
		"QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid rgb(216, 216, 216); color: rgb(220, 220, 220); } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } /* Style the tab using the tab sub-control. Note that  it reads QTabBar _not_ QTabWidget */ QTabBar::tab { border-image: none;  background: transparent;  border: 2px solid transparent;  padding: 2px; } QTabBar::tab:selected, QTabBar::tab:hover { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); } QFrame#frameSearches { background-color: rgb(241, 243, 240); }";

	// Colors
	logColorInformation = "";
	logWeightInformation = "";
	logColorSecurity = QColor( qRgb( 170, 170, 0 ) );
	logWeightSecurity = "font-weight:600;";
	logColorNotice = QColor( qRgb( 0, 170, 0 ) );
	logWeightNotice = "font-weight:600;";
	logColorDebug = QColor( qRgb( 117, 117, 117 ) );
	logWeightDebug = "";
	logColorWarning = QColor( qRgb( 255, 0, 0 ) );
	logWeightWarning = "";
	logColorError = QColor( qRgb( 170, 0, 0 ) );
	logWeightError = "font-weight:600;";
	logColorCritical = QColor( qRgb( 255, 0, 0 ) );
	logWeightCritical = "font-weight:600;";
	listsColorNormal = QColor( 0, 0, 0 );
	listsWeightNormal = QFont::Normal;
	listsColorActive = QColor( 0, 0, 180 );
	listsWeightActive = QFont::Normal;
	listsColorSpecial = QColor( 0, 0, 180 );
	listsWeightSpecial = QFont::Normal;
	listsColorHighlighted = QColor( 0, 0, 180 );
	listsWeightHighlighted = QFont::Normal;

}

void SkinSettings::setLunaskin()
{
	// ListViews
	listViews =
		"QAbstractItemView { border: 1px solid rgb(127, 157, 185); color: black; background-color: white; selection-color: white; selection-background-color: rgb(22, 101, 203); } QHeaderView::section, QHeaderView::section:checked, QHeaderView::section:selected { border-image: url(:/Resource/Luna/itemViewHeader.png); color: black; padding-right: 15px; border: 3px solid transparent; } QHeaderView::section:hover { border-image: url(:/Resource/Luna/itemViewHeaderHoverChecked.png); } /* style the sort indicator */ QHeaderView::down-arrow {  image: url(:/Resource/scrollDownArrow.png); width: 13px; height: 12px; subcontrol-position: center right; } QHeaderView::up-arrow {  image: url(:/Resource/scrollUpArrow.png); width: 13px; height: 12px; subcontrol-position: center right; }";

	// Sidebar
	sidebarBackground =
		"QFrame {  background-color: qlineargradient(spread:pad, x1:0.506667, y1:0, x2:0.528889, y2:1, stop:0 rgba(122, 161, 230, 255), stop:1 rgba(99, 117, 214, 255)); } QToolButton { border: 3px solid transparent; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonDisabled.png); }";
	sidebarTaskBackground =
		"QFrame[frameShape=\"6\"] { border: 1px solid white; background-color: rgb(214, 223, 247); } .QFrame { border: 1px solid white; background-color: rgb(214, 223, 247); } QLabel[frameShape=\"6\"] { border: 1px solid white; background-color: rgb(214, 223, 247); } QLabel { background-color: rgb(214, 223, 247); } QToolButton { border: 3px solid transparent; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonDisabled.png); }";
	sidebarTaskHeader =
		"QToolButton { border-image: url(:/Resource/Luna/taskHeader.png); border: 2px solid transparent; border-bottom-left-radius: 0px; border-bottom-right-radius: 0px; color: white; font-size: 16px; font-weight: bold; } QToolButton:!checked { border-image: url(:/Resource/Luna/taskHeaderOff.png); } QToolButton:hover { color: lightgrey; }";
	addSearchButton =
		"QToolButton { border-image: url(:/Resource/Luna/addSearch.png); border: 2px solid transparent; color: black; font-size: 16px; font-weight: bold; } QToolButton:hover { color: rgb(98, 98, 98); }";

	// Toolbars
	toolbars =
		"QToolBar { border: 0px solid transparent; background-color: rgb(241, 243, 240); } QToolBar::handle:horizontal { border-image: url(:/Resource/Luna/toolBarGrip.png) repeat; border-bottom: 2px; max-width: 7px; max-height: 7px; } QToolBar::handle:vertical { border-image: url(:/Resource/Luna/toolBarGrip.png) repeat; border-bottom: 2px; border-right: 1px; max-width: 7px; max-height: 7px; } QToolButton { border: 3px solid transparent; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonDisabled.png); } QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */  padding-right: 20px; /* make way for the popup button */ } /* the subcontrols below are used only in the MenuButtonPopup mode */ QToolButton::menu-button {  border: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-button:hover {  border: 1px solid transparent; border-left-color: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-arrow:open {  top: 1px; left: 1px; /* shift it a bit */ } ";
	navigationToolbar =
		"QToolBar { border: 0px solid grey; background-color: rgb(241, 243, 240); } QToolBar::handle:horizontal { border-image: url(:/Resource/Luna/toolBarGrip.png) repeat; border-bottom: 2px; max-width: 7px; max-height: 7px; } QToolBar::handle:vertical { border-image: url(:/Resource/Luna/toolBarGrip.png) repeat; border-bottom: 2px; border-right: 1px; max-width: 7px; max-height: 7px; } QToolButton { border: 3px solid transparent; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonPressed.png); } /*I was using the text property but that property gets changed during translations. What's This help buttons are someting I do not plan to implement into Quazaa so this is a way to use the property constructively.*/ QToolButton:hover[whatsThis=\"Home\"], QToolButton:checked[whatsThis=\"Home\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonHome.png); } QToolButton:hover[whatsThis=\"Library\"], QToolButton:checked[whatsThis=\"Library\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonLibrary.png); } QToolButton:hover[whatsThis=\"Media\"], QToolButton:checked[whatsThis=\"Media\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonMedia.png); } QToolButton:hover[whatsThis=\"Search\"], QToolButton:checked[whatsThis=\"Search\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonSearch.png); } QToolButton:hover[whatsThis=\"Transfers\"], QToolButton:checked[whatsThis=\"Transfers\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonTransfers.png); } QToolButton:hover[whatsThis=\"Security\"], QToolButton:checked[whatsThis=\"Security\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonSecurity.png); } QToolButton:hover[whatsThis=\"Activity\"], QToolButton:checked[whatsThis=\"Activity\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonActivity.png); } QToolButton:hover[whatsThis=\"Chat\"], QToolButton:checked[whatsThis=\"Chat\"] { border: 4px solid transparent; background-color: transparent; border-radius: 0px; border-image: url(:/Resource/Luna/toolBarButtonChat.png); }";

	// Headers
	genericHeader =
		".QFrame { background-color: #003399; border-bottom: 5px; padding-left: 2px; } QLabel { font-size: 15px; font-weight: bold; color: white; }";
	dialogHeader =
		".QFrame { font-size: 15px; font-weight: bold; border-image: url(:/Resource/Luna/genericHeaderBackground.png); border-bottom: 5px; padding-left: 2px; } QLabel { color: white; }";
	taskHeader =
		"QToolButton { border-image: url(:/Resource/Luna/unclickableTaskHeader.png); color: black; font-size: 16px; font-weight: bold; }";

	// Media
	seekSlider =
		"QSlider::groove:horizontal {  border: 1px solid rgb(127, 157, 185); border-image: none;  height: 12px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */  background: black; } QSlider::handle:horizontal {  background: rgb(22, 101, 203);  border: 1px solid black; border-image: none;  width: 10px;  margin: 0px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ }";
	volumeSlider =
		"QSlider { min-height: 23px; } QSlider::groove:horizontal {  border: 2px solid transparent;  height: 2px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */  position: absolute;  border-image: url(:/Resource/Luna/sliderTrack.png);  margin: 8px 0; } QSlider::handle:horizontal { border: 1px solid transparent; border-image: url(:/Resource/Luna/slider.png);  width: 11px; height: 21px;  margin: -9 0px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */  position: absolute; } QSlider::handle:horizontal:hover { border-image: url(:/Resource/Luna/sliderHover.png); } QSlider::handle:horizontal:pressed { border-image: url(:/Resource/Luna/sliderPressed.png); } QSlider::handle:horizontal:disabled { border-image: url(:/Resource/Luna/sliderDisabled.png); }";
	mediaToolbar =
		"QToolBar { border: 0px solid transparent; background-color: rgb(241, 243, 240); } QToolBar::handle:horizontal { border-image: url(:/Resource/Luna/toolBarGrip.png) repeat; border-bottom: 2px; max-width: 7px; max-height: 7px; } QToolBar::handle:vertical { border-image: url(:/Resource/Luna/toolBarGrip.png) repeat; border-bottom: 2px; border-right: 1px; max-width: 7px; max-height: 7px; } QToolButton { border: 3px solid transparent; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonDisabled.png); } QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */  padding-right: 20px; /* make way for the popup button */ } /* the subcontrols below are used only in the MenuButtonPopup mode */ QToolButton::menu-button {  border: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-button:hover {  border: 1px solid transparent; border-left-color: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-arrow:open {  top: 1px; left: 1px; /* shift it a bit */ }";

	// Specialised Tab Widgets
	libraryNavigator =
		"QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid transparent; } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } /* Style the tab using the tab sub-control. Note that  it reads QTabBar _not_ QTabWidget */ QTabBar::tab { border-image: none;  border: 2px solid transparent;  padding: 2px; color: white; } QTabBar::tab:selected, QTabBar::tab:hover { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); color: black; }";
	tabSearches =
		"QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid rgb(216, 216, 216); color: rgb(220, 220, 220); } QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid transparent; } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } /* Style the tab using the tab sub-control. Note that  it reads QTabBar _not_ QTabWidget */ QTabBar::tab { border-image: none;  background: transparent;  border: 2px solid transparent;  padding: 2px; } QTabBar::tab:selected, QTabBar::tab:hover { background-color: transparent; border-image: url(:/Resource/Luna/toolBarButtonHoverChecked.png); } QFrame#frameSearches { background-color: rgb(241, 243, 240); }";

	// Colors
	logColorInformation = "";
	logWeightInformation = "";
	logColorSecurity = QColor( qRgb( 170, 170, 0 ) );
	logWeightSecurity = "font-weight:600;";
	logColorNotice = QColor( qRgb( 0, 170, 0 ) );
	logWeightNotice = "font-weight:600;";
	logColorDebug = QColor( qRgb( 117, 117, 117 ) );
	logWeightDebug = "";
	logColorWarning = QColor( qRgb( 255, 0, 0 ) );
	logWeightWarning = "";
	logColorError = QColor( qRgb( 170, 0, 0 ) );
	logWeightError = "font-weight:600;";
	logColorCritical = QColor( qRgb( 255, 0, 0 ) );
	logWeightCritical = "font-weight:600;";
	listsColorNormal = QColor( 0, 0, 0 );
	listsWeightNormal = QFont().weight();
	listsColorActive = QColor( 0, 0, 180 );
	listsWeightActive = QFont().weight();
	listsColorSpecial = QColor( 0, 0, 180 );
	listsWeightSpecial = QFont().weight();
	listsColorHighlighted = QColor( 0, 0, 180 );
	listsWeightHighlighted = QFont().weight();
}

void SkinSettings::setAeroSkin()
{
	// Standard Items
	standardItems = "";

	// ListViews
	listViews =
		"QAbstractItemView { border: 1px solid rgb(217, 212, 218); color: black; background-color: white; selection-color: black; selection-background-color: white; } QAbstractItemView::item:selected { /* when user selects item using mouse or keyboard */ border: 3px solid transparent; border-image: url(:/Resource/Aero/itemHoverSelected.png); } QHeaderView::section, QHeaderView::section:checked, QHeaderView::section:selected { border-image: url(:/Resource/Aero/headerSection.png); color: black; padding-right: 15px; border: 3px solid transparent; } QHeaderView::section:hover { border-image: url(:/Resource/Aero/headerSectionHover.png); } QHeaderView::section:pressed { border-image: url(:/Resource/Aero/headerSectionPressed.png); } /* style the sort indicator */ QHeaderView::down-arrow {  image: url(:/Resource/scrollDownArrow.png); width: 13px; height: 12px; subcontrol-position: center right; } QHeaderView::up-arrow {  image: url(:/Resource/scrollUpArrow.png); width: 13px; height: 12px; subcontrol-position: center right; }";

	// Sidebar
	sidebarBackground = ".QFrame { border-image: url(:/Resource/Aero/sidebarBackground.png); }";
	sidebarTaskBackground =
		"QFrame { border-image: none; border: 0px solid transparent; background-color: transparent; color: rgb(16,21,59); }";
	sidebarTaskHeader =
		"QToolButton { border-image: none; image: url(:/Resource/Aero/taskArrowUnchecked.png); image-position: right; border: 0px solid transparent; background-color: transparent; font-weight: 100; color: rgb(16,21,59); } QToolButton:hover { color: blue; text-decoration: underline; } QToolButton:checked { image: url(:/Resource/Aero/taskArrowChecked.png); font-weight: bold; }";
	addSearchButton = "";

	// Toolbars
	toolbars =
		"QToolBar { border-image: url(:/Resource/Aero/toolbar.png); border: 2px; border-bottom: 3px; } QToolBar::handle:horizontal { border-image: url(:/Resource/Aero/toolbarGrip.png); border-left: 3px; border-right: 3px; width: 2px; } QToolBar::handle:vertical { border-image: url(:/Resource/Aero/toolBarGripVertical.png); border-top: 3px; border-bottom: 3px; height: 2px; } QToolButton { border: 3px solid transparent; border-image: none; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Aero/toolButtonDisabled.png); } QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */  padding-right: 20px; /* make way for the popup button */ } /* the subcontrols below are used only in the MenuButtonPopup mode */ QToolButton::menu-button {  border: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-button:hover {  border: 1px solid transparent; border-left-color: 1px solid transparent;  /* 16px width + 4px for border = 20px allocated above */  width: 16px; } QToolButton::menu-arrow:open {  top: 1px; left: 1px; /* shift it a bit */ } ";
	navigationToolbar =
		"QToolButton { border: 3px solid transparent; border-image: none; background-color: transparent; border-radius: 0px; } QToolButton:hover, QToolButton:checked { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); } QToolButton:pressed { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonPressed.png); } QToolButton:disabled { color: black; border-color: transparent; background-color: transparent; border-image: url(:/Resource/Aero/toolButtonDisabled.png); } QToolButton[popupMode=\"1\"] { /* only for MenuButtonPopup */      padding-right: 20px; /* make way for the popup button */  } /* the subcontrols below are used only in the MenuButtonPopup mode */ QToolButton::menu-button {      border: 1px solid transparent;      /* 16px width + 4px for border = 20px allocated above */      width: 16px;  } QToolButton::menu-button:hover {     border: 1px solid transparent; border-left-color: 1px solid transparent;      /* 16px width + 4px for border = 20px allocated above */      width: 16px;  } QToolButton::menu-arrow:open {      top: 1px; left: 1px; /* shift it a bit */  } /*I was using the text property but that property gets changed during translations.  What's This help buttons are someting I do not plan to implement into Quazaa so this  is a way to use the property constructively.*/ QToolButton:hover[whatsThis=\"Home\"] { border-image: url(:/Resource/Aero/homeToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Home\"] { border-image: url(:/Resource/Aero/homeToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Library\"] { border-image: url(:/Resource/Aero/libraryToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Library\"] { border-image: url(:/Resource/Aero/libraryToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Media\"] { border-image: url(:/Resource/Aero/mediaToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Media\"] { border-image: url(:/Resource/Aero/mediaToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Search\"] { border-image: url(:/Resource/Aero/searchToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Search\"] { border-image: url(:/Resource/Aero/searchToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Transfers\"] { border-image: url(:/Resource/Aero/transfersToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Transfers\"] { border-image: url(:/Resource/Aero/transfersToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Security\"] { border-image: url(:/Resource/Aero/securityToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Security\"] { border-image: url(:/Resource/Aero/securityToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Activity\"] { border-image: url(:/Resource/Aero/activityToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Activity\"] { border-image: url(:/Resource/Aero/activityToolButtonPressed.png); } QToolButton:hover[whatsThis=\"Chat\"] { border-image: url(:/Resource/Aero/chatToolButtonHoverChecked.png); } QToolButton:checked[whatsThis=\"Chat\"] { border-image: url(:/Resource/Aero/chatToolButtonPressed.png); }";

	genericHeader =
		".QFrame { border-image: url(:/Resource/Aero/header.png) stretch; border: 2px solid transparent; } QLabel { color: white; font-size: 15px; font-weight: bold; }";
	dialogHeader =
		".QFrame { border-image: url(:/Resource/Aero/dialogHeader.png) stretch; border: 2px solid transparent; } QLabel { color: black; font-size: 15px; font-weight: bold; }";
	taskHeader =
		"QToolButton { border-image: url(:/Resource/Aero/taskHeader.png); border: 2px solid transparent; background-color: transparent; font-weight: 100; color: white; } QToolButton:pressed { border-image: url(:/Resource/Aero/taskHeaderPressed.png); }";

	// Media
	seekSlider =
		"QSlider::groove:horizontal { border: 1px solid rgb(82, 111, 174); border-image: none; height: 16px; background: black; margin: 3px 0; } QSlider::handle:horizontal { background: qlineargradient(spread:pad, x1:0.510526, y1:0, x2:0.511, y2:1, stop:0 rgba(206, 215, 255, 255), stop:0.184211 rgba(82, 107, 192, 255), stop:0.342105 rgba(55, 80, 167, 255), stop:0.484211 rgba(17, 26, 148, 255), stop:0.636842 rgba(0, 0, 0, 255), stop:0.8 rgba(24, 46, 171, 255), stop:0.984211 rgba(142, 142, 255, 255)); border: 1px solid rgb(82, 111, 174); border-image: none; border-radius: 0px; width: 4px; margin: 1px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ }";
	volumeSlider =
		"QSlider::groove:horizontal { border: 1px solid rgb(0, 61, 89); border-image: none; height: 3px; background: black; margin: 2px 0; } QSlider::handle:horizontal { background: qlineargradient(spread:pad, x1:0.510526, y1:0, x2:0.511, y2:1, stop:0 rgba(206, 215, 255, 255), stop:0.184211 rgba(82, 107, 192, 255), stop:0.342105 rgba(55, 80, 167, 255), stop:0.484211 rgba(17, 26, 148, 255), stop:0.636842 rgba(0, 0, 0, 255), stop:0.8 rgba(24, 46, 171, 255), stop:0.984211 rgba(142, 142, 255, 255)); border: 1px solid rgb(82, 111, 174); border-image: none; width: 6px; margin: -4px 0; } ";
	mediaToolbar = "QToolBar { color: white; background: black; }";

	// Specialised Tab Widgets
	libraryNavigator =
		"QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid transparent; } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } /* Style the tab using the tab sub-control. Note that  it reads QTabBar _not_ QTabWidget */ QTabBar::tab { border-image: none;  border: 2px solid transparent;  padding: 2px; color: white; } QTabBar::tab:selected, QTabBar::tab:hover { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); color: black; }";
	tabSearches =
		"QTabWidget::pane { /* The tab widget frame */  border-top: 2px solid rgb(216, 216, 216); color: rgb(220, 220, 220); } QTabWidget::tab-bar {  left: 5px; /* move to the right by 5px */ } /* Style the tab using the tab sub-control. Note that  it reads QTabBar _not_ QTabWidget */ QTabBar::tab { border-image: none;  background: transparent;  border: 2px solid transparent;  padding: 2px; } QTabBar::tab:selected, QTabBar::tab:hover { background-color: transparent; border-image: url(:/Resource/Aero/toolButtonHoverChecked.png); } QFrame#frameSearches { background-color: rgb(241, 243, 240); }";

	// Colors
	logColorInformation = "";
	logWeightInformation = "";
	logColorSecurity = QColor( qRgb( 170, 170, 0 ) );
	logWeightSecurity = "font-weight:600;";
	logColorNotice = QColor( qRgb( 0, 170, 0 ) );
	logWeightNotice = "font-weight:600;";
	logColorDebug = QColor( qRgb( 117, 117, 117 ) );
	logWeightDebug = "";
	logColorWarning = QColor( qRgb( 255, 0, 0 ) );
	logWeightWarning = "";
	logColorError = QColor( qRgb( 170, 0, 0 ) );
	logWeightError = "font-weight:600;";
	logColorCritical = QColor( qRgb( 255, 0, 0 ) );
	logWeightCritical = "font-weight:600;";
	listsColorNormal = QColor( 0, 0, 0 );
	listsWeightNormal = QFont::Normal;
	listsColorActive = QColor( 0, 0, 180 );
	listsWeightActive = QFont::Normal;
	listsColorSpecial = QColor( 0, 0, 180 );
	listsWeightSpecial = QFont::Normal;
	listsColorHighlighted = QColor( 0, 0, 180 );
	listsWeightHighlighted = QFont::Normal;
}
