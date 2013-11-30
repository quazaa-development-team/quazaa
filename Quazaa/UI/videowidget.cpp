#include "videowidget.h"

#include <QKeyEvent>
#include <QMouseEvent>

VideoWidget::VideoWidget(QWidget *parent) :
	QVideoWidget(parent)
{
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	setPalette(p);

	setAttribute(Qt::WA_OpaquePaintEvent);

	//mediaOverlay = new MediaOverlay(this);
}

VideoWidget::~VideoWidget()
{

}

void VideoWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape && isFullScreen()) {
		setFullScreen(false);
		event->accept();
	} else if (event->key() == Qt::Key_Enter && event->modifiers() & Qt::Key_Alt) {
		setFullScreen(!isFullScreen());
		event->accept();
	} else {
		QVideoWidget::keyPressEvent(event);
	}
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	setFullScreen(!isFullScreen());
	event->accept();
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
	QVideoWidget::mousePressEvent(event);
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
	//mediaOverlay->resize(event->size());
	event->accept();
}
