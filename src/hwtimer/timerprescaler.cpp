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

#include "timerprescaler.h"
#include "traceval.h"

HWPrescaler::HWPrescaler(AvrDevice *core, const std::string &tracename):
    Hardware(core),
    _resetBit(-1),
    _resetSyncBit(-1),
    countEnable(true)
{
    core->AddToCycleList(this);
    trace_direct(&(core->coreTraceGroup), "PRESCALER" + tracename, &preScaleValue);
    resetRegister = NULL;
}

HWPrescaler::HWPrescaler(AvrDevice *core,
                         const std::string &tracename,
                         IOSpecialReg *ioreg,
                         int resetBit):
    Hardware(core),
    _resetBit(resetBit),
    _resetSyncBit(-1),
    countEnable(true)
{
    core->AddToCycleList(this);
    trace_direct(&(core->coreTraceGroup), "PRESCALER" + tracename, &preScaleValue);
    resetRegister = ioreg;
    ioreg->connectSRegClient(this);
}

HWPrescaler::HWPrescaler(AvrDevice *core,
                         const std::string &tracename,
                         IOSpecialReg *ioreg,
                         int resetBit,
                         int resetSyncBit):
    Hardware(core),
    _resetBit(resetBit),
    _resetSyncBit(resetSyncBit),
    countEnable(true)
{
    core->AddToCycleList(this);
    trace_direct(&(core->coreTraceGroup), "PRESCALER" + tracename, &preScaleValue);
    resetRegister = ioreg;
    ioreg->connectSRegClient(this);
}

unsigned char HWPrescaler::set_from_reg(const IOSpecialReg *reg, unsigned char nv) {
    // check, if this is the right register
    if(reg != resetRegister) return nv;
    // extract corresponding reset bit
    int reset = (1 << _resetBit) & nv;
    // extract reset sync bit, if available
    int sync = 0;
    if(_resetSyncBit >= 0)
        sync = (1 << _resetSyncBit) & nv;
    
    if(reset) {
        Reset();  // reset requested
        if(sync)
            countEnable = false; // sync asserted, stop counting
        else {
            countEnable = true;  // let the counter run
            return ~(1 << _resetBit) & nv; // reset the reset bit immediately, if no sync asserted
        }
    }
    return nv;  // return value unchanged
}

HWPrescalerAsync::HWPrescalerAsync(AvrDevice *core,
                                   const std::string &tracename,
                                   PinAtPort tosc,
                                   IOSpecialReg *asyreg,
                                   int clockSelBit,
                                   IOSpecialReg *ioreg,
                                   int resetBit):
    HWPrescaler(core, tracename, ioreg, resetBit),
    tosc_pin(tosc),
    clockSelectBit(clockSelBit)
{
    asyncRegister = asyreg;
    asyreg->connectSRegClient(this);
    pinstate = tosc_pin.GetPin();
    clockselect = false;
}

HWPrescalerAsync::HWPrescalerAsync(AvrDevice *core,
                                   const std::string &tracename,
                                   PinAtPort tosc,
                                   IOSpecialReg *asyreg,
                                   int clockSelBit,
                                   IOSpecialReg *ioreg,
                                   int resetBit,
                                   int resetSyncBit):
    HWPrescaler(core, tracename, ioreg, resetBit, resetSyncBit),
    tosc_pin(tosc),
    clockSelectBit(clockSelBit)
{
    asyncRegister = asyreg;
    asyreg->connectSRegClient(this);
    pinstate = tosc_pin.GetPin();
    clockselect = false;
}

unsigned int HWPrescalerAsync::CpuCycle() {
    bool e = true;
    if(clockselect) {
      bool ps = tosc_pin.GetPin();
      if(pinstate || !ps) e = false; // count on positive edge!
      pinstate = ps;
    }
    if(e && countEnable) {
      preScaleValue++;
      if(preScaleValue > 1023) preScaleValue = 0;
    }
    return 0;
}

unsigned char HWPrescalerAsync::set_from_reg(const IOSpecialReg *reg, unsigned char nv) {
    unsigned char v = HWPrescaler::set_from_reg(reg, nv);
    if(reg != asyncRegister) return v;
    if((1 << clockSelectBit) & v) {
        clockselect = true;
        //tosc_pin.SetAlternatePort(true);
    } else {
        clockselect = false;
        //tosc_pin.SetAlternatePort(false);
    }
    return v;
}

