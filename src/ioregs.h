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

#ifndef IOREGS
#define IOREGS

#include "traceval.h"
#include "avrdevice.h"

/** Used for RAMPZ and MCUCR registers. */
class AddressExtensionRegister: public Hardware, public TraceValueRegister {
  
    private:
        unsigned char reg_val;
        unsigned char reg_mask;

    public:
        AddressExtensionRegister(AvrDevice *core, const std::string &regname, unsigned bitsize);
        void Reset() { reg_val = 0; }
        unsigned char GetRegVal() { return reg_val; }
        void SetRegVal(unsigned char val) { reg_val = val & reg_mask; }

        IOReg<AddressExtensionRegister> ext_reg;
};

#endif
