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

#include "randomtablewidget.h"
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QDoubleValidator>
#include "comboboxdelegate.h"

class ProportionEditDelegate: public QStyledItemDelegate
{
public:
    ProportionEditDelegate( QObject* parent = 0 ) : QStyledItemDelegate( parent )
    {
        validator.setBottom( 0.0 );
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



RandomTableWidget::RandomTableWidget( QWidget* parent ):
    SequenceTableWidget( parent )
{
    setItemDelegateForColumn( 1, new ProportionEditDelegate( this ) );
}



void RandomTableWidget::initialize( const MonomerSequence& sequence )
{
    SequenceTableWidget::initialize( sequence );

    for ( int i = 0; i < sequence.count(); i++ )
    {
        insertRow( i );

        const Monomer* m = sequence.monomer( i );

        QTableWidgetItem* it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        it->setText( m->name() );
        setItem( i, 0, it );

        it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        it->setText( QString::number( m->proportion() ) );

        setItem( i, 1, it );
    }
}



void RandomTableWidget::addRowActionTriggered()
{
    int cur_row = currentRow();
    insertRow( cur_row + 1 );

    QTableWidgetItem* it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    setItem( cur_row + 1, 0, it );

    it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    it->setText( "1.0" );
    setItem( cur_row + 1, 1, it );
}



