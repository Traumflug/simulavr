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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */
#ifndef SCOPE
#define SCOPE


#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include "simulationmember.h"
#include "ui.h"
#include "pin.h"


class Scope : public SimulationMember {
    protected:
        UserInterface *ui;
        string name;
        unsigned char myPortValue;
        map<string, Pin*> allPins;

        vector<Pin*> vecPin;
        vector<int> lastVal;
        unsigned int noOfChannels;


    public:
        Scope(UserInterface *ui, const string &name, unsigned int noOfChannels, char *baseWindow);
        virtual ~Scope();
        Pin *GetPin(unsigned int no); 
        virtual int Step(bool &trueHwStep, unsigned long long *timeToNextStepIn_ns){return 0;} //what we should step here?
        void SetInStateForChannel(unsigned int channel, const Pin& p);
};

#endif
