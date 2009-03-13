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

#ifndef HWPORT
#define HWPORT

#include "hardware.h"
#include "pin.h"
#include "rwmem.h"

#include <string>

using namespace std;

class HWPort: public Hardware {
    protected:
        string myName;

        unsigned char port;
        unsigned char pin;
        unsigned char ddr;

        unsigned char alternateDdr;
        unsigned char useAlternateDdr;

        unsigned char alternatePort;
        unsigned char useAlternatePort;

        unsigned char useAlternatePortIfDdrSet; //special case for the ocr1a&b is selected on pin
        //which only be send to pin if ddr is set to output
        Pin p[8];

    public:
        void CalcOutputs();  //Calculate the new output value to be transmitted to the environment
        string GetPortString();
        ~HWPort() {}

        //Example:
        //if the UART Tx will be enabled, the UART set alternate DDR to output, useAltDdr to 1 and sets port according to Tx Pin value
        //thats all :-)


    public:
        HWPort(AvrDevice *core, const string &name);
        void Reset();
        void SetPort(unsigned char val) { port=val; CalcOutputs();}
        void SetDdr(unsigned char val) { ddr=val;CalcOutputs();}
        Pin& GetPin(unsigned char pinNo) ;
        void CalcPin();

        unsigned char GetPort() { return port; }
        unsigned char GetDdr() { return ddr;}
        unsigned char GetPin() { /*CalcPin(); */ return pin; } 

        friend class PinAtPort;
};


class RWPort : public RWMemoryMembers {
    protected:
        HWPort* hwport;
    public:
        RWPort(AvrDevice *c,  HWPort *port): RWMemoryMembers(c), hwport(port) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWPin : public RWMemoryMembers {
    protected:
        HWPort* hwport;
    public:
        RWPin(AvrDevice *c,  HWPort *port) : RWMemoryMembers(c), hwport(port){}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWDdr : public RWMemoryMembers {
    protected:
        HWPort* hwport;
    public:
        RWDdr(AvrDevice *c,  HWPort *port): RWMemoryMembers(c), hwport(port) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

#endif
