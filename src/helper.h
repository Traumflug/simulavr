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

#include <iostream>
#include <vector>
#ifndef HELPER
#define HELPER

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

std::ostream &operator << (std::ostream &os, const HexChar &h);
std::ostream &operator << (std::ostream &os, const HexShort &h);
std::ostream &operator << (std::ostream &os, const DecLong &h);

//! Convert an int into a string
std::string int2str(int i);

//! Convert int into hex string
std::string int2hex(int i);

//! Reads one line from a stream.
std::string readline(std::istream &is);

//! Splits a string into a vector of strings at delimiters splitc
std::vector<std::string> split(const std::string &inp, std::string splitc="\t\n\r\b ");
#endif	
