#ifndef SEQUENCETABLEWIDGET_H
#define SEQUENCETABLEWIDGET_H

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

#include <QTableWidget>
#include "monomersequence.h"
#include "comboboxdelegate.h"


class SequenceTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    SequenceTableWidget( QWidget* parent = 0 );

    void setMonomerNames( const QStringList& list );
    virtual void initialize( const MonomerSequence& sequence );
    void removeAllRows();

protected:
    QAction* addRowAction;
    QAction* removeRowAction;
    ComboBoxDelegate* cb_delegate;

public slots:
    virtual void addRowActionTriggered();
    virtual void removeRowActionTriggered();
    virtual void removeRowsWithMonomer( const QString& name );
};

#endif // SEQUENCETABLEWIDGET_H
