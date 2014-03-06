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

#ifndef SCOPE
#define SCOPE


#include <iostream>
#include <string>
#include <vector>

#include "../systemclocktypes.h"
#include "../simulationmember.h"
#include "ui.h"
#include "../pin.h"


class Scope : public SimulationMember {
    protected:
        UserInterface *ui;
	std::string name;
        unsigned char myPortValue;
	std::map<std::string, Pin*> allPins;

	std::vector<Pin*> vecPin;
	std::vector<int> lastVal;
        unsigned int noOfChannels;


    public:
        Scope(UserInterface *ui, const std::string &name, unsigned int noOfChannels, const char *baseWindow);
        virtual ~Scope();
        Pin *GetPin(unsigned int no); 
        virtual int Step(bool &trueHwStep, SystemClockOffset *timeToNextStepIn_ns){return 0;} //what we should step here?
        void SetInStateForChannel(unsigned int channel, Pin& p);
};

#endif
