#include "tableview.h"
#include <QHeaderView>
#include <QKeyEvent>

CTableView::CTableView( QWidget* parent ) :
	QTableView( parent )
{
	// Quazaa default view settings for table views with complete row selection.

	this->setFocusPolicy( Qt::StrongFocus );
	this->setContextMenuPolicy( Qt::CustomContextMenu );
	this->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->setTabKeyNavigation( false );
	this->setDropIndicatorShown( false );
	this->setAlternatingRowColors( true );
	this->setSelectionBehavior( QAbstractItemView::SelectRows );
	this->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
	this->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
	this->setShowGrid( false );
	this->setGridStyle( Qt::NoPen );
	this->setSortingEnabled( true );
	this->setWordWrap( false );
	this->setCornerButtonEnabled( false );
	this->horizontalHeader()->setHighlightSections( false );
	this->horizontalHeader()->setStretchLastSection( true );
	this->verticalHeader()->setVisible( false );
	this->verticalHeader()->setDefaultSectionSize( 20 );
	this->verticalHeader()->setMinimumSectionSize( 14 );
	this->verticalHeader()->setHighlightSections( false );
}

void CTableView::keyPressEvent( QKeyEvent* e )
{
	switch ( e->key() )
	{
	case Qt::Key_Escape:
	{
		setCurrentIndex( QModelIndex() );
		break;
	}

	case Qt::Key_Right:
	{
		QAbstractItemModel* model = QAbstractItemView::model();

		if ( model )
		{
			QModelIndex currentIndex = QAbstractItemView::currentIndex();

			if ( currentIndex.isValid() ) // If there is a selected entry...
			{
				int currentRow = currentIndex.row();
				// ...and if we're not at the end of the list...
				if ( ++currentRow != model->rowCount( QModelIndex() ) )
				{
					// Makes sure the condition above is sufficient.
					Q_ASSERT( currentRow < model->rowCount( QModelIndex() ) && currentRow > 0 );

					// ...select the item that comes after the current.
					setCurrentIndex( model->index( currentRow, 0 ) );
				}
			}
			else if ( model->rowCount( QModelIndex() ) ) // If no selection exists and there are rules...
			{
				// ...select the first entry.
				setCurrentIndex( model->index( 0, 0 ) );
			}
		}

		break;
	}

	case Qt::Key_Left:
	{
		QAbstractItemModel* model = QAbstractItemView::model();

		if ( model )
		{
			QModelIndex currentIndex = QAbstractItemView::currentIndex();

			if ( currentIndex.isValid() ) // If there is a selected entry...
			{
				int currentRow = currentIndex.row();
				// ...and if we're not at the head of the list...
				if ( !currentRow-- )
				{
					// Makes sure the condition above is sufficient.
					Q_ASSERT( currentRow < model->rowCount( QModelIndex() ) && currentRow > -1 );

					// ...select the previous item.
					setCurrentIndex( model->index( currentRow, 0 ) );
				}
			}
			else if ( model->rowCount( QModelIndex() ) ) // If no selection exists and there are rules...
			{
				// ...select the last entry.
				setCurrentIndex( model->index( model->rowCount( QModelIndex() ) - 1, 0 ) );
			}
		}

		break;
	}

	case Qt::Key_Home:
	{
		QAbstractItemModel* model = QAbstractItemView::model();

		if ( model && model->rowCount( QModelIndex() ) )
		{
			setCurrentIndex( model->index( 0, 0 ) );
		}

		break;
	}

	case Qt::Key_End:
	{
		QAbstractItemModel* model = QAbstractItemView::model();

		if ( model )
		{
			int lastRow = model->rowCount( QModelIndex() );
			if ( lastRow != 0 ) // If there are Items in the list...
			{
				setCurrentIndex( model->index( --lastRow, 0 ) );
			}
		}
		break;
	}

	default:
		QTableView::keyPressEvent( e );
	}
}
