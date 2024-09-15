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

#include <QFileDialog>
#include <QIntValidator>
#include <QMessageBox>
#include <QSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qwt3d_graphplot.h>
#include "exportgriddialog.h"
#include "random.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "interface.h"
#include "grow.h"
#include "vector.h"
#include "monomersequence.h"
#include "exposuredialog.h"
#include "revision.h"

static Grid            g_grid;
static Parameters      g_params;
static Grow_Parameters g_grow_params;
const QString MAIN_WINDOW_GROUP( "MainWindow" );
const QString MODEL_FOLDER_NAME( "ModelFolder" );
const QString OUTPUT_FOLDER_NAME( "OutputFolder" );

static ChainList chain_list;
static AdditiveClusterListList additive_list_list;

ChainList& chainList() { return chain_list; }
AdditiveClusterListList& additiveListList() { return additive_list_list; }


MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    density( 1.00 ),
    brush_density( 1.00 ),
    x_proportion( 1.00 ),
    y_proportion( 1.00 ),
    target_chain_length( 0 ),
    total_num_chains( 0 ),
    timer(),
    elapsed_time(),
    scan_index( 0 ),
    monomer_type_list(),
    sequence_list(),
    has_point_cloud_calculated( false ),
    file_name(),
    version_text( QString( "%1 rev %2, Build date %3" ).arg( QCoreApplication::applicationName() ).arg( REVISION ).arg( __DATE__ ) ),
    model_folder(),
    output_folder()
{
    ui->setupUi( this );

    makeConnections();

    createPlot();

    enableRunButtons( false );
    ui->actionExport_As_Grid->setEnabled( false );
    ui->actionExport_as_CSV->setEnabled( false );
    ui->actionSave->setEnabled( false );

    QIntValidator* int_validator = new QIntValidator();
    int_validator->setBottom( 1 );
    ui->monomerSeedEdit->setValidator( int_validator );

    updateActivations();

    QSettings settings;
    settings.beginGroup( MAIN_WINDOW_GROUP );

    model_folder = settings.value( MODEL_FOLDER_NAME ).toString();
    output_folder = settings.value( OUTPUT_FOLDER_NAME ).toString();
    settings.endGroup();
}



void MainWindow::createPlot()
{
    ui->graphWidget->configure();
}



MainWindow::~MainWindow()
{
    QSettings settings;
    settings.beginGroup( MAIN_WINDOW_GROUP );

    settings.setValue( MODEL_FOLDER_NAME, model_folder );
    settings.setValue( OUTPUT_FOLDER_NAME, output_folder );
    settings.endGroup();

    delete ui;
}



bool MainWindow::initialize( int argc, char* argv[] )
{

    timer.setInterval( 5000 );

    connect( &timer, SIGNAL( timeout() ), this, SLOT( onTimeout() ) );
    ui->textEdit->hide();
    bool rc =  parseParameters( argc, argv );

    if ( true == rc )
    {
        initializeUserEntryFields();
    }

    appendText( versionTextStr() );

    return rc;
}


const char* MainWindow::versionText() const
{
    return version_text.toUtf8();
}



void MainWindow::refreshChains()
{
    QGuiApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ui->graphWidget->refreshChains();
    QGuiApplication::restoreOverrideCursor();
}



void MainWindow::makeConnections()
{
    connect( ui->actionNew, SIGNAL( triggered( bool ) ), this, SLOT( newButtonClicked() ) );
    connect( ui->actionOpen, SIGNAL( triggered( bool ) ), this, SLOT( loadButtonClicked() ) );
    connect( ui->actionSave, SIGNAL( triggered( bool ) ), this, SLOT( saveButtonClicked() ) );
    connect( ui->actionExport_As_Grid, SIGNAL( triggered( bool ) ), this, SLOT( exportAsGridButtonClicked() ) );
    connect( ui->actionExport_as_CSV, SIGNAL( triggered( bool ) ), this, SLOT( exportAsCsvButtonClicked() ) );
    connect( ui->actionImport_Monomer_data, SIGNAL( triggered( bool ) ), this, SLOT( importMonomerButtonClicked() ) );
    connect( ui->actionExit, SIGNAL( triggered( bool ) ), this, SLOT( exitButtonClicked() ) );
    connect( ui->actionExpose, SIGNAL( triggered( bool ) ), this, SLOT( exposeButtonClicked() ) );
    connect( ui->actionReload, SIGNAL( triggered( bool ) ), this, SLOT( reloadButtonClicked() ) );
    connect( ui->actionCalculate_Rg, SIGNAL( triggered( bool ) ), this, SLOT( calculateRgClicked() ) );

    pauseAction = new QAction( "Pause" );
    connect( pauseAction, SIGNAL( triggered( bool ) ), this, SLOT( pauseButtonClicked() ) );

    abortAction = new QAction( "Abort" );
    connect( abortAction, SIGNAL( triggered( bool ) ), this, SLOT( abortButtonClicked() ) );

    stepAction = new QAction( "Step" );
    connect( stepAction, SIGNAL( triggered( bool ) ), this, SLOT( singleStepButtonClicked() ) );

    slowAction = new QAction( "Slow" );
    connect( slowAction, SIGNAL( triggered( bool ) ), this, SLOT( slowButtonClicked() ) );

    fastAction = new QAction( "Fast" );
    connect( fastAction, SIGNAL( triggered( bool ) ), this, SLOT( fastButtonClicked() ) );

    ui->fastButton->setDefaultAction( fastAction );
    ui->slowButton->setDefaultAction( slowAction );
    ui->stepButton->setDefaultAction( stepAction );
    ui->abortButton->setDefaultAction( abortAction );
    ui->pauseButton->setDefaultAction( pauseAction );

    connect( ui->brushRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( brushRadioButtonClicked() ) );
    connect( ui->thinFilmRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( filmRadioButtonClicked() ) );
    connect( ui->unconstrainedRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( unconstrainedRadioButtonClicked() ) );
    connect( ui->pointCloudRadioButton, SIGNAL( clicked( bool ) ), this, SLOT( pointCloudRadioButtonClicked() ) );
    connect( ui->scanButton, SIGNAL( clicked( bool ) ), this, SLOT( scanButtonClicked() ) );
    connect( ui->reverseScanButton, SIGNAL( clicked( bool ) ), this, SLOT( reverseScanButtonClicked() ) );
    connect( ui->resetScanButton, SIGNAL( clicked( bool ) ), this, SLOT( resetScanButtonClicked() ) );
    connect( ui->endOfScanButton, SIGNAL( clicked( bool ) ), this, SLOT( endOfScanButtonClicked() ) );

    connect( ui->showMonomerCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( showMonomerCheckBoxClicked() ) );

    connect( ui->sequenceTypeComboBox, SIGNAL( activated( int ) ), this, SLOT( sequenceTypeComboBoxActivated( int ) ) );

    connect( ui->monomerAddButton, SIGNAL( clicked( bool ) ), ui->monomerTableWidget, SLOT( addRowActionTrigggered() ) );
    connect( ui->monomerRemoveButton, SIGNAL( clicked( bool ) ), ui->monomerTableWidget, SLOT( removeRowActionTriggered() ) );

    connect( ui->sequenceAddButton, SIGNAL( clicked( bool ) ), this, SLOT( sequenceAddButtonClicked() ) );
    connect( ui->sequenceRemoveButton, SIGNAL( clicked( bool ) ), this, SLOT( sequenceRemoveButtonClicked() ) );

    connect( ui->additiveAddButton, SIGNAL( clicked( bool ) ), this, SLOT( additiveAddButtonClicked() ) );
    connect( ui->additiveRemoveButton, SIGNAL( clicked( bool ) ), this, SLOT( additiveRemoveButtonClicked() ) );

    connect( ui->monomerTableWidget, SIGNAL( dataEdited() ), this, SLOT( updateMonomerList() ) );
    connect( ui->monomerTableWidget, SIGNAL( rowDeleted( QString ) ), this, SLOT( deleteRemovedMonomerFromSequenceLists( QString ) ) );

    connect( ui->colorationComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( colorationComboBoxChanged( int ) ) );
    connect( ui->applySpeciationButton, SIGNAL( clicked( bool ) ), this, SLOT( speciateButtonClicked( ) ) );

    connect( ui->scanChainSlider, SIGNAL( valueChanged( int ) ), this, SLOT( scanChainSliderValueChanged( int ) ) );
}



void MainWindow::sequenceTypeComboBoxActivated( int index )
{
    ui->sequenceStackedWidget->setCurrentIndex( index );

    updateActivations();
}



void MainWindow::monomerRemoveButtonClicked( )
{
    int row = ui->monomerTableWidget->currentRow();
    ui->monomerTableWidget->removeRow( row );
}



void MainWindow::sequenceRemoveButtonClicked( )
{
    switch ( ui->sequenceStackedWidget->currentIndex() )
    {
        case 0:
            break;

        case 1:
            ui->randomTableWidget->removeRowActionTriggered();
            break;

        case 2:
            ui->definedTableWidget->removeRowActionTriggered();
            break;
    }

    updateActivations();
}



void MainWindow::additiveAddButtonClicked()
{
    ui->additiveTableWidget->addRowActionTriggered();
    updateActivations();
}



void MainWindow::additiveRemoveButtonClicked()
{
    ui->additiveTableWidget->removeRowActionTriggered();
    updateActivations();
}



void MainWindow::updateMonomerList( )
{
    //reconstruct monomer list
    monomer_type_list.clearMonomerList();
    int row_count = ui->monomerTableWidget->rowCount();

    for ( int i = 0; i < row_count; i++ )
    {
        QColor qcolor = ui->monomerTableWidget->item( i, 1 )->background().color();
        Qwt3D::RGBA color( qcolor.redF(), qcolor.greenF(), qcolor.blueF(), qcolor.alphaF() );
        monomer_type_list.addMonomer( i,
                                      ui->monomerTableWidget->item( i, 0 )->text(),
                                      color,
                                      0.5
                                    );
    }

    // now communicate list of monomers to tables widgets
    ui->randomTableWidget->setMonomerNames( monomer_type_list.monomerNameList() );
    ui->definedTableWidget->setMonomerNames( monomer_type_list.monomerNameList() );
    ui->additiveTableWidget->setMonomerNames( monomer_type_list.monomerNameList() );

    updateActivations();
}



void MainWindow::deleteRemovedMonomerFromSequenceLists( QString name )
{
    ui->randomTableWidget->removeRowsWithMonomer( name );
    ui->definedTableWidget->removeRowsWithMonomer( name );
    ui->additiveTableWidget->removeRowsWithMonomer( name );

    updateActivations();
}



void MainWindow::updateActivations()
{
    if ( ui->pointCloudRadioButton->isChecked() )
    {
        ui->actionExport_as_CSV->setEnabled( has_point_cloud_calculated );
        ui->applySpeciationButton->setEnabled( has_point_cloud_calculated );
        ui->actionExport_As_Grid->setEnabled( false );
        ui->actionSave->setEnabled( false );
        ui->scanChainsGroupBox->setEnabled( false );

        //        pauseAction->setEnabled( false );
        //        abortAction->setEnabled( false );
        //        stepAction->setEnabled( false );
        //        slowAction->setEnabled( false );
    }
    else
    {
        ui->scanChainsGroupBox->setEnabled( true );
        stepAction->setEnabled( true );
        slowAction->setEnabled( true );

        if ( 0 == g_grid.max_chains )
        {
            ui->applySpeciationButton->setEnabled( false );
            ui->actionExport_As_Grid->setEnabled( false );
            ui->actionExport_as_CSV->setEnabled( false );
        }
        else
        {
            ui->actionExport_As_Grid->setEnabled( true );
            ui->actionExport_as_CSV->setEnabled( true );

            switch ( ui->sequenceStackedWidget->currentIndex() )
            {
                case 0:
                    ui->applySpeciationButton->setEnabled( true );
                    break;

                case 1:
                    ui->applySpeciationButton->setEnabled( ui->randomTableWidget->rowCount() > 0 );
                    break;

                case 2:
                    ui->applySpeciationButton->setEnabled( ui->definedTableWidget->rowCount() > 0 );
                    break;
            }
        }
    }
}



void MainWindow::clearAdditiveListList()
{
    for ( int i = 0; i < additive_list_list.count(); i++ )
    {
        delete additive_list_list.at( i );
    }

    additive_list_list.clear();
}



void MainWindow::scanChainSliderValueChanged( int newValue )
{
    scan_index = newValue;
    ui->graphWidget->scanChains( scan_index );
}



void MainWindow::calculateRgClicked()
{
    float radius = ui->graphWidget->avgRadiusOfGyration( &g_grid );
    float dist =  ui->graphWidget->avgEndToEndDistance( &g_grid );

    QString msg = QString( "Radius of gyration for file %1 is %2, end to end distance is %3" ).arg( file_name ).arg( radius ).arg( dist );
    QMessageBox::information( this, "Calculate", msg );
}



void MainWindow::monomerAddButtonClicked( )
{
    int row = ui->monomerTableWidget->currentRow();
    if ( row < 0 )
    {
        row = 0;
    }

    ui->monomerTableWidget->insertRow( row );
}



void MainWindow::sequenceAddButtonClicked( )
{
    switch ( ui->sequenceStackedWidget->currentIndex() )
    {
        case 0:
            break;

        case 1:
            ui->randomTableWidget->addRowActionTriggered();
            break;

        case 2:
            ui->definedTableWidget->addRowActionTriggered();
            break;
    }

    updateActivations();
}



int MainWindow::setupAdditiveCalculation()
{
    return 0;
}


void MainWindow::readInputs()
{
    g_params.bond_angle = ui->bondAngleEdit->text().toFloat() * M_PI / 180.0;
    g_params.bond_len = ui->bondLengthEdit->text().toFloat();
    g_params.atom_radius = ui->monomerRadiusEdit->text().toFloat();
    g_params.kappa = ui->kappaEdit->text().toFloat();
    g_params.z_exponent = ui->zExponentEdit->text().toFloat();
    g_grow_params.chain_len = ui->degreeOfPolymEdit->text().toFloat();
    g_grow_params.dispersity = ui->polydispersityEdit->text().toFloat();
    g_grow_params.seed = ui->randomNumSeedEdit->text().toInt();
    g_grow_params.nr_particles = ui->numMonomersEdit->text().toInt();
    g_grow_params.max_overlap = ui->maxOverlapEdit->text().toFloat();
    g_params.film = ui->thinFilmRadioButton->isChecked();
    g_params.brush = ui->brushRadioButton->isChecked();
    g_grow_params.ahead_depth = ui->searchDepthSpinBox->value();
    g_grow_params.nr_angles  = ui->numAnglesSpinBox->value( );
    g_grow_params.nr_chain_trials  = ui->numTrialSpinBox->value( );
    g_params.point_cloud = ui->pointCloudRadioButton->isChecked();
    brush_density = ui->brushDensityEdit->text().toFloat();
    density = ui->densityEdit->text().toFloat();
    x_proportion = ui->xProportionEdit->text().toFloat();
    y_proportion = ui->yProportionEdit->text().toFloat();

    calculateBoxSize();

    if ( 0 != ui->useAdditivesComboBox->currentIndex() )
    {
        chainList().prependAdditives( 2 == ui->useAdditivesComboBox->currentIndex() );
        additive_list.loadListFromTable( ui->additiveTableWidget, monomer_type_list );
        g_grow_params.nr_particles = additive_list.apportionParticles( g_grow_params.nr_particles );
    }
}



void MainWindow::calculateBoxSize()
{
    if ( density < EPS ) density = EPS;
    if ( brush_density < EPS ) brush_density = EPS;

    float volume = g_grow_params.nr_particles / density;

    if ( g_params.brush && g_grow_params.chain_len > 0 )
    {
        int nr_chains = g_grow_params.nr_particles / g_grow_params.chain_len;
        if ( nr_chains < 1 ) nr_chains = 1;
        float area = 1.0 * nr_chains / brush_density;
        g_params.box_size.x = sqrt( area );
        g_params.box_size.y = sqrt( area );
        g_params.box_size.z = volume / area;
    }
    else
    {
        float len = exp( 1.0 / 3.0 * log( volume / ( x_proportion * y_proportion ) ) ); /* ^(1/3) */
        g_params.box_size.x = len * x_proportion;
        g_params.box_size.y = len * y_proportion;
        g_params.box_size.z = len;
    }

    appendText( QString( "bounding box: x = (0.0,%1), y = (0.0,%2), z = (0.0,%3)" ).arg( g_params.box_size.x ).arg( g_params.box_size.y ).arg( g_params.box_size.z ) );
}



void MainWindow::drawChains()
{
    ui->graphWidget->setShowMonomer( ui->showMonomerCheckBox->isChecked() );
    ui->graphWidget->setColoration( coloration() );

    if ( g_params.point_cloud )
    {
        seedRandomNumberGenerator( g_grow_params.seed );
        ui->graphWidget->drawPointCloud( g_params, g_grow_params.nr_particles, sequenceList(), additiveList() );
        has_point_cloud_calculated = true;
    }
    else
    {
        ui->graphWidget->drawChains( &g_grid, monomerList() );
    }
}



void MainWindow::setCurrentChainTargetLength( int length )
{
    ui->chainProgressBar->setMaximum( length );
    ui->chainProgressBar->setValue( 0 );
    target_chain_length = length;
}



void MainWindow::updateCurrentChainLength( int length )
{
    ui->chainProgressBar->setValue( length );
}



enum COLORATION MainWindow::coloration() const
{
    switch ( ui->colorationComboBox->currentIndex() )
    {
        case 0:
                default:
                        return MONOCHROME;
            break;

        case 1:
            return CHAIN;
            break;

        case 2:
            return MONOMER;
            break;
    }
}



void MainWindow::parametersToDefaultVaLues()
{
    density = 1.00;
    brush_density = 1.00;

    g_params.bond_len    = 0.9;
    g_params.atom_radius = 0.5;

    g_params.angle_fixed = true;
    g_params.brush       = false;
    g_params.bond_angle  = 70.5  * M_PI / 180.0; // tetrahedral  nagle - 180 -109.5 degrees conveted to radians
    g_params.kappa       = 4.0;
    g_params.z_exponent  = 0.0;
    g_params.film       = false;
    g_params.point_cloud = false;

    g_grow_params.nr_packings = 1;
    g_grow_params.chain_len   = 100;
    g_grow_params.nr_chain_trials = 0;
    g_grow_params.max_overlap = 0.1;
    g_grow_params.nr_angles   = 20;
    g_grow_params.ahead_depth = 10;
    g_grow_params.nr_particles = 10000;
    g_grow_params.seed = 1427;
    g_grow_params.dispersity = 1.0;
}



bool MainWindow::parseParameters( int argc, char* argv[] )
{
    int i;

    parametersToDefaultVaLues();

    // parse command line for changed values

    for ( i = 1; i < argc; i++ )
    {
        if ( !strncmp( argv[i], "-l", 2 ) )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%f", &g_params.bond_len );
        }
        else if ( !strncmp( argv[i], "-a", 2 ) )
        {
            i++;
            if ( i < argc )
            {
                sscanf( argv[i], "%f", &g_params.bond_angle );
                g_params.bond_angle *= M_PI / 180.0;
            }
        }
        else if ( strncmp( argv[i], "-r", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%f", &g_params.atom_radius );
        }

        else if ( strncmp( argv[i], "-m", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%i", &g_grow_params.nr_particles );
        }

        else if ( strncmp( argv[i], "-c", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%i", &g_grow_params.chain_len );
        }

        else if ( strncmp( argv[i], "-p", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%i", &g_grow_params.nr_packings );
        }
        else if ( strncmp( argv[i], "-d", 2 ) == 0 )
        {
            i++;
            if ( i < argc )
            {
                sscanf( argv[i], "%f", &density );
            }
        }
        else if ( strncmp( argv[i], "-t", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%i", &g_grow_params.nr_chain_trials );
        }
        else if ( strncmp( argv[i], "-o", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%f", &g_grow_params.max_overlap );
        }
        else if ( strncmp( argv[i], "-s", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%i", &g_grow_params.ahead_depth );
        }
        else if ( strncmp( argv[i], "-k", 2 ) == 0 )
        {
            i++;
            if ( i < argc )
            {
                sscanf( argv[i], "%f", &g_params.kappa );
                g_params.angle_fixed = false;
            }
        }
        else if ( strncmp( argv[i], "-n", 2 ) == 0 )
        {
            i++;
            if ( i < argc ) sscanf( argv[i], "%i", &g_grow_params.nr_angles );
        }
        else if ( strncmp( argv[i], "-b", 2 ) == 0 )
        {
            g_params.brush = true;
            i++;
            if ( i < argc )
            {
                sscanf( argv[i], "%f", &brush_density );
            }
        }

        else if ( strncmp( argv[i], "-f", 2 ) == 0 )
        {
            g_params.film = true;
            i++;
        }

        else if ( strncmp( argv[i], "-z", 2 ) == 0 )
        {
            i++;
            if ( i < argc )
            {
                sscanf( argv[i], "%f", &g_params.z_exponent );
                g_params.z_exponent = fabs( g_params.z_exponent );
            }
        }
        else
        {
            return false;
        }
    }

    calculateBoxSize();

    return true;
}



void MainWindow::initializeMonomerWidgets()
{
    ui->monomerTableWidget->initialize( monomer_type_list );
    ui->definedTableWidget->setMonomerNames( monomer_type_list.monomerNameList() );
    ui->randomTableWidget->setMonomerNames( monomer_type_list.monomerNameList() );
    ui->additiveTableWidget->setMonomerNames( monomer_type_list.monomerNameList() );

    ui->sequenceTypeComboBox->setCurrentIndex( sequenceList()->sequenceType() );
    sequenceTypeComboBoxActivated( sequenceList()->sequenceType() );

    switch ( sequenceList()->sequenceType() )
    {
        case MonomerSequence::HOMOPOLYMER:
            break;

        case MonomerSequence::RANDOM:
            ui->randomTableWidget->initialize( *sequenceList() );
            break;

        case MonomerSequence::ORDERED:
            ui->definedTableWidget->initialize( *sequenceList() );
            break;
    }

    ui->additiveTableWidget->initialize( *additiveList() );
}



void  MainWindow::updateWindowTitle( const QString& filename )
{
    setWindowTitle( QString( "Polymer Glass Growth %1 %2" ).arg( !filename.isEmpty() ? " - " : "" ).arg( !filename.isEmpty() ? filename : "" ) );
}



void MainWindow::initializeUserEntryFields()
{
    ui->textEdit->clear();

    ui->bondAngleEdit->setText( QString::number( g_params.bond_angle * 180.0 / M_PI ) );

    ui->bondLengthEdit->setText( QString::number( g_params.bond_len ) );
    ui->monomerRadiusEdit->setText( QString::number( g_params.atom_radius ) );
    ui->kappaEdit->setText( QString::number( g_params.kappa ) );
    ui->zExponentEdit->setText( QString::number( g_params.z_exponent ) );

    ui->degreeOfPolymEdit->setText( QString::number( g_grow_params.chain_len ) );
    ui->polydispersityEdit->setText( QString::number( g_grow_params.dispersity ) );

    ui->yProportionEdit->setText( QString::number( g_params.box_size.y / g_params.box_size.z ) );
    ui->xProportionEdit->setText( QString::number( g_params.box_size.x / g_params.box_size.z ) );

    ui->randomNumSeedEdit->setText( QString::number( g_grow_params.seed ) );
    ui->maxOverlapEdit->setText( QString::number( g_grow_params.max_overlap ) );
    ui->numMonomersEdit->setText( QString::number( g_grow_params.nr_particles ) );

    ui->brushDensityEdit->setText( QString::number( brush_density ) );
    ui->densityEdit->setText( QString::number( density ) );

    if ( true == g_params.film )
    {
        ui->thinFilmRadioButton->setChecked( true );
        ui->brushDensityLabel->setEnabled( false );
        ui->brushDensityEdit->setEnabled( false );
    }
    else
    {
        if ( g_params.brush )
        {
            ui->brushRadioButton->setChecked( true );
            ui->brushDensityLabel->setEnabled( true );
            ui->brushDensityEdit->setEnabled( true );
        }
        else
        {
            if ( g_params.point_cloud )
            {
                ui->pointCloudRadioButton->setChecked( true );
                ui->brushDensityLabel->setEnabled( false );
                ui->brushDensityEdit->setEnabled( false );
            }
            else
            {
                ui->unconstrainedRadioButton->setChecked( true );
                ui->brushDensityLabel->setEnabled( false );
                ui->brushDensityEdit->setEnabled( false );
            }
        }

    }

    ui->searchDepthSpinBox->setValue( g_grow_params.ahead_depth );
    ui->numAnglesSpinBox->setValue( g_grow_params.nr_angles );
    ui->numTrialSpinBox->setValue( g_grow_params.nr_chain_trials );

    initializeMonomerWidgets();

    updateActivations();
}




void MainWindow::appendText( const char* s )
{
    appendText( QString( s ) );
}



void MainWindow::appendText( const QString& s )
{
    ui->textEdit->show();

    ui->textEdit->append( s );
}



void MainWindow::updateStatus( int chain_num, int monomer_num )
{
    ui->calculationProgressBar->setValue( monomer_num );
    ui->calculationProgressText->setText( QString( "%1 monomers placed" ).arg( monomer_num ) );
    ui->chainProgressText->setText( QString( "chain number: %1 of %3, target chain length: %2" ).arg( chain_num + 1 ).arg( target_chain_length ).arg( total_num_chains ) );
    num_chains_created = chain_num + 1;
}



void MainWindow::exportAsGridButtonClicked()
{
    ExportGridDialog dlg( &g_grid, this );

    dlg.exec();

}



void MainWindow::exportAsCsvButtonClicked()
{
    const QString EXTENSION( ".csv" );

    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save File" ), output_folder, tr( "CSV files (*.csv)" ) );

    if ( false == fileName.isEmpty() )
    {
        if ( false == fileName.endsWith( EXTENSION, Qt::CaseInsensitive ) )
        {
            fileName += EXTENSION;
        }

        exportAsCSV( fileName );
        setOutputFolder( QFileInfo( fileName ).absolutePath() );
    }
    else
    {
        return;
    }
}



void MainWindow::importMonomerButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr( "Import from File" ) );

    if ( false == fileName.isEmpty() )
    {
        QFile inputFile( fileName );

        if ( inputFile.open( QIODevice::ReadOnly ) )
        {
            QTextStream in( &inputFile );

            while ( !in.atEnd() )
            {
                QString line = in.readLine();

                if ( line.contains( "monomer_list    =" ) )
                {
                    monomer_type_list.fromJsonString( line.remove( 0, 20 ) );
                }

                if ( line.contains( "sequence_list   =" ) )
                {
                    sequence_list.fromJsonString( line.remove( 0, 20 ), monomer_type_list );
                }

                if ( line.contains( "additive_list   =" ) )
                {
                    additive_list.fromJsonString( line.remove( 0, 20 ), monomer_type_list );
                    inputFile.close();
                    initializeMonomerWidgets();
                    updateActivations();
                    return;
                }
            }
            QMessageBox::warning( this, "Warning", "Valid Monomer data were not found in the file" );
            inputFile.close();
        }
    }
}



bool MainWindow::exportAsCSV( const QString& fileName )
{
    QStringList name_list = monomer_type_list.monomerNameList();
    QFile file( fileName );

    file.open( QIODeviceBase::WriteOnly );

    QTextStream stream( &file );

    stream << "x,y,z,monomer_name,monomer_id,chain_index" << Qt::endl;

    const AtomVector& monomers = ui->graphWidget->monomerArray();
    int num_monomers = monomers.size();
    for ( int i = 2; i < num_monomers; i++ )
    {
        Atom m = monomers[i];

        stream << m.pos.x << "," << m.pos.y << "," << m.pos.z;

        if ( 0 <= m.monomer_type && 0 < name_list.count() )
        {
            stream << "," << name_list.at( m.monomer_type ) << "," << m.monomer_type;
        }

        if ( Qwt3D::INVALID_CHAIN_INDEX != m.chain_index )
        {
            stream << "," << m.chain_index;
        }

        stream << Qt::endl;
    }

    file.close();
    return true;
}



void MainWindow::exitButtonClicked()
{
    abortButtonClicked();
    qApp->exit();
}



void MainWindow::exposeButtonClicked()
{
    ExposureDialog dlg( &monomer_type_list, ui->graphWidget->monomerArray(), this );
    dlg.exec();
}



void MainWindow::abortButtonClicked()
{
    abortAction->setEnabled( false );

    timer.stop();

    enableRunButtons( false );
    pauseAction->setEnabled( false );

    setInterfaceState( STATE_ABORT );
}



void MainWindow::pauseButtonClicked()
{
    timer.stop();

    setInterfaceState( STATE_PAUSE );
}



void MainWindow::singleStepButtonClicked()
{
    timer.stop();
    enableRunButtons( true );

    if ( STATE_NOT_STARTED == getInterfaceState() )
    {
        setInterfaceState( STATE_STEP );
        startCalculation();
    }
    else
    {
        setInterfaceState( STATE_STEP );
    }

}



void MainWindow::slowButtonClicked()
{
    timer.stop();

    enableRunButtons( true );

    if ( STATE_NOT_STARTED == getInterfaceState() )
    {
        setInterfaceState( STATE_GO );
        startCalculation();
    }
    else
    {
        setInterfaceState( STATE_GO );
    }
}



void MainWindow::fastButtonClicked()
{
    enableRunButtons( true );

    timer.start();

    if ( STATE_NOT_STARTED == getInterfaceState() )
    {
        setInterfaceState( STATE_FAST );
        startCalculation();
    }
    else
    {
        setInterfaceState( STATE_FAST );
    }
}



void MainWindow::newButtonClicked()
{
    parametersToDefaultVaLues();
    calculateBoxSize();

    initializeUserEntryFields();

    ui->monomerTableWidget->removeAllRows();
    ui->randomTableWidget->removeAllRows();
    ui->definedTableWidget->removeAllRows();
    ui->additiveTableWidget->removeAllRows();

    ui->sequenceTypeComboBox->setCurrentIndex( 0 );
    ui->sequenceStackedWidget->setCurrentIndex( 0 );
    ui->colorationComboBox->setCurrentIndex( 0 );

    updateActivations();

    ui->graphWidget->clear();
}



void MainWindow::brushRadioButtonClicked()
{
    ui->brushDensityLabel->setEnabled( true );
    ui->brushDensityEdit->setEnabled( true );
}



void MainWindow::filmRadioButtonClicked()
{
    ui->brushDensityLabel->setEnabled( false );
    ui->brushDensityEdit->setEnabled( false );
}



void MainWindow::unconstrainedRadioButtonClicked()
{
    ui->brushDensityLabel->setEnabled( false );
    ui->brushDensityEdit->setEnabled( false );
}



void MainWindow::pointCloudRadioButtonClicked()
{
    ui->brushDensityLabel->setEnabled( false );
    ui->brushDensityEdit->setEnabled( false );
    has_point_cloud_calculated = false;
    updateActivations();
}



void MainWindow::scanButtonClicked()
{
    if ( scan_index < num_chains_created )
    {
        ui->graphWidget->scanChains( scan_index );
        scan_index++;
        ui->scanChainSlider->setValue( scan_index );

    }
}



void MainWindow::endOfScanButtonClicked()
{
    scan_index = num_chains_created - 1;

    ui->graphWidget->scanChains( scan_index );
    ui->scanChainSlider->setValue( scan_index );

}



void MainWindow::reverseScanButtonClicked()
{
    if ( scan_index > 0 )
    {
        scan_index--;
    }
    ui->graphWidget->scanChains( scan_index );
    ui->scanChainSlider->setValue( scan_index );
}



void MainWindow::resetScanButtonClicked()
{
    scan_index = 0;
    scanButtonClicked();
    ui->scanChainSlider->setValue( scan_index );
}



void MainWindow::showMonomerCheckBoxClicked()
{
    drawChains();
}



void MainWindow::colorationComboBoxChanged( int )
{
    drawChains();
}



void MainWindow::speciateSpecies()
{
    updateMonomerList();
    sequence_list.setRandomNumberSeed( ui->monomerSeedEdit->text().toInt() );

    switch ( ui->sequenceTypeComboBox->currentIndex() )
    {
        case 0:
            sequence_list.loadSequenceFromTable( MonomerSequence::HOMOPOLYMER, monomer_type_list, 0 );
            break;

        case 1:
            sequence_list.loadSequenceFromTable( MonomerSequence::RANDOM, monomer_type_list, ui->randomTableWidget );
            break;

        case 2:
            sequence_list.loadSequenceFromTable( MonomerSequence::ORDERED, monomer_type_list, ui->definedTableWidget );
            break;
    }

    if ( g_params.point_cloud )
    {
        if ( has_point_cloud_calculated )
        {
            ui->graphWidget->recolorPointCloud( &sequence_list, &additive_list );
        }
        else
        {
            drawChains();
        }
    }
    else
    {
        if ( false == chainList().additivesPrepended() )
        {
            int chain_nr = 0;
            int num_chains = ( g_grid.max_chains < chain_list.polymerChainCount() ) ?  g_grid.max_chains : chain_list.polymerChainCount() ;

            // do the polymer chains first
            for ( ; chain_nr < num_chains; chain_nr++ )
            {
                Chain* chain = &g_grid.chains[chain_nr];
                int len = chain->last - chain->first;
                if ( len <= 0 ) continue;

                for ( int i = chain->first; i < chain->last; i++ )
                {

                    int k = i - chain->offset;
                    if ( i == chain->first )
                    {
                        const Monomer* m = sequence_list.firstMonomer();
                        chain->atoms[k].monomer_type = m->typeID();
                    }
                    else
                    {
                        const Monomer* m = sequence_list.nextMonomer();
                        chain->atoms[k].monomer_type = m->typeID();
                    }
                }
            }
            //  then colorize any monomers

            additive_list.loadListFromTable( ui->additiveTableWidget, monomer_type_list );

            for ( int j = 0; j < chain_list.numAdditives(); j++ )
            {
                int additive_type_id  = additive_list.additive( j )->typeID();
                num_chains += chain_list.additiveChainCount( j );

                if ( num_chains  >  g_grid.max_chains )
                {
                    num_chains  =  g_grid.max_chains;
                }

                for ( ; chain_nr < num_chains; chain_nr++ )
                {
                    Chain* chain = &g_grid.chains[chain_nr];
                    int len = chain->last - chain->first;
                    if ( len <= 0 ) continue;

                    for ( int i = chain->first; i < chain->last; i++ )
                    {
                        int k = i - chain->offset;

                        chain->atoms[k].monomer_type = additive_type_id;
                    }
                }
            }

        }
        else
        {
            int chain_nr = 0;
            int num_chains = 0;

            //  first colorize any monomers

            additive_list.loadListFromTable( ui->additiveTableWidget, monomer_type_list );

            for ( int j = chain_list.numAdditives() - 1; j >= 0 ; j-- )
            {
                int additive_type_id  = additive_list.additive( j )->typeID();
                num_chains += chain_list.additiveChainCount( j );

                if ( num_chains  >  g_grid.max_chains )
                {
                    num_chains  =  g_grid.max_chains;
                }

                for ( ; chain_nr < num_chains; chain_nr++ )
                {
                    Chain* chain = &g_grid.chains[chain_nr];
                    int len = chain->last - chain->first;
                    if ( len <= 0 ) continue;

                    for ( int i = chain->first; i < chain->last; i++ )
                    {
                        int k = i - chain->offset;

                        chain->atoms[k].monomer_type = additive_type_id;
                    }
                }
            }

            num_chains += chain_list.polymerChainCount();

            if ( num_chains  >  g_grid.max_chains )
            {
                num_chains  =  g_grid.max_chains;
            }

            // do the polymer chains next
            for ( ; chain_nr < num_chains; chain_nr++ )
            {
                Chain* chain = &g_grid.chains[chain_nr];
                int len = chain->last - chain->first;
                if ( len <= 0 ) continue;

                for ( int i = chain->first; i < chain->last; i++ )
                {

                    int k = i - chain->offset;
                    if ( i == chain->first )
                    {
                        const Monomer* m = sequence_list.firstMonomer();
                        chain->atoms[k].monomer_type = m->typeID();
                    }
                    else
                    {
                        const Monomer* m = sequence_list.nextMonomer();
                        chain->atoms[k].monomer_type = m->typeID();
                    }
                }
            }
        }

        drawChains();
    }

    const MonomerCount& mcount = ui->graphWidget->updateMonomerCount( monomer_type_list.count() );

    int total_monomers = 0;

    for ( int i = 0; i < mcount.count(); i++ )
    {
        total_monomers += mcount[i];
    }

    QString msg( "Monomer counts: " );

    for ( int j = 0; j < mcount.count(); j++ )
    {
        msg += QString( "%1 = %2 (%3%)" ).arg( monomer_type_list.monomer( j )->name() ).arg( mcount[j] ).arg( ( double ) 100.0 * mcount[j] / total_monomers, 0, 'g', 4 );

        if ( j < mcount.count() - 1 )
        {
            msg += ", ";
        }
    }

    appendText( msg );
}



void MainWindow::speciateButtonClicked()
{
    speciateSpecies();
}



void MainWindow::saveButtonClicked()
{
    file_name = QFileDialog::getSaveFileName( this, tr( "Save File" ) );

    if ( false == file_name.isEmpty() )
    {
        QString monomer_str = monomer_type_list.toJsonString();
        QString sequence_str = sequence_list.toJsonString();

        additive_list.setUseAdditives( ( AdditiveList::ADDITIVE_USE ) ui->useAdditivesComboBox->currentIndex() );
        QString additive_str = additive_list.toJsonString();

        Save_System( file_name.toLocal8Bit().data(), &g_grid, &g_grow_params, monomer_str.toLocal8Bit(), sequence_str.toLocal8Bit(), additive_str.toLocal8Bit() );
        updateWindowTitle( file_name );

        model_folder = QFileInfo( file_name ).absolutePath();
    }
}



void MainWindow::loadModel()
{
    if ( false == file_name.isEmpty() )
    {
        QGuiApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

        Grid_free( &g_grid );

        QByteArray json_monomer_list;
        QByteArray json_sequence_list;
        QByteArray json_additive_list;

        num_chains_created = Load_System( file_name.toLocal8Bit().data(), &g_grid, &g_grow_params, json_monomer_list, json_sequence_list, json_additive_list );
        monomer_type_list.fromJsonString( QString( json_monomer_list ) );
        sequence_list.fromJsonString( QString( json_sequence_list ), monomer_type_list );
        additive_list.fromJsonString( QString( json_additive_list ), monomer_type_list );
        updateWindowTitle( file_name );
        g_params = g_grid.params;

        initializeUserEntryFields();
        ui->useAdditivesComboBox->setCurrentIndex( ( int ) additive_list.useAdditives() );
        chainList().prependAdditives( AdditiveList::Prepended == additive_list.useAdditives() );

        ui->elapsedTimeLabel->setText( QString( "%2 chains loaded from file %1" ).arg( file_name ).arg( num_chains_created ) );

        ui->graphWidget->clear();
        ui->scanChainSlider->setMaximum( num_chains_created );
        drawChains();
        speciateSpecies();


        model_folder = QFileInfo( file_name ).absolutePath();

        QGuiApplication::restoreOverrideCursor();
    }
}



void MainWindow::loadButtonClicked()
{
    file_name = QFileDialog::getOpenFileName( this, tr( "Save File" ), model_folder );

    loadModel();
}



void MainWindow::reloadButtonClicked()
{
    loadModel();
}



void MainWindow::onTimeout()
{
    drawChains();
    updateElapsedTime();
}



QString MainWindow::runningTime()
{
    int msec = elapsed_time.elapsed();

    QTime tmptime = QTime( 0, 0, 0, 0 ).addMSecs( msec );
    return tmptime.toString( "h 'hr' m 'min' s 'sec'" );
}



void MainWindow::enableRunButtons( bool state )
{
    ui->actionExit->setEnabled( !state );
    ui->actionSave->setEnabled( !state );
    ui->actionOpen->setEnabled( !state );
    ui->actionExport_As_Grid->setEnabled( !state );
    ui->actionExport_as_CSV->setEnabled( !state );
    pauseAction->setEnabled( state );
    abortAction->setEnabled( state );
}



void MainWindow::updateElapsedTime()
{
    ui->elapsedTimeLabel->setText( QString( "calculation running time %1" ).arg( runningTime() ) );
}



void MainWindow::startCalculation()
{
    Grid_free( &g_grid );

    readInputs();

    int old_coloration = ui->colorationComboBox->currentIndex();
    ui->colorationComboBox->setCurrentIndex( 0 );

    ui->graphWidget->clear();

    target_chain_length = 0;
    num_chains_created = 0;

    chain_list.configure( g_grow_params.nr_particles, g_grow_params.chain_len, g_grow_params.dispersity, g_grow_params.seed );


    for ( int i = 0; i < additiveList()->count(); i++ )
    {
        const Additive* a = additiveList()->additive( i );
        chainList().appendAdditive( a->numParticles(), a->avgClusterSize(), a->usePoisson() );
        g_grow_params.nr_particles += a->numParticles();
    }

    ui->calculationProgressBar->setMaximum( g_grow_params.nr_particles );
    ui->calculationProgressBar->setValue( 0 );

    total_num_chains = chain_list.chainCount();

    seedRandomNumberGenerator( g_grow_params.seed );
    Vector_set_kappa( g_params.kappa );

    ui->elapsedTimeLabel->setText( "" );

    elapsed_time.start();
    if ( g_params.point_cloud )
    {
        setInterfaceState( STATE_FAST );
        drawChains();
    }
    else
    {
        ui->graphWidget->setTitle( QString( "Polymer chains" ) );
        Grid_init( &g_grid, &g_params );
        Pack( &g_grid, &g_grow_params );
    }
    timer.stop();

    ui->chainProgressBar->setValue( ui->chainProgressBar->maximum() );
    ui->calculationProgressBar->setValue( ui->calculationProgressBar->maximum() );

    ui->calculationProgressText->setText( QString( "%1 monomers placed" ).arg( STATE_NOT_STARTED != getInterfaceState() ? g_grow_params.nr_particles : ui->calculationProgressBar->value() ) );

    setInterfaceState( STATE_NOT_STARTED );

    if ( !g_params.point_cloud )
    {
        ui->elapsedTimeLabel->setText( QString( "calculation of %1 chains completed in %2, max overlap %3" ).arg( num_chains_created ).arg( runningTime() ).arg( Grid_max_overlap( &g_grid, &g_grow_params ) ) );
        appendText( QString( "Final avg radius of gyration: %1" ).arg( ui->graphWidget->avgRadiusOfGyration() ) );
        speciateSpecies();
    }

    enableRunButtons( false );

    ui->scanChainSlider->setMaximum( num_chains_created );
    ui->scanChainSlider->setValue( 0 );

    updateActivations();

    ui->colorationComboBox->setCurrentIndex( old_coloration );
    drawChains();
}
