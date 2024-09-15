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

#include "exportgriddialog.h"
#include "ui_exportgriddialog.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

ExportGridDialog::ExportGridDialog( Grid* g,  QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::ExportDialog ),
    grid( g )
{
    ui->setupUi( this );

    connect( ui->fileSelectButton, SIGNAL( clicked( bool ) ), this, SLOT( fileSelectionButtonClicked() ) );
    connect( ui->buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked( bool ) ), this, SLOT( applyButtonClicked() ) );
}

ExportGridDialog::~ExportGridDialog()
{
    delete ui;

}

void ExportGridDialog::fileSelectionButtonClicked()
{
    const QString EXTENSION( ".csv" );

    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save File" ), QString(), tr( "CSV files (*.csv)" ) );

    if ( false == fileName.isEmpty() )
    {
        if ( false == fileName.endsWith( EXTENSION, Qt::CaseInsensitive ) )
        {
            fileName += EXTENSION;
        }
        ui->fileNameLabel->setText( fileName );
        ui->fileNameLabel->setToolTip( fileName );
    }
}



void ExportGridDialog::applyButtonClicked()
{
    int num_cols = ui->numColsSpinBox->value();
    int num_rows = ui->numRowsSpinBox->value();
    int num_layers = ui->numLayersSpinBox->value();

    int initial_val = 0;

    QVector < QVector < QVector< int > > > vec( num_cols,
            QVector < QVector <int > > ( num_rows,
                                         QVector < int > ( num_layers, initial_val ) ) );

    float grid_step_x = grid->params.box_size.x / ( num_cols );
    float grid_step_y = grid->params.box_size.y / ( num_rows );
    float grid_step_z = grid->params.box_size.z / ( num_layers );

    // scan through chains and count how may monomers are in each box

    int counter = 0 ;
    for ( int chain_nr = 0; chain_nr < grid->max_chains; chain_nr++ )
    {
        Chain* chain = &grid->chains[chain_nr];

        if ( chain->last - chain->first <= 0 ) continue;

        for ( int j = chain->first; j < chain->last; j++ )
        {
            int k = j - chain->offset;

            Vector v = Vector_periodic_box( chain->atoms[k], grid->params.box_size );

            int col_loc = 0;
            int row_loc = 0;
            int layer_loc = 0;

            float distance = grid_step_x;

            while ( distance < v.x )
            {
                col_loc++;
                distance += grid_step_x;
            }

            distance = grid_step_y;

            while ( distance < v.y )
            {
                row_loc++;
                distance += grid_step_y;
            }

            distance = grid_step_z;

            while ( distance < v.z )
            {
                layer_loc++;
                distance += grid_step_z;
            }

            vec[col_loc][row_loc][layer_loc] = vec[col_loc][row_loc][layer_loc] + 1;
            counter++;
        }
    }

    // write out to file...

    int total = 0;
    QFile data( ui->fileNameLabel->text() );

    if ( data.open( QFile::WriteOnly | QFile::Truncate ) )
    {
        QTextStream out( &data );

        out << QString( "column,row,layer,value" ) << Qt::endl;

        for ( int col = 0; col < num_cols; col++ )
        {
            for ( int row = 0; row < num_rows; row++ )
            {
                for ( int layer = 0; layer < num_layers; layer++ )
                {
                    total += vec[col][row][layer];
                    out << ( col + 1 ) << "," << ( row + 1 ) << "," << ( layer + 1 ) << "," << vec[col][row][layer] << Qt::endl;
                }
            }
        }
    }

    qDebug() << " total monomers: " << total << counter;
}
