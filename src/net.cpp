/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph       
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "net.h"
#include "pin.h"

void Net::Add(Pin *p) {
    push_back(p);
    p->RegisterNet(this);
    CalcNet();
}

void Net::Delete(Pin *p) {
    iterator ii;
    for(ii = begin(); ii != end(); ii++) {
        if((Pin*)(*ii) == p) {
            erase(ii);
            break;
        }
    }
}

Net::~Net() {
    while(begin() != end())
        (*begin())->UnRegisterNet(this);
}

bool Net::CalcNet() {
    Pin result(Pin::TRISTATE);
    iterator ii;
    for(ii = begin(); ii != end(); ii++)
        result += ((*ii)->GetPin()); //get state of pin (TRISTATE, HIGH, LOW ....)

    //new result is now found, so set all pins in the Net to new state
    for(ii = begin(); ii != end(); ii++)
        (*ii)->SetInState( result); //In-State that means the state of register PIN not the complete pin here

    return (bool)result;
}

