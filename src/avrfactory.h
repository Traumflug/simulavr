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

#ifndef AVRFACTORY
#define AVRFACTORY

#include <string>

class AvrDevice;

class AvrFactory {
 public:
    /*! Produces an AVR device according to the configuration string.
      Right now, the configuration string is simply the full name of the AVR
      device, like AT90S4433 or ATMEGA128.
      */
    AvrDevice* makeDevice(const char *device);

    //! Singleton class access. 
    static AvrFactory& instance();
 private:
    AvrFactory();
};


#endif
