/*******************************************/
/*  VECTOR TOOLS                           */
/*                                         */
/*  MM  Mar. 6. 98                         */
/*                                         */
/* Matthias H. Müller. Diss. Techn. Wiss.  */
/* ETH Zürich, Nr. 13096, 1999.            */
/*******************************************/

/*******************************************/
/*                                         */
/* modified by WD Hinsberg 12.2.2023       */
/*                                         */
/*******************************************/

#include <QtMath>
#include "vector.h"
#include "random.h"

#include <stdlib.h>
#include <time.h>


typedef struct
{
    float cosfxy, sinfxy;
    float cosfxz, sinfxz;
    float cosfyz, sinfyz;
} Rotation;





/* ============ Tools ======================================== */

static clock_t  g_clock;

/* ----------------------------------------------------------------------------------------- */
void start_clock()
/* ----------------------------------------------------------------------------------------- */
{
    g_clock = clock();
}


/* ----------------------------------------------------------------------------------------- */
int get_clock()
/* ----------------------------------------------------------------------------------------- */
{
    return ( clock() - g_clock ) / CLOCKS_PER_SEC;
}




/* ============ Vector operations ======================================== */


/* ------------------------------------------------------------------- */
Vector Vector_null()
/* ------------------------------------------------------------------- */
{
    Vector v;
    v.x = 0.0;
    v.y = 0.0;
    v.z = 0.0;
    return v;
}


/* ------------------------------------------------------------------- */
Vector Vector_sum( Vector a, Vector b )
/* ------------------------------------------------------------------- */
{
    Vector v;
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    v.z = a.z + b.z;
    return v;
}


/* ------------------------------------------------------------------- */
Vector Vector_diff( Vector a, Vector b )
/* ------------------------------------------------------------------- */
{
    Vector v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    v.z = a.z - b.z;
    return v;
}


/* ------------------------------------------------------------------- */
Vector Vector_unit_diff( Vector a, Vector b )
/* ------------------------------------------------------------------- */
{
    Vector u;
    float  l;

    u = Vector_diff( a, b );
    l = Vector_length( u );
    if ( l < EPS ) l = EPS;
    return Vector_stretch( u, 1.0 / l );
}


/* ------------------------------------------------------------------- */
float Vector_length( Vector a )
/* ------------------------------------------------------------------- */
{
    return sqrt( a.x * a.x + a.y * a.y + a.z * a.z );
}


/* ------------------------------------------------------------------- */
float Vector_scalar_prod( Vector a, Vector b )
/* ------------------------------------------------------------------- */
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


/* ------------------------------------------------------------------- */
Vector Vector_prod( Vector a, Vector b )
/* ------------------------------------------------------------------- */
{
    Vector vec;

    vec.x = a.y * b.z - b.y * a.z;
    vec.y = a.z * b.x - b.z * a.x;
    vec.z = a.x * b.y - b.x * a.y;
    return vec;
}


/* ------------------------------------------------------------------- */
float Vector_angle( Vector a, Vector b, Vector c )
/* ------------------------------------------------------------------- */
{
    float ca, l1, l2;
    Vector d1, d2;

    d1 = Vector_diff( b, a );
    l1 = Vector_length( d1 );
    d2 = Vector_diff( c, b );
    l2 = Vector_length( d2 );
    ca = Vector_scalar_prod( d1, d2 ) / l1 / l2;
    if ( ca > 1.0 ) ca = 1.0;
    if ( ca < -1.0 ) ca = -1.0;
    return acos( ca );
}


/* ------------------------------------------------------------------- */
float Vector_torsion( Vector a, Vector b, Vector c, Vector d )
/* ------------------------------------------------------------------- */
{
    Vector n1, n2;
    float ca, l1, l2;

    n1 = Vector_prod( Vector_diff( b, a ), Vector_diff( c, b ) );
    n2 = Vector_prod( Vector_diff( c, b ), Vector_diff( d, c ) );
    l1 = Vector_length( n1 );
    l2 = Vector_length( n2 );
    ca = Vector_scalar_prod( n1, n2 ) / l1 / l2;
    if ( ca > 1.0 ) ca = 1.0;
    if ( ca < -1.0 ) ca = -1.0;
    return acos( ca );
}


/* ------------------------------------------------------------------- */
Vector Vector_stretch( Vector a, float f )
/* ------------------------------------------------------------------- */
{
    a.x *= f;
    a.y *= f;
    a.z *= f;
    return a;
}


/*--------------------------------------------------------*/
float Vector_dist( Vector a, Vector b )
/*--------------------------------------------------------*/
{
    return ( sqrt( ( a.x - b.x ) * ( a.x - b.x ) + ( a.y - b.y ) * ( a.y - b.y ) + ( a.z - b.z ) * ( a.z - b.z ) ) );
}


/*--------------------------------------------------------*/
float Vector_square_dist( Vector a, Vector b )
/*--------------------------------------------------------*/
{
    return ( ( a.x - b.x ) * ( a.x - b.x ) + ( a.y - b.y ) * ( a.y - b.y ) + ( a.z - b.z ) * ( a.z - b.z ) );
}


/*--------------------------------------------------------*/
float Vector_periodic_square_dist( Vector a, Vector b, Vector period )
/*--------------------------------------------------------*/
{
    float dx, dy, dz;
    Vector period2;
    int i;

    period2 = Vector_stretch( period, 0.5 );

    dx = fabs( a.x - b.x );
    dy = fabs( a.y - b.y );
    dz = fabs( a.z - b.z );
    if ( period.x > 0.0 )
    {
        i = dx / period.x;
        dx = dx - i * period.x;
        if ( dx > period2.x ) dx = period.x - dx;
    }
    if ( period.y > 0.0 )
    {
        i = dy / period.y;
        dy = dy - i * period.y;
        if ( dy > period2.y ) dy = period.y - dy;
    }
    if ( period.z > 0.0 )
    {
        i = dz / period.z;
        dz = dz - i * period.z;
        if ( dz > period2.z ) dz = period.z - dz;
    }
    return dx * dx + dy * dy + dz * dz;
}


/*--------------------------------------------------------*/
Vector Vector_periodic_diff( Vector a, Vector b, Vector period )
/*--------------------------------------------------------*/
{
    Vector period2;
    int i;
    Vector vec;

    period2 = Vector_stretch( period, 0.5 );

    vec.x = a.x - b.x;
    vec.y = a.y - b.y;
    vec.z = a.z - b.z;

    if ( period.x > 0.0 )
    {
        i = vec.x / period.x;
        vec.x -= i * period.x;
        if ( vec.x >  period2.x ) vec.x -= period.x;
        if ( vec.x < -period2.x ) vec.x += period.x;
    }
    if ( period.y > 0.0 )
    {
        i = vec.y / period.y;
        vec.y -= i * period.y;
        if ( vec.y >  period2.y ) vec.y -= period.y;
        if ( vec.y < -period2.y ) vec.y += period.y;
    }
    if ( period.z > 0.0 )
    {
        i = vec.z / period.z;
        vec.z -= i * period.z;
        if ( vec.z >  period2.z ) vec.z -= period.z;
        if ( vec.z < -period2.z ) vec.z += period.z;
    }
    return vec;
}


/*--------------------------------------------------------*/
Vector Vector_periodic_box( Vector p, Vector period )
/*--------------------------------------------------------*/
{
    int i;

    if ( period.x > 0.0 )
    {
        i = p.x / period.x;
        p.x -= period.x * i;
        if ( p.x < 0.0 ) p.x += period.x;
    }
    if ( period.y > 0.0 )
    {
        i = p.y / period.y;
        p.y -= period.y * i;
        if ( p.y < 0.0 ) p.y += period.y;
    }
    if ( period.z > 0.0 )
    {
        i = p.z / period.z;
        p.z -= period.z * i;
        if ( p.z < 0.0 ) p.z += period.z;
    }
    return p;
}


/* ------------------------------------------------------------------- */
void Vector_rotation( Vector a, Vector b, Vector c, Rotation* rot )
/* ------------------------------------------------------------------- */
/* c to origin, b on x axies, a on positive xy plane */
{
    float x, y, z, r;
    float cosfxy, sinfxy, cosfxz, sinfxz, cosfyz, sinfyz;

    b = Vector_diff( b, c );  /* translate c to origin */
    a = Vector_diff( a, c );

    r = sqrt( b.x * b.x + b.y * b.y );    /* xy rotation b */
    if ( r < EPS ) { cosfxy = 1.0; sinfxy = 0.0; }
    else { cosfxy = b.x / r; sinfxy = -b.y / r; }
    b.x  = r;
    x    = cosfxy * a.x - sinfxy * a.y;
    y    = sinfxy * a.x + cosfxy * a.y;
    a.x = x;
    a.y = y;

    r = sqrt( b.x * b.x + b.z * b.z );    /* xz rotation b */
    if ( r < EPS ) { cosfxz = 1.0; sinfxz = 0.0; }
    else { cosfxz = b.x / r; sinfxz = -b.z / r; }
    x    = cosfxz * a.x - sinfxz * a.z;
    z    = sinfxz * a.x + cosfxz * a.z;
    a.x = x;
    a.z = z;

    r =  sqrt( a.y * a.y + a.z * a.z );   /* yz rotation a */
    if ( r < EPS ) { cosfyz = 1.0; sinfyz = 0.0; }
    else { cosfyz = a.y / r; sinfyz = -a.z / r; }

    rot->cosfxy = cosfxy;
    rot->sinfxy = sinfxy;
    rot->cosfxz = cosfxz;
    rot->sinfxz = sinfxz;
    rot->cosfyz = cosfyz;
    rot->sinfyz = sinfyz;
}


/* ------------------------------------------------------------------- */
Vector Vector_rotate_back( Vector vec, Rotation* rot )
/* ------------------------------------------------------------------- */
{
    float x, y, z;

    y =  rot->cosfyz * vec.y + rot->sinfyz * vec.z;
    z = -rot->sinfyz * vec.y + rot->cosfyz * vec.z;
    vec.y = y;
    vec.z = z;
    x =  rot->cosfxz * vec.x + rot->sinfxz * vec.z;
    z = -rot->sinfxz * vec.x + rot->cosfxz * vec.z;
    vec.x = x;
    vec.z = z;
    x =  rot->cosfxy * vec.x + rot->sinfxy * vec.y;
    y = -rot->sinfxy * vec.x + rot->cosfxy * vec.y;
    vec.x = x;
    vec.y = y;
    return vec;
}


/* ------------------------------------------------------------------- */
Vector Vector_rotate_forward( Vector vec, Rotation* rot )
/* ------------------------------------------------------------------- */
{
    float x, y, z;

    x =  rot->cosfxy * vec.x - rot->sinfxy * vec.y;
    y =  rot->sinfxy * vec.x + rot->cosfxy * vec.y;
    vec.x = x;
    vec.y = y;
    x =  rot->cosfxz * vec.x - rot->sinfxz * vec.z;
    z =  rot->sinfxz * vec.x + rot->cosfxz * vec.z;
    vec.x = x;
    vec.z = z;
    y =  rot->cosfyz * vec.y - rot->sinfyz * vec.z;
    z =  rot->sinfyz * vec.y + rot->cosfyz * vec.z;
    vec.y = y;
    vec.z = z;
    return vec;
}


/* ------------------------------------------------------------------- */
void Vector_positions( Vector a, Vector b, Vector c, float length, float angle,
                       char random_start, int nr_vecs, Vector* vecs )
/* ------------------------------------------------------------------- */
{
    Rotation rot;
    Vector vec;
    float r, f, df;
    int i;

    Vector_rotation( a, b, c, &rot );
    vec.x = - cos( angle ) * length;
    r = sin( angle ) * length;
    df = 2.0 * M_PI / nr_vecs;
    if ( random_start )
    {
        i = randomInt() % nr_vecs;
        f = i * df;
    }
    else f = 0.0;
    for ( i = 0; i < nr_vecs; i++ )
    {
        vec.y = r * cos( f );
        vec.z = r * sin( f );
        vecs[i] = Vector_sum( c, Vector_rotate_back( vec, &rot ) );
        f = f + df;
    }
}


/* ------------------------------------------------------------------- */
Vector Random_direction( float length )
/* ------------------------------------------------------------------- */
{
    Vector vec;
    int i;
    float r;

    for ( i = 0; i < 10; i++ )
    {
        vec.x = -1.0 + 2.0 * randomFloat();
        vec.y = -1.0 + 2.0 * randomFloat();
        vec.z = -1.0 + 2.0 * randomFloat();
        r = Vector_length( vec );
        if ( r <= 1.0 ) break;
    }
    if ( r < EPS ) { vec.x = 0.0; vec.y = 0.0; vec.z = length; }
    else vec = Vector_stretch( vec, length / r );
    return vec;
}



/* ------------------------------------------------------------------- */
Vector ConstrainedRandom_direction( float length, float min, float max )
/* ------------------------------------------------------------------- */
{
    Vector vec;
    int i;
    float r;

    char in_bounds = false;

    while ( !in_bounds )
    {
        for ( i = 0; i < 10; i++ )
        {
            vec.x = -1.0 + 2.0 * randomFloat();
            vec.y = -1.0 + 2.0 * randomFloat();
            vec.z = -1.0 + 2.0 * randomFloat();
            r = Vector_length( vec );
            if ( r <= 1.0 ) break;
        }
        if ( r < EPS ) { vec.x = 0.0; vec.y = 0.0; vec.z = length; }
        else vec = Vector_stretch( vec, length / r );

        if ( vec.z >= min && vec.z <= max )
        {
            in_bounds = true;
        }
    }

    return vec;
}



/* ------------------------------------------------------------------- */
Vector Random_up_direction( float length, float max_teta )
/* ------------------------------------------------------------------- */
{
    Vector vec;
    float phi, teta;

    phi  = randomFloat() * 2.0 * M_PI;
    teta = randomFloat() * max_teta;

    vec.x = length * cos( phi ) * sin( teta );
    vec.y = length * sin( phi ) * sin( teta );
    vec.z = length * cos( teta );
    return vec;
}


/* ============ Distribution functions ============================================ */

#define MAX_SAMPLES 100
#define MAX_PROBABILITY 1.0     /* max of probability function */

static float g_kappa     = 4.0;
static float g_max_angle = 1.6;


/* ------------------------------------------------------------------- */
float Vector_angle_probability( float angle )
/* ------------------------------------------------------------------- */
{
    return exp( -g_kappa * ( 1.0 - cos( angle ) ) );
}


float Distr_angle()
{
    int i;
    float angle, prob;

    for ( i = 0; i < MAX_SAMPLES; i++ )
    {
        angle = randomFloat() * g_max_angle;
        prob  = randomFloat() * MAX_PROBABILITY;
        if ( prob <= Vector_angle_probability( angle ) )
            return angle;
    }
    return 0.0;
}


/* ------------------------------------------------------------------- */
void Vector_set_kappa( float kappa )
/* ------------------------------------------------------------------- */
{
    g_kappa     = kappa;
    g_max_angle = 0.0;
    while ( ( g_max_angle < M_PI ) &&
            ( Vector_angle_probability( g_max_angle ) > 0.01 ) )
        g_max_angle += 0.1;
}


/* ------------------------------------------------------------------- */
void Vector_positions_distributed( Vector a, Vector b, Vector c, float length,
                                   int nr_vecs, Vector* vecs )
/* ------------------------------------------------------------------- */
{
    Rotation rot;
    Vector vec;
    float angle, r, f, df;
    int i;

    Vector_rotation( a, b, c, &rot );

    df = 2.0 * M_PI / nr_vecs;
    i = randomInt() % nr_vecs;
    f = i * df;
    for ( i = 0; i < nr_vecs; i++ )
    {
        angle = Distr_angle();
        r = sin( angle ) * length;
        vec.x = - cos( angle ) * length;
        vec.y = r * cos( f );
        vec.z = r * sin( f );
        vecs[i] = Vector_sum( c, Vector_rotate_back( vec, &rot ) );
        f = f + df;
    }
}


/* ------------------------------------------------------------------- */
void Vector_positions_sampled( Vector a, Vector b, Vector c, float length,
                               int nr_samples, int nr_vecs, Vector* vecs )
/* ------------------------------------------------------------------- */
{
    Rotation rot;
    Vector vec;
    float angle, r, f, df, da, sumsin;
    int i, j, nr, nr_diheds;

    Vector_rotation( a, b, c, &rot );

    if ( nr_samples < 2 ) nr_samples = 2;
    da = g_max_angle / ( nr_samples - 1 );
    sumsin = 0.0;
    for ( i = 0; i < nr_samples; i++ )
        sumsin += sin( da * i );

    nr = 0;
    for ( i = 0; i < nr_samples; i++ )
    {
        angle = da * i;
        nr_diheds = ceil( sin( angle ) / sumsin * nr_vecs );
        if ( nr_diheds < 1 ) nr_diheds = 1;
        df = 2.0 * M_PI / nr_diheds;
        j = randomInt() % nr_diheds;
        f = j * df;
        for ( j = 0; j < nr_diheds; j++ )
        {
            if ( nr < nr_vecs )
            {
                r = sin( angle ) * length;
                vec.x = - cos( angle ) * length;
                vec.y = r * cos( f );
                vec.z = r * sin( f );
                vecs[nr] = Vector_sum( c, Vector_rotate_back( vec, &rot ) );
                nr++;
            }
            f = f + df;
        }
    }
}
