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


#include "chaingraph.h"
#include "colorring.h"
#include "mainwindow.h"
#include "random.h"
#include <QFileDialog>

const char* FONT_STYLE = "Helvetica";
const int TITLE_POINT_SIZE = 10;
const int AXIS_TITLE_POINT_SIZE = 10;
const int AXIS_NUMBER_POINT_SIZE = 10;
const int LEGEND_POINT_SIZE = 10;
const Qwt3D::SHADINGSTYLE SHADING = Qwt3D::GOURAUD;   // GOURAUD or FLAT
const int FIRST_MONOMER_NODE = 2;


ChainGraph::ChainGraph( QWidget* w ):
    Qwt3D::GraphPlot( w ),
    main_win( ( MainWindow* ) w ),
    chain_count( 0 ),
    show_monomers( false ),
    space_filling( true ),
    coloration( MONOCHROME ),
    popup_menu( new QMenu() ),
    monomer_list( main_win->monomerList() ),
    monomer_count(),
    avg_radius_of_gyration( 0.0 ),
    current_scanned_chain( 0 )
{
    showDefaultView();
    setScale( 1, 1, 1 );
    setShift( 0.15, 0, 0 );
    setZoom( 0.9 );

    updateData();
    updateGL();

    QAction* action  = new QAction( QString( "Default View" ), this );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( showDefaultView() ) );
    popup_menu->addAction( action );

    action  = new QAction( QString( "Top View" ), this );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( showTopView() ) );
    popup_menu->addAction( action );

    action  = new QAction( QString( "Front View" ), this );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( showFrontView() ) );
    popup_menu->addAction( action );

    action  = new QAction( QString( "Side View" ), this );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( showSideView() ) );
    popup_menu->addAction( action );

    action  = new QAction( QString( "Save Snapshot" ), this );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( saveSnapshot() ) );
    popup_menu->addAction( action );

}


Vector ChainGraph::NormalizePoint( Grid* grid, Vector v, float gScale )
{
    v.x = ( v.x - 0.5 * grid->params.box_size.x ) * gScale;
    v.y = ( v.y - 0.5 * grid->params.box_size.y ) * gScale;
    v.z = ( v.z - 0.5 * grid->params.box_size.z ) * gScale;

    return v;
}

const MonomerCount& ChainGraph::updateMonomerCount( int numMonomers )
{
    monomer_count.resize( numMonomers );

    if ( 0 < numMonomers )
    {
        for ( int i = 0; i < monomer_count.count(); i++ )
        {
            monomer_count[i] = 0;
        }

        for ( int j = FIRST_MONOMER_NODE; j < nodes.size(); j++ )
        {
            int mtype = nodes[j].monomer_type;
            monomer_count[mtype] = monomer_count[mtype] + 1;
        }
    }

    return monomer_count;
}



void ChainGraph::scanChains( int chainIndex )
{
    int length = 0;

    for ( int j = FIRST_MONOMER_NODE; j < nodes.size(); j++ )
    {
        if ( chainIndex == nodes[j].index )
        {
            nodes[j].col.a = 1.0;
            length++;
        }
        else
        {
            nodes[j].col.a = 0.0;
        }
    }

    for ( int k = 0; k < bonds.size(); k++ )
    {
        if ( chainIndex == bonds[k].index )
        {
            bonds[k].col.a = 1.0;
        }
        else
        {
            bonds[k].col.a = 0.0;
        }
    }

    setTitle( QString( "Showing chain %1, length %2" ).arg( chainIndex + 1 ).arg( length ) );
    createDataset( nodes, bonds );

    updateGL();
    current_scanned_chain = chainIndex + 1;
}

void ChainGraph::refreshChains()
{
    createDataset( nodes, bonds );
    configure();
    updateGL();
}



void ChainGraph::clear()
{
    nodes.clear();
    bonds.clear();

    createDataset( nodes, bonds );

    chain_count = 0;

    //  updateTitle();

    updateGL();
}



void ChainGraph::contextMenuEvent( QContextMenuEvent* event )
{
    popup_menu->exec( event->globalPos() );
}


void ChainGraph::recolorPointCloud( MonomerSequence* sequence, AdditiveList* additiveList )
{
    RGBA current_col = Monomer().color();
    int type_id = 0;

    if ( MONOMER == coloration )
    {
        const Monomer* m = sequence->firstMonomer();
        current_col = m->color();
        type_id = m->typeID();
    }

    additiveList->apportionParticles( nodes.size() - FIRST_MONOMER_NODE );
    int num_additive_particles = additiveList->totalParticles();

    int i = FIRST_MONOMER_NODE;
    for ( ; i < nodes.size() - num_additive_particles; i++ )
    {
        current_col.a = 0.4;

        nodes.at( i ).col = current_col;
        nodes.at( i ).monomer_type = type_id;

        if ( MONOMER == coloration )
        {
            const Monomer* m = sequence->nextMonomer();
            current_col = m->color();
            type_id = m->typeID();
        }
    }

    for ( int j = 0; j < additiveList->count(); j++ )
    {
        const Additive* a = additiveList->additive( j );
        num_additive_particles -= a->numParticles();

        current_col = a->color();
        type_id = a->typeID();

        for ( ; i < nodes.size() - num_additive_particles; i++ )
        {
            current_col.a = 0.4;

            nodes.at( i ).col = current_col;
            nodes.at( i ).monomer_type = type_id;
        }
    }

    updateGL();
}


void ChainGraph::drawPointCloud( Parameters& gParams, int num_particles,  MonomerSequence* sequence, AdditiveList* additiveList )
{
    nodes.clear();
    bonds.clear();

    // add points at extremes of box to set overall scale
    // the first two nodes are used for this...

    nodes.push_back( Atom( -1,  Triple( 0, 0, 0 ), 0.05, RGBA( 0, 0, 0, 0 ) ) );
    nodes.push_back( Atom( -1, Triple( gParams.box_size.x, gParams.box_size.y, gParams.box_size.z ), 0.05, RGBA( 0, 0, 0, 0 ) ) );

    // a dummy bond - for the point cloud only
    bonds.push_back( Bond( -1, Triple( 0, 0, 0 ), Triple( gParams.box_size.x, gParams.box_size.y, gParams.box_size.z ), RGBA( 0, 0, 0, 0 ) ) );

    double radius =  gParams.atom_radius * 0.2;

    for ( int i = 0; i < num_particles; i++ )
    {
        Vector v;
        v.x = randomDouble() * gParams.box_size.x;
        v.y = randomDouble() * gParams.box_size.y;
        v.z = randomDouble() * gParams.box_size.z;

        nodes.push_back( Atom( i, Triple( v.x, v.y, v.z ), radius ) );
    }

    recolorPointCloud( sequence, additiveList );
    setTitle( QString( "Random Point Cloud: %1 points" ).arg( nodes.size() - 2 ) );

    createDataset( nodes, bonds );
    configure();
    updateGL();
}



void ChainGraph::drawChains( Grid* grid, MonomerList* monomer_list )
{
    nodes.clear();
    bonds.clear();
    ColorRing col_ring;
    col_ring.setRandomColors( true );

    double radius = space_filling ? grid->params.atom_radius : grid->params.atom_radius * 0.2;

    float gScale = grid->params.box_size.x;
    if ( grid->params.box_size.y > gScale ) gScale = grid->params.box_size.y;
    if ( grid->params.box_size.z > gScale ) gScale = grid->params.box_size.z;
    if ( gScale <= 0.0 ) gScale = 1.0;
    gScale = 2.0 / gScale;
    int count = 0;

    // add points at extremes of box to set overall scale
    // this uses the first two nodes for this specfici purspose - they are not monomers

    nodes.push_back( Atom( -1,  Triple( 0, 0, 0 ), 0.05, RGBA( 0, 0, 0, 0 ) ) );
    nodes.push_back( Atom( -1, Triple( grid->params.box_size.x, grid->params.box_size.y, grid->params.box_size.z ), 0.05, RGBA( 0, 0, 0, 0 ) ) );

    RGBA current_col;

    for ( int i = 0; i < grid->max_chains; i++ )
    {
        switch ( coloration )
        {
            case MONOCHROME:
            case MONOMER:
                current_col = Monomer().color();
                break;

            case CHAIN:
                current_col = col_ring.nextColor();
                break;
        }

        Chain* chain = &grid->chains[i];

        // add monomers
        if ( show_monomers )
        {
            current_col.a = 0.4;
            if ( chain->last - chain->first <= 0 ) continue;
            count++;
            for ( int j = chain->first; j < chain->last; j++ )
            {
                int k = j - chain->offset;
                Vector v = Vector_periodic_box( chain->atoms[k], grid->params.box_size );

                // v = NormalizePoint( grid, v, gScale );
                if ( MONOMER == coloration && monomer_list->count() > 0 && -1 != v.monomer_type && v.monomer_type < monomer_list->count() )
                {
                    current_col = monomer_list->monomer( v.monomer_type )->color();
                    current_col.a = 0.4;
                }

                nodes.push_back( Atom( i, Triple( v.x, v.y, v.z ), radius, current_col, v.monomer_type, i + 1 ) );
            }
            current_col.a = 1.0;
        }

        // add bonds

        if ( chain->last - chain->first <= 1 ) continue;

        if ( MONOMER == coloration )
        {
            current_col = RGBA( 0.5, 0.5, 0.5 );
        }

        for ( int j = chain->first + 1; j < chain->last; j++ )
        {
            int k = j - chain->offset;
            Vector v0 = Vector_periodic_box( chain->atoms[k - 1], grid->params.box_size );
            Vector v1 = Vector_periodic_box( chain->atoms[k], grid->params.box_size );

            if ( fabs( Vector_dist( v0, v1 ) - grid->params.bond_len ) < grid->params.bond_len )
            {
                bonds.push_back( Bond( i, Triple( v0.x, v0.y, v0.z ), Triple( v1.x, v1.y, v1.z ), current_col ) );
            }
            else
            {
                Vector v = Vector_diff( chain->atoms[k], chain->atoms[k - 1] );
                Vector v0 = Vector_periodic_box( chain->atoms[k - 1], grid->params.box_size );
                Vector v1 = Vector_sum( v0, v );
                bonds.push_back( Bond( i, Triple( v0.x, v0.y, v0.z ), Triple( v1.x, v1.y, v1.z ), current_col ) );

                v0 = Vector_periodic_box( chain->atoms[k], grid->params.box_size );
                v1 = Vector_diff( v0, v );
                bonds.push_back( Bond( i, Triple( v0.x, v0.y, v0.z ), Triple( v1.x, v1.y, v1.z ), current_col ) );
            }
        }

    }

    chain_count = count;
    avg_radius_of_gyration = avgRadiusOfGyration( grid );
    refreshChains();
}

void ChainGraph::setSpaceFilling( bool b )
{
    space_filling = b;
    //    drawChains( g );
}

void ChainGraph::setShowMonomer( bool b )
{
    show_monomers = b;
}

void ChainGraph::updateTitle()
{
    //   Triple dim = hull().maxVertex - hull().minVertex;
    //  setTitle( QString( "x length: %1, y length: %2, z length: %3" ).arg( dim.x ).arg( dim.y ).arg( dim.z ) );
    setTitle( QString( "Monomers: %1, Bonds: %2, Chains: %3" ).arg( nodes.size() ).arg( bonds.size() ).arg( chain_count ) );
}

void ChainGraph::showDefaultView()
{
    setRotation( 30, 0, 15 );
    setScale( 1, 1, 1 );
    setShift( 0.15, 0, 0 );
    setZoom( 0.9 );
}

void ChainGraph::showTopView()
{
    setRotation( 90, 0, -90 );
}

void ChainGraph::showFrontView()
{
    setRotation( 0, 0, -90 );

}

void ChainGraph::showSideView()
{
    setRotation( 0, 0, 0 );
}


void ChainGraph::saveSnapshot()
{
    QPixmap pm;

    pm.convertFromImage( grabFramebuffer( ) );

    QString base_filename = QString( "chain%1.png" ).arg( current_scanned_chain );

    QString filename = QFileDialog::getSaveFileName( this, tr( "Save Snapshot" ), base_filename );

    pm.save( filename, "png" );
}



void ChainGraph::configure()
{
    enableLighting();
    setCoordinateStyle( BOX );

    illuminate( 0 );

    coordinates()->axes[X1].setLabelString( "X" );
    coordinates()->axes[X2].setLabelString( "X" );
    coordinates()->axes[X3].setLabelString( "X" );
    coordinates()->axes[X4].setLabelString( "X" );

    coordinates()->axes[Y1].setLabelString( "Y" );
    coordinates()->axes[Y2].setLabelString( "Y" );
    coordinates()->axes[Y3].setLabelString( "Y" );
    coordinates()->axes[Y4].setLabelString( "Y" );

    coordinates()->axes[Z1].setLabelString( "Z" );
    coordinates()->axes[Z2].setLabelString( "Z" );
    coordinates()->axes[Z3].setLabelString( "Z" );
    coordinates()->axes[Z4].setLabelString( "Z" );


    for ( unsigned i = 0; i != coordinates()->axes.size(); ++i )
    {
        coordinates()->axes[i].setMajors( 5 );
        coordinates()->axes[i].setMinors( 4 );
        coordinates()->axes[i].setLabelColor( RGBA( 0.0, 0.0, 0.0, 1.0 ) );
    }

    setTitleFont( FONT_STYLE, TITLE_POINT_SIZE, QFont::Bold );
    setTitlePosition( 0.99 );  // near the top center of the plot window
    setShift( 0.15, 0, 0 );

    //  coordinates()->setAutoScale( false );
    coordinates()->setStandardScale();
    coordinates()->setLabelFont( FONT_STYLE, AXIS_TITLE_POINT_SIZE, QFont::Bold );

    //  setScale( 1, 1, 1 );

    setShading( SHADING );
    // disableLighting();
    //  setSmoothMesh( true );  // needed ?


    // required for treament of translucent objects
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

}


Vector ChainGraph::calculateCenterOfMMass( Chain* chain )
{
    int num_points = chain->last - chain->first;

    //    if ( isnan( ( float ) num_points ) )
    //    {
    //        qDebug() << QString( "isnan0" );
    //    }

    Vector sum = Vector_null();

    // assume they all have equal mass = 1;
    for ( int j = chain->first; j < chain->last; j++ )
    {
        int k = j - chain->offset;
        sum =  Vector_sum( sum, chain->atoms[k] );

        //        if ( isnan( sum.x ) || isnan( sum.y ) || isnan( sum.z ) )
        //        {
        //            qDebug() << QString( "isnan1" );
        //        }
    }

    sum.x =  sum.x / num_points;
    sum.y =  sum.y / num_points;
    sum.z =  sum.z / num_points;

    return sum;
}



float ChainGraph::calculateRadiusOfGyration( Chain* chain )
{
    Vector ctr_of_mass = calculateCenterOfMMass( chain );

    float sum_of_squares = 0.0;

    int  num_points = 0;

    for ( int j = chain->first; j < chain->last; j++ )
    {
        int k = j - chain->offset;

        //        qDebug() << QString( "coords = %1,%2,%3, j = %4" ).arg( chain->atoms[k].x ).arg( chain->atoms[k].y ).arg( chain->atoms[k].z ).arg( j );

        float radius = Vector_dist( ctr_of_mass, chain->atoms[k] );
        sum_of_squares  += ( radius * radius );
        num_points++;
    }

    //    if ( isnan( sum_of_squares ) || isnan( ( float ) num_points ) )
    //    {
    //        qDebug() << QString( "isnan2" );
    //    }

    float retval =  sqrt( sum_of_squares / ( float )num_points );

    //   qDebug() << QString( "CtrOfMass = %1,%2,%3, RadOfGyr = %4, len = %5" ).arg( ctr_of_mass.x ).arg( ctr_of_mass.y ).arg( ctr_of_mass.z ).arg( retval ).arg( num_points );

    return retval;
}


float ChainGraph::calculateEndToEndDistance( Chain* chain )
{
    Vector first = chain->atoms[chain->first - chain->offset];
    Vector last = chain->atoms[chain->last - 1 - chain->offset];

    float dist =  abs( Vector_dist( first, last ) );

    return abs( dist );
}


float ChainGraph::avgRadiusOfGyration( Grid* grid )
{
    float sum = 0;
    int num_chains = 0;
    //   qDebug() << QString( "chain,Rg" );

    for ( int i = 0; i < grid->max_chains; i++ )
    {
        Chain* chain = &grid->chains[i];

        if ( 0 != chain && 0 != chain->atoms && chain->first != chain->last )
        {
            double rg = calculateRadiusOfGyration( chain );
            sum += rg;
            qDebug() << QString( "%1,%2" ).arg( i ).arg( rg );
            num_chains++;
        }
    }

    return sum / ( float ) num_chains;
}



float ChainGraph::avgEndToEndDistance( Grid* grid )
{
    float sum = 0;
    int num_chains = 0;

    for ( int i = 0; i < grid->max_chains; i++ )
    {
        Chain* chain = &grid->chains[i];

        if ( 0 != chain && 0 != chain->atoms && chain->first != chain->last )
        {
            double rg = calculateEndToEndDistance( chain );
            sum += rg;
            qDebug() << QString( "%1,%2" ).arg( i ).arg( rg );
            num_chains++;
        }
    }

    return sum / ( float ) num_chains;
}
