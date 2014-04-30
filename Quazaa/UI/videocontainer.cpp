#include "videocontainer.h"
#include "ui_videocontainer.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>

VideoContainer::VideoContainer( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::VideoContainer )
{
	ui->setupUi( this );

	m_oVideoWidget = new VideoWidget();
	ui->verticalLayoutContents->addWidget( m_oVideoWidget );

	m_oMediaControls = new MediaControls( this );
	ui->verticalLayoutContents->addWidget( m_oMediaControls );
	m_oMediaControls->hide();

	m_tMediaControls = new QTimer( this );
	m_tMediaControls->setSingleShot( true );
	connect( m_tMediaControls, SIGNAL( timeout() ), m_oMediaControls, SLOT( hide() ) );
}

VideoContainer::~VideoContainer()
{
	delete ui;
}

void VideoContainer::keyPressEvent( QKeyEvent* event )
{
	if ( event->key() == Qt::Key_Escape && isFullScreen() )
	{
//		setFullScreen(false);
		event->accept();
	}
	else if ( event->key() == Qt::Key_Enter && event->modifiers() & Qt::Key_Alt )
	{
//		setFullScreen(!isFullScreen());
		event->accept();
	}
	else
	{
		QWidget::keyPressEvent( event );
	}
}

void VideoContainer::mouseDoubleClickEvent( QMouseEvent* event )
{
	emit doubleClicked();
	event->accept();
}

void VideoContainer::mousePressEvent( QMouseEvent* event )
{
	QWidget::mousePressEvent( event );
}

void VideoContainer::mouseMoveEvent( QMouseEvent* event )
{
	if ( underMouse() || m_oVideoWidget->underMouse() || m_oMediaControls->underMouse() )
	{
		if ( !m_oMediaControls->isVisible() )
		{
			m_oMediaControls->show();
		}

		m_tMediaControls->start( 5000 );
	}

	QWidget::mouseMoveEvent( event );
}

VideoWidget* VideoContainer::videoWidget()
{
	return m_oVideoWidget;
}

MediaControls* VideoContainer::mediaControls()
{
	return m_oMediaControls;
}


