#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

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


#include <QItemDelegate>
#include <QStringList>


extern const char* GAS_STR;
extern const char* LIQUID_STR;
extern const char* SOLID_STR;


class ComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate( const QStringList& itemList, QObject* parent = 0 );

    QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option,
                           const QModelIndex& index ) const;

    void setEditorData( QWidget* editor, const QModelIndex& index ) const;
    void setModelData( QWidget* editor, QAbstractItemModel* model,
                       const QModelIndex& index ) const;

    void updateEditorGeometry( QWidget* editor,
                               const QStyleOptionViewItem& option, const QModelIndex& index ) const;

protected:
    QStringList item_list;


};

#endif // COMBOBOXDELEGATE_H
