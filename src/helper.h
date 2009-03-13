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

#include <iostream>
#ifndef HELPER
#define HELPER

using namespace std;

class HexChar {
	public:
		unsigned char val;
		HexChar(unsigned char x);// { val=x; }
};


class HexShort {
	public:
		unsigned short val;
		HexShort(unsigned short x);// { val=x; }
};

class DecLong {
    public:
        unsigned long val;
        DecLong(unsigned long v);
};

ostream &operator << (ostream &os, const HexChar &h);
ostream &operator << (ostream &os, const HexShort &h);
ostream &operator << (ostream &os, const DecLong &h);


#endif	
