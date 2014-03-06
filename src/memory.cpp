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

#include <string.h> //strcpy()
#include <sstream>
#include <iostream>

#include "memory.h"
#include "avrerror.h"

using namespace std;

unsigned int Memory::GetAddressAtSymbol(const string &s) {
  
    // feature: use a number instead of a symbol
    char *dummy;
    char *copy = avr_new(char, s.length() + 1);
    unsigned int retval = 0;
    unsigned int convlen = 0;
    
    strcpy(copy, s.c_str());
    retval = strtoul(copy, &dummy, 16);
    convlen = (unsigned int)(dummy - copy);
    avr_free(copy);
    
    if((retval != 0) && ((unsigned int)s.length() == convlen)) {
        // number found, return this
        return retval;
    }

    // isn't a number, try to find symbol ...
    multimap<unsigned int, string>::iterator ii;

    for(ii = sym.begin(); ii != sym.end(); ii++) {
        if(ii->second == s) {
            return ii->first;
        }
    }

    avr_error("symbol '%s' not found!", s.c_str());

    return 0; // to avoid warnings, avr_error aborts the program
}

string Memory::GetSymbolAtAddress(unsigned int add){
    string lastName;
    unsigned int lastAddr = 0;
    multimap<unsigned int, string>::iterator ii;
    multimap<unsigned int, string>::iterator last_ii;

    ii = sym.begin();
    last_ii = ii;
    if(ii == sym.end())
        return ""; // we have no symbols at all
    do {
        if(lastAddr != ii->first) {
            last_ii = ii;
            lastName = ii->second;
        }
        lastAddr = ii->first;

        if(ii->first == add)
            break; // found symbol
        ii++;
        if((ii != sym.end()) && (ii->first > add))
            break; // behind the right symbol
    } while(ii != sym.end());
    
    ostringstream os;

    os << lastName;
    ii = last_ii;
    while((++ii) != sym.end()) { 
        if(lastAddr != ii->first)
            break;
        os << "," << ii->second;
    };

    unsigned int offset = add - lastAddr;
    if((offset) != 0) {
        os << "+0x" << hex << offset;
    }

    return os.str();
}

Memory::Memory(int _size): size(_size) {
    myMemory = avr_new(unsigned char, size);
}

