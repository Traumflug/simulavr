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

#include "timerprescaler.h"
#include "traceval.h"

HWPrescaler::HWPrescaler(AvrDevice *core, const std::string &tracename):
    Hardware(core),
    countEnable(true),
    _resetBit(-1),
    _resetSyncBit(-1)
{
    core->AddToCycleList(this);
    trace_direct(core, "PRESCALER" + tracename, &preScaleValue);
}

HWPrescaler::HWPrescaler(AvrDevice *core,
                         const std::string &tracename,
                         IOSpecialReg *ioreg,
                         int resetBit):
    Hardware(core),
    countEnable(true),
    _resetBit(resetBit),
    _resetSyncBit(-1)
{
    core->AddToCycleList(this);
    trace_direct(core, "PRESCALER" + tracename, &preScaleValue);
    ioreg->connectSRegClient(this);
}

HWPrescaler::HWPrescaler(AvrDevice *core,
                         const std::string &tracename,
                         IOSpecialReg *ioreg,
                         int resetBit,
                         int resetSyncBit):
    Hardware(core),
    countEnable(true),
    _resetBit(resetBit),
    _resetSyncBit(resetSyncBit)
{
    core->AddToCycleList(this);
    trace_direct(core, "PRESCALER" + tracename, &preScaleValue);
    ioreg->connectSRegClient(this);
}

unsigned char HWPrescaler::set_from_reg(unsigned char nv) {
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

