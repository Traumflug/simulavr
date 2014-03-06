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

#ifndef NET
#define NET

#include <vector>

#include "pin.h"

//! Connect Pins to each other and transfers a output change from a pin to input values for all pins
class Net
#ifndef SWIG
    : public std::vector <Pin *>
#endif
{
    public:
        //Net() {} //!< Common Constructor, initially it'a a "empty net" and useless!
        virtual ~Net(); //!< Destructor, disconnects save all pins, which are connected
        void Add(Pin *p); //!< Add a pin to net, e.g. connect a pin to others
        virtual void Delete(Pin *p); //!< Remove a pin from net
         //! Calculate a "electrical potential" on the net and set all pin inputs with this value
        virtual bool CalcNet();
        
    private:
        friend void Pin::RegisterNet(Net*);
};

#endif
