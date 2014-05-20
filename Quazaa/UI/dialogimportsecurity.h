#ifndef CDIALOGIMPORTSECURITY_H
#define CDIALOGIMPORTSECURITY_H

#include <QDialog>

namespace Ui
{
class DialogImportSecurity;
}

namespace SecurityFilterType
{
enum FilterType
{
	Quazaa, Shareaza, P2P, Dat
};
}

class DialogImportSecurity : public QDialog
{
	Q_OBJECT

public:
	explicit DialogImportSecurity( QWidget* parent = 0 );
	~DialogImportSecurity();

private slots:
	void on_toolButtonChooseFile_clicked();
	void on_pushButtonOK_clicked();
	void on_pushButtonCancel_clicked();
	void on_comboBoxFilterType_currentIndexChanged( int index );

private:
	Ui::DialogImportSecurity* ui;
};

#endif // CDIALOGIMPORTSECURITY_H
