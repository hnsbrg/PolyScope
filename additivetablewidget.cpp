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

#include "additivetablewidget.h"
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QDoubleValidator>


class FractionEditDelegate: public QStyledItemDelegate
{
public:
    FractionEditDelegate( QObject* parent = 0 ) :
        QStyledItemDelegate( parent )
    {
        validator.setTop( 1.0 );
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





AdditiveTableWidget::AdditiveTableWidget( QWidget* parent ):
    SequenceTableWidget( parent )
{
    setItemDelegateForColumn( 1, new FractionEditDelegate( this ) );
}

void initialize( );

void AdditiveTableWidget::initialize( const AdditiveList& additiveList )
{
    removeAllRows();

    for ( int i = 0; i < additiveList.count(); i++ )
    {
        insertRow( i );

        const Additive* a = additiveList.additive( i );

        QTableWidgetItem* it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        it->setText( a->name() );
        setItem( i, 0, it );

        it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        it->setText( QString::number( a->fraction() ) );
        setItem( i, 1, it );

        it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
        it->setText( QString::number( a->avgClusterSize() ) );
        setItem( i, 2, it );

        it = new QTableWidgetItem();
        it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        it->setCheckState( a->usePoisson() ? Qt::Checked : Qt::Unchecked );
        setItem( i, 3, it );
    }
}





void AdditiveTableWidget::addRowActionTriggered()
{
    int cur_row = currentRow();
    insertRow( cur_row + 1 );

    QTableWidgetItem* it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    setItem( cur_row + 1, 0, it );

    it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    it->setText( "0.1" );
    setItem( cur_row + 1, 1, it );

    it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    it->setText( "1" );
    setItem( cur_row + 1, 2, it );

    it = new QTableWidgetItem();
    it->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    it->setCheckState( Qt::Unchecked );
    setItem( cur_row + 1, 3, it );
}
