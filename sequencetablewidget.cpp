// ----------------------------------------------------------------------------
//
// PolyScope
// authored by William Hinsberg
//
// Copyright (C) 2024 Columbia Hill Technical Consulting
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
//
// ----------------------------------------------------------------------------

#include "sequencetablewidget.h"
#include <QHeaderView>

SequenceTableWidget::SequenceTableWidget( QWidget* parent ):
    QTableWidget( parent ),
    cb_delegate( 0 )
{
    horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
}



void SequenceTableWidget::addRowActionTriggered()
{
}



void SequenceTableWidget::removeRowActionTriggered()
{
    removeRow( currentRow() );
    emit itemChanged( currentItem() );
}



void SequenceTableWidget::removeRowsWithMonomer( const QString& name )
{
    for ( int i = rowCount() - 1; 0 < i; i-- )
    {
        if ( name == item( i, 0 )->text() )
        {
            removeRow( i );
        }
    }
}



void SequenceTableWidget::setMonomerNames( const QStringList& list )
{
    if ( 0 != cb_delegate )
    {
        delete  cb_delegate;
    }

    cb_delegate = new ComboBoxDelegate( list );
    setItemDelegateForColumn( 0, cb_delegate );
}



void SequenceTableWidget::initialize( const MonomerSequence& sequence )
{
    removeAllRows();
}



void SequenceTableWidget::removeAllRows()
{
    clearContents();
    while ( 0 != rowCount() )
    {
        removeRow( 0 );
    }
}
