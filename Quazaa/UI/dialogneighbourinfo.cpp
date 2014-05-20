#include "dialogneighbourinfo.h"
#include "ui_dialogneighbourinfo.h"

#include "geoiplist.h"
#include "neighbours.h"
#include "networktype.h"
#include "networkiconprovider.h"

DialogNeighbourInfo::DialogNeighbourInfo( NeighboursTableModel::NeighbourData* pNeighbour, QWidget* parent ) :
	QDialog( parent ),
	ui( new Ui::CDialogNeighbourInfo )
{
	ui->setupUi( this );

	ui->textBrowserConnectionInfo->setHtml(
		QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> p, li { white-space: pre-wrap; }</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\"><p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\"><a href=\"http://%1\">%1</a><br /></span><img src=\"%2\" width=\"20\" height=\"20\" /><span style=\" font-size:8pt;\"> %3<br />%4<br />%5</span></p><p><pre>%6</pre></p></body></html>" )
		.arg( pNeighbour->oAddress.toStringWithPort() )
		.arg( ":/Resource/Flags/" + pNeighbour->sCountryCode.toLower() + ".png" )
		.arg( pNeighbour->sCountry )
		.arg( pNeighbour->sUserAgent )
		.arg( neighbourConnectionDescription( pNeighbour ) )
		.arg( pNeighbour->sHandshake.replace( QRegularExpression( "\r\n|\n" ), "<br>" ) ) );
	ui->labelNetworkImage->setPixmap( pNeighbour->iNetwork.pixmap( QSize( 41, 41 ) ) );
	ui->labelCurrentInbound->setText( QString().sprintf( "%1.3f", double( pNeighbour->nBandwidthIn ) ) );
	ui->labelCurrentOutbound->setText( QString().sprintf( "%1.3f", double( pNeighbour->nBandwidthOut ) ) );
	ui->labelTotalInbound->setText( QString().sprintf( "%1.3f KB", pNeighbour->nBytesReceived / 1024.0f ) );
	ui->labelTotalOutbound->setText( QString().sprintf( "%1.3f KB", pNeighbour->nBytesSent / 1024.0f ) );
	ui->labelRatioInbound->setText( QString().sprintf( "%1.2f%%", 100.0f * pNeighbour->nCompressionIn ) );
	ui->labelRatioOutbound->setText( QString().sprintf( "%1.2f%%", 100.0f * pNeighbour->nCompressionOut ) );
}

DialogNeighbourInfo::~DialogNeighbourInfo()
{
	delete ui;
}

QString DialogNeighbourInfo::neighbourConnectionDescription( NeighboursTableModel::NeighbourData* pNeighbour )
{
	if ( pNeighbour->nDiscoveryProtocol == DiscoveryProtocol::G2 )
	{
		QString selfMode;
		QString otherMode;

		if ( neighbours.isG2Hub() )
		{
			selfMode = "Hub";
		}
		else
		{
			selfMode = "Leaf";
		}

		switch ( pNeighbour->nType )
		{
		case G2_HUB:
			otherMode = "Hub";
			break;
		case G2_LEAF:
			otherMode = "Leaf";
			break;
		default:
			otherMode = tr( "Unknown" );
			break;
		}

		return tr( "%1: %2-to-%3 Connection" ).arg( "Gnutella 2" ).arg( selfMode ).arg( otherMode );
	}
	else
	{
		return tr( "Unknown Connection Type" );
	}
}
