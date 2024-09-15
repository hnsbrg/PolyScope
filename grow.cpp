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

//#define VERBOSE

#include <QDebug>

#include <stdlib.h>
#include <string.h>

#include "grow.h"
#include "interface.h"
#include "stdio.h"
#include "mainwindow.h"
#include "random.h"

static int* hist_atoms;
static int* hist_chains;

const int BUFFER_LENGTH = 128;
static char  buffer[BUFFER_LENGTH];

#define MAX_ANGLES 200
#define NR_ITERATIONS 20
#define CUT_LEN 20
#define MAX_CUTS 2
#define ANGLE_HIST_LEN 18

#define FILE_FORMAT_NR 2

static Grow_Parameters g_grow_params;

typedef struct
{
    int   angle_nr;
    float value;
} Angle_val;


/* ----------------------------------------------------------------------------------------- */
char Chain_start( Grid* grid, int chain_len, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int i, site_nr;
    char OK;
    Vector vec, test_vec;

    site_nr = Grid_any_empty_site( grid );
    if ( site_nr == NO_SITE )
    {
        sprintf( buffer, "no empty site left!\n" );
        appWindow()->appendText( buffer );
        return false;
    }

    for ( i = 0; i < 5; i++ )
    {
        test_vec = Grid_site_to_vec( grid, site_nr );
        if ( grid->params.brush ) test_vec.z = 0.0;

        Grid_chain_append_tail( grid, chain_nr, test_vec );
        OK = Grid_relax( grid, test_vec, g_grow_params.max_overlap );
        if ( OK ) return true;
        Grid_chain_remove_tail( grid, chain_nr );
        site_nr = Grid_any_empty_site( grid );
    }
    return false;
}


/* ----------------------------------------------------------------------------------------- */
char Chain_second( Grid* grid, int chain_len, int chain_nr )
/* ----------------------------------------------------------------------------------------- */
{
    int i, nr, last_atom;
    char OK;
    float ol;
    Vector test_vec, last_vec, dir_vec;

    last_atom = Grid_chain_tail_atom( grid, chain_nr );
    if ( last_atom == NIL ) return false;
    last_vec = Grid_atom_to_vec( grid, chain_nr, last_atom );

    for ( i = 0; i < 5; i++ )
    {
        if ( grid->params.brush )
        {
            dir_vec = Random_up_direction( grid->params.bond_len, 0.1 * M_PI );
        }
        else
        {
            if ( grid->params.film )
            {
                dir_vec = ConstrainedRandom_direction( grid->params.bond_len, 0.0, grid->params.box_size.z );
            }
            else
            {
                dir_vec = Random_direction( grid->params.bond_len );
            }
        }


        test_vec = Vector_sum( last_vec, dir_vec );
        Grid_chain_append_tail( grid, chain_nr, test_vec );
        OK = Grid_relax( grid, test_vec, g_grow_params.max_overlap );
        if ( OK ) return true;
        Grid_chain_remove_tail( grid, chain_nr );
    }
    return false;
}


/* ----------------------------------------------------------------------------------------- */
char Chain_grow( Grid* grid, int chain_nr, char head,
                 int chain_len, int remaining, char sparse )
/* ----------------------------------------------------------------------------------------- */
{
    char OK;
    int i, j, k;
    int last_atom, this_atom;
    int look_ahead;
    int ol_chain, ol_atom;
    int take_this, best_i;
    int nr_angles;
    float max_overlap;
    float cost, best_cost;
    float ol, max_ol, best_ol;
    Vector vecs[MAX_ANGLES];
    float  probs[MAX_ANGLES];
    float  prob_sum;
    Angle_val vals[MAX_ANGLES];

    nr_angles   = g_grow_params.nr_angles;
    max_overlap = g_grow_params.max_overlap;

    Grid_chain_new_vectors( grid, chain_nr, head, nr_angles, vecs, probs );
    if ( head )
    {
        last_atom = Grid_chain_head_atom( grid, chain_nr );
        this_atom = last_atom - 1;
    }
    else
    {
        last_atom = Grid_chain_tail_atom( grid, chain_nr );
        this_atom = last_atom + 1;
    }
    look_ahead = g_grow_params.ahead_depth;
    if ( remaining < look_ahead ) look_ahead = remaining;
    if ( sparse ) look_ahead = 0;
    take_this = -1;
    best_cost = -1.0;
    prob_sum = 0.0;

    for ( i = 0; i < nr_angles; i++ )
    {
        if ( head )
            OK = Grid_chain_head_check( grid, chain_nr, vecs[i], look_ahead, nr_angles, max_overlap );
        else
            OK = Grid_chain_tail_check( grid, chain_nr, vecs[i], look_ahead, nr_angles, max_overlap );
        if ( OK )
        {
            /*
                  if (grid->params.angle_fixed) {
                    cost = Grid_distance_error(grid, vecs[i], chain_nr, this_atom);
                    if ((best_cost < 0.0) || (cost < best_cost)) {
                      best_cost = cost;
                      take_this = i;
                    }
                  }
            */
            if ( randomFloat() * ( prob_sum + probs[i] ) >= prob_sum )
            {
                take_this = i;
            }
            prob_sum += probs[i];
        }
        if ( getInterfaceState() != STATE_FAST )
        {
            sprintf( buffer, "%i: OK = %i, cost = %f, take %i\n", i, OK, cost, take_this );
            appWindow()->appendText( buffer );

            Grid_chain_append_atom( grid, chain_nr, vecs[i], head );
            interfaceStateProcess();
            Grid_chain_remove_atom( grid, chain_nr, head );
        }
        if ( getInterfaceState() == STATE_ABORT )
        {
            return false;
        }

    } /* for angles */

    if ( take_this >= 0 )
    {
        Grid_chain_append_atom( grid, chain_nr, vecs[take_this], head );
        return true;
    }

    best_ol = 1.0;
    for ( i = 0; i < nr_angles; i++ )
    {
        OK = false;
        k = max_overlap * 10.0;
        max_ol = 0.1 * k;
        while ( !OK && ( max_ol < 1.0 ) )
        {
            if ( head )
                OK = Grid_chain_head_check( grid, chain_nr, vecs[i], look_ahead, nr_angles, max_ol );
            else
                OK = Grid_chain_tail_check( grid, chain_nr, vecs[i], look_ahead, nr_angles, max_ol );
            if ( !OK ) max_ol += 0.1;
        }
        j = i;
        while ( ( j > 0 ) && ( vals[j - 1].value > max_ol ) ) /* sort */
        {
            vals[j] = vals[j - 1];
            j--;
        }
        vals[j].angle_nr = i;
        vals[j].value = max_ol;
    }
    for ( k = 0; k < nr_angles; k++ )
    {
        i = vals[k].angle_nr;
        Grid_chain_append_atom( grid, chain_nr, vecs[i], head );
        ol = Grid_overlap( grid, vecs[i], chain_nr, this_atom );
        if ( ol <= max_overlap ) return true;
        OK = Grid_relax( grid, vecs[i], max_overlap );
        if ( OK ) return true;

        Grid_chain_remove_atom( grid, chain_nr, head );

        if ( getInterfaceState() != STATE_FAST )
        {
            sprintf( buffer, "make place for %i: ol = %f\n", i, ol );
            appWindow()->appendText( buffer );

            interfaceStateProcess();
        }
        if ( getInterfaceState() == STATE_ABORT )
        {
            return false;
        }
    }

    return false;
}




/* ----------------------------------------------------------------------------------------- */
int Place_chain( Grid* grid, int chain_nr, int chain_len, char sparse )
/* ----------------------------------------------------------------------------------------- */
{
    int  i, len, remaining, nr_cuts;
    char OK;

    nr_cuts = 0;
    len = 0;

    appWindow()->setCurrentChainTargetLength( chain_len );

    OK = Chain_start( grid, chain_len, chain_nr );
    if ( getInterfaceState() != STATE_FAST )
    {
        sprintf( buffer, "%i: starting %i\n", chain_nr, OK );
        appWindow()->appendText( buffer );
    }
    if ( getInterfaceState() == STATE_ABORT )
    {

        return false;
    }

    if ( OK )
    {

        if ( chain_len > 1 )
        {
            len++;
            OK = Chain_second( grid, chain_len, chain_nr );
            if ( getInterfaceState() != STATE_FAST )
            {
                sprintf( buffer, "%i: second %i\n", chain_nr, OK );
                appWindow()->appendText( buffer );
            }
            if ( getInterfaceState() == STATE_ABORT )
            {
                return false;
            }
        }

        if ( OK )
        {
            len++;
            while ( len < chain_len )
            {
                remaining = chain_len - len;

                OK = Chain_grow( grid, chain_nr, false, chain_len, remaining, sparse );
                if ( getInterfaceState() != STATE_FAST )
                {
                    sprintf( buffer, "%i: growing tail %i, len = %i, chain_len %i\n", chain_nr, OK, len, chain_len );
                    appWindow()->appendText( buffer );
                }
                if ( getInterfaceState() == STATE_ABORT )
                {
                    return false;
                }

                if ( !OK )
                {
                    if ( nr_cuts > MAX_CUTS ) break;
                    if ( len <= 2 ) break;
                    for ( i = 0; i < CUT_LEN; i++ )
                    {
                        if ( len > 2 )
                        {
                            Grid_chain_remove_tail( grid, chain_nr );
                            len--;
                        }
                    }
                    nr_cuts++;
#ifdef VERBOSE
                    sprintf( buffer, " cut " );
                    appWindow()->appendText( buffer );
#endif
                }
                else len++;
                appWindow()->updateCurrentChainLength( len );
#ifdef VERBOSE
                sprintf( buffer, "%i ", len );
                appWindow()->appendText( buffer );
#endif
            }
            while ( !grid->params.brush && ( len < chain_len ) )
            {
                remaining = chain_len - len;
                OK = Chain_grow( grid, chain_nr, true, chain_len, remaining, sparse );
                if ( getInterfaceState() != STATE_FAST )
                {
                    sprintf( buffer, "%i: growing head %i, len = %i, chain_len %i\n", chain_nr, OK, len, chain_len );
                    appWindow()->appendText( buffer );
                }
                if ( getInterfaceState() == STATE_ABORT )
                {
                    return false;
                }

                if ( !OK )
                {
                    if ( nr_cuts > MAX_CUTS ) break;
                    if ( len <= CUT_LEN + 2 ) break;
                    for ( i = 0; i < CUT_LEN; i++ )
                    {
                        if ( len > 2 )
                        {
                            Grid_chain_remove_head( grid, chain_nr );
                            len--;
                        }
                    }
                    nr_cuts++;
#ifdef VERBOSE
                    sprintf( buffer, " cut " );
                    appWindow()->appendText( buffer );

                    //              fflush( stdout );
#endif
                }
                else len++;
#ifdef VERBOSE
                sprintf( buffer, "%i ", len );
                appWindow()->appendText( buffer );

                //                fflush( stdout );
#endif
            }
        }
    }
#ifdef VERBOSE
    sprintf( buffer, "\n" );
    appWindow()->appendText( buffer );

#endif
    interfaceStateProcess();
    return len;
}





int Get_chain_len( int chain_nr )
{
    return chainList().chainLength( chain_nr );
}





/* ----------------------------------------------------------------------------------------- */
void Random_pack( Grid* grid )
/* ----------------------------------------------------------------------------------------- */
{
    int chain_nr, chain_len, len, i;
    int nr_chains, nr_tries, particles;
    char OK, sparse;
    Vector first_vec;

    chain_nr = 0;
    particles = 0;
    sparse = true;

    int num_monomers = 0;
    while ( true )
    {
        appWindow()->updateStatus( chain_nr, particles );
#ifdef VERBOSE
        sprintf( buffer, "   chain %3i, %i of %i particles\n",
                 chain_nr, particles, g_grow_params.nr_particles );
        appWindow()->appendText( buffer );
#endif
        /* XWinRefresh(); */
        chain_len = Get_chain_len( chain_nr );

        if ( chain_len < 1 )
        {
            return;
        }

        nr_tries = 0;
        while ( ( g_grow_params.nr_chain_trials == 0 ) ||
                ( nr_tries < g_grow_params.nr_chain_trials ) )
        {
            len = Place_chain( grid, chain_nr, chain_len, sparse );
            if ( len >= chain_len )
            {
                num_monomers += len;
                break;
            }
            Grid_chain_remove( grid, chain_nr );
            len = 0;

            if ( getInterfaceState() == STATE_ABORT )
            {
                return;
            }

#ifdef VERBOSE
            sprintf( buffer, "chain removed\n" );
            appWindow()->appendText( buffer );

            if ( sparse )
            {
                sprintf( buffer, "sparse becomes false\n" );
                appWindow()->appendText( buffer );
            }
#endif
            sparse = false;
            nr_tries++;
        }
        if ( len > 0 )
        {
            hist_chains[len]++;
            hist_atoms[len] += len;
        }
        if ( len < chain_len ) break;
        chain_nr++;
        particles += chain_len;
        if ( particles >= g_grow_params.nr_particles ) break;
    }
    if ( particles < g_grow_params.nr_particles )
    {
        sprintf( buffer, "failed\n" );
        appWindow()->appendText( buffer );
    }

    qDebug() << "monomer count in grow: " << num_monomers;
}


/* ----------------------------------------------------------------------------------------- */
void Pack( Grid* grid, Grow_Parameters* grow_params )
/* ----------------------------------------------------------------------------------------- */
{
    int i, j, len, total;

    hist_atoms = ( int* ) malloc( ( grid->nr_sites + 1 ) * sizeof( int ) );
    hist_chains = ( int* ) malloc( ( grid->nr_sites + 1 ) * sizeof( int ) );

    g_grow_params = *grow_params;
    if ( g_grow_params.nr_angles >= MAX_ANGLES )
        g_grow_params.nr_angles = MAX_ANGLES - 1;

    for ( i = 0; i <= grid->nr_sites; i++ )
    {
        hist_atoms[i] = 0;
        hist_chains[i] = 0;
    }

    start_clock();
    for ( i = 0; i < g_grow_params.nr_packings; i++ )
    {
#ifdef VERBOSE
        sprintf( buffer, "===== packing %i =======\n", i );
        appWindow()->appendText( buffer );
#endif
        Grid_clear( grid );
        Random_pack( grid );
        if ( getInterfaceState() == STATE_ABORT )
        {
            setInterfaceState( STATE_NOT_STARTED );
            return ;
        }
    }
    // Save_System( "out.pack", grid, &g_grow_params );
#ifdef VERBOSE

    sprintf( buffer, "saved out.pack\n\n" );
    appWindow()->appendText( buffer );
    sprintf( buffer, "maximum overlap %f\n\n", Grid_max_overlap( grid ) );
    appWindow()->appendText( buffer );

    sprintf( buffer, "time used: %i s\n", get_clock() );
    appWindow()->appendText( buffer );

    total = 0;
    for ( i = 0; i <= grid->nr_sites; i++ )
    {
        if ( hist_chains[i] > 0 )
        {
            sprintf( buffer, "%4i : %4i %4i\n", i, hist_chains[i], hist_atoms[i] );
            appWindow()->appendText( buffer );
        }
        total += hist_atoms[i];
    }
    sprintf( buffer, "total atoms %i\n", total );
    appWindow()->appendText( buffer );

#endif
    free( ( void* )hist_atoms );
    free( ( void* )hist_chains );
}

#define MAX_LINE_LEN 64000

/* ----------------------------------------------------------------------------------------- */
int Load_System( char filename[], Grid* grid, Grow_Parameters* grow_params, QByteArray& monomerJsonList, QByteArray& sequenceJsonList, QByteArray& additiveJsonList )
/* ----------------------------------------------------------------------------------------- */
{
    FILE* f = fopen( filename, "r" );

    if ( f == NULL )
    {
        return false;
    }

    char  line[MAX_LINE_LEN];
    char  prefix[MAX_LINE_LEN];

    // read first three lines and discard
    fgets( line, MAX_LINE_LEN, f );
    fgets( line, MAX_LINE_LEN, f );
    fgets( line, MAX_LINE_LEN, f );

    int tmp;
    int rc;

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, & grid->params.box_size.x );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, & grid->params.box_size.y );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, & grid->params.box_size.z );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grid->params.atom_radius );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grid->params.bond_len );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &tmp );
    grid->params.brush = ( 0 != tmp );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &tmp );
    grid->params.angle_fixed = ( 0 != tmp );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grid->params.bond_angle );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grid->params.kappa );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grid->params.z_exponent );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &tmp );
    grid->params.film = ( 0 != tmp );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &tmp );
    grid->params.point_cloud = ( 0 != tmp );

    // skip two lines
    fgets( line, MAX_LINE_LEN, f );
    fgets( line, MAX_LINE_LEN, f );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &grow_params->nr_particles );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &grow_params->chain_len );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grow_params->max_overlap );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &grow_params->nr_angles );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &grow_params->ahead_depth );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &grow_params->nr_chain_trials );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %f", prefix, &grow_params->dispersity );

    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &grow_params->seed );

    int nr_chains = 0;
    fgets( line, MAX_LINE_LEN, f );
    rc = sscanf( line, "%s %*s %i", prefix, &nr_chains );

    // skip two lines
    fgets( line, MAX_LINE_LEN, f );
    fgets( line, MAX_LINE_LEN, f );

    fgets( line, MAX_LINE_LEN, f );
    monomerJsonList.append( line );
    monomerJsonList.remove( 0, 20 );

    fgets( line, MAX_LINE_LEN, f );
    sequenceJsonList.append( line );
    sequenceJsonList.remove( 0, 20 );

    fgets( line, MAX_LINE_LEN, f );
    additiveJsonList.append( line );
    additiveJsonList.remove( 0, 20 );

    // end of header - read empty line
    fgets( line, MAX_LINE_LEN, f );

    Grid_init( grid, &grid->params );

    int monomer_count = 0;
    for ( int chain_nr = 0; chain_nr < nr_chains; chain_nr++ )
    {
        fgets( line, MAX_LINE_LEN, f );

        int chain_len;

        sscanf( line, "%i", &chain_len );
        for ( int i = 0; i < chain_len; i++ )
        {
            Vector vec;

            fgets( line, MAX_LINE_LEN, f );
            sscanf( line, "%f %f %f %i", &vec.x, &vec.y, &vec.z, &vec.monomer_type );
            Grid_chain_append_tail( grid, chain_nr, vec );
            monomer_count++;
        }
    }

    qDebug() << "monomer count upon load: " << monomer_count;

    return nr_chains;
}



/* ----------------------------------------------------------------------------------------- */
char Save_System( char filename[], Grid* grid, Grow_Parameters* grow_params, const char* monomerJsonList, const char* sequenceJsonList, const char* additiveJsonList )
/* ----------------------------------------------------------------------------------------- */
{
    FILE*  f;
    int   nr_chains, chain_nr, nr_angles, max_len;
    int   nr, len, i, ia, k, i1, i2, angle1, angle2;
    Chain* chain;
    float d, d1, d2, a, a1, a2, prob_sum;
    int   ang_hist[ANGLE_HIST_LEN + 1];
    int*   len_hist;
    Vector u;
    float urow[3];
    float A[3][3];
    int Anum;

    f = fopen( filename, "w" );
    if ( f == NULL ) return false;

    fprintf( f, appWindow()->versionText() );
    fprintf( f, "\n\n" );
    fprintf( f, "Grid parameters:\n" );
    fprintf( f, "  box_size.x  = %f\n", grid->params.box_size.x );
    fprintf( f, "  box_size.y  = %f\n", grid->params.box_size.y );
    fprintf( f, "  box_size.z  = %f\n", grid->params.box_size.z );
    fprintf( f, "  atom_radius = %f\n", grid->params.atom_radius );
    fprintf( f, "  bond_len    = %f\n", grid->params.bond_len );
    fprintf( f, "  brush       = %s\n", grid->params.brush ? "1" : "0" );
    fprintf( f, "  angle_fixed = %s\n", grid->params.angle_fixed ? "1" : "0" );
    fprintf( f, "  bond_angle  = %f\n", grid->params.bond_angle );
    fprintf( f, "  kappa       = %f\n", grid->params.kappa );
    fprintf( f, "  z-exponent  = %f\n", grid->params.z_exponent );
    fprintf( f, "  film        = %s\n", grid->params.film ? "1" : "0" );
    fprintf( f, "  point_cloud = %s\n", grid->params.point_cloud ? "1" : "0" );

    fprintf( f, "\n" );
    fprintf( f, "Grow parameters:\n" );
    fprintf( f, "  nr_particles    = %i\n", grow_params->nr_particles );
    fprintf( f, "  chain_len       = %i\n", grow_params->chain_len );
    fprintf( f, "  max_overlap     = %f\n", grow_params->max_overlap );
    fprintf( f, "  nr_angles       = %i\n", grow_params->nr_angles );
    fprintf( f, "  search_depth    = %i\n", grow_params->ahead_depth );
    fprintf( f, "  nr_chain_trials = %i\n", grow_params->nr_chain_trials );
    fprintf( f, "  dispersity      = %f\n", grow_params->dispersity );
    fprintf( f, "  seed            = %i\n", grow_params->seed );

    nr_chains = 0;
    max_len = 0;
    for ( chain_nr = 0; chain_nr < grid->max_chains; chain_nr++ )
    {
        chain = &grid->chains[chain_nr];
        len = chain->last - chain->first;
        if ( len > 0 ) nr_chains++;
        if ( len > max_len ) max_len = len;
    }
    max_len++;
    len_hist = ( int* ) malloc( max_len * sizeof( int ) );
    for ( i = 0; i < max_len; i++ ) len_hist[i] = 0;
    for ( i1 = 0; i1 < 3; i1++ )
        for ( i2 = 0; i2 < 3; i2++ )
            A[i1][i2] = 0.0;
    Anum = 0;

    fprintf( f, "  nr_chains       = %i\n", nr_chains );

    fprintf( f, "\n" );
    fprintf( f, "Monomer parameters:\n" );
    fprintf( f, "  monomer_list    = %s\n", monomerJsonList );
    fprintf( f, "  sequence_list   = %s\n", sequenceJsonList );
    fprintf( f, "  additive_list   = %s\n", additiveJsonList );

    fprintf( f, "\n" );

    a1 = 1000.0;
    a2 = 0.0;
    d1 = 1000.0;
    d2 = 0.0;
    for ( i = 0; i <= ANGLE_HIST_LEN; i++ ) ang_hist[i] = 0;
    nr_angles = 0;

    int monomer_count = 0;

    for ( chain_nr = 0; chain_nr < grid->max_chains; chain_nr++ )
    {
        chain = &grid->chains[chain_nr];
        len = chain->last - chain->first;
        if ( len <= 0 ) continue;
        len_hist[len]++;
        fprintf( f, "%i\n", len );
        for ( i = chain->first; i < chain->last; i++ )
        {
            k = i - chain->offset;
            fprintf( f, "  %9.4f %9.4f %9.4f %i\n", chain->atoms[k].x, chain->atoms[k].y, chain->atoms[k].z, chain->atoms[k].monomer_type );
            monomer_count++;
            if ( i > chain->first )
            {
                u = Vector_unit_diff( chain->atoms[k], chain->atoms[k - 1] );
                urow[0] = u.x;
                urow[1] = u.y;
                urow[2] = u.z;
                for ( i1 = 0; i1 < 3; i1++ )
                    for ( i2 = 0; i2 < 3; i2++ )
                        A[i1][i2] += urow[i1] * urow[i2];
                A[0][0] -= 1.0 / 3;
                A[1][1] -= 1.0 / 3;
                A[2][2] -= 1.0 / 3;
                Anum++;

                d = Vector_dist( chain->atoms[k], chain->atoms[k - 1] );
                if ( d < d1 ) d1 = d;
                if ( d > d2 ) d2 = d;
            }
            if ( i > chain->first + 1 )
            {
                a = Vector_angle( chain->atoms[k], chain->atoms[k - 1], chain->atoms[k - 2] );
                if ( a < a1 ) a1 = a;
                if ( a > a2 ) a2 = a;
                ia = a / M_PI * ANGLE_HIST_LEN;
                if ( ia < 0 ) ia = 0;
                if ( ia > ANGLE_HIST_LEN ) ia = ANGLE_HIST_LEN;
                ang_hist[ia]++;
                nr_angles++;
            }
        }
    }
    fprintf( f, "/* bond_len   from %f to %f */\n", d1, d2 );
    fprintf( f, "/* bond_angle from %f to %f */\n", a1, a2 );

    fprintf( f, "/* chain_len distribution : */\n" );
    for ( i = 1; i < max_len; i++ )
        fprintf( f, "/*   %5i : %i */\n", i, len_hist[i] );
    fprintf( f, "\n" );

    prob_sum = 0.0;
    for ( i = 0; i <= ANGLE_HIST_LEN; i++ )
        prob_sum += Vector_angle_probability( ( i + 0.5 ) * M_PI / ANGLE_HIST_LEN );

    fprintf( f, "/* bond_angle distribution : */\n" );
    for ( i = 0; i <= ANGLE_HIST_LEN; i++ )
    {
        nr = Vector_angle_probability( ( i + 0.5 ) * M_PI / ANGLE_HIST_LEN ) / prob_sum * nr_angles + 0.5;
        angle1 = 1.0 * i * 180 / ANGLE_HIST_LEN;
        angle2 = 1.0 * ( i + 1 ) * 180 / ANGLE_HIST_LEN;

        fprintf( f, "/*   %3i..%3i : %6i  (%6i) */\n", angle1, angle2, ang_hist[i], nr );
    }
    fprintf( f, "\n" );

    fprintf( f, "/* maximum overlap %f */\n", Grid_max_overlap( grid, grow_params ) );
    fprintf( f, "\n" );
    fprintf( f, "/* Orientation matrix */\n" );
    if ( Anum <= 0 ) Anum = 1;
    for ( i1 = 0; i1 < 3; i1++ )
    {
        fprintf( f, "/*   " );
        for ( i2 = 0; i2 < 3; i2++ )
            fprintf( f, "%8.3f  ", A[i1][i2] / Anum );
        fprintf( f, " */\n" );
    }

    fclose( f );
    free( ( void* )len_hist );

    qDebug() << "monomer count upon save: " << monomer_count;
    return true;
}

