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

using namespace std;


#include "ui.h"
#include "trace.h"
#include "hardware.h"
#include "pin.h"

int waitOnAckFromTcl=0;

UserInterface::UserInterface(int port): Socket(port) {
}

UserInterface::~UserInterface() {
}


int UserInterface::Step(bool dummy1, unsigned long long *nextStepIn_ns) {
    if (nextStepIn_ns!=0) *nextStepIn_ns=0;
    do { 
        if (Poll()!=0) {
            ssize_t len = 0;
            static string dummy;
            len=Read(dummy);

            string debug=dummy;

            while (len>0) {
                
                string::size_type pos;

                pos=dummy.find(" ");
                
                string net=dummy.substr(0, pos);
                string rest=dummy.substr(pos+1); //vfrom pos+1 to end

                string par;
                int pos2=rest.find(" ");

                if (pos2<=0) break;

                par= rest.substr(0, pos2);
                dummy=rest.substr(pos2+1);

                if (net == "__ack" ) {
                    waitOnAckFromTcl=0;
                } else {
                    map<string, ExternalType*>::iterator ii;
                    ii=extPins.find(net);
                    if (ii != extPins.end() ) {
                        (ii->second)->SetNewValueFromUi(par);
                    } else {
                        cerr << "Netz nicht gefunden:" << net << endl;
                        cerr << "Start with string >>" << debug << "<<" << endl;
                    }

                    if (trace_on!=0) traceOut << "Net: " << net << "changed to " << par << endl;

                } //__ack

                len=dummy.size(); //recalc size from rest of string

            } // len > 0
        } //poll
    }while (waitOnAckFromTcl!=0); 

    return 0;
}



void UserInterface::SendUiNewState(const string &s, const char &c)  {
    ostringstream os;
    static map<string, char> LastState;

    if (LastState[s]==c) {
        return;
    }
    LastState[s]=c;

    os << "set " << s << " " << c << endl;
    Write(os.str());

    waitOnAckFromTcl=1;
}


