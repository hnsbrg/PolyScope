#ifndef ADDITIVELIST_H
#define ADDITIVELIST_H

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
#include <QTableWidget>
#include "monomerlist.h"

class AdditiveList
{
public:
    enum ADDITIVE_USE { None, Appended, Prepended };

    AdditiveList();
    ~AdditiveList();

    const Additive* additive( int i ) const { return additive_list.at( i );}
    bool loadListFromTable( QTableWidget* table, const MonomerList& typeList );
    QString toJsonString() const;
    void fromJsonString( const QString& vals, const MonomerList& typeList );
    int count() const { return additive_list.count(); }
    double totalFraction() const;
    int apportionParticles( int totalSystemParticles );
    int particles( int additiveIndex ) const { return additive( additiveIndex )->numParticles(); }
    enum ADDITIVE_USE useAdditives() const { return use_additives;}
    void setUseAdditives( enum ADDITIVE_USE s ) {  use_additives = s;}
    int totalParticles() const;

protected:
    QList<Additive*> additive_list;
    enum ADDITIVE_USE use_additives;

    void  clearList();
    void addAdditiveToSequence( const Monomer* mon, double fraction, int clusterSize,  bool usePoisson );
};


#endif // ADDITIVELIST_H
