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
#ifndef _adcpinh_
#define _adcpinh_

#include <fstream>
#include "avrdevice.h"

//! Pin class to provide a analog input signal
/*! This class allows the analog simulator access
  to the analogValue field of the pin and causes
  the Net to update (CalcNet). Note there is
  no dependency on the UserInterface class. */
class AdcAnalogPin: public Pin {
    
    public:
        //! Set the analog value and propagte through Net.
        inline void setAnalogValue(float value) throw() {
            //analogValue = value;
            //connectedTo->CalcNet();
            SetAnalogValue(value);
        }
        
};

//! Provides input of aanalog signal into simulator
/*! The purpose of this class is to stimulate a pin
  with an analog pattern specified by a file.
  The file will contain an "analog sample value" on
  each line, along with a duration in nano-seconds
  that must elapse before the value is changed. */
class AdcPin: public SimulationMember {
    
    private:
        AdcAnalogPin        _analogPin; //!< Output to AVR

        //! The analog input file.
        std::ifstream       _anaFile;

        // SimulationMember
        int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns = 0);
        
    public:
        AdcPin(const char* fileName, Net& pinNet) throw();
        
};

#endif
