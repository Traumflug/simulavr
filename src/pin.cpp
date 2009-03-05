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

#include <stdlib.h> // use atoi
#include <limits.h> // for INT_MAX

#include "pin.h"
#include "net.h"
#include "ui.h"
#include "hardware.h"

#include <sstream>
using namespace std;

void Pin::SetOutState( T_Pinstate s) { 
    outState=s;
}

int Pin::GetAnalog() const {
    switch (outState) {
        case ANALOG: 
            return analogValue; //reflext that we are self outputting an analog value

        case HIGH:
        case PULLUP:
            return INT_MAX;

        case TRISTATE:          //if we are input! we read the analog value
            return analogValue;

        case LOW:
        case PULLDOWN:
            return 0;

        default:
            return 0;
    }
}

void Pin::RegisterCallback(HasPinNotifyFunction *h) {
    notifyList.push_back(h);
}
void Pin::SetInState ( const Pin &p) { 
    analogValue=p.analogValue;

    if (pinOfPort!=0) {
        if (p) {       //is (bool)(Pin) -> is LOW or HIGH for the pin
            *pinOfPort |=mask;
        } else { 
            *pinOfPort &=0xff-mask;
        }
    }

    vector<HasPinNotifyFunction*>::iterator ii;
    vector<HasPinNotifyFunction*>::iterator ee;

    ee=notifyList.end();
    for (ii=notifyList.begin(); ii!=ee; ii++) {
        (*ii)->PinStateHasChanged(this);
    } 
}

Pin::Pin(T_Pinstate ps) { 
    pinOfPort=0; 
    connectedTo=new MirrorNet(this); 
    outState=ps;

    switch (ps) {
        case HIGH: 
        case PULLUP:
            analogValue=INT_MAX; 
            break;

        case LOW:
        case PULLDOWN:
            analogValue=0;
            break;

        case TRISTATE:
            analogValue=INT_MAX/2;
            break;

        default:
            analogValue=0;
    }
}

Pin::Pin() { 
    pinOfPort=0; 
    connectedTo= new MirrorNet(this); 
    outState=TRISTATE;
    analogValue=INT_MAX/2;
    //ui=0;
}

Pin::Pin( unsigned char *parentPin, unsigned char _mask) { 
    pinOfPort=parentPin;
    mask=_mask;
    connectedTo=new MirrorNet(this);
    outState=TRISTATE;
    analogValue=INT_MAX;
}

void Pin::RegisterNet(Net *n) {
    if (connectedTo!=0) { // we are allready connected!
        connectedTo->Delete(this); //remove it from old Net
        //Attention: Delete can also destroy the Net itself if
        //the Net is from type MirrorNet. So it is not allowed
        //to use connectedTo anymore after the Delete call.
        //in this function it is absolutly the correct semantic,
        //because we set directly the new net and the
        //old value is not longer accessable.
    }

    connectedTo=n;
}

Pin::operator unsigned char() const { 
    switch(outState) {
        case SHORTED: return 'S';
        case HIGH: return 'H';
        case PULLUP: return 'h';
        case TRISTATE: return 't';
        case PULLDOWN: return 'l';
        case LOW: return 'L';
        case ANALOG: return 'A';
        case ANALOG_SHORTED: return 'a';

    }
    return 'S'; //only default, should never be reached
}
/* patch removed -> test again , not working as expected
Pin::operator bool() const {
    if (outState==HIGH) return 1;
    if ((outState==ANALOG) || (outState==TRISTATE) || (outState==PULLUP) || (outState==PULLDOWN)) 
    { //maybe for TRISTATE not handled complete in simulavr... TODO
        if (analogValue > (INT_MAX/2)) {
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}
*/

Pin::operator bool() const {
    if ((outState==HIGH)||(outState==PULLUP)) return 1;
    if ((outState==ANALOG) || (outState==TRISTATE)) { //maybe for TRISTATE not handled complete in simulavr... TODO
        if (analogValue > (INT_MAX/2)) {
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}


Pin& Pin::operator= (unsigned char c) {
    switch (c) {
        case 'S': outState=SHORTED; analogValue=0; break;
        case 'H': outState=HIGH; analogValue=INT_MAX; break;
        case 'h': outState=PULLUP; analogValue=INT_MAX; break;
        case 't': outState=TRISTATE; analogValue=INT_MAX/2; break;
        case 'l': outState=PULLDOWN; analogValue=0; break;
        case 'L': outState=LOW; analogValue=0; break;

    }

    connectedTo->CalcNet();

    return *this;
}


Pin OpenDrain::operator+= (const Pin& p) {
    *pin= *this+p;
    return *this;
}

Pin Pin::operator+= (const Pin& p) {
    *this= *this+p;
    return *this;
}

Pin::Pin(const OpenDrain &od) {
    bool res=(bool) od;
    if (res==0) {
        outState=TRISTATE;
        analogValue=INT_MAX/2;
    }
    else {
        outState=LOW; 
        analogValue=0;
    }
}

Pin OpenDrain::GetPin() {
    bool res=(bool) *pin;
    if (res==0) return Pin(TRISTATE);
    else return Pin(LOW); 
}



Pin OpenDrain::operator +(const Pin &p) {
    Pin dummy; 
    bool parent=(bool)*pin;
    if (parent==0) dummy=Pin(TRISTATE);    //if the result 
    else dummy=Pin(LOW);

    return dummy+p;
}

Pin Pin::operator+ (const Pin& p) {
    if (outState==SHORTED) return Pin(SHORTED);
    if (outState==ANALOG_SHORTED) return Pin(ANALOG_SHORTED);
    if ((outState==ANALOG) && ( p.outState != TRISTATE)) {
        return Pin(ANALOG_SHORTED);
    }
    switch (p.outState) {
        case SHORTED:
            return Pin(SHORTED);
            break;

        case HIGH:
            if (outState==LOW) return Pin(SHORTED);
            return Pin(HIGH);
            break;

        case PULLUP:
            if (outState==LOW) return Pin(LOW);
            if (outState==HIGH) return Pin(HIGH);
            if (outState==PULLDOWN) return Pin(TRISTATE); //any other idea?
            return Pin(PULLUP);
            break;

        case TRISTATE:
            //return Pin(outState);
            return *this;
            break;

        case PULLDOWN:
            if (outState==LOW) return Pin(LOW);
            if (outState==HIGH) return Pin(HIGH);
            if (outState==PULLUP) return Pin(TRISTATE); //any other idea?
            return Pin(PULLDOWN);
            break;

        case LOW:
            if (outState==HIGH) return Pin(SHORTED);
            return Pin(LOW);
            break;

        case ANALOG:
            if (outState!=TRISTATE) return Pin(ANALOG_SHORTED);
            //outstate is TRISTATE and we have an anlog value so return pin ANALOG and value set
            return p; 
            break;

        case ANALOG_SHORTED:
            return Pin(ANALOG_SHORTED);

    }
    return Pin(TRISTATE);	//never used
}



OpenDrain::operator bool() const
{ 
    return 0;
}


ExtPin::ExtPin ( T_Pinstate ps, UserInterface *_ui, const char *_extName, const char *baseWindow) : Pin(ps), ui(_ui), extName(_extName) {
    ostringstream os;
    outState=ps;
    os << "create Net "<< _extName << " "<<baseWindow << " " << endl;
    ui->Write(os.str());

    ui->AddExternalType(extName, this);
}

void ExtPin::SetInState(const Pin &p) {
    ui->SendUiNewState(extName, (unsigned char)p);
}

void ExtPin::SetNewValueFromUi(const string& s) {
    Pin tmp;
    tmp= s[0];
    //outState= tmp.GetOutState();
    outState= tmp.outState;

    connectedTo->CalcNet();
}

//----------------------
void ExtAnalogPin::SetNewValueFromUi(const string& s) {
    outState= ANALOG;
    analogValue=atol(s.c_str());
    connectedTo->CalcNet();
}


ExtAnalogPin::ExtAnalogPin ( unsigned int value, UserInterface *_ui, const char *_extName, const char* baseWindow) : Pin(Pin::TRISTATE), ui(_ui), extName(_extName) {
    ostringstream os;
    os << "create AnalogNet "<< _extName << " "<<baseWindow <<" " << endl;
    ui->Write(os.str());

    ui->AddExternalType(extName, this);
}

void ExtAnalogPin::SetInState(const Pin &p) {
    ui->SendUiNewState(extName, (unsigned char)p);
}

