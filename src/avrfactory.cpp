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

using namespace std;
typedef map<std::string, AvrFactory::AvrDeviceCreator> AVRDeviceMap;

AVRDeviceMap devmap;

void AvrFactory::reg(const std::string name,
				 AvrDeviceCreator create) {
    AVRDeviceMap::iterator i=devmap.find(name);
    if (i==devmap.end())
	devmap[name]=create;
    else {
	cerr << "Duplicate device specification " << name << endl;
	exit(1);
    }
}

AvrDevice* AvrFactory::makeDevice(const char *in) {
    AVRDeviceMap::iterator i=devmap.find(in);
    if (i==devmap.end()) {
	cerr << "Invalid device specification: " << in << endl;
	exit(1);
    }

    return devmap[in]();
}

std::string AvrFactory::supportedDevices() {
    std::string ret;
    
    for (AVRDeviceMap::iterator i=devmap.begin();
	 i!=devmap.end(); i++)
	ret+=i->first+"\n";
    return ret;
}

AvrFactory& AvrFactory::instance() {
    static AvrFactory *f;
    if (!f) {
	f=new AvrFactory();
	return *f;
    }
}

AvrFactory::AvrFactory() {}

