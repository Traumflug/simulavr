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
 */
#include <iostream>
#include "helper.h"


HexChar::HexChar(unsigned char x) { val=x; }
HexShort::HexShort(unsigned short x) { val=x; }
DecLong::DecLong(unsigned long x) { val=x; }

ostream &operator << (ostream &os, const HexChar &h) {
    os << "0x";
    os.width(2);
    os.fill('0');
    os << hex << (unsigned int) h.val << dec ;
    return os;
}

ostream &operator << (ostream &os, const HexShort &h) {
    os << "0x" ;
    os.width(4);
    os.fill('0');
    os << hex << (unsigned int) h.val << dec ;
    return os;
}

ostream &operator << (ostream &os, const DecLong &h) {
    os.width(9);
    os.fill(' ');
    os << dec << (unsigned long) h.val << dec ;
    return os;
}







