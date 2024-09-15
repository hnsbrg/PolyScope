#ifndef CHAINGRAPH_H
#define CHAINGRAPH_H

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


#include "qwt3d_graphplot.h"
#include "grid.h"
#include "coloration.h"
#include "monomerlist.h"
#include "monomersequence.h"
#include "additivelist.h"

#include <QMenu>

using namespace Qwt3D;

typedef QList<int> MonomerCount;

class MainWindow;

class ChainGraph : public Qwt3D::GraphPlot
{
    Q_OBJECT

public:
    ChainGraph( QWidget* w );

    void configure();
    void drawChains( Grid* grid, MonomerList* list );
    void drawPointCloud( Parameters& gParams, int num_particles,  MonomerSequence* sequence, AdditiveList* additiveList );
    void recolorPointCloud( MonomerSequence* sequence, AdditiveList* additiveList );
    void setShowMonomer( bool b );
    void setSpaceFilling( bool b );
    void scanChains( int numChains );
    void refreshChains();
    void clear();
    void setColoration( enum COLORATION c ) { coloration = c;}
    AtomVector& monomerArray() { return nodes;}
    const MonomerCount& monomerCount() { return monomer_count; }
    const MonomerCount& updateMonomerCount( int numMonomers );
    float avgRadiusOfGyration() const { return avg_radius_of_gyration; }
    float avgRadiusOfGyration( Grid* grid );
    float avgEndToEndDistance( Grid* grid );

protected:
    MainWindow* main_win;
    AtomVector nodes;
    BondVector bonds;
    int chain_count;
    bool show_monomers;
    bool space_filling;
    enum COLORATION coloration;
    QMenu* popup_menu;
    const MonomerList* monomer_list;
    MonomerCount monomer_count;
    float avg_radius_of_gyration;
    int current_scanned_chain;

    virtual void contextMenuEvent( QContextMenuEvent* event );
    Vector NormalizePoint( Grid* grid, Vector v, float gScale );
    void updateTitle();
    Vector calculateCenterOfMMass( Chain* chain );
    float calculateRadiusOfGyration( Chain* chain );
    float calculateEndToEndDistance( Chain* chain );

protected slots:
    void showDefaultView();
    void showTopView();
    void showFrontView();
    void showSideView();
    void saveSnapshot();
};

#endif // CHAINGRAPH_H
