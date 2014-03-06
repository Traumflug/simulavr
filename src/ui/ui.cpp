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

#include <time.h>
#include "externaltype.h"
#include "ui.h"
#include "hardware.h"
#include "pin.h"
#include "systemclock.h"
#include "avrerror.h"
#include <sstream>

using namespace std;

UserInterface::UserInterface(int port, bool _withUpdateControl): Socket(port), updateOn(1), pollFreq(100000)  {
    if (_withUpdateControl) {
        waitOnAckFromTclRequest=0;
        waitOnAckFromTclDone=0;
        ostringstream os;
        os << "create UpdateControl dummy dummy " << endl; 
        Write(os.str());
        AddExternalType("UpdateControl", this);
    }
}

UserInterface::~UserInterface() {
}

void UserInterface::SwitchUpdateOnOff(bool yesNo) {
    updateOn=yesNo;
}


int UserInterface::Step(bool &dummy1, SystemClockOffset *nextStepIn_ns) {
    if (nextStepIn_ns!=0) {
        *nextStepIn_ns=pollFreq;
    }

    static time_t oldTime=0;
    time_t newTime=time(NULL);

    if (updateOn || (newTime!=oldTime)) {
        oldTime=newTime;

        do { 
            if (Poll()!=0) {
                ssize_t len = 0;
                len=Read(dummy);

                //string debug=dummy;

                while (len>0) {

                    string::size_type pos;

                    pos=dummy.find(" ");

                    string net=dummy.substr(0, pos);
                    string rest=dummy.substr(pos+1); //vfrom pos+1 to end

                    if (net == "exit" )
                        avr_error("Exiting at external UI request");

                    string par;
                    int pos2=rest.find(" ");

                    if (pos2<=0) break;

                    par= rest.substr(0, pos2);
                    dummy=rest.substr(pos2+1);

                    // cerr << "UI: net=" << net << "- rest=" << rest << endl;
                    if (net == "__ack" ) {
                        waitOnAckFromTclDone++;
                    } else {
                        map<string, ExternalType*>::iterator ii;
                        ii=extMembers.find(net);
                        if (ii != extMembers.end() ) {
                            (ii->second)->SetNewValueFromUi(par);
                        } else {
                            // cerr << "Netz nicht gefunden:" << net << endl;
                            // cerr << "Start with string >>" << net << "<<" << endl;
                        }

                        //if (trace_on!=0) traceOut << "Net: " << net << "changed to " << par << endl;

                    } //__ack

                    len=dummy.size(); //recalc size from rest of string

                } // len > 0
            } //poll
        }while (waitOnAckFromTclRequest > waitOnAckFromTclDone+500); 


        if (waitOnAckFromTclRequest!=waitOnAckFromTclDone) {
            waitOnAckFromTclRequest=waitOnAckFromTclDone=0;
        }

    } //if (update_on  | look for reenable again)

    return 0;
}



void UserInterface::SendUiNewState(const string &s, const char &c)  {
    ostringstream os;
    //static map<string, char> LastState;

    if (LastState[s]==c) {
        return;
    }
    LastState[s]=c;

    os << "set " << s << " " << c << endl;
    Write(os.str());

    //    SystemClock::Instance().Rescedule(this, 1000); //read ack back as fast as possible
}

void UserInterface::SetNewValueFromUi(const string &value){
    if (value=="0") {
        updateOn=false;
    } else {
        updateOn=true;
    }

}

void UserInterface::Write(const string &s) {
    if (updateOn) {

        for (unsigned int tt = 0; tt< s.length() ; tt++) {
            if (s[tt]=='\n') {
                waitOnAckFromTclRequest++;
            }
        }
        Socket::Write(s);
    }
} 

