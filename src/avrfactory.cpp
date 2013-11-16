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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

void AvrFactory::reg(const std::string name,
                     AvrDeviceCreator create) {
    string devname(name);
    for(unsigned int i = 0; i < devname.size(); i++)
        devname[i] = tolower(devname[i]);
    AVRDeviceMap & devmap = instance().devmap;
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
        avr_error("Device type not specified, use -d | --device TYPE or "
                  "insert a SIMINFO_DEVICE(name) macro into your source to "
                  "specify the device name");
    AVRDeviceMap::iterator i = devmap.find(devname);
    if(i == devmap.end())
        avr_error("Invalid device specification: %s", in);

    return devmap[devname]();
}

std::string AvrFactory::supportedDevices() {
    std::string ret;
    AVRDeviceMap & devmap = instance().devmap;

    for(AVRDeviceMap::iterator i = devmap.begin(); i != devmap.end(); i++)
        ret += i->first + "\n";
    return ret;
}

AvrFactory& AvrFactory::instance() {
    static AvrFactory f;
    return f;
}

