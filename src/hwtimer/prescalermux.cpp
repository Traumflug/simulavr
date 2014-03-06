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

#include "prescalermux.h"
#include "avrerror.h"

PrescalerMultiplexer::PrescalerMultiplexer(HWPrescaler *ps):
    prescaler(ps) {}

bool PrescalerMultiplexer::isClock(unsigned int cs) {
    unsigned short pv = prescaler->GetValue();
    
    switch(cs) {
    case 0: // no clock, counter stop
        return false;
      
    case 1: // every clock
        return true;
      
    case 2: // CKx8
        return (bool)((pv % 8) == 0);
      
    case 3: // CKx32
        return (bool)((pv % 32) == 0);
      
    case 4: // CKx64
        return (bool)((pv % 64) == 0);
      
    case 5: // CKx128
        return (bool)((pv % 128) == 0);
      
    case 6: // CKx256
        return (bool)((pv % 256) == 0);
      
    case 7: // CKx1024
        return (bool)((pv % 1024) == 0);
      
    default:
        avr_error("wrong prescaler multiplex value: %d", cs);
    }
}

PrescalerMultiplexerExt::PrescalerMultiplexerExt(HWPrescaler *ps, PinAtPort pi):
    PrescalerMultiplexer(ps),
    clkpin(pi) {
    clkpin_old = (bool)(clkpin == 1);
}

bool PrescalerMultiplexerExt::isClock(unsigned int cs) {
    bool current = (bool)(clkpin == 1);
  
    switch(cs) {
    case 0: // no clock, counter stop
        return false;
      
    case 1: // every clock
        return true;
      
    case 2: // CKx8
        return (bool)((prescaler->GetValue() % 8) == 0);
      
    case 3: // CKx64
        return (bool)((prescaler->GetValue() % 64) == 0);
      
    case 4: // CKx256
        return (bool)((prescaler->GetValue() % 256) == 0);
      
    case 5: // CKx1024
        return (bool)((prescaler->GetValue() % 1024) == 0);
      
    case 6: // pin falling edge
        if(current == clkpin_old) return false; // no change
        clkpin_old = current;
        return (bool)(current == false); // old = true, current = false
      
    case 7: // pin rising edge
        if(current == clkpin_old) return false; // no change
        clkpin_old = current;
        return (bool)(current == true); // old = false, current = true
      
    default:
        avr_error("wrong prescaler multiplex value: %d", cs);
    }
}

PrescalerMultiplexerT15::PrescalerMultiplexerT15(HWPrescaler *ps):
    PrescalerMultiplexer(ps) {}

bool PrescalerMultiplexerT15::isClock(unsigned int cs) {
    avr_warning("method not implemented");
    return false;
}

