#ifndef MONOMERSEQUENCE_H
#define MONOMERSEQUENCE_H

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
#include <QTableWidget>
#include "monomerlist.h"
#include <QRandomGenerator>

class MonomerSequence
{
public:
    enum SEQUENCE { HOMOPOLYMER, RANDOM, ORDERED };

    MonomerSequence();
    ~MonomerSequence();

    enum SEQUENCE  sequenceType() const { return sequence_type; }
    const Monomer* firstMonomer();
    const Monomer* nextMonomer();
    const Monomer* monomer( int i ) const { return sequence_list.at( i );}
    bool loadSequenceFromTable( enum SEQUENCE sequenceType, const MonomerList& typeList, QTableWidget* table );
    QString toJsonString() const;
    bool fromJsonString( const QString& vals, const MonomerList& typeList );
    int count() const { return sequence_list.count(); }
    void setRandomNumberSeed( int s ) { rng.seed( s ); }

protected:
    enum SEQUENCE sequence_type;
    QList<Monomer*> sequence_list;
    int current_monomer_index;
    double sum_of_proportions;
    QRandomGenerator rng;

    void setSequenceType( enum SEQUENCE seqType ) { sequence_type = seqType; }
    void  clearSequenceList();
    void addMonomerToSequence( const Monomer* mon, double proportion = 1.0 );
};

#endif // MONOMERSEQUENCE_H
