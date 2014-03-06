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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */
#include <iostream>
#include <cstdlib>
#include "specialmem.h"
#include "avrerror.h"

using namespace std;

RWWriteToFile::RWWriteToFile(TraceValueRegister *registry,
                             const string &tracename,
                             const string &filename):
    RWMemoryMember(registry, tracename),
    os(filename=="-" ? cout : ofs)
{
    if(filename != "-")
        ofs.open(filename.c_str());
}

void RWWriteToFile::set(unsigned char val) {
    os << val;
    os.flush();
}

unsigned char RWWriteToFile::get() const {
    avr_warning("Invalid read access to RWWriteToFile register.");
    return 0;
} 

RWReadFromFile::RWReadFromFile(TraceValueRegister *registry,
                               const string &tracename,
                               const string &filename):
    RWMemoryMember(registry, tracename),
    is((filename=="-") ? cin : ifs)
{
    if(filename != "-")
        ifs.open(filename.c_str());
}

void RWReadFromFile::set(unsigned char val) {
    avr_warning("Invalid write access to RWWriteToFile register with value %d.", (int)val);
}

unsigned char RWReadFromFile::get() const { 
    char val;
    is.get(val);
    return val; 
} 


RWExit::RWExit(TraceValueRegister *registry,
               const string &tracename) :
    RWMemoryMember(registry, tracename) {}


void RWExit::set(unsigned char c) {
    avr_message("Exiting at simulated program request (write)");
    DumpManager::Instance()->stopApplication();
    sysConHandler.ExitApplication(c); 
}

unsigned char RWExit::get() const {
    avr_message("Exiting at simulated program request (read)");
    DumpManager::Instance()->stopApplication();
    sysConHandler.ExitApplication(0); 
    return 0;
}

RWAbort::RWAbort(TraceValueRegister *registry,
                 const string &tracename) :
    RWMemoryMember(registry, tracename) {}

void RWAbort::set(unsigned char c) {
    avr_warning("Aborting at simulated program request (write)");
    DumpManager::Instance()->stopApplication();
    sysConHandler.AbortApplication(c);
}

unsigned char RWAbort::get() const {
    avr_warning("Aborting at simulated program request (read)");
    DumpManager::Instance()->stopApplication();
    sysConHandler.AbortApplication(0);
    return 0;
}

