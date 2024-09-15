#ifndef COLORRING_H
#define COLORRING_H

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


#include "qwt3d_types.h"
#include <QRandomGenerator>

using namespace Qwt3D;

class ColorRing
{
public:
    ColorRing();

    static RGBA  nextColor();
    static void reset() { index = 0; }
    static int numColors();
    static void setRandomColors( bool b ) { random_color = b; }
    static void setOpacity( double op );

protected:
    static int index;
    static bool random_color;
    static QRandomGenerator rng;
    static double opacity;
};

#endif // COLORRING_H
