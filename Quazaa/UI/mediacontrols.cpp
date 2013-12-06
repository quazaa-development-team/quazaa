#include "mediacontrols.h"
#include "ui_mediacontrols.h"

#include <QDebug>

MediaControls::MediaControls(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MediaControls)
{
	ui->setupUi(this);

	m_tVolumeControl = new QTimer(this);
	m_tVolumeControl->setSingleShot(true);
	connect(m_tVolumeControl, SIGNAL(timeout()), ui->horizontalSliderVolume, SLOT(hide()));

	ui->horizontalSliderVolume->hide();
}

MediaControls::~MediaControls()
{
	delete ui;
}

void MediaControls::mouseMoveEvent(QMouseEvent *e)
{
	if(ui->toolButtonMute->underMouse()) {
		if(!ui->horizontalSliderVolume->isVisible())
			ui->horizontalSliderVolume->show();

		m_tVolumeControl->start(5000);
	}

	QWidget::mouseMoveEvent(e);
}

QToolButton *MediaControls::playPauseButton()
{
	return ui->toolButtonPlayPause;
}

QToolButton *MediaControls::previousButton()
{
	return ui->toolButtonPrevious;
}

QToolButton *MediaControls::stopButton()
{
	return ui->toolButtonStop;
}

QToolButton *MediaControls::nextButton()
{
	return ui->toolButtonNext;
}

QToolButton *MediaControls::openButton()
{
	return ui->toolButtonOpen;
}

QToolButton *MediaControls::replayButton()
{
	return ui->toolButtonReplay;
}

QToolButton *MediaControls::shuffleButton()
{
	return ui->toolButtonShuffle;
}

QToolButton *MediaControls::fullScreenButton()
{
	return ui->toolButtonFullScreen;
}

QToolButton *MediaControls::settingsButton()
{
	return ui->toolButtonSettings;
}

QToolButton *MediaControls::muteButton()
{
	return ui->toolButtonMute;
}
