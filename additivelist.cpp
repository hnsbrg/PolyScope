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

#include "additivelist.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AdditiveList::AdditiveList():
    additive_list(),
    use_additives( None )
{
}



AdditiveList::~AdditiveList()
{
    clearList();
}



bool AdditiveList::loadListFromTable( QTableWidget* table, const MonomerList& typeList )
{
    clearList();

    for ( int i = 0; i < table->rowCount(); i++ )
    {
        addAdditiveToSequence( typeList.monomer( table->item( i, 0 )->text() ),   table->item( i, 1 )->text().toDouble(), table->item( i, 2 )->text().toInt(),  Qt::Checked == table->item( i, 3 )->checkState() );
    }

    return true;
}

QString AdditiveList::toJsonString() const
{
    QJsonDocument doc;
    QJsonObject main_obj;
    QJsonArray array;

    for ( int i = 0; i < additive_list.count(); i++ )
    {
        Additive* a = additive_list.at( i );

        QJsonObject obj;

        obj.insert( "name", a->name() );
        obj.insert( "clustersize", a->avgClusterSize() );
        obj.insert( "poisson", a->usePoisson() );
        obj.insert( "fraction", a->fraction() );

        array.append( obj );
    }

    main_obj.insert( "additivelist", array );
    main_obj.insert( "use_additives", ( int ) use_additives );

    doc.setObject( main_obj );

    return QString( doc.toJson( QJsonDocument::Compact ) );
}


void AdditiveList::fromJsonString( const QString& vals, const MonomerList& typeList )
{
    clearList();

    QJsonDocument doc = QJsonDocument::fromJson( vals.toUtf8() );

    QJsonObject main_obj = doc.object();

    QJsonValue val =  main_obj.value( "use_additives" );

    if ( val.isBool() )
    {
        bool bval = main_obj.value( "use_additives" ).toBool();
        use_additives = ( true == bval ) ? Appended : None;
    }
    else
    {
        use_additives = ( enum ADDITIVE_USE ) val.toInt();
    }

    QJsonArray array = main_obj.value( "additivelist" ).toArray();

    for ( int i = 0; i < array.size(); i++ )
    {
        QJsonObject additive_obj = array.at( i ).toObject();

        addAdditiveToSequence( typeList.monomer( additive_obj.value( "name" ).toString() ),  additive_obj.value( "fraction" ).toDouble(), additive_obj.value( "clustersize" ).toInt(), additive_obj.value( "poisson" ).toBool() );
    }

    return;
}



double AdditiveList::totalFraction() const
{
    double total = 0.0;
    for ( int i = 0; i < additive_list.count(); i++ )
    {
        total += additive_list[i]->fraction();
    }

    return total;
}

int AdditiveList::totalParticles() const
{
    int total = 0;
    for ( int i = 0; i < additive_list.count(); i++ )
    {
        total += additive_list[i]->numParticles();
    }

    return total;
}


int AdditiveList::apportionParticles( int totalSystemParticles )
{
    int particles_allocated = 0;

    for ( int i = 0; i < additive_list.count(); i++ )
    {
        int particles = qRound( additive_list[i]->fraction() * totalSystemParticles );
        additive_list[i]->setNumParticles( particles );
        particles_allocated += particles;
    }

    return totalSystemParticles - particles_allocated;
}



void AdditiveList::clearList()
{
    for ( int i = 0; i < additive_list.count(); i++ )
    {
        delete additive_list[i];
    }

    additive_list.clear();
}

void AdditiveList::addAdditiveToSequence( const Monomer* mon, double fraction, int clusterSize, bool usePoisson )
{
    Additive* a = new Additive( *mon );
    a->setFraction( fraction );
    a->setAvgClusterSize( clusterSize );
    a->setUsePoisson( usePoisson );
    a->setNumParticles( 0 );
    additive_list.append( a );
}
