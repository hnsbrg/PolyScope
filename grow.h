/*******************************************/
/*  AMORPHOUS PACKING (OFF LATTICE)        */
/*                                         */
/*  MM  Mar. 6. 98                         */
/*                                         */
/* Matthias H. Müller. Diss. Techn. Wiss.  */
/* ETH Zürich, Nr. 13096, 1999.            */
/*******************************************/

/*******************************************/
/*                                         */
/* modified by WD Hinsberg 2.2.2024        */
/*                                         */
/*******************************************/

#ifndef GROW_H
#define GROW_H

#include "grid.h"
#include <QByteArray>


typedef struct
{
    int   nr_packings;
    int   chain_len;
    int   nr_particles;
    int   nr_chain_trials;
    float dispersity;

    float max_overlap;
    int   nr_angles;
    int   ahead_depth;
    int   seed;
} Grow_Parameters;

void Pack( Grid* grid, Grow_Parameters* grow_params );
float    Grid_max_overlap( Grid* grid, Grow_Parameters* grow_params );


char Save_System( char filename[], Grid* grid, Grow_Parameters* grow_params, const char* monomerJsonList, const char* sequenceJsonList, const char* additivreJsonList );
int Load_System( char filename[], Grid* grid, Grow_Parameters* grow_params, QByteArray& monomerJsonList, QByteArray& sequenceJsonList,  QByteArray& additiveJsonList );

#endif
