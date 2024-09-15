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

#include "additiveclusterlist.h"
#include "random.h"
#include <QDebug>
#include <math.h>

AdditiveClusterList::AdditiveClusterList():
    ChainList()
{
}

AdditiveClusterList::~AdditiveClusterList()
{
}

void AdditiveClusterList::initialize()
{
    const double PI = 3.14159265;

    // for the AdditiveList we are storing the user-entered std dev in the varoabel
    // named 'dispersity; - let's get that and make this more readbale

    double std_dev = dispersity;

    int monomers_left = num_monomers;

    chain_vec.clear();

    int monomer_count = 0;

    while ( 0 < monomers_left )
    {
        double length;

        // Box-Muller Transform - generates a standard Normal Distribution centered at zero with std dev = 1
        //  e multiply the B-M value by out std dev and offset set it by our mean (degree_of_polymerization)
        do
        {
            length =  sqrt( -2.0 * log( randomFloat() ) ) * cos( 2 * PI * randomFloat() ) * std_dev + degree_of_polymerization;
        }
        while ( length < 1.5 );

        int current_chain_length = qRound( length );

        if ( current_chain_length <= monomers_left )
        {
            // don't want to be left with a single monomer so add to last chain if there is only one left
            // so we do not allow a chain length of 1

            if ( 1 == ( monomers_left - current_chain_length ) )
            {
                current_chain_length++;
            }

            monomer_count += current_chain_length;
            chain_vec.append( current_chain_length );
            monomers_left -= current_chain_length;
        }
        else
        {
            monomer_count += monomers_left;
            chain_vec.append( monomers_left );
            monomers_left = 0;
        }
    }

    qsort( chain_vec.data(), chain_vec.count(), sizeof( int ), compare );

    qDebug() << "monomer count in chain list construction:" << monomer_count;
}
