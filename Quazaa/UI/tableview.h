#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class CTableView : public QTableView
{
public:
	CTableView( QWidget* parent = NULL );

protected:
	// This overwrites certain key events...
	virtual void keyPressEvent( QKeyEvent* event );
};

#endif // TABLEVIEW_H
