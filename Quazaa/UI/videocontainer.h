#ifndef VIDEOCONTAINER_H
#define VIDEOCONTAINER_H

#include <QWidget>
#include <QEvent>

#include "videowidget.h"
#include "mediacontrols.h"

namespace Ui {
class VideoContainer;
}

class VideoContainer : public QWidget
{
	Q_OBJECT

public:
	explicit VideoContainer(QWidget *parent = 0);
	~VideoContainer();
	VideoWidget* videoWidget();
	MediaControls* mediaControls();

protected:
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

signals:
	void doubleClicked();

private:
	Ui::VideoContainer *ui;
	VideoWidget *m_oVideoWidget;
	MediaControls *m_oMediaControls;
	QTimer* m_tMediaControls;
};

#endif // VIDEOCONTAINER_H
