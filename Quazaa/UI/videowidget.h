#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QVideoWidget>
#include "mediacontrols.h"

class VideoWidget : public QVideoWidget
{
	Q_OBJECT
public:
	explicit VideoWidget( QWidget* parent = 0 );
	~VideoWidget();

protected:

private:

public slots:
};

#endif // VIDEOWIDGET_H
