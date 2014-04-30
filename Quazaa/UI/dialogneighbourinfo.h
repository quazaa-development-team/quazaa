#ifndef DIALOGNEIGHBOURINFO_H
#define DIALOGNEIGHBOURINFO_H

#include "neighbourstablemodel.h"
#include <QDialog>

namespace Ui
{
class CDialogNeighbourInfo;
}

class DialogNeighbourInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogNeighbourInfo( NeighboursTableModel::NeighbourData* pNeighbour, QWidget* parent = 0 );
	~DialogNeighbourInfo();

private:
	Ui::CDialogNeighbourInfo* ui;
	QString neighbourConnectionDescription( NeighboursTableModel::NeighbourData* pNeighbour );
};

#endif // DIALOGNEIGHBOURINFO_H
