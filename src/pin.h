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

#ifndef PIN
#define PIN
#include <cstddef>
#include <vector>

#include "pinnotify.h"

class Net;

#define REL_FLOATING_POTENTIAL 0.55

/** Notify a change on analog pin (for analog comparator) */
class AnalogSignalChange {
    public:
        virtual ~AnalogSignalChange() {}

        virtual void NotifySignalChanged(void)=0;
};

//! Implements "real" analog value as float.
/*! Problem is, that the Vcc level isn't
    normally not known and so it's not possible to calculate correct value. So, here
    the value is calculated, if GetAnalogValue method is called. If no analog value
    is set by SetAnalogValue method, a replacement value is calculated. An analog value
    set by GetAnalogValue method is valid till it's not rewritten by a "digital"
    replacement value. */
class AnalogValue {

    private:
        int  dState;   //!< digital state and validity of aValue
        float aValue;  //!< analog value from setA method or constructor (not checked to valid range!)

    public:
        enum {
            ST_GND,         //!< digital state, ground potential
            ST_FLOATING,    //!< floating potential, not connected or tristate, assumed as FLOATING_POTENTIAL
            ST_VCC,         //!< digital state, Vcc potential
            ST_ANALOG       //!< valid analog value between ground and Vcc (and included)
        };

        //! standard constructor, status is floating
        AnalogValue(void) { dState = ST_FLOATING; aValue = 0.0; }
        //! analog value constructor, set real analog value
        AnalogValue(float val) { dState = ST_ANALOG; aValue = val; }
        //! digital value constructor, set a digital state
        AnalogValue(int dig) { dState = dig; aValue = 0.0; }
#ifndef SWIG
        //! copy operator
        AnalogValue &operator= (const AnalogValue& a) { dState = a.dState; aValue = a.aValue; return *this; }
#endif
        //! set a digital state, see enum definition
        void setD(int dig) { dState = dig; aValue = 0.0; }
        int getD(void) const { return dState; }
        //! set analog value, no check to value range between ground and vcc
        void setA(float val) { dState = ST_ANALOG; aValue = val; }
        //! calculate real voltage potential, needs value of Vcc potential
        float getA(float vcc);
        //! get raw analog value (no calculation, just content of aValue
        float getRaw(void) const { return aValue; }
        //! test, if real analog value is available
        bool analogValid(void) const { return dState == ST_ANALOG; }
};

//! Pin class, handles input and output to external parts
/*! This isn't a simple electrical point with a electrical potential. Pin class
  simulates mostly complete Input/Output circuit. So you have a output stage and
  a input state. Such a pin is connected by a net (see Net class) with other pins.
  Attention! The variable outState isn't the electrical state of a pin, it's only the
  state of the output stage. Only in case of no connected Net instance (aka no physical
  connection to other sink / source) it's also the real pin state! */
class Pin {
    
    protected:
        unsigned char *pinOfPort; //!< points to HWPort::pin or NULL
        unsigned char mask; //!< byte mask for HWPort::pin
        AnalogValue analogVal; //!< "real" analog voltage value

        Net *connectedTo; //!< the connection to other pins (NULL, if not connected)

    public:

        //! Possible PIN states.
        /*! This are the discrete states of output stage and input value.
          \warning Please do not change the order of these values without
          thinking twice, as for example the simulavr VPI interface depends
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

        T_Pinstate outState; //!< discrete value of output stage
        std::vector<HasPinNotifyFunction*> notifyList; //!< listeners for change of input value

        Pin(void); //!< common constructor, initial output state is tristate
        Pin(const Pin& p); //!< copy constructor, copy values but no refs to Net or HWPort
        Pin(T_Pinstate ps); //!< copy constructor from pin state
        Pin(unsigned char *parentPin, unsigned char mask); //!< constructor for a port pin
        Pin(float analog); //!< constructor for analog pin
        virtual ~Pin(); //!< pin destructor, breaks save connection to other pins, if necessary
        
#ifndef SWIG
        operator char() const; //!< return char representation for output stage
        virtual Pin &operator= (char); //!< set output stage to (digital) state, set value for ANALOG state separately
        virtual operator bool() const; //!< return boolean state of output stage
        virtual Pin operator+ (const Pin& p); //!< calculate common state from 2 connected pins
        virtual Pin operator+= (const Pin& p); //!< calculate common state from connected other pin to this pin
#endif

        virtual void SetInState(const Pin &p); //!< handles the input value from net
        virtual void RegisterNet(Net *n); //!< registers Net instance on pin
        virtual void UnRegisterNet(Net *n); //!< deletes Net instance registration for pin
        virtual Pin GetPin(void) { return *this;} //!< "cast method" to get back a Pin instance
        int GetAnalog(void); //!< Get analog value as integer from 0 to INT_MAX (for backward compatibility, will be deprecated later)
        float GetRawAnalog(void) const { return analogVal.getRaw(); } //!< get back raw analog value (just variable content!)
        float GetAnalogValue(float vcc) { return analogVal.getA(vcc); } //!< Returns real analog input value of pin
        Pin& SetAnalogValue(float value);  //!< Sets the pin to an real analog value
        void SetRawAnalog(float value) { analogVal.setA(value); }
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

//! Open drain Pin class, a special pin with open drain behavior
class OpenDrain: public Pin {
    protected:
        Pin *pin;        // the connected pin, which control input

    public:
        OpenDrain(Pin *p);
        virtual Pin GetPin();
};

#endif
