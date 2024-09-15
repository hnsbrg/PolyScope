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

#include "exposuredialog.h"
#include "ui_exposuredialog.h"
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

// for qsettings
const QString EXPOSURE_DIALOG_GROUP( "ExposureDialog" );
const QString DEPROTECTED_NAME( "DeprotectedSpecies" );
const QString PROTECTED_NAME( "ProtectedSpecies" );
const QString PAG_NAME( "PAGSpecies" );
const QString EXPOSED_PAG_NAME( "ExposedPAGSpecies" );
const QString QUENCHER_NAME( "QuencherSpecies" );
const QString NONREACTIVE_IONIZABLE_NAME( "NonReactiveIonizableSpecies" );
const QString NEUTRALIZED_QUENCHER_NAME( "NeutralizedQuencherSpecies" );
const QString FRACTION_EXPOSED_NAME( "FractionExposed" );
const QString VOLUME_RADIUS_NAME( "VolumeRadius" );
const QString SEED_NAME( "Seed" );
const QString EXPOSURE_TYPE_NAME( "ExopsureType" );
const QString ACIDS_PER_PHOTON_NAME( "AcidsPerPhoton" );
const QString INTERACTION_RADIUS_NAME( "InteractionRadius" );
const QString SOLBILITY_THRESHOLD_NAME( "SolubilityThreshols" );

ExposureDialog::ExposureDialog( MonomerList* monomerList, Qwt3D::AtomVector& monomers, MainWindow* parent ) :
    QDialog( parent ),
    ui( new Ui::ExposureDialog ),
    seed( 0 ),
    exposed_fraction( 0.0 ),
    deprotection_radius( 0.0 ),
    monomer_vector( monomers ),
    monomer_list( monomerList ),
    pag_indices(),
    protected_and_quencher_indices(),
    deprotected_indices(),
    neutralized_quencher_indices(),
    photolyzed_pag_indices(),
    clustered_photolyzed_pag_indices(),
    exposure_type( Random ),
    acid_cluster_radius( 1.0 ),
    acids_per_photon( 3.0 ),
    rng(),
    apply_button( 0 ),
    reset_button( 0 ),
    main_window( ( MainWindow* ) parent )
{
    ui->setupUi( this );
    initialize();
}



ExposureDialog::~ExposureDialog()
{
    delete ui;
}



void ExposureDialog::initialize()
{
    apply_button = ui->buttonBox->button( QDialogButtonBox::Apply );
    reset_button = ui->buttonBox->button( QDialogButtonBox::Reset );

    if ( main_window->fileName().isEmpty() )
    {
        reset_button->setEnabled( false );
    }

    QStringList monomer_names = monomer_list->monomerNameList();
    ui->deprotectedComboBox->addItems( monomer_names );
    ui->protectedComboBox->addItems( monomer_names );
    ui->pagComboBox->addItems( monomer_names );
    ui->quencherComboBox->addItems( monomer_names );
    ui->neutralizedQuencherComboBox->addItems( monomer_names );
    ui->exposedPAGComboBox->addItems( monomer_names );
    ui->ionizableComboBox->addItems( monomer_names );

    QSettings settings;
    settings.beginGroup( EXPOSURE_DIALOG_GROUP );

    QString name = settings.value( DEPROTECTED_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->deprotectedComboBox->setCurrentText( name );
    }

    name = settings.value( PROTECTED_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->protectedComboBox->setCurrentText( name );
    }

    name = settings.value( PAG_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->pagComboBox->setCurrentText( name );
    }

    name = settings.value( EXPOSED_PAG_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->exposedPAGComboBox->setCurrentText( name );
    }

    name = settings.value( QUENCHER_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->quencherComboBox->setCurrentText( name );
    }

    name = settings.value( NEUTRALIZED_QUENCHER_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->neutralizedQuencherComboBox->setCurrentText( name );
    }

    name = settings.value( NONREACTIVE_IONIZABLE_NAME ).toString();
    if ( !name.isEmpty() )
    {
        ui->ionizableComboBox->setCurrentText( name );
    }

    double val = settings.value( FRACTION_EXPOSED_NAME, -1.0 ).toDouble();
    if ( 0.0 < val )
    {
        ui->fractionExposedSpinBox->setValue( val );
    }

    val = settings.value( VOLUME_RADIUS_NAME, -1.0 ).toDouble();
    if ( 0.0 < val )
    {
        ui->reactionRadiusSpinBox->setValue( val );
    }

    val = settings.value( ACIDS_PER_PHOTON_NAME, -1.0 ).toDouble();
    if ( 0.0 < val )
    {
        ui->acidsPerPhotonSpinBox->setValue( val );
    }

    val = settings.value( INTERACTION_RADIUS_NAME, -1.0 ).toDouble();
    if ( 0.0 < val )
    {
        ui->electronInteractionDistanceSpinBox->setValue( val );
    }

    val = settings.value( SOLBILITY_THRESHOLD_NAME, -1.0 ).toDouble();
    if ( 0.0 < val )
    {
        ui->solubilityThresholdSpinBox->setValue( val );
    }

    name = settings.value( SEED_NAME, -1 ).toString();
    if ( 0 < name.toInt() )
    {
        ui->randomNumberSeedEdit->setText( name );
    }

    ui->exposureTypeComboBox->setCurrentIndex( settings.value( EXPOSURE_TYPE_NAME, 0 ).toInt() );

    settings.endGroup();

    connect( ui->selectFileButton, SIGNAL( clicked( bool ) ), this, SLOT( selectFileButtonClicked() ) );
    connect( apply_button, SIGNAL( clicked( bool ) ), this, SLOT( apply() ) );
    connect( reset_button, SIGNAL( clicked( bool ) ), this, SLOT( reset() ) );
    connect( ui->exposureTypeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( exposureTypeChanged( int ) ) );

    exposureTypeChanged( ui->exposureTypeComboBox->currentIndex() );
}




void ExposureDialog::readInputs()
{
    ui->textEdit->append( QString( "reading inputs..." ) );

    int protected_type = ui->protectedComboBox->currentIndex();
    int pag_type = ui->pagComboBox->currentIndex();
    int quencher_type = ui->quencherComboBox->currentIndex();

    rng.seed( ui->randomNumberSeedEdit->text().toInt() );
    deprotection_radius = ui->reactionRadiusSpinBox->value();
    exposed_fraction = ui->fractionExposedSpinBox->value();

    pag_indices.clear();
    protected_and_quencher_indices.clear();

    int num_monomers = monomer_vector.size();

    int monomer_count = 0;
    for ( int i = 2; i < num_monomers; i++ )
    {
        Qwt3D::Atom m = monomer_vector[i];
        monomer_count++;

        if ( protected_type == m.monomer_type  || quencher_type == m.monomer_type )
        {
            protected_and_quencher_indices.append( i );
        }

        if ( pag_type == m.monomer_type )
        {
            pag_indices.append( i );
        }
    }

    exposure_type = ( enum EXPOSURE_TYPE ) ui->exposureTypeComboBox->currentIndex();
    acids_per_photon = ui->acidsPerPhotonSpinBox->value();
    acid_cluster_radius = ui->electronInteractionDistanceSpinBox->value();

    preparePhotolyzedPAGList();

    ui->textEdit->append( QString( "Model File name : %1" ).arg( ( main_window->fileName() ) ) );
    ui->textEdit->append( QString( "Total of %1 monomers found" ).arg( monomer_count ) );
    ui->textEdit->append( QString( "%1 radiation-sensitive groups('%2') found" ).arg( pag_indices.count() ).arg( ui->pagComboBox->currentText() ) );
    ui->textEdit->append( QString( " Random number seed = %1" ).arg( ui->randomNumberSeedEdit->text() ) );
    ui->textEdit->append( QString( " Reaction volume radius = %1" ).arg( deprotection_radius ) );
    ui->textEdit->append( QString( " Fraction %1 exposed = %2" ).arg( ui->pagComboBox->currentText() ).arg( exposed_fraction ) );

    //    qDebug() << "avg num neighbors: r=2.1 " <<  numberOfNeighbors( 2.1 );
    //    qDebug() << "avg num neighbors: r=1.9 " <<  numberOfNeighbors( 1.9 );
    //    qDebug() << "avg num neighbors: r=1.7 " <<  numberOfNeighbors( 1.7 );
    //    qDebug() << "avg num neighbors: r=1.5 " <<  numberOfNeighbors( 1.5 );
    //    qDebug() << "avg num neighbors: r=1.3 " <<  numberOfNeighbors( 1.3 );
    //    qDebug() << "avg num neighbors: r=1.1 " <<  numberOfNeighbors( 1.1 );
    //    qDebug() << "avg num neighbors: r=0.9 " <<  numberOfNeighbors( 0.9 );

    ui->textEdit->append( QString( " Exposure type = %1, avg acids/photon = %2, interaction radius = %3" ).arg( ui->exposureTypeComboBox->currentText() )
                          .arg( Clustered == exposure_type ? QString::number( ui->acidsPerPhotonSpinBox->value() ) : "n/a" )
                          .arg( Clustered == exposure_type ? QString::number( ui->electronInteractionDistanceSpinBox->value() ) :  "n/a" ) );
}



void ExposureDialog::runCalculation()
{
    int quencher_type = ui->quencherComboBox->currentIndex();
    int nonreactive_ionizable_type = ui->ionizableComboBox->currentIndex();

    const int IS_NEUTRALIZED = -1;

    ui->textEdit->append( QString( "starting conversions..." ) );

    deprotected_indices.clear();

    ui->progressBar->setMaximum( pag_indices.count() );

    int num_exposed_pags = 0;
    int num_neutralizations = 0;

    // dumpAcidAndQuencherLocations();

    // fro each pag in the film
    for ( int i = 0; i < pag_indices.count(); i++ )
    {
        // decide by probability if it is an exposed one
        if ( isExposed( pag_indices[i] ) )
        {
            photolyzed_pag_indices.append( pag_indices[i] );
            num_exposed_pags++;

            bool neutralized = false;

            int num_deprotected_by_this_pag  = 0;

            // do through the list of sites containing a protecting group or quencher
            for ( int j = 0; ( false == neutralized ) && ( j < protected_and_quencher_indices.count() ); j++ )
            {
                int current_monomer_index = protected_and_quencher_indices[j];

                // if this monomer is a quencher that has been neutralized then do nothing
                if ( IS_NEUTRALIZED != current_monomer_index )
                {
                    // otherwise see if it is in the reaction volume
                    if ( isInVolume( pag_indices[i], current_monomer_index, deprotection_radius ) )
                    {
                        // if so and if it is a quencher then set the flag that ends the catalytic chain in this particular voluem
                        if ( quencher_type == monomer_vector[current_monomer_index].monomer_type )
                        {
                            neutralized = true;
                            num_neutralizations++;
                            // and set the monomer as neutralized so it does not appear again
                            neutralized_quencher_indices.append( protected_and_quencher_indices[j] );
                            protected_and_quencher_indices[j] = IS_NEUTRALIZED;
                            //   ui->textEdit->append( QString( "PAG %2 neutralized after %1 deprotections" ).arg( num_deprotected_by_this_pag ).arg( i ) );

                        }
                        else
                        {
                            // if it is not a quencher then it is a protecting group - add to the list of deprotected indeces
                            deprotected_indices.append( protected_and_quencher_indices[j] );
                            num_deprotected_by_this_pag++;
                        }
                    }
                }
            }

            //    ui->textEdit->append( QString( "Total %1 deprotections by PAG %2" ).arg( num_deprotected_by_this_pag ).arg( i ) );
        }

        ui->progressBar->setValue( i );
    }

    ui->progressBar->setValue( pag_indices.count() );

    ui->textEdit->append( QString( "number of PAGs exposed = %1" ).arg( num_exposed_pags ) );
    ui->textEdit->append( QString( "fraction of PAGs exposed = %1" ).arg( ( ( double )num_exposed_pags ) / ( ( double )pag_indices.count() ) ) );
    ui->textEdit->append( QString( "%1 deprotection operations applied" ).arg( deprotected_indices.count() ) );
    ui->textEdit->append( QString( "number of neutralizations = %1" ).arg( num_neutralizations ) );

    // go through the monomer vector and tag every monomer that has been deprotected
    int deprotected_type = ui->deprotectedComboBox->currentIndex();
    Qwt3D::RGBA deprotected_color = monomer_list->monomer( deprotected_type )->color();

    for ( int k = 0; k < deprotected_indices.count(); k++ )
    {
        monomer_vector[deprotected_indices[k]].monomer_type = deprotected_type;
        monomer_vector[deprotected_indices[k]].col = deprotected_color;
    }

    // do the same with the neutralized quenchers
    int neutralized_type = ui->neutralizedQuencherComboBox->currentIndex();
    Qwt3D::RGBA neutralized_color = monomer_list->monomer( neutralized_type )->color();

    for ( int n = 0; n < neutralized_quencher_indices.count(); n++ )
    {
        monomer_vector[neutralized_quencher_indices[n]].monomer_type = neutralized_type;
        monomer_vector[neutralized_quencher_indices[n]].col = neutralized_color;
    }

    // and with the exposed quenchers
    int exposed_type = ui->exposedPAGComboBox->currentIndex();
    Qwt3D::RGBA exposed_color = monomer_list->monomer( exposed_type )->color();

    for ( int x = 0; x < photolyzed_pag_indices.count(); x++ )
    {
        monomer_vector[photolyzed_pag_indices[x]].monomer_type = exposed_type;
        monomer_vector[photolyzed_pag_indices[x]].col = exposed_color;
    }

    // now scan through the mononomer_vector to do some statistics
    int num_monomers = monomer_vector.size();
    int final_count_deprotected = 0;
    int final_count_protected = 0;
    int final_count_neutralized = 0;
    int final_count_quencher = 0;
    int final_count_exposed = 0;
    int final_count_pag = 0;
    int final_count_nonreactive_ionizable = 0;

    int num_chains = monomer_vector[num_monomers - 1].chain_index;
    QList<int> chain_length( num_chains + 1, 0 );
    QList<int> protected_count( num_chains + 1, 0 );
    QList<int> deprotected_count( num_chains + 1, 0 );
    QList<int> neutralized_count( num_chains + 1, 0 );
    QList<int> quencher_count( num_chains + 1, 0 );
    QList<int> exposed_count( num_chains + 1, 0 );
    QList<int> pag_count( num_chains + 1, 0 );
    QList<int> nonreactive_ionizable_count( num_chains + 1, 0 );
    QList<int> is_soluble( num_chains + 1, 0 );

    int protected_type = ui->protectedComboBox->currentIndex();
    int pag_type = ui->pagComboBox->currentIndex();

    // always skip the first two members, they are used in qt opengl3d to set the volume extents on screen

    for ( int i = 2; i < num_monomers; i++ )
    {
        Qwt3D::Atom m = monomer_vector[i];

        chain_length[m.chain_index]++;

        if ( protected_type == m.monomer_type )
        {
            protected_count[m.chain_index]++;
            final_count_protected++;
        }

        if ( deprotected_type == m.monomer_type )
        {
            final_count_deprotected++;
            deprotected_count[m.chain_index]++;
        }

        if ( neutralized_type == m.monomer_type )
        {
            final_count_neutralized++;
            neutralized_count[m.chain_index]++;
        }

        if ( quencher_type == m.monomer_type )
        {
            final_count_quencher++;
            quencher_count[m.chain_index]++;
        }

        if ( exposed_type == m.monomer_type )
        {
            final_count_exposed++;
            exposed_count[m.chain_index]++;
        }

        if ( pag_type == m.monomer_type )
        {
            final_count_pag++;
            pag_count[m.chain_index]++;
        }

        if ( nonreactive_ionizable_type == m.monomer_type )
        {
            final_count_nonreactive_ionizable++;
            nonreactive_ionizable_count[m.chain_index]++;
        }
    }

    ui->textEdit->append( QString( "final count of deprotected groups ('%3') = %1 out of %2" ).arg( final_count_deprotected ).arg( final_count_deprotected + final_count_protected ).arg( ui->deprotectedComboBox->currentText() ) );
    ui->textEdit->append( QString( "Extent of deprotection = %1" ).arg( ( ( double )final_count_deprotected ) / ( ( double )( final_count_deprotected + final_count_protected ) ) ) );
    ui->textEdit->append( QString( "Total fraction ionizable = %1" ).arg( ( ( double )final_count_deprotected + ( double ) final_count_nonreactive_ionizable ) / ( ( double )( num_monomers ) ) ) );

    double sol_threshold = ui->solubilityThresholdSpinBox->value();

    int final_count_soluble = 0;
    int num_polymer_chains = 0;

    for ( int c = 1; c < chain_length.count(); c++ )
    {
        if ( 0 == deprotected_count[c] && 0 == nonreactive_ionizable_count[c] && 0 == protected_count[c] )
        {
            // not a polymer - do nothing
        }
        else
        {
            num_polymer_chains++;

            if ( ( ( double )( deprotected_count[c] + nonreactive_ionizable_count[c] ) / ( ( double )chain_length[c] ) ) > sol_threshold )
            {
                is_soluble[c] = 1;
                final_count_soluble++;
            }
        }
    }

    ui->textEdit->append( QString( "Number of polymer chains = %1 " ).arg( num_polymer_chains ) );
    ui->textEdit->append( QString( "Number of soluble chains = %1 (%2%)" ).arg( final_count_soluble ).arg( ( 100.0 * final_count_soluble  / ( ( double )( num_polymer_chains ) ) ) ) );

    if ( ui->saveGroupBox->isChecked() )
    {
        ui->textEdit->append( "" );
        ui->textEdit->append( QString( "ChainIndex,ChainLength,NumProtected,NumDeprotected,NumRemainingQuencher,NumNeutralizedQuencher,NumRemainingPAG,NumExposedPAG,NumNonReactiveIonizable,IsSoluble" ) );

        for ( int c = 1; c < chain_length.count(); c++ )
        {
            ui->textEdit->append( QString( "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10" ).arg( c ).arg( chain_length[c] ).arg( protected_count[c] ).arg( deprotected_count[c] ).arg( quencher_count[c] ).
                                  arg( neutralized_count[c] ).arg( pag_count[c] ).arg( exposed_count[c] ).
                                  arg( nonreactive_ionizable_count[c] ).arg( is_soluble[c] ) );
        }
    }
}




bool ExposureDialog::isInVolume( int centerIndex, int queriedIndex, double radius )
{
    Qwt3D::Triple p1 = monomer_vector[centerIndex].pos;
    Qwt3D::Triple p2 = monomer_vector[queriedIndex].pos;

    double distance = sqrt( pow( p1.x - p2.x, 2.0 ) + pow( p1.y - p2.y, 2.0 ) + pow( p1.z - p2.z, 2.0 ) );
    return ( distance <= radius );
}



bool ExposureDialog::isExposed( int pagIndex )
{
    if ( Random == exposure_type )
    {
        return ( rng.generateDouble() < exposed_fraction );
    }
    else
    {
        return clustered_photolyzed_pag_indices.contains( pagIndex );
    }
}



void ExposureDialog::preparePhotolyzedPAGList()
{
    clustered_photolyzed_pag_indices.clear();

    if ( Random == exposure_type )
    {
        return;
    }

    std::default_random_engine generator;
    QRandomGenerator rng( seed );

    generator.seed( seed );

    std::poisson_distribution<int> distribution( acids_per_photon );

    int target_num_acids = qRound( exposed_fraction * pag_indices.count() );

    while ( clustered_photolyzed_pag_indices.count() < target_num_acids )
    {
        int current_cluster_size =  0;

        while ( 0 == current_cluster_size )
        {
            current_cluster_size = distribution( generator );
        }

        // randomly select a pag from the entire population of pags
        int random_pag_selection = rng.bounded( pag_indices.count() - 1 );

        int current_pag = pag_indices[random_pag_selection];

        // add it to the exposed list if it is not already there
        if ( false == clustered_photolyzed_pag_indices.contains( current_pag ) )
        {
            clustered_photolyzed_pag_indices.append( current_pag );
            current_cluster_size--;

            // now find all the nearby pags within acid_cluster_radius
            QList<int>  nearby_pag_indices;

            for ( int  i = 0; i < pag_indices.count(); i++ )
            {
                int queried_pag = pag_indices[i];
                if ( ( current_pag != queried_pag ) &&
                     isInVolume( current_pag, queried_pag, acid_cluster_radius ) &&
                     false == nearby_pag_indices.contains( queried_pag ) )
                {
                    nearby_pag_indices.append( queried_pag );
                }
            }

            // now randomly select nearby pags and add to the exposed list until the cluster size is reached
            // stop if we exhaust the nearby pags before we get to the specifed cluster size,
            // stop if we get to the specified cluster size
            // and stop if we have reached the target number of acids
            for ( int k = 0; ( k < nearby_pag_indices.count() ) &&
                  ( 0 < current_cluster_size ) &&
                  ( clustered_photolyzed_pag_indices.count() < target_num_acids ); k++ )
            {
                int nearby_pag = nearby_pag_indices.at( 1 == nearby_pag_indices.count() ? 0 : rng.bounded( nearby_pag_indices.count() - 1 ) );

                if ( false == clustered_photolyzed_pag_indices.contains( nearby_pag ) )
                {
                    clustered_photolyzed_pag_indices.append( nearby_pag );
                    current_cluster_size--;
                }
            }
        }
    }

    qDebug() << "acid count:" << clustered_photolyzed_pag_indices.count() << ( double )( 100 * clustered_photolyzed_pag_indices.count() / ( double ) pag_indices.count() ) << "%";
}



double ExposureDialog::numberOfNeighbors( double radius )
{
    int num_monomers = monomer_vector.size();

    // always skip the first two members, they are used in qt opengl3d to set the volume extents on screen

    qDebug() << QString( "monomer,neighbors" );

    int sum = 0;
    for ( int i = 2; i < num_monomers; i++ )
    {
        int num_neighbors = 0;
        for ( int j = 2; j < num_monomers; j++ )
        {
            if ( j != i )
            {
                if ( isInVolume( i, j, radius ) )
                {
                    num_neighbors++;
                }
            }
        }

        sum += num_neighbors;
        //      qDebug() << i << "," << num_neighbors;
    }

    return ( double )sum / double( num_monomers );
}



void ExposureDialog::dumpAcidAndQuencherLocations()
{
    const QString EXTENSION( ".csv" );

    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save Acid/Quencher Lcoations File" ), main_window->outputFolder(), tr( "CSV files (*.csv)" ) );

    if ( false == fileName.isEmpty() )
    {
        if ( false == fileName.endsWith( EXTENSION, Qt::CaseInsensitive ) )
        {
            fileName += EXTENSION;
        }

        QFile outfile( fileName );

        if ( outfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            QTextStream str( &outfile );

            str << "x,y,z,monomer_name,monomer_id,chain_index\n";

            for ( int i = 0; i < clustered_photolyzed_pag_indices.count(); i++ )
            {
                int acid_index = clustered_photolyzed_pag_indices.at( i );
                Qwt3D::Atom m = monomer_vector[acid_index];
                str << QString( "%1,%2,%3,H+,%4,%5\n" ).arg( m.pos.x ).arg( m.pos.y ).arg( m.pos.z ).arg( m.monomer_type ).arg( m.chain_index );
            }

            int quencher_type = ui->quencherComboBox->currentIndex();

            for ( int j = 0; j < protected_and_quencher_indices.count(); j++ )
            {
                int species_index = protected_and_quencher_indices.at( j );
                Qwt3D::Atom m = monomer_vector[species_index];

                if ( quencher_type == m.monomer_type )
                {
                    str << QString( "%1,%2,%3,NR3,%4,%5\n" ).arg( m.pos.x ).arg( m.pos.y ).arg( m.pos.z ).arg( m.monomer_type ).arg( m.chain_index );
                }
            }
            outfile.close();
        }
    }
}



void ExposureDialog::apply()
{
    apply_button->setEnabled( false );
    readInputs();
    runCalculation();
    if ( ui->saveGroupBox->isChecked() && !ui->fileNameEdit->text().isEmpty() )
    {
        QFile f( ui->fileNameEdit->text() );
        f.open( QIODeviceBase::WriteOnly | QIODeviceBase::Text );

        QTextStream stream( &f );

        stream << ui->textEdit->toPlainText();

        f.close();
    }

    QSettings settings;

    settings.beginGroup( EXPOSURE_DIALOG_GROUP );

    settings.setValue( DEPROTECTED_NAME, ui->deprotectedComboBox->currentText() );
    settings.setValue( PROTECTED_NAME, ui->protectedComboBox->currentText() );
    settings.setValue( PAG_NAME, ui->pagComboBox->currentText() );
    settings.setValue( EXPOSED_PAG_NAME, ui->exposedPAGComboBox->currentText() );
    settings.setValue( QUENCHER_NAME, ui->quencherComboBox->currentText() );
    settings.setValue( NEUTRALIZED_QUENCHER_NAME, ui->neutralizedQuencherComboBox->currentText() );
    settings.setValue( NONREACTIVE_IONIZABLE_NAME, ui->ionizableComboBox->currentText() );
    settings.setValue( FRACTION_EXPOSED_NAME, ui->fractionExposedSpinBox->value() );
    settings.setValue( VOLUME_RADIUS_NAME, ui->reactionRadiusSpinBox->value() );
    settings.setValue( SEED_NAME, ui->randomNumberSeedEdit->text() );

    settings.setValue( ACIDS_PER_PHOTON_NAME, ui->acidsPerPhotonSpinBox->value() );
    settings.setValue( INTERACTION_RADIUS_NAME, ui->electronInteractionDistanceSpinBox->value() );
    settings.setValue( SOLBILITY_THRESHOLD_NAME, ui->solubilityThresholdSpinBox->value() );

    settings.setValue( EXPOSURE_TYPE_NAME, ui->exposureTypeComboBox->currentIndex() );

    settings.endGroup();

    main_window->refreshChains();
}



void ExposureDialog::selectFileButtonClicked()
{
    const QString EXTENSION( ".csv" );

    QString fileName = QFileDialog::getSaveFileName( this, tr( "Save File" ), main_window->outputFolder(), tr( "CSV files (*.csv)" ) );

    if ( false == fileName.isEmpty() )
    {
        if ( false == fileName.endsWith( EXTENSION, Qt::CaseInsensitive ) )
        {
            fileName += EXTENSION;
        }

        ui->fileNameEdit->setText( fileName );
        main_window->setOutputFolder( QFileInfo( fileName ).absolutePath() );
    }
}



void ExposureDialog::reset()
{
    main_window->reload();
    ui->textEdit->clear();
    apply_button->setEnabled( true );
}

void ExposureDialog::exposureTypeChanged( int newType )
{
    ui->clusterRadiusLabel->setEnabled( 0 != newType );
    ui->avgAcidsLabel->setEnabled( 0 != newType );
    ui->electronInteractionDistanceSpinBox->setEnabled( 0 != newType );
    ui->acidsPerPhotonSpinBox->setEnabled( 0 != newType );
}
