#ifndef MEDIAOVERLAY_H
#define MEDIAOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QToolButton>

namespace Ui
{
class MediaControls;
}

class MediaControls : public QWidget
{
	Q_OBJECT

public:
	explicit MediaControls( QWidget* parent = 0 );
	~MediaControls();

private:
	Ui::MediaControls* ui;
	QTimer* m_tVolumeControl;

protected:
	void mouseMoveEvent( QMouseEvent* e );

public slots:
	QToolButton* playPauseButton();
	QToolButton* previousButton();
	QToolButton* stopButton();
	QToolButton* nextButton();
	QToolButton* openButton();
	QToolButton* replayButton();
	QToolButton* shuffleButton();
	QToolButton* fullScreenButton();
	QToolButton* settingsButton();
	QToolButton* muteButton();
};

#endif // MEDIAOVERLAY_H
