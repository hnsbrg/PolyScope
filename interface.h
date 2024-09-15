#ifndef INTERFACE_H
#define INTERFACE_H

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


#define STATE_NOT_STARTED -1
#define STATE_PAUSE 0
#define STATE_STEP 1
#define STATE_GO 2
#define STATE_FAST 3
#define STATE_ABORT 4



int  getInterfaceState();
void setInterfaceState( int s );
void  interfaceStateProcess();



#endif // INTERFACE_H
