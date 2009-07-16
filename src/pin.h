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

class NetInterface;
class Net;
class OpenDrain;


/*! This constant is used to prevent memory leaks for pins when using
 simulavrxx as a Verilog device. It is an ugly workaround for a memory leak
 that should be properly fixed one day. If the contents of this variable is
 non-null, MirrorNets of pins will be destroyed when the pins they are
 connected to disappears. If this is not done, a lot of memory will be leaked
 in verilog mode. Unfortunately, enabling this for more complex
 Pin-Net-connection scenarios ignores assumptions by other parts of the AVR
 code and some tests will segfault.

 This is basically the patch commited in cvs-upstream @
 94df3ffc2226f99f739432db2166aae1b7c8c69f and reverted on the next morning
 in 854b6505abda34d9f16d3c6d21da1bf1a751d01f, but extended with this
 'safe-flag'.
 
 \todo FIX the memory leaks! */
extern bool pin_memleak_verilog_workaround;

class Pin {
    
    protected:
        unsigned char *pinOfPort; //points to HWPort::pin or 0L
        unsigned char mask;
        int analogValue;

        NetInterface *connectedTo;

        /*! iff mynet is true, the NetInterface connectedTo belongs to this
          PIN and must be memory-managed by this PIN! */
        bool myNet;
        
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
        Pin( unsigned char *parentPin, unsigned char mask); 
#ifndef SWIG
        operator char() const;
        virtual Pin &operator= (char);
        virtual operator bool() const;
        virtual Pin operator+ (const Pin& p);
        virtual Pin operator+= (const Pin& p);
#endif
        virtual void RegisterNet(Net *n);
        virtual Pin GetPin() { return *this;}
        virtual ~Pin();
        //T_Pinstate GetOutState();
        int GetAnalog() const;
        void RegisterCallback( HasPinNotifyFunction *);


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
