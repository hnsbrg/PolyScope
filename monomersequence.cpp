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

#include "monomersequence.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static const Monomer DEFAULT_MONOMER;


MonomerSequence::MonomerSequence():
    sequence_type( MonomerSequence::HOMOPOLYMER ),
    sequence_list(),
    current_monomer_index( -1 ),
    sum_of_proportions( 0.0 ),
    rng()
{
}

MonomerSequence::~MonomerSequence()
{
    clearSequenceList();
}



void  MonomerSequence::clearSequenceList()
{
    for ( int i = 0; i < sequence_list.count(); i++ )
    {
        delete sequence_list[i];
    }

    sequence_list.clear();
}



void MonomerSequence::addMonomerToSequence( const Monomer* mon, double proportion )
{
    Monomer* m = new Monomer( *mon );
    m->setProportion( proportion );
    sequence_list.append( m );
}



const Monomer* MonomerSequence::firstMonomer()
{
    switch ( sequence_type )
    {
        case MonomerSequence::HOMOPOLYMER:
            break;

        case MonomerSequence::RANDOM:

            sum_of_proportions  = 0.0;
            for ( int i = 0; i < sequence_list.count(); i++ )
            {
                sum_of_proportions += sequence_list.at( i )->proportion();
            }
            break;

        case MonomerSequence::ORDERED:
            current_monomer_index = 0;
            break;
    }
    return nextMonomer();
}



const Monomer* MonomerSequence::nextMonomer()
{
    switch ( sequence_type )
    {
        case MonomerSequence::HOMOPOLYMER:
        default:
            return &DEFAULT_MONOMER;
            break;

        case MonomerSequence::RANDOM:
        {
            if ( sequence_list.count() == 0 )
            {
                return &DEFAULT_MONOMER;
            }
            else
            {
                double random_prop = rng.generateDouble() * sum_of_proportions;
                double sum_prop = 0.0;
                for ( int i = 0; i < sequence_list.count(); i++ )
                {
                    sum_prop += sequence_list.at( i )->proportion();
                    if ( sum_prop > random_prop )
                    {
                        return sequence_list.at( i );
                    }
                }
            }

        }
        break;

        case MonomerSequence::ORDERED:
        {
            if ( current_monomer_index >= sequence_list.count() )
            {
                current_monomer_index = 0;
            }

            return  sequence_list.count() == 0 ? &DEFAULT_MONOMER : sequence_list.at( current_monomer_index++ );
        }
        break;
    }
}



bool MonomerSequence::loadSequenceFromTable( SEQUENCE sequenceType, const MonomerList& typeList, QTableWidget* table )
{
    setSequenceType( sequenceType );
    clearSequenceList();

    switch ( sequenceType )
    {
        case MonomerSequence::HOMOPOLYMER:
            break;

        case MonomerSequence::RANDOM:
        {
            for ( int i = 0; i < table->rowCount(); i++ )
            {
                addMonomerToSequence( typeList.monomer( table->item( i, 0 )->text() ), table->item( i, 1 )->text().toDouble() );
            }
        }
        break;

        case MonomerSequence::ORDERED:
        {
            for ( int i = 0; i < table->rowCount(); i++ )
            {
                addMonomerToSequence( typeList.monomer( table->item( i, 0 )->text() ) );
            }
        }
        break;

    }

    return true;
}



QString MonomerSequence::toJsonString() const
{
    QJsonDocument doc;
    QJsonObject main_obj;
    QJsonArray array;

    main_obj.insert( "sequence_type", sequence_type );

    for ( int i = 0; i < sequence_list.count(); i++ )
    {
        Monomer* m = sequence_list.at( i );

        QJsonObject obj;

        obj.insert( "name", m->name() );
        obj.insert( "proportion", m->proportion() );

        array.append( obj );
    }

    main_obj.insert( "sequence", array );
    doc.setObject( main_obj );

    return QString( doc.toJson( QJsonDocument::Compact ) );
}



bool MonomerSequence::fromJsonString( const QString& vals, const MonomerList& typeList )
{
    clearSequenceList();
    QJsonDocument doc = QJsonDocument::fromJson( vals.toUtf8() );

    QJsonObject main_obj = doc.object();
    sequence_type = ( enum SEQUENCE )  main_obj.value( "sequence_type" ).toInt();

    QJsonArray array = main_obj.value( "sequence" ).toArray();

    for ( int i = 0; i < array.size(); i++ )
    {
        QJsonObject monomer_obj = array.at( i ).toObject();

        addMonomerToSequence( typeList.monomer( monomer_obj.value( "name" ).toString() ),  monomer_obj.value( "proportion" ).toDouble() );
    }

    return true;
}
