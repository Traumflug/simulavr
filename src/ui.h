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

#ifndef UI
#define UI

#include "simulationmember.h"
#include "mysocket.h"

using namespace std;

class Pin;
class ExternalType;

class UserInterface: public SimulationMember, public Socket {
    protected:
        map<string, ExternalType*> extPins;

    public:
        void AddExternalType(const string &name, ExternalType *p) { extPins[name]=p;}
        UserInterface(int port);
        ~UserInterface();
        void SendUiNewState(const string &s, const char &c);

        int Step(bool, unsigned long long *nextStepIn_ns=0);

};

#endif
