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

#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include <map>
#include <sstream>

#include "../systemclocktypes.h"
#include "../simulationmember.h"
#include "mysocket.h"
#include "../pin.h"
#include "../externaltype.h"

/** Interfacing between "UI" application on TCP port and
ExternalType objects which interface with device peripherals.
*/
class UserInterface: public SimulationMember, private Socket, public ExternalType {
    protected:
        std::map<std::string, ExternalType*> extMembers;
        bool updateOn;
        SystemClockOffset pollFreq;
        std::string dummy; //replaces old dummy in Step which was static :-(
        std::map<std::string, char> LastState;
        int waitOnAckFromTclRequest; 
        int waitOnAckFromTclDone;

        //this is mainly for controlling the ui interface itself from the gui
        void SetNewValueFromUi(const std::string &);
    public:
        void AddExternalType(const char *name, ExternalType *p) {
            extMembers[name]=p;
        }
#ifndef SWIG
        void AddExternalType(const std::string& name, ExternalType *p) {
            AddExternalType(name.c_str(), p);
        }
#endif
        UserInterface(int port, bool withUpdateControl=true);
        ~UserInterface();
        void SendUiNewState(const std::string &s, const char &c);

        int Step(bool &, SystemClockOffset *nextStepIn_ns=0);
        void SwitchUpdateOnOff(bool PollFreq);
        void Write(const std::string &s);
};

#endif
