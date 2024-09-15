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


#include <QComboBox>

#include "comboboxdelegate.h"

const char* GAS_STR = "Gas";
const char* LIQUID_STR = "Liquid";
const char* SOLID_STR = "Solid";


ComboBoxDelegate::ComboBoxDelegate( const QStringList& itemList, QObject* parent ) :
    QItemDelegate( parent ),
    item_list( itemList )
{
}

QWidget* ComboBoxDelegate::createEditor( QWidget* parent,
        const QStyleOptionViewItem& /* option */,
        const QModelIndex& /* index */ ) const
{
    QComboBox* editor = new QComboBox( parent );
    editor->addItems( item_list );
    return editor;
}



void ComboBoxDelegate::setEditorData( QWidget* editor, const QModelIndex& index ) const
{
    QString value = index.model()->data( index, Qt::EditRole ).toString();

    QComboBox* cBox = static_cast<QComboBox*>( editor );
    cBox->setCurrentIndex( cBox->findText( value ) );
}



void ComboBoxDelegate::setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    QComboBox* cBox = static_cast<QComboBox*>( editor );
    QString value = cBox->currentText();

    model->setData( index, value, Qt::EditRole );
}



void ComboBoxDelegate::updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /* index */ ) const
{
    editor->setGeometry( option.rect );
}




