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

float AnalogValue::getA(float vcc) {
    switch(dState) {
        case ST_GND:
            return 0.0;
        case ST_FLOATING:
            return REL_FLOATING_POTENTIAL * vcc;
        case ST_VCC:
            return vcc;
        case ST_ANALOG:
            // check for valid value range
            if(aValue < 0.0)
                return 0.0;
            if(aValue > vcc)
                return vcc;
            return aValue;
    }
}

int Pin::GetAnalog(void) const {
    switch (outState) {
        case ANALOG: 
            return analogValue; // reflect that we are self a analog value source

        case HIGH:
        case PULLUP:
            return INT_MAX;

        case TRISTATE:          // if we are input, then we read the preset analog value
            return analogValue;

        case LOW:
        case PULLDOWN:
            return 0;

        default:
            return 0;
    }
}

float Pin::GetAnalogValue(float vcc) {
    switch (outState) {
        case ANALOG:
            return analogVal.getA(vcc); // reflect that this instance deliver an analog value as output

        case HIGH:
        case PULLUP:
            return vcc;

        case TRISTATE:
            // if we are input, we read the analog value
            return analogVal.getA(vcc);

        case LOW:
        case PULLDOWN:
            return 0.0;

        default:
            return 0.0;
    }
}

void Pin::RegisterCallback(HasPinNotifyFunction *h) {
    notifyList.push_back(h);
}

void Pin::SetInState(const Pin &p) { 
    analogValue = p.analogValue;
    analogVal = p.analogVal;

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
    mask = 0;
    
    outState = ps;

    // Initialization of analog value
    switch (ps) {
        case HIGH: 
        case PULLUP:
            analogValue = INT_MAX; 
            analogVal.setD(AnalogValue::ST_VCC);
            break;

        case LOW:
        case PULLDOWN:
            analogValue = 0;
            analogVal.setD(AnalogValue::ST_GND);
            break;

        case TRISTATE:
            analogValue = TRISTATE_ANALOG_VALUE;
            analogVal.setD(AnalogValue::ST_FLOATING);
            break;

        default:
            analogValue = 0;
            analogVal.setD(AnalogValue::ST_GND); // is this right? Which cases use this?
            break;
    }
}

Pin::Pin() { 
    pinOfPort = 0; 
    connectedTo = NULL;
    mask = 0;
    
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
    mask = 0;
    
    outState = p.outState;
    analogValue = p.analogValue;
    analogVal = p.analogVal;
}

Pin::Pin(float analog) {
    mask = 0;
    pinOfPort = 0;
    connectedTo = NULL;
    analogVal.setA(analog);

    outState = ANALOG;
    analogValue = TRISTATE_ANALOG_VALUE; // old analog state is wrong!
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
    return 'S'; // only default, should never be reached
}

Pin::operator bool() const {
    if((outState==HIGH) || (outState==PULLUP))
        return true;

    // maybe for TRISTATE not handled complete in simulavr... TODO
    if((outState==ANALOG) || (outState==TRISTATE)) {
        if((analogValue > (INT_MAX / 2)) ||
           analogVal.analogValid())         // this part of condition isn't really correct, because it depends on Vcc level
                                            // and this isn't known here!
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
            analogVal.setD(AnalogValue::ST_GND);
            break;
            
        case 'H':
            outState = HIGH;
            analogValue = INT_MAX;
            analogVal.setD(AnalogValue::ST_VCC);
            break;
            
        case 'h':
            outState = PULLUP;
            analogValue = INT_MAX;
            analogVal.setD(AnalogValue::ST_VCC);
            break;
            
        case 't':
            outState = TRISTATE;
            analogValue = TRISTATE_ANALOG_VALUE;
            analogVal.setD(AnalogValue::ST_FLOATING);
            break;
            
        case 'l':
            outState = PULLDOWN;
            analogValue = 0;
            analogVal.setD(AnalogValue::ST_GND);
            break;
            
        case 'L':
            outState = LOW;
            analogValue = 0;
            analogVal.setD(AnalogValue::ST_GND);
            break;
            
        case 'a':
            outState = ANALOG;
            analogValue = 0;
            analogVal.setD(AnalogValue::ST_FLOATING); // set to floating state, analog value, but not set
            break;
            
        case 'A':
            outState = ANALOG_SHORTED;
            analogValue = 0;
            analogVal.setD(AnalogValue::ST_GND);
            break;
    }

    CalcPin();

    return *this;
}

Pin& Pin::SetAnalog(int value) {
    // outState == ANALOG?
    analogValue = value;

    CalcPin();

    return *this;
}

Pin& Pin::SetAnalogValue(float value) {
    analogVal.setA(value);

     CalcPin();

     return *this;
 }

Pin Pin::operator+= (const Pin& p) {
    *this = *this + p;
    return *this;
}

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
                return Pin(TRISTATE); // any other idea?
            return Pin(PULLUP);
            break;

        case TRISTATE:
            return *this;
            break;

        case PULLDOWN:
            if(outState == LOW)
                return Pin(LOW);
            if(outState == HIGH)
                return Pin(HIGH);
            if(outState == PULLUP)
                return Pin(TRISTATE); // any other idea?
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
            // outState is TRISTATE and we have an analog value so return pin ANALOG and value set
            return p; 
            break;

        case ANALOG_SHORTED:
            return Pin(ANALOG_SHORTED);

    }
    return Pin(TRISTATE);   // never used
}

Pin OpenDrain::GetPin() {
    // get back state of output side
    bool input = (bool)*pin;
    if(input)
        return Pin(LOW);
    else
        return Pin(TRISTATE);
}

OpenDrain::OpenDrain(Pin *p) {
    pin = p;
}
