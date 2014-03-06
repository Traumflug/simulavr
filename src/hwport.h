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

#ifndef HWPORT
#define HWPORT

#include "hardware.h"
#include "pin.h"
#include "rwmem.h"
#include "traceval.h"

#include <string>

//! Defines a Port, e.g. a hardware device for GPIO
/*! Example for use alternateDdr and useAlternateDdr:
  If the UART Tx will be enabled, the UART set alternateDdr to output,
  useAlternateDdr to 1 and sets port according to Tx Pin value, thats all :-)
  
  useAlternatePortIfDdrSet: special case for the OCR outputs, which only be
  connected to pin if ddr is set to output! */
class HWPort: public Hardware, public TraceValueRegister {
    
    private:
        void CalcPin(void); //!< calculating the value for register "pin" from the Pin p[] array
        
    protected:
        std::string myName; //!< the "name" of the port

        unsigned char port; //!< port output register
        unsigned char pin;  //!< port input register
        unsigned char ddr;  //!< data direction register

        unsigned char alternateDdr; //!< data direction register for special functions
        unsigned char useAlternateDdr; //!< bit mask, which bit in alternateDdr is used

        unsigned char alternatePort; //!< output register for special functions
        unsigned char useAlternatePort; //!< bit mask, which bit in alternatePort is used

        /*! special case for the ocr1a&b is selected on pin, which only be send
          to pin if ddr is set to output */
        unsigned char useAlternatePortIfDdrSet; 
        
        Pin p[8]; //!< the port pins, e.g. the final IO stages
        TraceValue* pintrace[8]; //!< trace channel to trace output driver state
        unsigned int portSize; //!< how much bits does this port have [1..8]
        unsigned char portMask; //!< mask out unused bits, if necessary
        bool portToggleFeature; //!< controls functionality of SetPin method (write to PIN toggles port register)
        
    public:
        HWPort(AvrDevice *core, const std::string &name, bool portToggle = false, int size = 8);
        ~HWPort();
        
        void CalcOutputs(void);  //!< Calculate the new output value to be transmitted to the environment
        std::string GetPortString(void); //!< returns a string representation of output states
        void Reset(void);
        std::string GetName(void) { return myName; } //!< returns the port name as given in constructor
        Pin& GetPin(unsigned char pinNo); //!< returns a pin reference of pin with pin number
        int GetPortSize(void) { return portSize; } //!< returns, how much bits this port controls
        
        void SetPort(unsigned char val) { port = val & portMask; CalcOutputs(); } //!< setter method for port register
        void SetDdr(unsigned char val) { ddr = val & portMask; CalcOutputs(); } //!< setter method for data direction register
        void SetPin(unsigned char val); //!< setter method for PIN register (for new devices with toggle port)
        unsigned char GetPort(void) { return port; } //!< getter method for port register
        unsigned char GetDdr(void) { return ddr; } //!< getter method for data direction register
        unsigned char GetPin(void) { return pin; }  //!< getter method for PIN register

        friend class PinAtPort;

        IOReg<HWPort>
            port_reg,
            pin_reg,
            ddr_reg;
};

#endif
