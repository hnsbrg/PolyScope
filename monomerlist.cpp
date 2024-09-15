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

#include "monomerlist.h"
#include "random.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


MonomerList::MonomerList():
    monomer_list()
{
}



MonomerList::~MonomerList()
{
    clearMonomerList();
}



void MonomerList::clearMonomerList()
{
    for ( int i = 0; i < monomer_list.count(); i++ )
    {
        delete monomer_list.at( i );
    }

    monomer_list.clear();
}



void MonomerList::addMonomer( int type, const QString& name, Qwt3D::RGBA c, double radius )
{
    Monomer* m = new Monomer( type, name, c, radius );

    monomer_list.append( m );
}



QStringList MonomerList::monomerNameList()
{
    QStringList list;

    for ( int i = 0; i < monomer_list.count(); i++ )
    {
        list << monomer_list.at( i )->name();
    }

    return list;
}



const Monomer* MonomerList::monomer( const QString& name ) const
{
    for ( int i = 0; i < monomer_list.count(); i++ )
    {
        if ( name == monomer_list.at( i )->name() )
        {
            return monomer_list.at( i );
        }
    }

    return 0;
}



const Monomer* MonomerList::monomer( int typeID ) const
{
    return monomer_list.at( typeID );
}



QString MonomerList::toJsonString() const
{
    QJsonDocument doc;
    QJsonArray array;

    for ( int i = 0; i < monomer_list.count(); i++ )
    {
        Monomer* m = monomer_list.at( i );

        QJsonObject obj;

        obj.insert( "type_id", m->typeID() );
        obj.insert( "name", m->name() );

        QJsonObject rgbobj;
        Qwt3D::RGBA rgb = m->color();
        rgbobj.insert( "r", rgb.r );
        rgbobj.insert( "g", rgb.g );
        rgbobj.insert( "b", rgb.b );
        rgbobj.insert( "a", rgb.a );
        obj.insert( "color", rgbobj );

        obj.insert( "radius", m->radius() );

        array.append( obj );
    }

    doc.setArray( array );

    return QString( doc.toJson( QJsonDocument::Compact ) );
}



bool MonomerList::fromJsonString( const QString& vals )
{
    QJsonDocument doc = QJsonDocument::fromJson( vals.toUtf8() );

    if ( true == doc.isArray() )
    {
        clearMonomerList();

        QJsonArray array = doc.array();

        for ( int i = 0; i < array.size(); i++ )
        {
            QJsonObject monomer_obj = array.at( i ).toObject();
            int type_id = monomer_obj.value( "type_id" ).toInt();
            QString name  = monomer_obj.value( "name" ).toString();

            QJsonObject color_obj = monomer_obj.value( "color" ).toObject();
            Qwt3D::RGBA rgb;
            rgb.r = color_obj.value( "r" ).toDouble();
            rgb.g = color_obj.value( "g" ).toDouble();
            rgb.b = color_obj.value( "b" ).toDouble();
            rgb.a = color_obj.value( "a" ).toDouble();

            double radius  = monomer_obj.value( "radius" ).toDouble();

            addMonomer( type_id, name, rgb, radius );
        }

        return true;

    }
    else
    {
        return false;
    }
}

