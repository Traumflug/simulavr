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

#ifndef PYSIMULATIONMEMBER
#define PYSIMULATIONMEMBER

#include "simulationmember.h"

//! Interface class PySimulationMember to support usage of SimulationMember on python
/*! This interface class definition is only available on building python interface for
  simulavr! Don't use it outside!
  
  Because it's not possible to reflect back timeToNextStepIn_ns over parameter pointer
  it's necessary to introduce DoStep method. DoStep returns value for timeToNextStepIn_ns,
  this value will be used in call of method Step. Method Step here returns allways 0.*/
class PySimulationMember: public SimulationMember {
    public:

        virtual ~PySimulationMember() {}

        //! Process a time step, timeToNextStepIn_ns represents time to next call in ns.
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns) {
            SystemClockOffset t = DoStep(trueHwStep);
            if(timeToNextStepIn_ns != NULL)
                *timeToNextStepIn_ns = t;
            return 0;
        }
        //! Process a time step, returns time to next call in ns.
        /*! This is a abstract method and have to be overlayed in python! */
        virtual SystemClockOffset DoStep(bool &trueHwStep) = 0;
};

#endif 
        
