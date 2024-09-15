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

#include "interface.h"
#include <QApplication>

#include "mainwindow.h"

int gState = STATE_NOT_STARTED;


void setInterfaceState( int s )
{
    gState = s;
    qApp->processEvents();
}


int  getInterfaceState()
{
    static int ctr = 0;
    const int FREQUENCY = 1000;

    ctr++;

    if ( ctr >= FREQUENCY )
    {
        qApp->processEvents();
        ctr = 0;
    }
    return gState;
}




void  interfaceStateProcess()
{
    qApp->processEvents();

    switch ( gState )
    {
        case STATE_FAST :
            break;

        case STATE_GO :
        case STATE_ABORT :
            appWindow()->drawChains();
            qApp->processEvents();
            break;

        case STATE_STEP :
            gState = STATE_PAUSE;
            appWindow()->drawChains();

            while ( gState == STATE_PAUSE )
            {
                qApp->processEvents();
            }
            break;

        case STATE_PAUSE :
            appWindow()->drawChains( );
            while ( gState == STATE_PAUSE )
            {
                qApp->processEvents();
            }
            break;
    }
}
