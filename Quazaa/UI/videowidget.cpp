#include "videowidget.h"

#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent) :
	QVideoWidget(parent)
{
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	setPalette(p);

	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_TransparentForMouseEvents);
}

VideoWidget::~VideoWidget()
{

}
