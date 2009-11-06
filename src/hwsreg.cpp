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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "hwsreg.h"
#include "global.h"

#include <iostream>
using namespace std;

HWSreg_bool::operator int() {
    return C + (Z << 1) + (N << 2) + (V << 3) + (S << 4) + (H << 5) + (T << 6) + (I << 7);
}

HWSreg_bool::HWSreg_bool(const int i) { 
    C = i &0x01;
    Z = (i & 0x02) > 1;
    N = (i & 0x04) > 2;
    V = (i & 0x08) > 3;
    S = (i & 0x10) > 4;
    H = (i & 0x20) > 5;
    T = (i & 0x40) > 6;
    I = (i & 0x80) > 7;
}

HWSreg_bool::HWSreg_bool() {
    C = Z = N = V = S = H = T = I = 0;
}

HWSreg::operator string() {
    string s("SREG=[");
    if(I) s += "I"; else s += "-";
    if(T) s += "T"; else s += "-";
    if(H) s += "H"; else s += "-";
    if(S) s += "S"; else s += "-";
    if(V) s += "V"; else s += "-";
    if(N) s += "N"; else s += "-";
    if(Z) s += "Z"; else s += "-";
    if(C) s += "C"; else s += "-";
    s += "] ";
    return s;
}

HWSreg HWSreg::operator =(const int i) {
    C = i & 0x01;
    Z = (i & 0x02) > 1;
    N = (i & 0x04) > 2;
    V = (i & 0x08) > 3;
    S = (i & 0x10) > 4;
    H = (i & 0x20) > 5;
    T = (i & 0x40) > 6;
    I = (i & 0x80) > 7;
    return *this;
}

unsigned char RWSreg::get() const {
    return (*status);
}

void RWSreg::set(unsigned char val) {
    *status = val;
}

// EOF
