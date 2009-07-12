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

#include <string.h> //strcpy()
#include "memory.h"

#include <sstream>
#include <iostream>

#include "trace.h"

using namespace std;

unsigned int Memory::GetAddressAtSymbol(const string &s) {
    char *dummy;

    char *copy=(char*) malloc(s.length());
    strcpy(copy, s.c_str());

    multimap<unsigned int, string>::iterator ii;
    unsigned int retval=0;
    retval=strtoul(copy, &dummy, 16);

    if (retval !=0 && ((unsigned int)s.length() == (unsigned int)(dummy-copy))) {
        return retval;
    }

    for (ii=sym.begin(); ii!=sym.end(); ii++) {
        if (ii->second == s) {
            return ii->first;
        }
    }

    cerr << "Symbol " << s << " not found!" << endl;
    exit(0);

    return 0;
    
}

string Memory::GetSymbolAtAddress(unsigned int add){
    string lastName;
    unsigned int lastAddr=0;
    multimap<unsigned int, string>::iterator ii;
    multimap<unsigned int, string>::iterator last_ii;

    ii=sym.begin();
    last_ii=ii;
    if (ii==sym.end()) return ""; //we have no sym at all
    do {
        if(lastAddr!= ii->first) {
            last_ii=ii;
            lastName=ii->second;
        }
        lastAddr=ii->first;

        if (ii->first == add) break; //found symbol
        ii++;
        if ((ii!=sym.end()) && (ii->first> add)) break; //behind the right symbol
    } while (ii!=sym.end());
    ostringstream os;

    os << lastName;
    ii=last_ii;
    while ((++ii)!=sym.end()) { 
        if (lastAddr!= ii->first) break;
        os << "," << ii->second;
    };

    unsigned int offset=add-lastAddr;
    if ((offset)!=0) {
        os << "+0x"<<hex<<offset;
    }

    return os.str();

    return "";
}


Memory::Memory(int _size): size(_size){
    myMemory=(unsigned char*)malloc(size);
    if (myMemory==0) {
        cerr << "could not get memory for Memory::Memory" << endl;
        exit(0);
    }
}

//only 1 byte for malloc, not used anyway, we hold only the symbols here
Data::Data() : Memory(1) {
}

unsigned int Memory::GetSize() {
    return size;
}

void Memory::WriteMem(unsigned char*, unsigned int offset, unsigned int size){

};

void Memory::AddSymbol( pair<unsigned int, string> p) {
    sym.insert(p);
}
