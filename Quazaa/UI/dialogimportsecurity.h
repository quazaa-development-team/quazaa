#ifndef CDIALOGIMPORTSECURITY_H
#define CDIALOGIMPORTSECURITY_H

#include <QDialog>

namespace Ui {
class CDialogImportSecurity;
}

namespace SecurityFilterType {
	enum FilterType {
		Quazaa, Shareaza, P2P, Dat
	};
}

class CDialogImportSecurity : public QDialog
{
	Q_OBJECT

public:
	explicit CDialogImportSecurity(QWidget *parent = 0);
	~CDialogImportSecurity();

private slots:
	void on_toolButtonChooseFile_clicked();
	void on_pushButtonOK_clicked();
	void on_pushButtonCancel_clicked();
	void on_comboBoxFilterType_currentIndexChanged(int index);

private:
	Ui::CDialogImportSecurity *ui;
};

#endif // CDIALOGIMPORTSECURITY_H
