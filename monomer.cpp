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

Monomer::Monomer():
    type_id( -1 ),
    _name(),
    _color( 0.5, 0.5, 0.5 ),
    _radius( 0.5 ),
    _proportion( 1.0 )
{
}

Monomer::Monomer( int id, const QString& n, Qwt3D::RGBA c, double r ):
    type_id( id ),
    _name( n ),
    _color( c ),
    _radius( r ),
    _proportion( 1.0 )
{
}

Monomer::Monomer( const Monomer& m )
{
    type_id = m.type_id;
    _name = m._name;
    _color = m._color;
    _radius =  m._radius;
    _proportion = m._proportion;
}

//QString Monomer::toJsonString() const
//{
//    QJsonDocument doc;

//    QJsonObject obj;

//    obj.insert("type_id",type_id);
//    obj.insert("_name",_name);

//    QJsonObject rgbobj;

//    rgbobj.insert("r", _color.r);
//    rgbobj.insert("g", _color.g);
//    rgbobj.insert("b", _color.b);
//    rgbobj.insert("a", _color.a);

//    obj.insert("_color", rgbobj );

//    obj.insert("_radius", _radius);
//    obj.insert("_proportion",_proportion);

//    doc.setObject(obj);

//    return doc.toVariant().toString();
//}




//QTextStream& operator << ( QTextStream& out_stream, const Monomer& m )
//{
//    QJsonDocument doc;

//    QJsonObject obj;

//    obj.insert("type_id",m.typeID());
//    obj.insert("_name",m.name());

//    QJsonObject rgbobj;
//    Qwt3D::RGBA rgb = m.color();
//    rgbobj.insert("r", rgb.r);
//    rgbobj.insert("g", rgb.g);
//    rgbobj.insert("b", rgb.b);
//    rgbobj.insert("a", rgb.a);

//    obj.insert("_color", rgbobj );

//    obj.insert("_radius", m.radius());
//    obj.insert("_proportion",m.proportion());

//    doc.setObject(obj);

//    out_stream << doc.toVariant().toString();
//    return out_stream;
//}


//QTextStream& operator >> ( QTextStream& in_stream, MyColorVector& cv )
//{
////    Qwt3D::RGBA rgb;

////    cv.clear();

////    while ( false == in_stream.atEnd() )
////    {
////        in_stream >> rgb.r >> rgb.g >> rgb.b >> rgb.a;

////        cv.push_back( rgb );
////    }

//    return in_stream;
//}
