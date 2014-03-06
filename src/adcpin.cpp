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
#include "adcpin.h"
#include "avrerror.h"

AdcPin::AdcPin(const char* fileName, Net& pinNet) throw():
    _analogPin(),
    _anaFile(fileName)
{
    _analogPin.outState = Pin::ANALOG;
    pinNet.Add(&_analogPin);

    if(!_anaFile)
        avr_error("Cannot open Analog input file '%s'.", fileName);
}

char* readNextLine(std::ifstream& f, char* buffer, unsigned len, SystemClockOffset *timeToNextStepIn_ns) {
    for(unsigned i = 0; i < 2; ++i){
        while(f.getline(buffer, len)){
            // Skip comment lines
            if(buffer[0] == '#')
                continue;
            return buffer;
        }
        f.clear();
        f.seekg (0, std::ios::beg);
    }
    return 0;
}

int AdcPin::Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns) {
    char lineBuffer[1024];

    if(!readNextLine(_anaFile, lineBuffer, sizeof(lineBuffer), timeToNextStepIn_ns)) {
        _anaFile.close();
    }

    char* p = lineBuffer;
    unsigned long delayInNs = strtoul(p, &p, 0);
    int analogValue = (int)strtol(p, &p, 0);
    if(analogValue > 5000000) // limit to 5000000 = 5.0V
                              // this isn't correct, because it dosn't respect real Vcc level
        analogValue = 5000000;
    _analogPin.setAnalogValue(0.000001 * analogValue);

    *timeToNextStepIn_ns = delayInNs;

    return 0;
}

