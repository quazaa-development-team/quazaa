#include "dialogimportsecurity.h"
#include "ui_dialogimportsecurity.h"

#include "securitymanager.h"
#include "quazaasettings.h"

#include <QFileDialog>

DialogImportSecurity::DialogImportSecurity( QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::DialogImportSecurity )
{
	ui->setupUi( this );

	ui->progressBarLoad->setVisible( false );

	connect( &securityManager, &Security::Manager::updateLoadMax,
			 ui->progressBarLoad, &QProgressBar::setMaximum );
	connect( &securityManager, &Security::Manager::updateLoadProgress,
			 ui->progressBarLoad, &QProgressBar::setValue );

	setWindowFlags( windowFlags() |= Qt::FramelessWindowHint );
}

DialogImportSecurity::~DialogImportSecurity()
{
	delete ui;
}

void DialogImportSecurity::on_toolButtonChooseFile_clicked()
{
	QString sFilter;
	switch ( ui->comboBoxFilterType->currentIndex() )
	{
	case SecurityFilterType::P2P:
		sFilter = tr( "P2P Format Ban List (*.p2p *.txt)" );
		break;

	case SecurityFilterType::Dat:
		sFilter = tr( "Dat Format Ban List (*.dat *.txt)" );
		break;

	case SecurityFilterType::Shareaza:
	default:
		sFilter = tr( "Shareaza Security Filters (*.xml)" );
		break;
	}

	ui->lineEditFile->setText( QFileDialog::getOpenFileName( this,
															 tr( "Select File to Import" ),
															 quazaaSettings.Downloads.CompletePath,
															 sFilter ) );

	if ( !ui->lineEditFile->text().isEmpty() )
	{
		ui->pushButtonOK->setEnabled( true );
	}
}

void DialogImportSecurity::on_pushButtonOK_clicked()
{
	ui->progressBarLoad->setVisible( true );
	ui->comboBoxFilterType->setEnabled( false );
	ui->toolButtonChooseFile->setEnabled( false );
	ui->pushButtonOK->setEnabled( false );
	ui->pushButtonCancel->setEnabled( false );

	// TODO: maybe do something if importing failed?
	switch ( ui->comboBoxFilterType->currentIndex() )
	{
	case SecurityFilterType::P2P:
		securityManager.fromP2P( ui->lineEditFile->text() );
		break;

	case SecurityFilterType::Dat:
		break;

	case SecurityFilterType::Shareaza:
	default:
		securityManager.fromXML( ui->lineEditFile->text() );
		break;
	}

	close();
}

void DialogImportSecurity::on_pushButtonCancel_clicked()
{
	close();
}

void DialogImportSecurity::on_comboBoxFilterType_currentIndexChanged( int index )
{
	Q_UNUSED( index );

	ui->lineEditFile->text().clear();
	ui->pushButtonOK->setEnabled( false );
}
