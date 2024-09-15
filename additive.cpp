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

#include "additive.h"

Additive::Additive():
    Monomer(),
    avg_cluster_size( 1 ),
    poisson( false ),
    num_particles( 0 )
{
}



Additive::Additive( int id, const QString& n, Qwt3D::RGBA c, double r ):
    Monomer( id, n, c, r ),
    avg_cluster_size( 1 ),
    poisson( false ),
    num_particles( 0 )
{
}



Additive::Additive( const Monomer& m ):
    Monomer( m ),
    avg_cluster_size( 1 ),
    poisson( false ),
    num_particles( 0 )
{
}



Additive::Additive( const Additive& a ):
    Monomer( a )
{
    type_id = a.type_id;
    _name = a._name;
    _color = a._color;
    _radius = a._radius;
    _proportion = a._proportion;
    avg_cluster_size = a.avg_cluster_size;
    poisson = a.poisson;
    num_particles = a.num_particles;
}
