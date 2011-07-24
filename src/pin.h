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
#include <cstddef>
#include <vector>

#include "pinnotify.h"

/*! \todo OpenDrain class is disabled for the moment. I think, this functionality,
  to "wrap" a normal pin isn't right implemented and could be made more clear.
  And maybe it is useless, because to handle easily by normal Pin class. */
#define DISABLE_OPENDRAIN 1

class Net;
#ifndef DISABLE_OPENDRAIN
class OpenDrain;
#endif

//! Pin class, handles input and output to external parts
/*! This isn't a simple electrical point with a electrical potential. Pin class
  simulates mostly complete Input/Output circuit. So you have a output stage and
  a input state. Such a pin is connected by a net (see Net class) with other pins. */
class Pin {
    
    protected:
        unsigned char *pinOfPort; //!< points to HWPort::pin or NULL
        unsigned char mask; //!< byte mask for HWPort::pin
        int analogValue; //!< analog input value, from 0 to INT_MAX

        Net *connectedTo; //!< the connection to other pins (NULL, if not connected)

    public:

        //! Possible PIN states.
        /*! This are the discret states of output stage and input value.
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
        } T_Pinstate;

        T_Pinstate outState; //!< discret value of output stage
        std::vector<HasPinNotifyFunction*> notifyList; //!< listeners for change of input value

        Pin(void); //!< common constructor, initial output state is tristate
        Pin(const Pin& p); //!< copy constructor, copy values but no refs to Net or HWPort
#ifndef DISABLE_OPENDRAIN
        Pin(const OpenDrain &od); //!< copy constructor, if we take values from OpenDrain pin
#endif
        Pin(T_Pinstate ps); //!< copy constructor from pin state
        Pin(unsigned char *parentPin, unsigned char mask); //!< constructor for a port pin
        virtual ~Pin(); //!< pin destructor, breaks save connection to other pins, if necessary
        
#ifndef SWIG
        operator char() const;
        virtual Pin &operator= (char);
        virtual operator bool() const;
        virtual Pin operator+ (const Pin& p);
        virtual Pin operator+= (const Pin& p);
#endif

        virtual void SetInState(const Pin &p); //!< handles the input value from net
        virtual void RegisterNet(Net *n); //!< registers Net instance on pin
        virtual void UnRegisterNet(Net *n); //!< deletes Net instance registration for pin
        virtual Pin GetPin(void) { return *this;} //!< "cast method" to get back a Pin instance
        int GetAnalog(void) const; //!< Returns analog input value of pin
        Pin& SetAnalog(int value);  //!< Sets the pin to an analog value
        void RegisterCallback(HasPinNotifyFunction *); //!< register a listener for input value change
        //! Update input values from output values
        /*! If there is no connection to other pins, then it will reflect the own
        output value to own input value. Otherwise it calls Net::CalcNet method */
        bool CalcPin(void);

        bool isPortPin(void) { return pinOfPort != NULL; } //!< True, if it's a port pin
        bool isConnected(void) { return connectedTo != NULL; } //!< True, if it's connected to other pins
        bool hasListener(void) { return notifyList.size() != 0; } //!< True, if there change listeners

        friend class HWPort;
        friend class Net;

};

#ifndef DISABLE_OPENDRAIN

//! Open drain Pin class, a special pin with open drain behavior
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

#endif
