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

#include <limits.h> // for INT_MAX

#include "pin.h"
#include "net.h"

enum {
    //! Analog value for a tristate potential
    /*! This is the value used to set the analogValue
       when the pin output is TRISTATE. Originally, the
       value was simply (INT_MAX/2), but when I was debugging
       a glitch in a pin used as an open-drain output, I
       found that Pin::operator bool() chose to convert this
       to a LOW condition during a CalcNet() using MirroNet.
       Thus, I added one to the value and the glitch went away.
       This is probably not the absolute correct fix, but it
       works for this case. */
    TRISTATE_ANALOG_VALUE = (INT_MAX / 2) + 1
};

int Pin::GetAnalog(void) const {
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

void Pin::SetInState(const Pin &p) { 
    analogValue = p.analogValue;

    if(pinOfPort != 0) {
        if(p) {       //is (bool)(Pin) -> is LOW or HIGH for the pin
            *pinOfPort |= mask;
        } else { 
            *pinOfPort &= 0xff - mask;
        }
    }

    std::vector<HasPinNotifyFunction*>::iterator ii;
    std::vector<HasPinNotifyFunction*>::iterator ee = notifyList.end();

    for(ii = notifyList.begin(); ii != ee; ii++) {
        (*ii)->PinStateHasChanged(this);
    } 
}

bool Pin::CalcPin(void) {
    if(connectedTo == NULL) {
        // no net connected, transfer the output to own input
        SetInState(*this);
        return (bool)*this;
    } else {
        return connectedTo->CalcNet();
    }
}

Pin::Pin(T_Pinstate ps) { 
    pinOfPort = 0; 
    connectedTo = NULL;
    
    outState = ps;

    // initialisation of analog value
    switch (ps) {
        case HIGH: 
        case PULLUP:
            analogValue = INT_MAX; 
            break;

        case LOW:
        case PULLDOWN:
            analogValue = 0;
            break;

        case TRISTATE:
            analogValue = TRISTATE_ANALOG_VALUE;
            break;

        default:
            analogValue = 0;
    }
}

Pin::Pin() { 
    pinOfPort = 0; 
    connectedTo = NULL;
    
    outState = TRISTATE;
    analogValue = TRISTATE_ANALOG_VALUE;
}

Pin::~Pin() {
    // unregister myself on Net instance
    UnRegisterNet(connectedTo);
}

Pin::Pin( unsigned char *parentPin, unsigned char _mask) { 
    pinOfPort = parentPin;
    mask = _mask;
    connectedTo = NULL;
    
    outState = TRISTATE;
    analogValue = TRISTATE_ANALOG_VALUE;
}

Pin::Pin(const Pin& p) {
    pinOfPort = 0; // don't take over HWPort connection!
    connectedTo = NULL; // don't take over Net instance!
    
    outState = p.outState;
    analogValue = p.analogValue;
}

void Pin::RegisterNet(Net *n) {
    UnRegisterNet(connectedTo); // unregister old Net instance, if exists
    connectedTo = n; // register new Net instance
}

void Pin::UnRegisterNet(Net *n) {
    if(connectedTo == n && connectedTo != NULL)
        connectedTo->Delete(this);
    connectedTo = NULL;
}

Pin::operator char() const { 
    switch(outState) {
        case SHORTED: return 'S';
        case HIGH: return 'H';
        case PULLUP: return 'h';
        case TRISTATE: return 't';
        case PULLDOWN: return 'l';
        case LOW: return 'L';
        case ANALOG: return 'a';
        case ANALOG_SHORTED: return 'A';
    }
    return 'S'; //only default, should never be reached
}

Pin::operator bool() const {
    if((outState==HIGH) || (outState==PULLUP))
        return true;

    //maybe for TRISTATE not handled complete in simulavr... TODO
    if((outState==ANALOG) || (outState==TRISTATE)) {
        if(analogValue > (INT_MAX / 2))
            return true;
        else
            return false;
    }

    return false;
}

Pin& Pin::operator= (char c) {
    switch(c) {
        case 'S':
            outState = SHORTED;
            analogValue = 0;
            break;
            
        case 'H':
            outState = HIGH;
            analogValue = INT_MAX;
            break;
            
        case 'h':
            outState = PULLUP;
            analogValue = INT_MAX;
            break;
            
        case 't':
            outState = TRISTATE;
            analogValue = TRISTATE_ANALOG_VALUE;
            break;
            
        case 'l':
            outState = PULLDOWN;
            analogValue = 0;
            break;
            
        case 'L':
            outState = LOW;
            analogValue = 0;
            break;
            
        case 'a':
            outState = ANALOG;
            analogValue = 0;
            break;
            
        case 'A':
            outState = ANALOG_SHORTED;
            analogValue = 0;
            break;
    }

    CalcPin();

    return *this;
}

Pin& Pin::SetAnalog(int value) {
    //outState = ANALOG;
    analogValue = value;

    CalcPin();

    return *this;
}

Pin Pin::operator+= (const Pin& p) {
    *this = *this + p;
    return *this;
}

#ifndef DISABLE_OPENDRAIN
Pin::Pin(const OpenDrain &od) {
    bool res = (bool) od;
    if(res == 0) {
        outState = TRISTATE;
        analogValue = TRISTATE_ANALOG_VALUE;
    } else {
        outState = LOW; 
        analogValue = 0;
    }
}
#endif

Pin Pin::operator+ (const Pin& p) {
    if(outState == SHORTED)
        return Pin(SHORTED);
    if(outState == ANALOG_SHORTED)
        return Pin(ANALOG_SHORTED);
    if((outState == ANALOG) && (p.outState != TRISTATE))
        return Pin(ANALOG_SHORTED);
    switch(p.outState) {
        case SHORTED:
            return Pin(SHORTED);
            break;
            
        case HIGH:
            if(outState == LOW)
                return Pin(SHORTED);
            return Pin(HIGH);
            break;

        case PULLUP:
            if(outState == LOW)
                return Pin(LOW);
            if(outState == HIGH)
                return Pin(HIGH);
            if(outState == PULLDOWN)
                return Pin(TRISTATE); //any other idea?
            return Pin(PULLUP);
            break;

        case TRISTATE:
            //return Pin(outState);
            return *this;
            break;

        case PULLDOWN:
            if(outState == LOW)
                return Pin(LOW);
            if(outState == HIGH)
                return Pin(HIGH);
            if(outState == PULLUP)
                return Pin(TRISTATE); //any other idea?
            return Pin(PULLDOWN);
            break;

        case LOW:
            if(outState == HIGH)
                return Pin(SHORTED);
            return Pin(LOW);
            break;

        case ANALOG:
            if(outState != TRISTATE)
                return Pin(ANALOG_SHORTED);
            //outstate is TRISTATE and we have an anlog value so return pin ANALOG and value set
            return p; 
            break;

        case ANALOG_SHORTED:
            return Pin(ANALOG_SHORTED);

    }
    return Pin(TRISTATE);   //never used
}

#ifndef DISABLE_OPENDRAIN

Pin OpenDrain::operator+= (const Pin& p) {
    *pin= *this+p;
    return *this;
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

OpenDrain::operator bool() const
{ 
    return 0;
}

#endif
