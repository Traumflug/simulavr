/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph		
 * Copyright (C) 2009 Onno Kortmann
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
#include <cstdlib>
#include "specialmem.h"

using namespace std;

RWWriteToFile::RWWriteToFile(
    AvrDevice *core,
    const string &tracename,
    const string &filename)
    : RWMemoryMember(core, tracename),
      os(filename=="-" ? cout : ofs) {
  if(filename != "-")
      ofs.open(filename.c_str());
}

void RWWriteToFile::set(unsigned char val) {
    os << val; os.flush();
}

unsigned char RWWriteToFile::get() const {
    cerr << "Invalid read access to RWWriteToFile register." << endl;
    return 0;
} 

RWReadFromFile::RWReadFromFile(
    AvrDevice *core,
    const string &tracename,
    const string &filename)
    : RWMemoryMember(core, tracename),
     is((filename=="-") ? cin : ifs) {
    if (filename != "-")
	ifs.open(filename.c_str());
}

void RWReadFromFile::set(unsigned char val) {
    cerr << "Invalid write access to RWWriteToFile register with value " << val << "." << endl;
}

unsigned char RWReadFromFile::get() const { 
    char val;
    is.get(val);
    return val; 
} 


RWExit::RWExit(AvrDevice *core,
               const std::string &tracename) :
    RWMemoryMember(core, tracename) {}


void RWExit::set(unsigned char c) {
  cerr << "Exiting at simulated program request" << endl;
  exit(c); 
}

unsigned char RWExit::get() const {
  cerr << "Exiting at simulated program request" << endl;
  exit(0);
  return 0;
}

RWAbort::RWAbort(AvrDevice *core,
                 const std::string &tracename) :
    RWMemoryMember(core, tracename) {}

void RWAbort::set(unsigned char c) {
  cerr << "Aborting at simulated program request" << endl;
  abort();
}

unsigned char RWAbort::get() const {
    cerr << "Aborting at simulated program request" << endl;
    abort();
    return 0;
}

