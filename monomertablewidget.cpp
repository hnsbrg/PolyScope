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

#include "monomertablewidget.h"
#include <QColorDialog>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QDoubleValidator>

class RadiusEditDelegate: public QStyledItemDelegate
{
public:
    RadiusEditDelegate( QObject* parent = 0 ) : QStyledItemDelegate( parent )
    {
    }

    virtual QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        QLineEdit* e = ( QLineEdit* ) QStyledItemDelegate::createEditor( parent, option, index );

        e->setValidator( &validator );
        e->setToolTip( "a positive floating point value" );
        return e;
    }
protected:
    QDoubleValidator validator;
};




MonomerTableWidget::MonomerTableWidget( QWidget* parent ):
    QTableWidget( parent )
{
    setItemDelegateForColumn( 1, new RadiusEditDelegate( this ) );
    horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

    connect( this, SIGNAL( cellClicked( int, int ) ), this, SLOT( cellWasClicked( int, int ) ) );
    connect( this, SIGNAL( itemChanged( QTableWidgetItem* ) ), this, SLOT( itemDataChanged( QTableWidgetItem* ) ) );
}



void MonomerTableWidget::initialize( const MonomerList& list )
{
    bool old_state = blockSignals( true );
    removeAllRows();

    for ( int i = 0; i < list.count(); i++ )
    {
        insertRow( i );

        const Monomer* m = list.monomer( i );

        QTableWidgetItem* it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        it->setText( m->name() );
        setItem( i, 0, it );

        Qwt3D::RGBA rgb = m->color();
        QColor new_color;
        new_color.setRedF( rgb.r );
        new_color.setGreenF( rgb.g );
        new_color.setBlueF( rgb.b );
        new_color.setAlphaF( rgb.a );

        it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsEnabled );
        it->setBackground( QBrush( new_color ) );

        setItem( i, 1, it );
    }

    blockSignals( old_state );
}



void MonomerTableWidget::removeAllRows()
{
    clearContents();
    while ( 0 != rowCount() )
    {
        removeRow( 0 );
    }
}



void MonomerTableWidget::addRowActionTrigggered()
{
    bool old_state = blockSignals( true );

    int cur_row = currentRow();
    insertRow( cur_row + 1 );

    QTableWidgetItem* it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    setItem( cur_row + 1, 0, it );

    // it = new QTableWidgetItem();
    // it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    // setItem( cur_row + 1, 1, it );

    it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsEnabled );
    setItem( cur_row + 1, 1, it );

    blockSignals( old_state );

}



void MonomerTableWidget::removeRowActionTriggered()
{
    bool old_state = blockSignals( true );
    QString name = item( currentRow(), 0 )->text();

    removeRow( currentRow() );

    blockSignals( old_state );

    emit rowDeleted( name );
}



void MonomerTableWidget::cellWasClicked( int row, int column )
{
    if ( 1 == column )
    {
        QTableWidgetItem* it = item( row, column );

        if ( 0 == it )
        {
            it = new QTableWidgetItem();
        }

        QColor cur_color = ( it ? it->background().color() : Qt::white );

        QColor new_color = QColorDialog::getColor( cur_color, nullptr, tr( "Choose a color for this monomer" ), QColorDialog::ShowAlphaChannel );

        if ( true == new_color.isValid() )
        {
            it->setBackground( QBrush( new_color ) );
            //    setItem( row, column, it );
        }
    }
}



void MonomerTableWidget::itemDataChanged( QTableWidgetItem* item )
{
    emit dataEdited();
}
