/*******************************************/
/*  VECTOR TOOLS                           */
/*                                         */
/*  MM  Mar. 19. 98                        */
/*                                         */
/* Matthias H. Müller. Diss. Techn. Wiss.  */
/* ETH Zürich, Nr. 13096, 1999.            */
/*******************************************/

/*******************************************/
/*                                         */
/* modified by WD Hinsberg 12.2.2023       */
/*                                         */
/*******************************************/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <sys/types.h>

#define EPS 1e-8


typedef struct
{
    float x;
    float y;
    float z;
    int  monomer_type;
} Vector;


/* -------- Tools ----------------------------------- */

void     start_clock();
int      get_clock();

/* ---------- Procedures ----------------------------------------- */

Vector Vector_null();

Vector Vector_sum( Vector a, Vector b );
Vector Vector_diff( Vector a, Vector b );
Vector Vector_unit_diff( Vector a, Vector b );

float  Vector_scalar_prod( Vector a, Vector b );
Vector Vector_prod( Vector a, Vector b );

float  Vector_angle( Vector a, Vector b, Vector c );
float  Vector_torsion( Vector a, Vector b, Vector c, Vector d );

float  Vector_length( Vector a );
Vector Vector_stretch( Vector a, float f );

float  Vector_dist( Vector a, Vector b );
float  Vector_square_dist( Vector a, Vector b );

Vector Vector_periodic_diff( Vector a, Vector b, Vector period );
float  Vector_periodic_square_dist( Vector a, Vector b, Vector period );
Vector Vector_periodic_box( Vector p, Vector period );

void   Vector_positions( Vector a, Vector b, Vector c, float length, float angle,
                         char random_start, int nr_vecs, Vector* vecs );

Vector Random_direction( float length );
Vector ConstrainedRandom_direction( float length, float min, float max );

Vector Random_up_direction( float length, float max_teta );

void   Vector_set_kappa( float kappa );

float  Vector_angle_probability( float angle );

void   Vector_positions_distributed( Vector a, Vector b, Vector c, float length,
                                     int nr_vecs, Vector* vecs );

void   Vector_positions_sampled( Vector a, Vector b, Vector c, float length,
                                 int nr_samples, int nr_vecs, Vector* vecs );

#endif
