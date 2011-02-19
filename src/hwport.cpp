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

#include <iostream>
using namespace std;

#include "hwport.h"
#include "avrdevice.h"
#include "avrerror.h"
#include <assert.h>

HWPort::HWPort(AvrDevice *core, const string &name, bool portToggle, int size):
    Hardware(core),
    TraceValueRegister(core, "PORT" + name),
    myName(name),
    portToggleFeature(portToggle),
    port_reg(this, "PORT",
             this, &HWPort::GetPort, &HWPort::SetPort),
    pin_reg(this, "PIN",
            this, &HWPort::GetPin, &HWPort::SetPin),
    ddr_reg(this, "DDR",
            this, &HWPort::GetDdr, &HWPort::SetDdr)
{
    if(size > 8 || size < 1)
        size = 8;
    portSize = size;
    portMask = (unsigned char)((1 << size) - 1);
    
    Reset();

    for(int tt = 0; tt < portSize; tt++) { 
        string dummy = name + (char)('0' + tt);
        core->RegisterPin(dummy, &p[tt]);
        p[tt].mask = 1 << tt;
        p[tt].pinOfPort= &pin;
    }
}

void HWPort::Reset(void) {
    port = 0;
    pin = 0;
    ddr = 0;

    alternateDdr = 0;
    useAlternateDdr = 0;

    alternatePort = 0;
    useAlternatePort = 0;

    useAlternatePortIfDdrSet = 0;
    
    CalcOutputs();
}

Pin& HWPort::GetPin(unsigned char pinNo) {
    return p[pinNo];
}

void HWPort::CalcPin(void) {
    //calculating the value for register "pin" from the Pin p[] array
    pin = 0;
    for(int tt = 0; tt < portSize; tt++) {
        if(p[tt].CalcPin()) pin |= (1 << tt);
    }
}

void HWPort::CalcOutputs(void) { //Calculate the new output value to be transmitted to the environment
    unsigned char actualBit = 0x01;
    unsigned char actualBitNo = 0;

    do { //calc all bits
        unsigned char workingPort = 0;
        unsigned char workingDdr = 0;

        if(useAlternatePortIfDdrSet & actualBit) {
            if(ddr & actualBit) {
                workingPort |= alternatePort & actualBit;
                workingDdr |= actualBit;
            } else
                workingPort |= port & actualBit;
        } else {
            if(useAlternateDdr & actualBit)
                workingDdr |= alternateDdr & actualBit;
            else
                workingDdr |= ddr & actualBit;

            if(useAlternatePort & actualBit)
                workingPort |= alternatePort & actualBit;
            else
                workingPort |= port & actualBit;
        }

        if(workingDdr & actualBit) { //Ddr is output
            if(workingPort & actualBit) //Port is High
                p[actualBitNo].outState = Pin::HIGH;
            else
                p[actualBitNo].outState = Pin::LOW;
        } else { //ddr is low
            if(workingPort & actualBit)
                p[actualBitNo].outState = Pin::PULLUP;
            else 
                p[actualBitNo].outState = Pin::TRISTATE;
        }

        actualBit <<= 1;
        actualBitNo++;
        
    } while(actualBitNo < portSize); // as long as all bits are calculated
    
    CalcPin(); //now transfer the result also to all HWPort::pin instances
} //end of Calc()

string HWPort::GetPortString(void) {
    string dummy;
    dummy.resize(portSize);
    assert(portSize < sizeof(p)/sizeof(p[0]));
    for(int tt = 0; tt < portSize; tt++)
        dummy[portSize - 1 - tt] = p[tt];  // calls Pin::operator char()

    return dummy;
}

void HWPort::SetPin(unsigned char val) {
    if(portToggleFeature) {
        port ^= val;
        CalcOutputs();
    } else
        avr_warning("Writing of 'PORT%s.PIN' (with %d) is not supported.", myName.c_str(), val);
}

