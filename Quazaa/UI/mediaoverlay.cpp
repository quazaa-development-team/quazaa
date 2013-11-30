#include "mediaoverlay.h"
#include "ui_mediaoverlay.h"

MediaOverlay::MediaOverlay(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MediaOverlay)
{
	ui->setupUi(this);

	m_tMediaControls = new QTimer(this);
	m_tMediaControls->setSingleShot(true);
	connect(m_tMediaControls, SIGNAL(timeout()), ui->frameMediaControls, SLOT(hide()));
	m_tVolumeControl = new QTimer(this);
	m_tVolumeControl->setSingleShot(true);
	connect(m_tVolumeControl, SIGNAL(timeout()), ui->horizontalSliderVolume, SLOT(hide()));

	ui->frameMediaControls->hide();
	ui->horizontalSliderVolume->hide();
}

MediaOverlay::~MediaOverlay()
{
	delete ui;
}

void MediaOverlay::mouseMoveEvent(QMouseEvent *e)
{
	if(underMouse()) {
		if(!ui->frameMediaControls->isVisible())
			ui->frameMediaControls->show();

		m_tMediaControls->start(5000);
	}

	if(ui->toolButtonMute->underMouse()) {
		if(!ui->horizontalSliderVolume->isVisible())
			ui->horizontalSliderVolume->show();

		m_tVolumeControl->start(5000);
	}

	QWidget::mouseMoveEvent(e);
}
