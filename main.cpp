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

#include "mainwindow.h"
#include <QApplication>

MainWindow* mainwin = 0;

MainWindow* appWindow() { return mainwin; }

int main( int argc, char* argv[] )
{
    QApplication a( argc, argv );

    QCoreApplication::setOrganizationName( "Columbia Hill Technical Consulting" );
    QCoreApplication::setOrganizationDomain( "hinsberg.net" );
    QCoreApplication::setApplicationName( "PolyScope" );

    MainWindow w;
    mainwin = &w;

    if ( false == w.initialize( argc, argv ) )
    {
        printf( "\n" );
        printf( "usage: %s [options] \n", argv[0] );
        printf( "  [-m count] total number of monomers\n" );
        printf( "  [-c length] nominal chain length\n" );
        printf( "  [-d density] in particles per cube unit\n" );
        printf( "  [-r atom_radius]\n" );
        printf( "  [-l bond_len]\n" );
        printf( "  [-a bond_angle]\n" );
        printf( "  [-k kappa] for bond angle distribution\n" );
        printf( "  [-t trials]\n" );
        printf( "  [-f] with film interfaces\n" );
        printf( "  [-o max_overlap]\n" );
        printf( "  [-n nr_angles]\n" );
        printf( "  [-s search_depth]\n" );
        printf( "  [-b density] brush, density in chains per square unit\n" );
        printf( "  [-z exponent] z-Axis alignment, (p(u) = uz^exponent)\n" );
        printf( "  [-p nr_packings]\n" );
        printf( "  [-x] with X interface\n" );
        printf( "  chain_len = 0 means chain_len distribution\n" );
        printf( "\n" );
        exit( 1 );
    }
    else
    {
        w.show();
        return a.exec();
    }
}
