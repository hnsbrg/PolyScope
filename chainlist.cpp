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

#include "chainlist.h"
#include "random.h"
#include <QDebug>
#include <math.h>
#include <random>


int compare( const void* a, const void* b )
{
    return ( *( const int* )b - * ( const int* )a );
}


ChainList::ChainList() :
    num_monomers( 0 ),
    degree_of_polymerization( 0 ),
    dispersity( 0 ),
    chain_length_array( 0 ),
    seed( 0 ),
    chain_vec(),
    additives_prepended( false )
{
}



ChainList::~ChainList()
{
    if ( 0 != chain_length_array )
    {
        delete[] chain_length_array;
    }

    chain_vec.clear();
}



void ChainList::configure( int numMonomers, int degPolym, double polydispersity, int rnSeed )
{
    num_monomers = numMonomers;
    degree_of_polymerization = degPolym;
    dispersity = polydispersity;
    seed = rnSeed;

    additive_chain_count.clear();

    initialize();
}



int ChainList::chainLength( int chainNum )
{
    if ( chainNum < chain_vec.count() )
    {
        return chain_vec.at( chainNum );
    }
    else
    {
        return 0;
    }
}



void ChainList::appendAdditive( int numMolecules, int meanClusterSize, bool usePoisson )
{
    if ( true == additivesPrepended() )
    {
        prependAdditive( numMolecules, meanClusterSize, usePoisson );
        return;
    }

    std::default_random_engine generator;
    generator.seed( seed );

    std::poisson_distribution<int> distribution( meanClusterSize );

    int molecules_left = numMolecules;

    int molecule_count = 0;
    int num_chains_for_additive = 0;

    while ( 0 < molecules_left )
    {
        int current_cluster_size =  0;

        while ( 0 == current_cluster_size )
        {
            current_cluster_size = ( true == usePoisson ) ? distribution( generator ) : meanClusterSize;
        }

        qDebug() << "current_cluster_size:" << current_cluster_size;

        if ( current_cluster_size <= molecules_left )
        {
            molecule_count += current_cluster_size;
            chain_vec.append( current_cluster_size );
            num_chains_for_additive++;
            molecules_left -= current_cluster_size;
        }
        else
        {
            molecule_count += molecules_left;
            chain_vec.append( molecules_left );
            num_chains_for_additive++;
            molecules_left = 0;
        }
    }

    additive_chain_count.append( num_chains_for_additive );
    qDebug() << "molecule count in additive append:" << molecule_count;
    qDebug() << "chain_vec size:" << chain_vec.count();
}


void ChainList::prependAdditive( int numMolecules, int meanClusterSize, bool usePoisson )
{
    std::default_random_engine generator;
    generator.seed( seed );

    std::poisson_distribution<int> distribution( meanClusterSize );

    int molecules_left = numMolecules;

    int molecule_count = 0;
    int num_chains_for_additive = 0;

    QVector<int>  temp_chain_vec;

    while ( 0 < molecules_left )
    {
        int current_cluster_size =  0;

        while ( 0 == current_cluster_size )
        {
            current_cluster_size = ( true == usePoisson ) ? distribution( generator ) : meanClusterSize;
        }

        qDebug() << "current_cluster_size:" << current_cluster_size;

        if ( current_cluster_size <= molecules_left )
        {
            molecule_count += current_cluster_size;
            temp_chain_vec.append( current_cluster_size );
            num_chains_for_additive++;
            molecules_left -= current_cluster_size;
        }
        else
        {
            molecule_count += molecules_left;
            temp_chain_vec.append( molecules_left );
            num_chains_for_additive++;
            molecules_left = 0;
        }
    }

    temp_chain_vec.append( chain_vec );
    chain_vec = temp_chain_vec;

    additive_chain_count.append( num_chains_for_additive );
    qDebug() << "molecule count in additive prepend:" << molecule_count;
    qDebug() << "chain_vec size:" << chain_vec.count();
}



int ChainList::totalAdditiveChainCount() const
{
    int sum = 0;

    for ( int i = 0 ; i < additive_chain_count.count(); i++ )
    {
        sum += additive_chain_count[i];
    }

    return sum;
}



void ChainList::initialize()
{
    const double PI = 3.14159265;

    // from Rane and Choi, Chem Matls 17, 926 (2005)

    double std_dev =  sqrt( dispersity - 1.0 ) * degree_of_polymerization;

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


