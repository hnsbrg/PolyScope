#ifndef MONOMER_H
#define MONOMER_H

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

#include "qwt3d_types.h"
#include <QList>

class Monomer
{
public:
    Monomer();
    Monomer( int id, const QString& n, Qwt3D::RGBA c, double r );
    Monomer( const Monomer& m );

    int typeID() const { return  type_id; }
    Qwt3D::RGBA color() const {return _color;}
    double radius() const {return _radius; }
    const QString& name() const { return _name; }
    double proportion() const {return _proportion;}

    void setTypeID( int id )  { type_id = id; }
    void setColor( Qwt3D::RGBA c )  { _color = c;}
    void setRadius( double r )  { _radius = r; }
    void setName( const QString& name )  {  _name = name; }
    void setProportion( double val )  { _proportion = val;}

    //    QString toJsonString() const;

protected:
    int type_id;
    QString _name;
    Qwt3D::RGBA _color;
    double _radius;
    double _proportion;
};



QTextStream& operator << ( QTextStream& out_stream, const Monomer& m );
QTextStream& operator >> ( QTextStream& in_stream, Monomer& m );

#endif // MONOMER_H
