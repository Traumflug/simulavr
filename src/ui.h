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
#include "pin.h"

using namespace std;


class UserInterface: public SimulationMember, public Socket, public ExternalType {
    protected:
        map<string, ExternalType*> extPins;
        bool updateOn;
        unsigned long long pollFreq;
        string dummy; //replaces old dummy in Step which was static :-(
        map<string, char> LastState;
        int waitOnAckFromTclRequest; 
        int waitOnAckFromTclDone;

    public:
        void SetNewValueFromUi(const string &); //this is mainly for conroling the ui interface itself from the gui
        void AddExternalType(const string &name, ExternalType *p) { extPins[name]=p;}
        UserInterface(int port, bool withUpdateControl=true);
        ~UserInterface();
        void SendUiNewState(const string &s, const char &c);

        int Step(bool, unsigned long long *nextStepIn_ns=0);
        void SwitchUpdateOnOff(bool PollFreq);
        void Write(const string &s);

};

#endif
