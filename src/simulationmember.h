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

#ifndef SIMULATIONMEMBER
#define SIMULATIONMEMBER

#include "systemclocktypes.h"

/** Any class which is needs to be notified at certain time implements this.
* Implementor usually calls SystemClock::Add(this) and its SimulationMember::Step()
* will be called later. People, please avoid polling. */
class SimulationMember {
    public:
        virtual ~SimulationMember() { }
        /// Return nonzero if a breakpoint was hit.
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns=0)=0;
};

#endif 
        
