#ifndef ADDITIVE_H
#define ADDITIVE_H

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



#include "monomer.h"

class Additive : public Monomer
{
public:
    Additive();
    Additive( int id, const QString& n, Qwt3D::RGBA c, double r );
    Additive( const Monomer& m );
    Additive( const Additive& a );

    double fraction() const { return proportion();}
    void setFraction( double f ) { setProportion( f );}

    int avgClusterSize() const { return avg_cluster_size;}
    void setAvgClusterSize( int c )  {  avg_cluster_size = c;}

    bool usePoisson() const { return poisson;}
    void setUsePoisson( bool s )  {  poisson = s;}

    int numParticles() const { return num_particles;}
    void setNumParticles( int c )  {  num_particles = c;}

protected:
    int avg_cluster_size;
    bool poisson;
    int num_particles;
};

#endif // ADDITIVE_H
