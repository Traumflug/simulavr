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
 *
 *  $Id$
 */

#ifndef UI
#define UI

#include "systemclocktypes.h"
#include "simulationmember.h"
#include "mysocket.h"
#include "pin.h"

using namespace std;


class UserInterface: public SimulationMember, public Socket, public ExternalType {
    protected:
        map<string, ExternalType*> extMembers;
        bool updateOn;
        SystemClockOffset pollFreq;
        string dummy; //replaces old dummy in Step which was static :-(
        map<string, char> LastState;
        int waitOnAckFromTclRequest; 
        int waitOnAckFromTclDone;

    public:
        //this is mainly for controlling the ui interface itself from the gui
        void SetNewValueFromUi(const string &);
        void AddExternalType(const char *name, ExternalType *p) {
          extMembers[name]=p;
        }
#ifndef SWIG
        void AddExternalType(const string name, ExternalType *p) {
          AddExternalType(name.c_str(), p);
        }
#endif
        UserInterface(int port, bool withUpdateControl=true);
        ~UserInterface();
        void SendUiNewState(const string &s, const char &c);

        int Step(bool &, SystemClockOffset *nextStepIn_ns=0);
        void SwitchUpdateOnOff(bool PollFreq);
        void Write(const string &s);

};

#endif
