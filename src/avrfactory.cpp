/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph       
 * Copyright (C) 2007 Onno Kortmann
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
#include <map>
#include <iostream>
#include <cstdlib>
#include "config.h"
#include "avrfactory.h"
#include "avrerror.h"

using namespace std;

typedef map<std::string, AvrFactory::AvrDeviceCreator> AVRDeviceMap;

#ifdef _MFC_VER
#pragma data_seg(".crt$xct")
typedef void (__cdecl *PF)(void);
__declspec(allocate(".crt$xct")) const PF InitSegEnd1 = (PF) 0;
#pragma data_seg(".crt")
__declspec(allocate(".crt"))     const PF InitSegEnd2 = (PF) 0;

#pragma section(".mine$a", read)
__declspec(allocate(".mine$a")) const PF InitSegStart = (PF)1;
#pragma section(".mine$z",read)
__declspec(allocate(".mine$z")) const PF InitSegEnd = (PF)1;
// by default, goes into a read only section
//#pragma init_seg(".mine$m")



// creates new section in executable
#pragma section(".crt", read)
#pragma section(".crt$xct", read)
// force initialization of 'devmap' before initializers of AVRFactoryRegistryMaker execute
//#pragma init_seg(".crt$xct")
//#pragma init_seg("CRT$xcl")
#pragma init_seg(lib)

bool g_afas = 1;
int g_nInitTime = g_afas ? 44 : 55;
#endif

//! map of registered AVR devices
AVRDeviceMap devmap;

void AvrFactory::reg(const std::string name,
                     AvrDeviceCreator create) {
    string devname(name);
    for(unsigned int i = 0; i < devname.size(); i++)
        devname[i] = tolower(devname[i]);
	if(devmap.empty())
		devmap = AVRDeviceMap();
    AVRDeviceMap::iterator i = devmap.find(devname);
    if(i == devmap.end())
        devmap[devname] = create;
    else
        avr_error("Duplicate device specification: %s", devname.c_str());
}

AvrDevice* AvrFactory::makeDevice(const char *in) {
    string devname(in);
    for(unsigned int i = 0; i < devname.size(); i++)
        devname[i] = tolower(devname[i]);
    if(devname == "unknown")
        avr_error("Device type not specified, use --device TYPE", in);
    AVRDeviceMap::iterator i = devmap.find(devname);
    if(i == devmap.end())
        avr_error("Invalid device specification: %s", in);

    return devmap[devname]();
}

std::string AvrFactory::supportedDevices() {
    std::string ret;
    
    for(AVRDeviceMap::iterator i = devmap.begin(); i != devmap.end(); i++)
        ret += i->first + "\n";
    return ret;
}

AvrFactory& AvrFactory::instance() {
    static AvrFactory f;
    return f;
}

