#ifndef CHAINLIST_H
#define CHAINLIST_H

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

#include <QVector>

int compare( const void* a, const void* b );

class ChainList
{
public:
    ChainList();
    virtual ~ChainList();

    void configure( int numMonomers, int degPolym, double polydispersity, int rnSeed );
    int chainLength( int chainNum );
    int chainCount() const { return chain_vec.count();}
    void appendAdditive( int numMolecules, int clusterSize, bool usePoisson );
    int totalAdditiveChainCount() const;
    int polymerChainCount() const { return chainCount() - totalAdditiveChainCount(); }
    int additiveChainCount( int index ) const { return additive_chain_count.at( index );}
    int numAdditives() const { return additive_chain_count.count();}
    void prependAdditives( bool b ) { additives_prepended = b;}
    bool additivesPrepended() const { return additives_prepended; }

protected:
    int num_monomers;
    int degree_of_polymerization;
    double dispersity;
    int* chain_length_array;
    int seed;
    QVector<int>  chain_vec;
    QVector<int>  additive_chain_count;
    bool additives_prepended;

    void prependAdditive( int numMolecules, int clusterSize, bool usePoisson );
    virtual void initialize();
};

#endif // CHAINLIST_H
