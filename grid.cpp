/*******************************************/
/*  CUBIC GRID OBJECT                      */
/*                                         */
/*  MM  Mar. 6. 98                         */
/*                                         */
/* Matthias H. Müller. Diss. Techn. Wiss.  */
/* ETH Zürich, Nr. 13096, 1999.            */
/*******************************************/

/*******************************************/
/*                                         */
/* modified by WD Hinsberg 12.28.2023      */
/*                                         */
/*******************************************/
#include <QDebug>
#include <stdio.h>
#include<stdlib.h>
#include "grid.h"
#include "vector.h"
#include "random.h"

#include "mainwindow.h"
#include "grow.h"

const int BUFFER_LENGTH = 128;
static char  buffer[BUFFER_LENGTH];

void Grid_test( Grid* grid );


/* ============ Grid ======================================== */

#define MAX_ATOMS_INCREMENT 1000
#define MAX_CHAINS_INCREMENT 100

#define NR_SAMPLES 3
#define NR_POSITIONS_ITERS 10
#define ENVIRONMENT_SIZE 1
#define RELAX_ITERS 100

#define OVER_RELAXATION 1.80

/* ----------------------------------------------------------------------------------------- */
void Grid_clear( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr;
    Site* site;

    grid->first_empty = 0;
    for ( i = 0; i < grid->nr_sites; i++ )
    {
        site = &grid->sites[i];
        site->nr_atoms = 0;
        site->nr_occ_neighbours = 0;
        site->empty.next = i + 1;
        site->empty.prev = i - 1;
        site->marked = 0;
    }
    grid->sites[0].empty.prev = NIL;
    grid->sites[grid->nr_sites - 1].empty.next = NIL;

    for ( i = 0; i < grid->max_chains; i++ )
    {
        nr = grid->chains[i].max_atoms / 2;
        grid->chains[i].offset = -nr;
        grid->chains[i].first = 0;
        grid->chains[i].last = 0;
    }
    grid->queue_first = 0;
    grid->queue_last = 0;
    grid->nr_marked = 0;
    grid->nr_relaxed_atoms = 0;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_init( Grid* grid, Parameters* params )
/* ----------------------------------------------------------------------------------------- */
{
    float w;
    int nr;

    grid->params = *params;
    w = 2.0 * params->atom_radius;
    if ( w <= 0.0 ) w = 1.0;
    nr = params->box_size.x / w;
    if ( nr < 1 ) nr = 1;
    if ( nr > 100 ) nr = 100;
    grid->site_width.x = params->box_size.x / nr;
    grid->Lx = nr;
    nr = params->box_size.y / w;
    if ( nr < 1 ) nr = 1;
    if ( nr > 100 ) nr = 100;
    grid->site_width.y = params->box_size.y / nr;
    grid->Ly = nr;
    nr = params->box_size.z / w;
    if ( nr < 1 ) nr = 1;
    if ( nr > 100 ) nr = 100;
    grid->site_width.z = params->box_size.z / nr;
    grid->Lz = nr;

    grid->nr_sites = grid->Lx * grid->Ly * grid->Lz;
    grid->sites = ( Site* ) malloc( grid->nr_sites * sizeof( Site ) );

    grid->max_chains = 0;
    grid->chains = NULL;

    grid->queue = ( int* ) malloc( grid->nr_sites * sizeof( int ) );
    grid->marked_list = ( int* ) malloc( grid->nr_sites * sizeof( int ) );

    grid->relaxed_atoms = NULL;
    grid->max_relaxed_atoms = 0;

    Grid_clear( grid );
}


/* ----------------------------------------------------------------------------------------- */
void Grid_free( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int i;

    free( ( void* )grid->sites );
    for ( i = 0; i < grid->max_chains; i++ )
        if ( grid->chains[i].max_atoms > 0 )
            free( ( void* )grid->chains[i].atoms );
    free( ( void* )grid->queue );
    free( ( void* )grid->marked_list );
    if ( grid->max_relaxed_atoms > 0 )
        free( ( void* )grid->relaxed_atoms );
}


/* ----------------------------------------------------------------------------------------- */
Location Grid_site_to_loc( Grid* grid, int site )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;
    int i;

    if ( site >= grid->nr_sites ) site = grid->nr_sites - 1;
    if ( site < 0 ) site = 0;
    loc.x = site % grid->Lx;
    site = site / grid->Lx;
    loc.y = site % grid->Ly;
    site = site / grid->Ly;
    loc.z = site;
    return loc;
}


/* ----------------------------------------------------------------------------------------- */
Location Grid_loc_period( Grid* grid, Location loc )
/* ----------------------------------------------------------------------------------------- */
{
    loc.x = loc.x % grid->Lx;
    if ( loc.x < 0 ) loc.x += grid->Lx;
    loc.y = loc.y % grid->Ly;
    if ( loc.y < 0 ) loc.y += grid->Ly;
    loc.z = loc.z % grid->Lz;
    if ( loc.z < 0 ) loc.z += grid->Lz;
    return loc;
}


/* ----------------------------------------------------------------------------------------- */
int Grid_loc_to_site( Grid* grid, Location loc )
/* ----------------------------------------------------------------------------------------- */
{
    loc = Grid_loc_period( grid, loc );
    return ( loc.z * grid->Ly + loc.y ) * grid->Lx + loc.x;
}


/* ----------------------------------------------------------------------------------------- */
Location Grid_loc_step( Grid* grid, Location loc, int dir )
/* ----------------------------------------------------------------------------------------- */
{
    if ( ( dir >= NUM_DIRS ) || ( dir < 0 ) ) return loc;
    loc.x += dir_vec[dir].x;
    loc.y += dir_vec[dir].y;
    loc.z += dir_vec[dir].z;
    return Grid_loc_period( grid, loc );
}


/* ----------------------------------------------------------------------------------------- */
int Grid_site_step( Grid* grid, int site, int dir )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;

    loc = Grid_site_to_loc( grid, site );
    loc = Grid_loc_step( grid, loc, dir );
    return Grid_loc_to_site( grid, loc );
}


/* ----------------------------------------------------------------------------------------- */
Location Grid_vec_to_loc( Grid* grid, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;

    loc.x = vec.x / grid->site_width.x;
    if ( vec.x < 0.0 ) loc.x--;
    loc.y = vec.y / grid->site_width.y;
    if ( vec.y < 0.0 ) loc.y--;
    loc.z = vec.z / grid->site_width.z;
    if ( vec.z < 0.0 ) loc.z--;

    return Grid_loc_period( grid, loc );
}

/* ----------------------------------------------------------------------------------------- */
Location Grid_vec_to_loc_min( Grid* grid, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;
    Vector w, w2;

    w = grid->site_width;
    w2 = Vector_stretch( w, 0.5 );
    loc.x = vec.x / w.x;
    if ( vec.x < 0.0 ) loc.x--;
    loc.y = vec.y / w.y;
    if ( vec.y < 0.0 ) loc.y--;
    loc.z = vec.z / w.z;
    if ( vec.z < 0.0 ) loc.z--;

    if ( ( vec.x - loc.x * w.x ) < w2.x ) loc.x--;
    if ( ( vec.y - loc.y * w.y ) < w2.y ) loc.y--;
    if ( ( vec.z - loc.z * w.z ) < w2.z ) loc.z--;

    return Grid_loc_period( grid, loc );
}

/* ----------------------------------------------------------------------------------------- */
Location Grid_vec_to_loc_max( Grid* grid, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;
    Vector w, w2;

    w = grid->site_width;
    w2 = Vector_stretch( w, 0.5 );
    loc.x = vec.x / w.x;
    if ( vec.x < 0.0 ) loc.x--;
    loc.y = vec.y / w.y;
    if ( vec.y < 0.0 ) loc.y--;
    loc.z = vec.z / w.z;
    if ( vec.z < 0.0 ) loc.z--;

    if ( ( vec.x - loc.x * w.x ) > w2.x ) loc.x++;
    if ( ( vec.y - loc.y * w.y ) > w2.y ) loc.y++;
    if ( ( vec.z - loc.z * w.z ) > w2.z ) loc.z++;

    return Grid_loc_period( grid, loc );
}


/* ----------------------------------------------------------------------------------------- */
int Grid_vec_to_site( Grid* grid, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;

    loc.x = vec.x / grid->site_width.x;
    if ( vec.x < 0.0 ) loc.x--;
    loc.y = vec.y / grid->site_width.y;
    if ( vec.y < 0.0 ) loc.y--;
    loc.z = vec.z / grid->site_width.z;
    if ( vec.z < 0.0 ) loc.z--;

    return Grid_loc_to_site( grid, loc );
}


/* ----------------------------------------------------------------------------------------- */
Vector Grid_site_to_vec( Grid* grid, int site_nr )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc;
    Vector vec;

    loc = Grid_site_to_loc( grid, site_nr );
    vec.x = grid->site_width.x * ( 0.5 + loc.x );
    vec.y = grid->site_width.y * ( 0.5 + loc.y );
    vec.z = grid->site_width.z * ( 0.5 + loc.z );

    return vec;
}


/* ----------------------------------------------------------------------------------------- */
Vector Grid_atom_to_vec( Grid* grid, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int nr;
    Vector vec;
    Chain* chain;

    vec.x = 0.0;
    vec.y = 0.0;
    vec.z = 0.0;
    if ( ( chain_nr < 0 ) || ( chain_nr >= grid->max_chains ) ) return vec;
    chain = &grid->chains[chain_nr];
    if ( ( atom_nr < chain->first ) || ( atom_nr >= chain->last ) )
        return vec;
    nr = atom_nr - chain->offset;
    return chain->atoms[nr];
}


/* ----------------------------------------------------------------------------------------- */
Vector Grid_vec_step( Grid* grid, Vector vec, int dir )
/* ----------------------------------------------------------------------------------------- */
{
    Vector diff;
    float d;

    if ( ( dir >= NUM_DIRS ) || ( dir < 0 ) ) return vec;
    diff.x = dir_vec[dir].x;
    diff.y = dir_vec[dir].y;
    diff.z = dir_vec[dir].z;
    d = Vector_length( diff );

    return Vector_sum( vec, Vector_stretch( diff, grid->params.bond_len / d ) );
}



/* ----------------------------------------------------------------------------------------- */
void Grid_site_add_atom( Grid* grid, int chain_nr, int atom_nr, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    int nr, site_nr, adj_nr;
    Site* site;
    Location loc, adj, min_loc, max_loc;

    loc = Grid_vec_to_loc( grid, vec );
    site_nr = Grid_loc_to_site( grid, loc );
    site = &grid->sites[site_nr];
    nr = site->nr_atoms;
    if ( nr >= MAX_ATOMS_SITE )
    {
        sprintf( buffer, "fatal: too many atoms per site\n" );
        appWindow()->appendText( buffer );
        exit( 1 );
    }
    site->atoms[nr].chain_nr = chain_nr;
    site->atoms[nr].atom_nr = atom_nr;
    site->atoms[nr].vec = vec;
    site->nr_atoms++;

    if ( nr == 0 )    /* first atom */
    {
        if ( site->empty.prev == NIL )
            grid->first_empty = site->empty.next;
        else
            grid->sites[site->empty.prev].empty.next = site->empty.next;

        if ( site->empty.next != NIL )
            grid->sites[site->empty.next].empty.prev = site->empty.prev;

        min_loc = Grid_vec_to_loc_min( grid, vec );
        max_loc = Grid_vec_to_loc_max( grid, vec );
        adj.x = min_loc.x;
        while ( true )
        {
            adj.y = min_loc.y;
            while ( true )
            {
                adj.z = min_loc.z;
                while ( true )
                {
                    adj_nr = Grid_loc_to_site( grid, adj );
                    grid->sites[adj_nr].nr_occ_neighbours++;

                    if ( adj.z == max_loc.z ) break;
                    adj.z++;
                    if ( adj.z >= grid->Lz ) adj.z = 0;
                }
                if ( adj.y == max_loc.y ) break;
                adj.y++;
                if ( adj.y >= grid->Ly ) adj.y = 0;
            }
            if ( adj.x == max_loc.x ) break;
            adj.x++;
            if ( adj.x >= grid->Lx ) adj.x = 0;
        }
    }
}


char Atoms_equal( int chain_nr, int atom_nr, Atom_Ref atom )
{
    return ( chain_nr == atom.chain_nr ) &&
           ( atom_nr  == atom.atom_nr );
}


/* ----------------------------------------------------------------------------------------- */
void Grid_site_remove_atom( Grid* grid, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int first, i, nr, site_nr, adj_nr;
    Site* site;
    Location loc, adj, min_loc, max_loc;
    Vector vec;
    vec = Grid_atom_to_vec( grid, chain_nr, atom_nr );
    loc = Grid_vec_to_loc( grid, vec );

    site_nr = Grid_loc_to_site( grid, loc );
    site = &grid->sites[site_nr];

    nr = 0;
    while ( ( nr < site->nr_atoms ) && !Atoms_equal( chain_nr, atom_nr, site->atoms[nr] ) )
        nr++;
    if ( nr >= site->nr_atoms ) return;

    site->nr_atoms--;
    for ( i = nr; i < site->nr_atoms; i++ )
        site->atoms[i] = site->atoms[i + 1];

    if ( site->nr_atoms == 0 )
    {
        first = grid->first_empty;
        site->empty.prev = NIL;
        site->empty.next = first;
        grid->first_empty = site_nr;
        if ( site->empty.next != NIL )
            grid->sites[site->empty.next].empty.prev = site_nr;

        min_loc = Grid_vec_to_loc_min( grid, vec );
        max_loc = Grid_vec_to_loc_max( grid, vec );
        adj.x = min_loc.x;
        while ( true )
        {
            adj.y = min_loc.y;
            while ( true )
            {
                adj.z = min_loc.z;
                while ( true )
                {
                    adj_nr = Grid_loc_to_site( grid, adj );
                    grid->sites[adj_nr].nr_occ_neighbours--;

                    if ( adj.z == max_loc.z ) break;
                    adj.z++;
                    if ( adj.z >= grid->Lz ) adj.z = 0;
                }
                if ( adj.y == max_loc.y ) break;
                adj.y++;
                if ( adj.y >= grid->Ly ) adj.y = 0;
            }
            if ( adj.x == max_loc.x ) break;
            adj.x++;
            if ( adj.x >= grid->Lx ) adj.x = 0;
        }
    }
}


/* ----------------------------------------------------------------------------------------- */
void Grid_new_chain( Grid* grid, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int i, old_max;

    if ( chain_nr >= grid->max_chains )
    {
        old_max = grid->max_chains;
        while ( grid->max_chains <= chain_nr )
            grid->max_chains += MAX_CHAINS_INCREMENT;
        grid->chains = ( Chain* )realloc( grid->chains, grid->max_chains * sizeof( Chain ) );
        for ( i = old_max; i < grid->max_chains; i++ )
        {
            grid->chains[i].first = 0;
            grid->chains[i].last = 0;
            grid->chains[i].offset = 0;
            grid->chains[i].max_atoms = 0;
            grid->chains[i].atoms = NULL;
        }
    }
}


/* ----------------------------------------------------------------------------------------- */
int Grid_chain_append_atom( Grid* grid, int chain_nr, Vector vec, char head )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr, last;
    Chain* chain;

    if ( chain_nr >= grid->max_chains ) Grid_new_chain( grid, chain_nr );
    chain = &grid->chains[chain_nr];

    if ( chain->max_atoms == 0 )
    {
        chain->max_atoms = MAX_ATOMS_INCREMENT;
        chain->atoms = ( Vector* )malloc( chain->max_atoms * sizeof( Vector ) );
        chain->first = 0;
        chain->last  = 0;
        chain->offset = - chain->max_atoms / 2;
    }
    if ( head )
    {
        if ( ( chain->first - chain->offset ) <= 0 )
        {
            chain->max_atoms += MAX_ATOMS_INCREMENT;
            chain->atoms = ( Vector* )realloc( chain->atoms, chain->max_atoms * sizeof( Vector ) );
            chain->offset -= MAX_ATOMS_INCREMENT;
            for ( i = chain->last - chain->offset - 1; i >= chain->first - chain->offset; i-- )
                chain->atoms[i] = chain->atoms[i - MAX_ATOMS_INCREMENT];
        }
        chain->first--;
        nr = chain->first;
    }
    else      /* tail */
    {
        if ( ( chain->last - chain->offset ) >= ( chain->max_atoms - 1 ) )
        {
            chain->max_atoms += MAX_ATOMS_INCREMENT;
            chain->atoms = ( Vector* )realloc( chain->atoms, chain->max_atoms * sizeof( Vector ) );
        }
        nr = chain->last;
        chain->last++;
    }
    chain->atoms[nr - chain->offset] = vec;
    Grid_site_add_atom( grid, chain_nr, nr, vec );
    return nr;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_remove_atom( Grid* grid, int chain_nr, char head )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr;
    Chain* chain;

    if ( chain_nr >= grid->max_chains ) return false;
    chain = &grid->chains[chain_nr];

    if ( chain->first >= chain->last ) return false;
    if ( head )
    {
        Grid_site_remove_atom( grid, chain_nr, chain->first );
        chain->first++;
    }
    else      /* tail */
    {
        Grid_site_remove_atom( grid, chain_nr, chain->last - 1 );
        chain->last--;
    }
    return true;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_remove( Grid* grid, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int nr;
    Chain* chain;

    if ( chain_nr >= grid->max_chains ) return false;
    chain = &grid->chains[chain_nr];

    if ( chain->first >= chain->last ) return false;

    for ( nr = chain->first; nr < chain->last; nr++ )
    {
        Grid_site_remove_atom( grid, chain_nr, nr );
    }
    nr = chain->max_atoms / 2;
    chain->offset = -nr;
    chain->first = 0;
    chain->last = 0;
    return true;
}



/* ----------------------------------------------------------------------------------------- */
int Grid_chain_append_head( Grid* grid, int chain_nr, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    return Grid_chain_append_atom( grid, chain_nr, vec, true );
}


/* ----------------------------------------------------------------------------------------- */
int Grid_chain_append_tail( Grid* grid, int chain_nr, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    return Grid_chain_append_atom( grid, chain_nr, vec, false );
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_remove_head( Grid* grid, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    return Grid_chain_remove_atom( grid, chain_nr, true );
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_remove_tail( Grid* grid, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    return Grid_chain_remove_atom( grid, chain_nr, false );
}


/* ----------------------------------------------------------------------------------------- */
float Grid_overlap_atom( Grid* grid, Vector vec, int chain_nr, int atom_nr,
                         int* overlap_chain, int* overlap_atom )
/* ----------------------------------------------------------------------------------------- */
{
    /* chain_nr/atom_nr is the index of the new atom
       used to neglect adjacent atoms */

    Location loc, adj;
    Site* site;
    float d, ol, r, r1, r2, max;
    float rad, radi1, box_height;
    int i;
    Atom_Ref* atom;

    rad     = grid->params.atom_radius;
    if ( rad <= 0.0 ) rad = EPS;
    radi1    = 1.0 / rad;
    r   = 2.0 * rad;
    r1  = 1.0 / r;
    r2  = r * r;
    loc = Grid_vec_to_loc( grid, vec );
    max = 0.0;
    *overlap_chain = -1;
    *overlap_atom  = -1;
    for ( adj.x = loc.x - 1; adj.x <= loc.x + 1; adj.x++ )
    {
        for ( adj.y = loc.y - 1; adj.y <= loc.y + 1; adj.y++ )
        {
            for ( adj.z = loc.z - 1; adj.z <= loc.z + 1; adj.z++ )
            {
                site = &grid->sites[Grid_loc_to_site( grid, adj )];
                for ( i = 0; i < site->nr_atoms; i++ )
                {
                    atom = &site->atoms[i];
                    if ( ( atom->chain_nr != chain_nr ) || ( abs( atom->atom_nr - atom_nr ) > 1 ) )
                    {
                        d = Vector_periodic_square_dist( vec, atom->vec, grid->params.box_size );
                        if ( d < r2 )
                        {
                            d = sqrt( d );
                            ol = ( r - d ) * r1;
                            if ( ol > max )
                            {
                                max = ol;
                                *overlap_chain = atom->chain_nr;
                                *overlap_atom  = atom->atom_nr;
                            }
                        }
                    }
                }
            }
        }
    }
    if ( ( grid->params.brush || grid->params.film ) && ( atom_nr > grid->chains[chain_nr].first ) )
    {
        box_height = grid->params.box_size.z;
        for ( i = 0; i < 2; i++ )
        {
            ol = 0.0;
            switch ( i )
            {
                case 0 :
                    if ( vec.z < rad ) ol = ( rad - vec.z ) * radi1;
                    break;
                case 1 :
                    if ( vec.z > ( box_height - rad ) ) ol = ( vec.z - ( box_height - rad ) ) * radi1;
                    break;
            }
            if ( ol > max )
            {
                max = ol;
                *overlap_chain = -1;
                *overlap_atom  = -1;
            }
        }
    }
    return max;
}


/* ----------------------------------------------------------------------------------------- */
float Grid_overlap( Grid* grid, Vector vec, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int ol_chain, ol_atom;

    return Grid_overlap_atom( grid, vec, chain_nr, atom_nr, &ol_chain, &ol_atom );
}


/* ----------------------------------------------------------------------------------------- */
float Grid_max_overlap( Grid* grid, Grow_Parameters* grow_params )
/* ----------------------------------------------------------------------------------------- */
{
    int nr, chain_nr;
    float ol, max;
    Chain* chain;
    int ol_chain, ol_atom;
    int max_chain0, max_chain1, max_atom0, max_atom1;

    max = 0.0;
    for ( chain_nr = 0; chain_nr < grid->max_chains; chain_nr++ )
    {
        chain = &grid->chains[chain_nr];
        for ( nr = chain->first; nr < chain->last; nr++ )
        {
            ol = Grid_overlap_atom( grid, chain->atoms[nr - chain->offset], chain_nr, nr,
                                    &ol_chain, &ol_atom );
            if ( ( ol > max ) && ( -1 != ol_chain )  && ( -1 != ol_atom ) )
            {
                max = ol;
                max_chain0 = ol_chain;
                max_atom0 = ol_atom;
                max_chain1 = chain_nr;
                max_atom1 = nr;
            }

            //            //    if ( ol > grow_params->max_overlap )
            //            {
            //                qDebug() << QString( "excess overlap %1 %2:%3 - %4:%5" ).arg( ol ).arg( ol_chain ).arg( ol_atom ).arg( chain_nr ).arg( nr );
            //            }


        }
    }
    qDebug() << QString( "max overlap %1 %2:%3 - %4:%5" ).arg( max ).arg( max_chain0 ).arg( max_atom0 ).arg( max_chain1 ).arg( max_atom1 );

    return max;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_reduce_overlap_vector( Grid* grid, Vector vec, int chain_nr, int atom_nr, float r,
                                 Vector* diff_sum, int* nr_diffs )
/* ----------------------------------------------------------------------------------------- */
{
    Location loc, adj;
    Site* site;
    Vector diff, sum;
    int i, nr;
    float d, r1, r2, rad, box_height;
    char  first_atom;
    Atom_Ref* atom;

    first_atom = ( atom_nr <= grid->chains[chain_nr].first );
    rad = 0.5 * r;
    r1  = 1.0 / r;
    r2  = r * r;
    loc = Grid_vec_to_loc( grid, vec );
    sum = Vector_null();
    nr = 0;
    for ( adj.x = loc.x - 1; adj.x <= loc.x + 1; adj.x++ )
    {
        for ( adj.y = loc.y - 1; adj.y <= loc.y + 1; adj.y++ )
        {
            for ( adj.z = loc.z - 1; adj.z <= loc.z + 1; adj.z++ )
            {
                site = &grid->sites[Grid_loc_to_site( grid, adj )];
                for ( i = 0; i < site->nr_atoms; i++ )
                {
                    atom = &site->atoms[i];
                    if ( ( atom->chain_nr != chain_nr ) || ( abs( atom->atom_nr - atom_nr ) > 1 ) )
                    {
                        diff = Vector_periodic_diff( vec, atom->vec, grid->params.box_size );
                        d = Vector_length( diff );
                        if ( d < r )
                        {
                            if ( d < EPS ) d = EPS;
                            diff = Vector_stretch( diff, 1.1 * ( r - d ) / d );
                            sum = Vector_sum( sum, diff );
                            nr++;
                        }
                    }
                }
            }
        }
    }
    /* 2 contstrains coming from xy planes for brushes and films */
    if ( ( grid->params.brush && !first_atom ) || grid->params.film )
    {
        box_height = grid->params.box_size.z;
        for ( i = 0; i < 2; i++ )
        {
            diff = Vector_null();
            switch ( i )
            {
                case 0 :
                    if ( vec.z < rad ) diff.z = rad - vec.z;
                    break;
                case 1 :
                    if ( vec.z > ( box_height - rad ) ) diff.z = box_height - rad - vec.z;
                    break;
            }
            if ( fabs( diff.z ) > 0.0 )
            {
                diff = Vector_stretch( diff, 1.1 );
                sum = Vector_sum( sum, diff );
                nr++;
            }
        }
    }
    if ( grid->params.brush && first_atom ) sum.z = 0.0;
    *diff_sum = sum;
    *nr_diffs = nr;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_reduce_overlap( Grid* grid, Vector* vec, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    Vector diff_sum;
    int    nr_diffs;
    float  r;

    r = 2.0 * grid->params.atom_radius;
    Grid_reduce_overlap_vector( grid, *vec, chain_nr, atom_nr, r, &diff_sum, &nr_diffs );

    if ( nr_diffs <= 0 ) return false;
    diff_sum = Vector_stretch( diff_sum, 1.0 / nr_diffs );

    *vec = Vector_sum( *vec, diff_sum );
    return true;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_reduce_overlap_bonded( Grid* grid, Vector last_vec,
                                 Vector* vec, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    Vector diff;
    float d;
    char changed;

    changed = Grid_reduce_overlap( grid, vec, chain_nr, atom_nr );
    if ( !changed ) return false;

    diff = Vector_diff( *vec, last_vec );
    d = Vector_length( diff );
    if ( d < EPS ) d = EPS;
    diff = Vector_stretch( diff, grid->params.bond_len / d );
    *vec = Vector_sum( last_vec, diff );
    return true;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_add_relaxed_atom( Grid* grid, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr, adj;
    Relaxed_Atom* atom;
    Chain* chain;
    Vector vec;

    for ( i = 0; i < grid->nr_relaxed_atoms; i++ )
    {
        atom = &grid->relaxed_atoms[i];
        if ( ( atom->chain_nr == chain_nr ) && ( atom->atom_nr == atom_nr ) )
            return;
    }
    nr = grid->nr_relaxed_atoms;
    if ( nr >= grid->max_relaxed_atoms )
    {
        grid->max_relaxed_atoms += 200;
        grid->relaxed_atoms = ( Relaxed_Atom* )realloc( grid->relaxed_atoms,
                              grid->max_relaxed_atoms * sizeof( Relaxed_Atom ) );
    }
    vec = Grid_atom_to_vec( grid, chain_nr, atom_nr );
    grid->relaxed_atoms[nr].chain_nr = chain_nr;
    grid->relaxed_atoms[nr].atom_nr = atom_nr;
    grid->relaxed_atoms[nr].vec = vec;
    grid->relaxed_atoms[nr].angle_dist0 = 0.0;
    grid->relaxed_atoms[nr].angle_dist1 = 0.0;
    chain = &grid->chains[chain_nr];
    adj = atom_nr - 2;
    if ( adj >= chain->first )
        grid->relaxed_atoms[nr].angle_dist0 = Vector_dist( vec, Grid_atom_to_vec( grid, chain_nr, adj ) );
    adj = atom_nr + 2;
    if ( adj < chain->last )
        grid->relaxed_atoms[nr].angle_dist1 = Vector_dist( vec, Grid_atom_to_vec( grid, chain_nr, adj ) );
    grid->nr_relaxed_atoms++;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_add_relaxed_atoms( Grid* grid, Vector vec )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr, delta;
    Location loc, adj;
    Site* site;
    Atom_Ref* atom;
    Chain* chain;

    grid->nr_relaxed_atoms = 0;
    delta = ENVIRONMENT_SIZE;

    loc = Grid_vec_to_loc( grid, vec );
    for ( adj.x = loc.x - delta; adj.x <= loc.x + delta; adj.x++ )
    {
        for ( adj.y = loc.y - delta; adj.y <= loc.y + delta; adj.y++ )
        {
            for ( adj.z = loc.z - delta; adj.z <= loc.z + delta; adj.z++ )
            {
                site = &grid->sites[Grid_loc_to_site( grid, adj )];
                for ( i = 0; i < site->nr_atoms; i++ )
                {
                    atom = &site->atoms[i];
                    chain = &grid->chains[atom->chain_nr];
                    for ( nr = atom->atom_nr - 2; nr <= atom->atom_nr + 2; nr++ )
                    {
                        if ( ( nr >= chain->first ) && ( nr < chain->last ) )
                            Grid_add_relaxed_atom( grid, atom->chain_nr, nr );
                    }
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------------------- */
void Grid_reset_relaxed_atoms( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int i;
    Relaxed_Atom* atom;
    Chain* chain;

    for ( i = 0; i < grid->nr_relaxed_atoms; i++ )
    {
        atom = &grid->relaxed_atoms[i];
        Grid_site_remove_atom( grid, atom->chain_nr, atom->atom_nr );
        Grid_site_add_atom( grid, atom->chain_nr, atom->atom_nr, atom->vec );
        chain = &grid->chains[atom->chain_nr];
        chain->atoms[atom->atom_nr - chain->offset] = atom->vec;
    }
    grid->nr_relaxed_atoms = 0;
}


/* ----------------------------------------------------------------------------------------- */
int Nr_smalls( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int chain_nr, len, i, k, nr;
    float a;
    Chain* chain;

    nr = 0;
    for ( chain_nr = 0; chain_nr < grid->max_chains; chain_nr++ )
    {
        chain = &grid->chains[chain_nr];
        len = chain->last - chain->first;
        if ( len <= 0 ) continue;
        for ( i = chain->first; i < chain->last; i++ )
        {
            k = i - chain->offset;
            if ( i > chain->first + 1 )
            {
                a = Vector_angle( chain->atoms[k], chain->atoms[k - 1], chain->atoms[k - 2] );
                if ( a / M_PI * 180.0 < 10.0 ) nr++;
            }
        }
    }
    return nr;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_relax( Grid* grid, Vector where, float max_overlap )
/* ----------------------------------------------------------------------------------------- */

/* Relax environment of vector 'where' */
{
    Vector vec, sum, diff;

    int i, j, nr, iter, new_offs;
    int adj, nr_diffs;
    int nr_ol_diffs;
    char lower_bound, first_atom;
    float f, r, d, c, ol, error, max_error;
    float bond_len, ang_len;
    Vector ol_diffs;
    Relaxed_Atom* atom;
    Chain* chain;

    r = 2.0 * grid->params.atom_radius;
    r = r * ( 1.0 - max_overlap );    /* weaker constraint */
    r = 1.01 * r;             /* over relaxation */

    bond_len = grid->params.bond_len;
    ang_len  = 2.0 * bond_len * sin( 0.5 * ( M_PI - grid->params.bond_angle ) );

    Grid_add_relaxed_atoms( grid, where );

    for ( iter = 0; iter < RELAX_ITERS; iter++ )
    {
        max_error = 0.0;
        for ( i = 0; i < grid->nr_relaxed_atoms; i++ )
        {
            nr_diffs = 0;
            sum = Vector_null();
            atom = &grid->relaxed_atoms[i];
            chain = &grid->chains[atom->chain_nr];
            nr = atom->atom_nr;
            first_atom = ( nr <= chain->first );
            vec = Grid_atom_to_vec( grid, atom->chain_nr, nr );

            /* four contstrains coming from bond_lens, bond_angles */
            for ( j = 0; j < 4; j++ )
            {
                lower_bound = false;
                switch ( j )
                {
                    case 0 :
                        adj = nr - 2;
                        c = ang_len;
                        break;
                    case 1 :
                        adj = nr - 1;
                        c = bond_len;
                        break;
                    case 2 :
                        adj = nr + 1;
                        c = bond_len;
                        break;
                    case 3 :
                        adj = nr + 2;
                        c = ang_len;
                        break;
                }
                if ( ( adj < chain->first ) || ( adj >= chain->last ) ) continue;
                diff = Vector_diff( vec, Grid_atom_to_vec( grid, atom->chain_nr, adj ) );
                if ( !grid->params.angle_fixed )
                {
                    if ( j == 0 ) { c = atom->angle_dist0; lower_bound = true; }
                    if ( j == 3 ) { c = atom->angle_dist1; lower_bound = true; }
                }
                d = Vector_length( diff );
                if ( ( d < c ) || !lower_bound )
                {
                    if ( d < EPS ) d = EPS;
                    f = ( c - d ) / d;
                    error = fabs( f );
                    if ( error > max_error ) max_error = error;
                    diff = Vector_stretch( diff, f );
                    sum = Vector_sum( sum, diff );
                    nr_diffs++;
                }
            }
            /* n contstrains coming from overlaps */
            Grid_reduce_overlap_vector( grid, vec, atom->chain_nr, nr, r,
                                        &ol_diffs, &nr_ol_diffs );
            nr_diffs += nr_ol_diffs;
            sum = Vector_sum( sum, ol_diffs );
            if ( grid->params.brush && first_atom ) sum.z = 0.0;

            if ( nr_diffs > 0 )
            {
                sum = Vector_stretch( sum, OVER_RELAXATION / nr_diffs );
                vec = Vector_sum( vec, sum );

                Grid_site_remove_atom( grid, atom->chain_nr, atom->atom_nr );
                Grid_site_add_atom( grid, atom->chain_nr, atom->atom_nr, vec );
                chain->atoms[nr - chain->offset] = vec;
            }
        } /* next atom */

        if ( ( iter > 5 ) && ( max_error <= 0.01 ) ) break;
    } /* iteration */

    /* printf("relaxed (%i iterations) ->",iter);  */
    if ( max_error > 0.01 )
    {
        Grid_reset_relaxed_atoms( grid );
        /* printf("iter error %f\n",max_error);  */
        return false;
    }

    for ( i = 0; i < grid->nr_relaxed_atoms; i++ )
    {
        atom = &grid->relaxed_atoms[i];
        vec = Grid_atom_to_vec( grid, atom->chain_nr, atom->atom_nr );
        ol = Grid_overlap( grid, vec, atom->chain_nr, atom->atom_nr );
        if ( ol > max_overlap )
        {
            Grid_reset_relaxed_atoms( grid );
            /* printf("overlap error %f\n",ol);  */
            return false;
        }
    }
    /* printf("OK\n",iter);    */
    return true;
}


/* ----------------------------------------------------------------------------------------- */
float Grid_distance_error( Grid* grid, Vector vec, int chain_nr, int atom_nr )
/* ----------------------------------------------------------------------------------------- */
{
    /* chain_nr/atom_nr is the index of the new atom
       used to neglect structually near atoms */

    Location loc, adj;
    Site* site;
    float d, r, r2, r3, r4, cost, max;
    int i;
    Atom_Ref* atom;

    r = grid->params.atom_radius;
    r2 = 2.0 * r;
    r2 = r2 * r2;
    r4 = 4.0 * r;
    r4 = r4 * r4;
    r3 = 3.0 * r;

    loc = Grid_vec_to_loc( grid, vec );
    max = 0.0;
    for ( adj.x = loc.x - 2; adj.x <= loc.x + 2; adj.x++ )
    {
        for ( adj.y = loc.y - 2; adj.y <= loc.y + 2; adj.y++ )
        {
            for ( adj.z = loc.z - 2; adj.z <= loc.z + 2; adj.z++ )
            {
                site = &grid->sites[Grid_loc_to_site( grid, adj )];
                for ( i = 0; i < site->nr_atoms; i++ )
                {
                    atom = &site->atoms[i];
                    if ( ( atom->chain_nr != chain_nr ) || ( abs( atom->atom_nr - atom_nr ) > 3 ) )
                    {
                        d = Vector_periodic_square_dist( vec, atom->vec, grid->params.box_size );
                        if ( ( r2 < d ) && ( d < r4 ) )
                        {
                            d = sqrt( d );
                            cost = r - fabs( r3 - d );    /* cost function */
                            if ( cost > max ) max = cost;
                        }
                        else if ( d < r2 )          /* overlap function */
                        {
                            d = sqrt( d );
                            cost = 2.0 * r - d;
                            if ( cost > max ) max = cost;
                        }
                    }
                }
            }
        }
    }
    return max;
}


/* ----------------------------------------------------------------------------------------- */
int Grid_chain_head_atom( Grid* grid, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    Chain* chain;

    if ( ( chain_nr < 0 ) || ( chain_nr >= grid->max_chains ) ) return NO_ATOM;
    chain = &grid->chains[chain_nr];
    if ( chain->first >= chain->last ) return NO_ATOM;
    return chain->first;
}


/* ----------------------------------------------------------------------------------------- */
int Grid_chain_tail_atom( Grid* grid, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    Chain* chain;

    if ( ( chain_nr < 0 ) || ( chain_nr >= grid->max_chains ) ) return NO_ATOM;
    chain = &grid->chains[chain_nr];
    if ( chain->first >= chain->last ) return NO_ATOM;
    return chain->last - 1;
}


/* ----------------------------------------------------------------------------------------- */
float Alignment_prob( Vector a, Vector b, float exponent )
/* ----------------------------------------------------------------------------------------- */
{
    Vector diff;
    float  len, z;

    if ( exponent <= 0.0 ) return 1.0;
    diff = Vector_diff( b, a );
    len = Vector_length( diff );
    if ( len < EPS ) len = EPS;
    z = fabs( diff.z / len );
    return exp( log( z ) * exponent ); /* z^k */
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_new_vectors( Grid* grid, int chain_nr, char head, int nr_vecs,
                             Vector* vecs, float* probs )
/* ----------------------------------------------------------------------------------------- */
{
    Chain* chain;
    Vector a, b, c;
    int i, j, nr, atom_nr;
    char changed;

    if ( ( chain_nr < 0 ) || ( chain_nr >= grid->max_chains ) ) return false;
    chain = &grid->chains[chain_nr];
    if ( chain->last - chain->first < 2 ) return false;

    if ( head )
    {
        atom_nr = chain->first - 1;
        nr = chain->first;
        c = chain->atoms[nr - chain->offset];
        nr++;
        b = chain->atoms[nr - chain->offset];
        nr++;
        if ( nr >= chain->last ) a = b;
        else a = chain->atoms[nr - chain->offset];
    }
    else
    {
        atom_nr = chain->last;
        nr = chain->last - 1;
        c = chain->atoms[nr - chain->offset];
        nr--;
        b = chain->atoms[nr - chain->offset];
        nr--;
        if ( nr < chain->first ) a = b;
        else a = chain->atoms[nr - chain->offset];
    }

    if ( grid->params.angle_fixed )
    {
        Vector_positions( a, b, c, grid->params.bond_len, grid->params.bond_angle,
                          true, nr_vecs, vecs );
        for ( i = 0; i < nr_vecs; i++ )
            probs[i] = Alignment_prob( c, vecs[i], grid->params.z_exponent );
    }
    else
    {
        Vector_positions_distributed( a, b, c, grid->params.bond_len,
                                      nr_vecs, vecs );
        for ( i = 0; i < nr_vecs; i++ )
        {
            for ( j = 0; j < NR_POSITIONS_ITERS; j++ )
            {
                changed = Grid_reduce_overlap_bonded( grid, c, &vecs[i], chain_nr, atom_nr );
                if ( !changed ) break;
            }
            probs[i] = Vector_angle_probability( Vector_angle( b, c, vecs[i] ) );
            probs[i] = probs[i] * Alignment_prob( c, vecs[i], grid->params.z_exponent );
        }
    }
    return true;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_check_ahead( Grid* grid, int chain_nr, int atom_nr,
                             Vector a, Vector b, Vector c, int depth, int nr_vecs, float limit )
/* ----------------------------------------------------------------------------------------- */
{
    int i, j;
    char changed;
    float ol;
    Vector vecs[MAX_VECS];

    if ( depth <= 0 ) return true;

    if ( grid->params.angle_fixed )
        Vector_positions( a, b, c, grid->params.bond_len, grid->params.bond_angle,
                          true, nr_vecs, vecs );
    else
    {
        nr_vecs = nr_vecs * NR_SAMPLES;
        if ( nr_vecs > MAX_VECS ) nr_vecs = MAX_VECS;
        Vector_positions_sampled( a, b, c, grid->params.bond_len, NR_SAMPLES + 1, nr_vecs, vecs );
    }

    for ( i = 0; i < nr_vecs; i++ )
    {
        ol = Grid_overlap( grid, vecs[i], chain_nr, atom_nr );
        if ( ol <= limit )
        {
            if ( Grid_chain_check_ahead( grid, chain_nr, atom_nr, b, c, vecs[i], depth - 1, nr_vecs, limit ) )
                return true;
        }
    }
    return false;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_head_check( Grid* grid, int chain_nr, Vector vec, int depth, int nr_vecs, float limit )
/* ----------------------------------------------------------------------------------------- */
{
    Chain* chain;
    Vector b, c;
    int nr;

    if ( ( chain_nr < 0 ) || ( chain_nr >= grid->max_chains ) ) return false;
    chain = &grid->chains[chain_nr];
    if ( chain->last - chain->first < 2 ) return false;

    nr = chain->first;
    c = chain->atoms[nr - chain->offset];
    b = chain->atoms[nr + 1 - chain->offset];

    if ( Grid_overlap( grid, vec, chain_nr, nr - 1 ) > limit ) return false;

    return Grid_chain_check_ahead( grid, chain_nr, nr - 2, b, c, vec, depth, nr_vecs, limit );
}


/* ----------------------------------------------------------------------------------------- */
char Grid_chain_tail_check( Grid* grid, int chain_nr, Vector vec, int depth, int nr_vecs, float limit )
/* ----------------------------------------------------------------------------------------- */
{
    Chain* chain;
    Vector b, c;
    int nr;

    if ( ( chain_nr < 0 ) || ( chain_nr >= grid->max_chains ) ) return false;
    chain = &grid->chains[chain_nr];
    if ( chain->last - chain->first < 2 ) return false;

    nr = chain->last - 1;
    c = chain->atoms[nr - chain->offset];
    b = chain->atoms[nr - 1 - chain->offset];

    if ( Grid_overlap( grid, vec, chain_nr, nr + 1 ) > limit ) return false;

    return Grid_chain_check_ahead( grid, chain_nr, nr + 2, b, c, vec, depth, nr_vecs, limit );
}


/* ----------------------------------------------------------------------------------------- */
char Grid_site_is_empty( Grid* grid, int site_nr, char soft )
/* ----------------------------------------------------------------------------------------- */
{
    if ( ( site_nr < 0 ) || ( site_nr >= grid->nr_sites ) ) return false;

    if ( soft )
        return ( grid->sites[site_nr].nr_occ_neighbours == 0 );
    else
        return ( grid->sites[site_nr].nr_atoms == 0 );
}


/* ----------------------------------------------------------------------------------------- */
int Grid_any_empty_site( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int site_nr, empty_nr, nr;
    Location loc;

    empty_nr = NO_SITE;
    nr = 0;
    site_nr = grid->first_empty;
    if ( site_nr == NIL ) return NO_SITE;

    loc = Grid_site_to_loc( grid, site_nr );
    if ( !grid->params.brush || ( loc.z == 0 ) )
    {
        nr = 1;
        empty_nr = site_nr;
    }
    site_nr = grid->sites[site_nr].empty.next;
    while ( site_nr != NIL )
    {
        loc = Grid_site_to_loc( grid, site_nr );
        if ( !grid->params.brush || ( loc.z == 0 ) )
        {
            nr++;
            if ( randomFloat() < ( 1.0 / nr ) ) empty_nr = site_nr;
        }
        site_nr = grid->sites[site_nr].empty.next;
    }
    return empty_nr;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_queue_clear( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    grid->queue_first = 0;
    grid->queue_last = 0;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_queue_add( Grid* grid, int site_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int next;

    next = grid->queue_last + 1;
    if ( next >= grid->nr_sites ) next = 0;
    if ( next == grid->queue_first ) return false;
    grid->queue[grid->queue_last] = site_nr;
    grid->queue_last = next;
    return true;
}


/* ----------------------------------------------------------------------------------------- */
int Grid_queue_get( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int site_nr, first;

    first = grid->queue_first;
    if ( first == grid->queue_last ) return NO_SITE;
    site_nr = grid->queue[first];
    first++;
    if ( first >= grid->nr_sites ) first = 0;
    grid->queue_first = first;
    return site_nr;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_unmark_sites( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int i;

    for ( i = 0; i < grid->nr_marked; i++ )
        grid->sites[grid->marked_list[i]].marked = false;
    grid->nr_marked = 0;
}


/* ----------------------------------------------------------------------------------------- */
int Grid_mark_empty_sites( Grid* grid, int site_nr, int max_sites, char soft )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr, next, dir;

    Grid_unmark_sites( grid );
    if ( !Grid_site_is_empty( grid, site_nr, soft ) )
        return 0;

    Grid_queue_clear( grid );
    nr = 0;
    while ( ( site_nr != NO_SITE ) && ( nr < max_sites ) )
    {
        grid->sites[site_nr].marked = true;
        grid->marked_list[nr] = site_nr;
        nr++;
        for ( dir = 0; dir < NUM_DIRS; dir++ )
        {
            next = Grid_site_step( grid, site_nr, dir );
            if ( !grid->sites[next].marked &&
                 Grid_site_is_empty( grid, next, soft ) )
                Grid_queue_add( grid, next );
        }
        site_nr = Grid_queue_get( grid );
    }
    grid->nr_marked = nr;
    return nr;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_available_sites( Grid* grid, int site_nr, int nr_sites, char soft )
/* ----------------------------------------------------------------------------------------- */
{
    int nr;

    nr = Grid_mark_empty_sites( grid, site_nr, nr_sites, soft );
    Grid_unmark_sites( grid );

    return nr >= nr_sites;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_sites_per_dir( Grid* grid, int site_nr, int max_sites, char soft, int* nr_sites )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr, next, next2, dir, dir2, d, nr_atoms;

    nr_atoms = grid->sites[site_nr].nr_atoms;  /* occupy temporary */
    grid->sites[site_nr].nr_atoms = 1;

    for ( dir = 0; dir < NUM_DIRS; dir++ )
        nr_sites[dir] = -1;

    for ( dir = 0; dir < NUM_DIRS; dir++ )
    {
        if ( nr_sites[dir] < 0 )
        {
            next = Grid_site_step( grid, site_nr, dir );
            nr = Grid_mark_empty_sites( grid, next, max_sites, soft );
            nr_sites[dir] = nr;
            for ( dir2 = 0; dir2 < NUM_DIRS; dir2++ )
            {
                next2 = Grid_site_step( grid, site_nr, dir2 );
                if ( grid->sites[next2].marked )
                    nr_sites[dir2] = nr;
            }
            Grid_unmark_sites( grid );
        }
    }
    grid->sites[site_nr].nr_atoms = nr_atoms;
}


/* ----------------------------------------------------------------------------------------- */
int Grid_min_sites_per_dir( Grid* grid, int site_nr, int max_sites, char soft )
/* ----------------------------------------------------------------------------------------- */
{
    int nr_sites[NUM_DIRS];
    int dir, nr, min;

    Grid_sites_per_dir( grid, site_nr, max_sites, soft, nr_sites );
    min = -1;
    for ( dir = 0; dir < NUM_DIRS; dir++ )
    {
        nr = nr_sites[dir];
        if ( nr > 0 )
        {
            if ( ( min < 0 ) || ( nr < min ) ) min = nr;
        }
    }
    if ( min < 0 ) min = 0;
    return min;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_site_splits( Grid* grid, int site_nr, char soft )
/* ----------------------------------------------------------------------------------------- */
{
#define NUM_DIR_SITES 6
    static char site_empty[27];
    static char site_marked[27];
    static int  dir_sites[NUM_DIR_SITES] = {4, 10, 12, 14, 16, 22};
    static char non_corner[27] =
    {0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0};

    static int  site_adj[27][4] =
    {
        { 1, 3, 9, -1}, { 0, 2, 4, 10}, { 1, 5, 11, -1},
        { 0, 4, 6, 12}, { 1, 3, 5, 7}, { 2, 4, 8, 14},
        { 3, 7, 15, -1}, { 4, 6, 8, 16}, { 5, 7, 17, -1},
        { 0, 10, 12, 18}, { 1, 9, 11, 19}, { 2, 10, 14, 20},
        { 3, 9, 15, 21}, { -1, -1, -1, -1}, { 5, 11, 17, 23},
        { 6, 12, 16, 24}, { 7, 15, 17, 25}, { 8, 14, 16, 26},
        { 9, 19, 21, -1}, {10, 18, 20, 22}, {11, 19, 23, -1},
        {12, 18, 22, 24}, {19, 21, 23, 25}, {14, 20, 22, 26},
        {15, 21, 25, -1}, {16, 22, 24, 26}, {17, 23, 25, -1}
    };

    int nr, dir, check, next, cnt;
    char splits;
    Location loc, test_loc;

    loc = Grid_site_to_loc( grid, site_nr );
    nr = 0;
    cnt = 0;
    for ( test_loc.x = loc.x - 1; test_loc.x <= loc.x + 1; test_loc.x++ )
    {
        for ( test_loc.y = loc.y - 1; test_loc.y <= loc.y + 1; test_loc.y++ )
        {
            for ( test_loc.z = loc.z - 1; test_loc.z <= loc.z + 1; test_loc.z++ )
            {
                next = Grid_loc_to_site( grid, test_loc );
                site_empty[nr] = Grid_site_is_empty( grid, next, soft );
                site_marked[nr] = false;
                if ( non_corner[nr] && site_empty[nr] ) cnt++;
                nr++;
            }
        }
    }
    if ( cnt > 14 ) return false;

    dir = 0;
    while ( dir < NUM_DIR_SITES )
    {
        check = dir_sites[dir];
        if ( site_empty[check] ) break;
        dir++;
    }
    if ( dir >= NUM_DIR_SITES ) return false;

    Grid_queue_clear( grid );
    while ( check != NO_SITE )
    {
        if ( !site_marked[check] )
        {
            site_marked[check] = true;
            for ( dir = 0; dir < 4; dir++ )
            {
                next = site_adj[check][dir];
                if ( next >= 0 )
                {
                    if ( site_empty[next] && !site_marked[next] )
                        Grid_queue_add( grid, next );
                }
            }
        }
        check = Grid_queue_get( grid );
    }
    splits = false;
    dir = 0;
    while ( dir < NUM_DIR_SITES )
    {
        check = dir_sites[dir];
        if ( site_empty[check] && !site_marked[check] )
        {
            splits = true;
            break;
        }
        dir++;
    }
    return splits;
}


/* ----------------------------------------------------------------------------------------- */
char Grid_site_splits_alldirs( Grid* grid, int site_nr, char soft )
/* ----------------------------------------------------------------------------------------- */
{
    static char site_empty[27];
    static char site_marked[27];
    static char non_corner[27] = {0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0};

    static int  site_adj[27][8] =
    {
        { 9, 10, 1, 4, 3, 12, -1, -1}, { 9, 10, 11, 2, 5, 4, 3, 0}, { 1, 10, 11, 14, 5, 4, -1, -1},
        { 0, 1, 4, 7, 6, 15, 12, 9}, { 0, 1, 2, 5, 8, 7, 6, 3}, { 1, 2, 11, 14, 17, 8, 7, 4},
        { 3, 4, 7, 16, 15, 12, -1, -1}, { 3, 4, 5, 8, 17, 16, 15, 6}, { 4, 5, 14, 17, 16, 7, -1, -1},
        { 0, 3, 12, 21, 18, 19, 10, 1}, { 0, 9, 18, 19, 20, 11, 2, 1}, { 1, 10, 19, 20, 23, 14, 5, 2},
        { 0, 3, 6, 15, 24, 21, 18, 9}, { -1, -1, -1, -1, -1, -1, -1, -1}, { 2, 5, 8, 17, 26, 23, 20, 11},
        { 3, 6, 7, 16, 25, 24, 21, 12}, { 6, 7, 8, 17, 26, 25, 24, 15}, { 5, 14, 23, 26, 25, 16, 7, 8},
        { 9, 12, 21, 22, 19, 10, -1, -1}, { 9, 18, 21, 22, 23, 20, 11, 10}, {10, 19, 22, 23, 14, 11, -1, -1},
        {12, 15, 24, 25, 22, 19, 18, 9}, {18, 21, 24, 25, 26, 23, 20, 19}, {19, 22, 25, 26, 17, 14, 11, 20},
        {12, 15, 16, 25, 22, 21, -1, -1}, {15, 16, 17, 26, 23, 22, 21, 24}, {16, 17, 14, 23, 22, 25, -1, -1}
    };


    int nr, dir, check, next, cnt, non_corners;
    char splits;
    Location loc, test_loc;

    loc = Grid_site_to_loc( grid, site_nr );
    nr = 0;
    cnt = 0;
    non_corners = 0;
    check = NO_SITE;
    for ( test_loc.x = loc.x - 1; test_loc.x <= loc.x + 1; test_loc.x++ )
    {
        for ( test_loc.y = loc.y - 1; test_loc.y <= loc.y + 1; test_loc.y++ )
        {
            for ( test_loc.z = loc.z - 1; test_loc.z <= loc.z + 1; test_loc.z++ )
            {
                next = Grid_loc_to_site( grid, test_loc );
                site_empty[nr] = Grid_site_is_empty( grid, next, soft );
                site_marked[nr] = false;
                if ( site_empty[nr] )
                {
                    if ( nr != 13 )
                    {
                        cnt++;
                        check = nr;
                    }
                    if ( non_corner[nr] ) non_corners++;
                }
                nr++;
            }
        }
    }
    if ( cnt > 20 ) return false;
    if ( non_corners > 14 ) return false;
    if ( check == NO_SITE ) return false;

    nr = 0;
    Grid_queue_clear( grid );
    while ( check != NO_SITE )
    {
        if ( !site_marked[check] )
        {
            site_marked[check] = true;
            nr++;
            for ( dir = 0; dir < 8; dir++ )
            {
                next = site_adj[check][dir];
                if ( next >= 0 )
                {
                    if ( site_empty[next] && !site_marked[next] )
                        Grid_queue_add( grid, next );
                }
            }
        }
        check = Grid_queue_get( grid );
    }
    splits = nr < cnt;

    return splits;
}




/* ----------------------------------------------------------------------------------------- */
void Grid_copy( Grid* source, Grid* dest, char first )
/* ----------------------------------------------------------------------------------------- */
{
    int i, j;

    dest->Lx = source->Lx;
    dest->Ly = source->Ly;
    dest->Lz = source->Lz;
    dest->site_width = source->site_width;
    dest->params = source->params;
    if ( first )
        dest->sites = ( Site* )malloc( source->nr_sites * sizeof( Site ) );
    else
    {
        if ( dest->nr_sites != source->nr_sites )
            dest->sites = ( Site* )realloc( dest->sites, source->nr_sites * sizeof( Site ) );
    }
    dest->nr_sites = source->nr_sites;
    for ( i = 0; i < dest->nr_sites; i++ )
        dest->sites[i] = source->sites[i];
    dest->first_empty = source->first_empty;

    if ( first )
        dest->chains = ( Chain* )malloc( source->max_chains * sizeof( Chain ) );
    else
    {
        if ( dest->max_chains != source->max_chains )
            dest->chains = ( Chain* )realloc( dest->chains, source->max_chains * sizeof( Chain ) );
    }
    dest->max_chains = source->max_chains;
    for ( i = 0; i < dest->max_chains; i++ )
    {
        dest->chains[i].first     = source->chains[i].first;
        dest->chains[i].last      = source->chains[i].last;
        dest->chains[i].offset    = source->chains[i].offset;
        if ( first )
            dest->chains[i].atoms = ( Vector* )malloc( source->chains[i].max_atoms * sizeof( Vector ) );
        else
        {
            if ( dest->chains[i].max_atoms != source->chains[i].max_atoms )
                dest->chains[i].atoms = ( Vector* )realloc( dest->chains[i].atoms, source->chains[i].max_atoms * sizeof( Vector ) );
        }
        dest->chains[i].max_atoms = source->chains[i].max_atoms;
        for ( j = 0; j < dest->chains[i].max_atoms; j++ )
            dest->chains[i].atoms[j] = source->chains[i].atoms[j];
    }
    dest->queue_first = source->queue_first;
    dest->queue_last = source->queue_last;
    if ( first )
        dest->queue = ( int* ) malloc( source->nr_sites * sizeof( int ) );
    for ( i = 0; i < dest->nr_sites; i++ )
        dest->queue[i] = source->queue[i];

    dest->nr_marked = source->nr_marked;
    if ( first )
        dest->marked_list = ( int* ) malloc( source->nr_sites * sizeof( int ) );
    for ( i = 0; i < dest->nr_sites; i++ )
        dest->marked_list[i] = source->marked_list[i];
}


/* ----------------------------------------------------------------------------------------- */
char Grid_compare( Grid* source, Grid* dest )
/* ----------------------------------------------------------------------------------------- */
{
    int i, j, nr, os, od, fs, fd;

    if ( dest->Lx != source->Lx ) return false;
    if ( dest->Ly != source->Ly ) return false;
    if ( dest->Lz != source->Lz ) return false;
    if ( dest->site_width.x != source->site_width.x ) return false;
    if ( dest->site_width.y != source->site_width.y ) return false;
    if ( dest->site_width.z != source->site_width.z ) return false;

    if ( dest->params.bond_len != source->params.bond_len ) return false;
    if ( dest->params.bond_angle != source->params.bond_angle ) return false;
    if ( dest->params.atom_radius != source->params.atom_radius ) return false;
    if ( dest->params.box_size.x == source->params.box_size.x ) return false;
    if ( dest->params.box_size.y == source->params.box_size.y ) return false;
    if ( dest->params.box_size.z == source->params.box_size.z ) return false;

    if ( dest->nr_sites != source->nr_sites ) return false;
    for ( i = 0; i < dest->nr_sites; i++ )
    {
        if ( dest->sites[i].nr_atoms != source->sites[i].nr_atoms ) return false;
        if ( dest->sites[i].nr_occ_neighbours != source->sites[i].nr_occ_neighbours ) return false;
        if ( dest->sites[i].marked != source->sites[i].marked ) return false;
        for ( j = 0; j < dest->sites[i].nr_atoms; j++ )
        {
            if ( dest->sites[i].atoms[j].chain_nr != source->sites[i].atoms[j].chain_nr ) return false;
            if ( dest->sites[i].atoms[j].atom_nr != source->sites[i].atoms[j].atom_nr ) return false;
            if ( dest->sites[i].atoms[j].vec.x != source->sites[i].atoms[j].vec.x ) return false;
            if ( dest->sites[i].atoms[j].vec.y != source->sites[i].atoms[j].vec.y ) return false;
            if ( dest->sites[i].atoms[j].vec.z != source->sites[i].atoms[j].vec.z ) return false;
        }
    }
    /* if (dest->first_empty != source->first_empty) return false; */

    if ( dest->max_chains != source->max_chains ) return false;
    for ( i = 0; i < dest->max_chains; i++ )
    {
        if ( dest->chains[i].first  != source->chains[i].first ) return false;
        if ( dest->chains[i].last   != source->chains[i].last ) return false;

        od = dest->chains[i].offset;
        os = source->chains[i].offset;
        fd = dest->chains[i].first;
        fs = source->chains[i].first;
        nr = dest->chains[i].last - fd;
        for ( j = 0; j < nr; j++ )
        {
            if ( dest->chains[i].atoms[fd + j - od].x != source->chains[i].atoms[fs + j - os].x ) return false;
            if ( dest->chains[i].atoms[fd + j - od].y != source->chains[i].atoms[fs + j - os].y ) return false;
            if ( dest->chains[i].atoms[fd + j - od].z != source->chains[i].atoms[fs + j - os].z ) return false;
        }
    }
    return true;
}


/* ----------------------------------------------------------------------------------------- */
void Grid_test( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    static char site_empty[27];
    static char site_marked[27];
    static char non_corner[27] = {0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0};
    static int  site_adj[27][8] =
    {
        { 9, 10, 1, 4, 3, 12, -1, -1}, { 9, 10, 11, 2, 5, 4, 3, 0}, { 1, 10, 11, 14, 5, 4, -1, -1},
        { 0, 1, 4, 7, 6, 15, 12, 9}, { 0, 1, 2, 5, 8, 7, 6, 3}, { 1, 2, 11, 14, 17, 8, 7, 4},
        { 3, 4, 7, 16, 15, 12, -1, -1}, { 3, 4, 5, 8, 17, 16, 15, 6}, { 4, 5, 14, 17, 16, 7, -1, -1},
        { 0, 3, 12, 21, 18, 19, 10, 1}, { 0, 9, 18, 19, 20, 11, 2, 1}, { 1, 10, 19, 20, 23, 14, 5, 2},
        { 0, 3, 6, 15, 24, 21, 18, 9}, { -1, -1, -1, -1, -1, -1, -1, -1}, { 2, 5, 8, 17, 26, 23, 20, 11},
        { 3, 6, 7, 16, 25, 24, 21, 12}, { 6, 7, 8, 17, 26, 25, 24, 15}, { 5, 14, 23, 26, 25, 16, 7, 8},
        { 9, 12, 21, 22, 19, 10, -1, -1}, { 9, 18, 21, 22, 23, 20, 11, 10}, {10, 19, 22, 23, 14, 11, -1, -1},
        {12, 15, 24, 25, 22, 19, 18, 9}, {18, 21, 24, 25, 26, 23, 20, 19}, {19, 22, 25, 26, 17, 14, 11, 20},
        {12, 15, 16, 25, 22, 21, -1, -1}, {15, 16, 17, 26, 23, 22, 21, 24}, {16, 17, 14, 23, 22, 25, -1, -1}
    };


    int max, test, bit, i, nr, dir, check, next, cnt, nonc;
    char splits;
    Location loc, test_loc;

    max = 0;
    for ( test = 0; test < ( 1 << 27 ); test++ )
    {
        nr = 0;
        cnt = 0;
        bit = 1;
        check = NO_SITE;
        nonc = 0;
        for ( nr = 0; nr < 27; nr++ )
        {
            site_empty[nr] = !( test & bit );
            site_marked[nr] = false;
            if ( site_empty[nr] && ( nr != 13 ) )
            {
                cnt++;
                check = nr;
            }
            if ( non_corner[nr] && site_empty[nr] )
                nonc++;
            bit = bit << 1;
        }
        if ( check == NO_SITE ) splits = false;
        else
        {
            nr = 0;
            Grid_queue_clear( grid );
            while ( check != NO_SITE )
            {
                if ( !site_marked[check] )
                {
                    site_marked[check] = true;
                    nr++;
                    for ( dir = 0; dir < 8; dir++ )
                    {
                        next = site_adj[check][dir];
                        if ( next >= 0 )
                        {
                            if ( site_empty[next] && !site_marked[next] )
                                Grid_queue_add( grid, next );
                        }
                    }
                }
                check = Grid_queue_get( grid );
            }
            splits = nr < cnt;
        }
        if ( splits && ( nonc > max ) )
        {
            max = nonc;
            sprintf( buffer, "new max %i, test = %i\n", max, test );
            appWindow()->appendText( buffer );

        }
    }
    sprintf( buffer, "done\n" );
    appWindow()->appendText( buffer );

    exit( 1 );
}
