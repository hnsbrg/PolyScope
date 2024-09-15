/*******************************************/
/*  CUBIC GRID OBJECT                      */
/*                                         */
/*  MM  Mar. 19. 98                        */
/*                                         */
/* Matthias H. Müller. Diss. Techn. Wiss.  */
/* ETH Zürich, Nr. 13096, 1999.            */
/*******************************************/
/*                                         */
/* modified by WD Hinsberg 12.28.2023      */
/*                                         */
/*******************************************/

#ifndef GRID_H
#define GRID_H

#include "vector.h"

#define NO_CHAIN -1
#define NO_SITE -1
#define NIL -1
#define NO_ATOM 1000000

#ifndef false
#define false 0
#define true !false
#endif

#define MAX_ATOMS_SITE 5

#define NO_DIR -1

#define NUM_DIRS 6
/* #define NUM_DIRS 26 */

#define MAX_VECS 100

typedef struct
{
    int x;
    int y;
    int z;
} Location;

static Location dir_vec[] =
{{1, 0, 0}, { -1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};

/*
static Location dir_vec[] =
  {{-1,-1,-1},{-1,-1, 0},{-1,-1, 1},
   {-1, 0,-1},{-1, 0, 0},{-1, 0, 1},
   {-1, 1,-1},{-1, 1, 0},{-1, 1, 1},
   { 0,-1,-1},{ 0,-1, 0},{ 0,-1, 1},
   { 0, 0,-1},           { 0, 0, 1},
   { 0, 1,-1},{ 0, 1, 0},{ 0, 1, 1},
   { 1,-1,-1},{ 1,-1, 0},{ 1,-1, 1},
   { 1, 0,-1},{ 1, 0, 0},{ 1, 0, 1},
   { 1, 1,-1},{ 1, 1, 0},{ 1, 1, 1}};
*/

/* --------- Grid ------------------------------ */

typedef struct
{
    Vector box_size;

    float atom_radius;
    float bond_len;

    char  angle_fixed;
    char  brush;
    float bond_angle; /* for fixed bond angle */
    float kappa;      /* bond angle distr f(angle) = exp(-k*(1-cos(angle))) */
    float z_exponent; /* z-Axis alignment */
    char  film;
    char  point_cloud;
} Parameters;


typedef struct
{
    int first;        /* first, last indirect indices */
    int last;
    int offset;       /* nr of atoms[0] */
    int max_atoms;    /* length of atoms[] */
    Vector* atoms;
} Chain;


typedef struct
{
    int chain_nr;
    int atom_nr;
    Vector vec;
} Atom_Ref;


typedef struct
{
    int chain_nr;
    int atom_nr;
    Vector vec;
    float angle_dist0;
    float angle_dist1;
} Relaxed_Atom;


typedef struct
{
    int prev;
    int next;
} List;


typedef struct
{
    int  nr_atoms;
    Atom_Ref atoms[MAX_ATOMS_SITE];
    List empty;
    int  nr_occ_neighbours;
    int  marked;
} Site;


typedef struct
{
    int Lx, Ly, Lz;
    Vector site_width;

    Parameters params;

    int nr_sites;
    Site* sites;
    int first_empty;

    int max_chains;
    Chain* chains;

    int* queue;       /* breath first search */
    int queue_first;
    int queue_last;

    int* marked_list;
    int nr_marked;

    Relaxed_Atom* relaxed_atoms;
    int max_relaxed_atoms;
    int nr_relaxed_atoms;
} Grid;


/* -------- Methods ----------------------------------- */

void     Grid_init( Grid* gird, Parameters* params );
void     Grid_clear( Grid* gird );
void     Grid_free( Grid* gird );

Location Grid_site_to_loc( Grid* gird, int site );
int      Grid_loc_to_site( Grid* gird, Location loc );
int      Grid_vec_to_site( Grid* grid, Vector vec );
Vector   Grid_site_to_vec( Grid* grid, int site_nr );
Vector   Grid_atom_to_vec( Grid* grid, int chain_nr, int atom_nr );

Location Grid_loc_step( Grid* gird, Location loc, int dir );
int      Grid_site_step( Grid* gird, int site, int dir );
Vector   Grid_vec_step( Grid* grid, Vector vec, int dir );

char     Grid_site_is_empty( Grid* gird, int site_nr, char soft );
int      Grid_any_empty_site( Grid* gird );
char     Grid_available_sites( Grid* gird, int site_nr, int nr_sites, char soft );
void     Grid_sites_per_dir( Grid* gird, int site_nr, int max_sites, char soft, int* nr_sites );
int      Grid_min_sites_per_dir( Grid* grid, int site_nr, int max_sites, char soft );

char     Grid_site_splits( Grid* gird, int site_nr, char soft );

float    Grid_overlap_atom( Grid* grid, Vector vec, int chain_nr, int atom_nr,
                            int* overlap_chain, int* overlap_atom );
float    Grid_overlap( Grid* grid, Vector vec, int chain_nr, int atom_nr );

char     Grid_reduce_overlap( Grid* grid, Vector* vec, int chain_nr, int atom_nr );
char     Grid_reduce_overlap_bonded( Grid* grid, Vector last_vec,
                                     Vector* vec, int chain_nr, int atom_nr );
char     Grid_relax( Grid* grid, Vector where, float max_overlap );

float    Grid_distance_error( Grid* grid, Vector vec, int chain_nr, int atom_nr );

int      Grid_chain_head_atom( Grid* grid, int chain_nr );
int      Grid_chain_tail_atom( Grid* grid, int chain_nr );

char     Grid_chain_new_vectors( Grid* grid, int chain_nr, char head, int nr_vecs,
                                 Vector* vecs, float* probs );

char     Grid_chain_head_check( Grid* grid, int chain_nr, Vector vec, int depth, int nr_vecs, float limit );
char     Grid_chain_tail_check( Grid* grid, int chain_nr, Vector vec, int depth, int nr_vecs, float limit );

void     Grid_new_chain( Grid* gird, int chain_nr );

int      Grid_chain_append_atom( Grid* grid, int chain_nr, Vector vec, char head );
int      Grid_chain_append_head( Grid* gird, int chain_nr, Vector vec );
int      Grid_chain_append_tail( Grid* gird, int chain_nr, Vector vec );

char     Grid_chain_remove_atom( Grid* grid, int chain_nr, char head );
char     Grid_chain_remove_head( Grid* grid, int chain_nr );
char     Grid_chain_remove_tail( Grid* grid, int chain_nr );

char     Grid_chain_remove( Grid* grid, int chain_nr );


void     Grid_copy( Grid* source, Grid* dest, char first );
char     Grid_compare( Grid* source, Grid* dest );


#endif
