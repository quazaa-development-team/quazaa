#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QVideoWidget>
#include "mediaoverlay.h"

class VideoWidget : public QVideoWidget
{
	Q_OBJECT
public:
	explicit VideoWidget(QWidget *parent = 0);
	~VideoWidget();

protected:
	void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);

private:
	//MediaOverlay *mediaOverlay;

signals:

public slots:

};

#endif // VIDEOWIDGET_H
