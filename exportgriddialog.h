#ifndef EXPORTGRIDDIALOG_H
#define EXPORTGRIDDIALOG_H

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


#include <QDialog>

#include "grid.h"
#include "grow.h"

namespace Ui
{
class ExportDialog;
}

class ExportGridDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportGridDialog( Grid* g,  QWidget* parent = nullptr );
    ~ExportGridDialog();

private:
    Ui::ExportDialog* ui;
    Grid* grid;

protected slots:
    void fileSelectionButtonClicked();
    void applyButtonClicked();
};

#endif // EXPORTGRIDDIALOG_H
