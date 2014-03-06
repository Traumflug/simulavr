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

#include "scope.h"
#include "systemclock.h"


using namespace std;

class ScopePin : public Pin {
    protected:
        Scope *scope;
        unsigned int channel;

    public:
        ScopePin(Scope *s, unsigned int c ):scope(s), channel(c){};
        void SetInState(Pin& p) {
            scope->SetInStateForChannel(channel, p);    //transmit the pin state to the scope
        }
};

Scope::Scope( UserInterface *u, const string & n, unsigned int cnt, const char *baseWindow)
	: ui(u), name(n), vecPin(cnt), lastVal(cnt), noOfChannels(cnt)
{
    for (unsigned int tt=0; tt< cnt; tt++) {
        vecPin[tt]=new ScopePin(this, tt);
        lastVal[tt]=0;
    }

    //  setup the corresponding ui in tcl from here
    ostringstream os;
    os << "create Scope " << name  << " "<<baseWindow <<" " <<  noOfChannels << endl;
    ui->Write(os.str());
}

Scope::~Scope() {}

Pin *Scope::GetPin(unsigned int n) {
    return vecPin[n];
}

void Scope::SetInStateForChannel(unsigned int channel, Pin& p) {
    if ( lastVal[channel]!= p.GetAnalog() ) {
        ostringstream os;
        os << name << " ChangeValue " << SystemClock::Instance().GetCurrentTime() << " " << channel << " " << p.GetAnalog()<<endl;

        ui->Write(os.str());
        //cout << "Set last val for channel " << channel << " value " << p.GetAnalog() << endl;
        lastVal[channel]=p.GetAnalog();
        //cout << "OK" << endl << endl;
    }
}







