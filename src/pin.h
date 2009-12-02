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

#ifndef PIN
#define PIN
#include <vector>

#include "pinnotify.h"

class Net;
class OpenDrain;

class Pin {
    
    protected:
        unsigned char *pinOfPort; //points to HWPort::pin or 0L
        unsigned char mask;
        int analogValue;

        Net *connectedTo;

    public:

        Pin(const OpenDrain &od);

        /*! Possible PIN states.
          \warning Please do not change the order of these values without
          thinking twice, as for example the simulavrxx VPI interface depends
          on this/exports this to verilog. */
        typedef enum {
            LOW,
            HIGH,
            SHORTED,
            PULLUP,
            TRISTATE,
            PULLDOWN,
            ANALOG,
            ANALOG_SHORTED
        }T_Pinstate;

        T_Pinstate outState;
        std::vector<HasPinNotifyFunction*> notifyList;

    public:
        void SetOutState( T_Pinstate s);
        virtual void SetInState ( const Pin &p);
    
        Pin(T_Pinstate ps);
        Pin();
        Pin(const Pin& p);
        Pin( unsigned char *parentPin, unsigned char mask); 
#ifndef SWIG
        operator char() const;
        virtual Pin &operator= (char);
        virtual operator bool() const;
        virtual Pin operator+ (const Pin& p);
        virtual Pin operator+= (const Pin& p);
#endif
        virtual void RegisterNet(Net *n);
        virtual void UnRegisterNet(Net *n);
        virtual Pin GetPin() { return *this;}
        virtual ~Pin();
        int GetAnalog() const;
        void RegisterCallback( HasPinNotifyFunction *);
        bool CalcPin(void);

        friend class HWPort;
        friend class Net;

};

class OpenDrain: public Pin {
    protected:
        Pin *pin;

    public:
        OpenDrain(Pin *p) { pin=p;}
#ifndef SWIG
        virtual operator bool() const;
        virtual Pin operator+ (const Pin& p);
        virtual Pin operator+= (const Pin& p);
#endif
        virtual Pin GetPin();
        void RegisterNet(Net *n) { pin->RegisterNet(n);}
        virtual ~OpenDrain() {}
};

#endif
