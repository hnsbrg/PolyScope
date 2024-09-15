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


#include "colorring.h"

using namespace Qwt3D;

int ColorRing::index = 0;
double ColorRing::opacity = 1.0;
bool ColorRing::random_color = false;
QRandomGenerator ColorRing::rng;


#if 1
RGBA colors[] =
{
    RGBA( 0.33, 0.33, 0.33, 1.0 ),
    RGBA( 0.67, 0.33, 0.33, 1.0 ),
    RGBA( 0.33, 0.67, 0.333, 1.0 ),
    RGBA( 0.33, 0.33, 0.67, 1.0 ),
    RGBA( 0.67, 0.33, 0.67, 1.0 ),
    RGBA( 0.122,    0.471,  0.706, 1.0 ),
    RGBA( 0.89, 0.102,  0.11, 1.0 ),
    RGBA( 0.5, 0.5, 0.0, 1.0 ),
    RGBA( 0.5, 0.0, 0.5, 1.0 ),
    RGBA( 0.0, 0.5, 0.5, 1.0 ),
    RGBA( 0.0, 0.33, 0.67, 1.0 ),
    RGBA( 0.0, 0.67, 0.33, 1.0 ),
    RGBA( 0.33, 0.00, 0.67, 1.0 ),
    RGBA( 0.67, 0.00, 0.33, 1.0 ),
    RGBA( 0.33, 0.67, 0.00, 1.0 ),
    RGBA( 0.67, 0.33, 0.00, 1.0 ),
    RGBA( 0.0, 0.0, 0.0, 0.0 )  // transparent
};
#else

RGBA colors[] =
{
    RGBA( 0.651,    0.808,  0.89, OPACITY ),
    RGBA( 0.122,    0.471,  0.706, OPACITY ),
    RGBA( 0.698,    0.875,  0.541, OPACITY ),
    RGBA( 0.2,  0.627,  0.173, OPACITY ),
    RGBA( 0.984,    0.604,  0.6, OPACITY ),
    RGBA( 0.89, 0.102,  0.11, OPACITY ),
    RGBA( 0.992,    0.749,  0.435, OPACITY ),
    RGBA( 1,    0.498,  0, OPACITY ),
    RGBA( 0.792,    0.698,  0.839, OPACITY ),
    RGBA( 0.416,    0.239,  0.604, OPACITY ),
    RGBA( 1,    1,  0.6, OPACITY ),
    RGBA( 0.694,    0.349,  0.157, OPACITY ),
    RGBA( 0.0, 0.0, 0.0, 0.0 )  // transparent
};

#endif



ColorRing::ColorRing()
{
    rng.seed( 456123 );
    reset();
}



int ColorRing::numColors()
{
    return  random_color ? 0 : ( sizeof colors / sizeof colors[0] ) - 1;
}

void ColorRing::setOpacity( double op )
{
    opacity = op;

    if ( false == random_color )
    {
        int num_colors = numColors();
        for ( int i = 0; i < num_colors; i++ )
        {
            colors[i].a = opacity;
        }
    }
}



RGBA  ColorRing::nextColor()
{
    if ( random_color )
    {
        return  RGBA( rng.generateDouble() - 0.33, rng.generateDouble() - 0.33, rng.generateDouble() - 0.33, opacity );
    }
    else
    {
        if ( 0.0 == colors[index ].a )
        {
            reset();
        }
        return colors[index++];
    }
}
