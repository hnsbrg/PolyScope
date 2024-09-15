#ifndef EXPOSUREDIALOG_H
#define EXPOSUREDIALOG_H

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
#include <QStringList>
#include"qwt3d_types.h"
#include <QRandomGenerator>
#include "monomerlist.h"
#include "mainwindow.h"
#include <QPushButton>


namespace Ui
{
class ExposureDialog;
}

class ExposureDialog : public QDialog
{
    Q_OBJECT

    enum EXPOSURE_TYPE { Random, Clustered };

public:
    explicit ExposureDialog( MonomerList* monomerList, Qwt3D::AtomVector& monomers, MainWindow* parent );
    ~ExposureDialog();

private:
    Ui::ExposureDialog* ui;

protected:
    int seed;
    double exposed_fraction;
    double deprotection_radius;
    Qwt3D::AtomVector& monomer_vector;
    MonomerList* monomer_list;
    QList<int>  pag_indices;
    QList<int>  protected_and_quencher_indices;
    QList<int>  deprotected_indices;
    QList<int>  neutralized_quencher_indices;
    QList<int>  photolyzed_pag_indices;
    QList<int>  clustered_photolyzed_pag_indices;
    enum EXPOSURE_TYPE exposure_type;
    double acid_cluster_radius;
    double acids_per_photon;
    QRandomGenerator rng;
    QPushButton* apply_button;
    QPushButton* reset_button;
    MainWindow* main_window;

    void initialize();
    void readInputs();
    void runCalculation();
    bool isInVolume( int pagIndex, int protectedIndex, double radius );
    bool isExposed( int pagIndex );
    void preparePhotolyzedPAGList();
    double numberOfNeighbors( double radius );
    void dumpAcidAndQuencherLocations();

protected slots:
    virtual void apply();
    virtual void selectFileButtonClicked();
    virtual void reset();
    virtual void exposureTypeChanged( int newType );
};

#endif // EXPOSUREDIALOG_H
