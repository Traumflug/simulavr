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

#ifndef MEMORY
#define MEMORY

#include <string>
#include <map>

#include "decoder.h"

class Memory {
	protected:
        unsigned int size;
	public:

		unsigned char *myMemory;
		std::multimap<unsigned int, std::string> sym;
		std::string GetSymbolAtAddress(unsigned int add);
		unsigned int GetAddressAtSymbol(const std::string &s);
		void AddSymbol( std::pair<unsigned int, std::string> p);
		Memory(int size);
		unsigned int GetSize();
		void WriteMem(unsigned char*, unsigned int offset, unsigned int size);
};

//the data class holds only all symbols from rw data space but NOT the content of the rw memory itself
//this should be changed some days later
class Data : public Memory {
    public:
    Data();
};
#endif
