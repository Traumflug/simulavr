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

#ifndef HWACOMP
#define HWACOMP

#include "hardware.h"
#include "rwmem.h"
#include "irqsystem.h"
#include "pinatport.h"
#include "pinnotify.h"
#include "traceval.h"

class HWAcomp: public Hardware, public HasPinNotifyFunction, public TraceValueRegister {
    protected:
        HWIrqSystem *irqSystem;
        PinAtPort pinAin0;
        PinAtPort pinAin1;

        unsigned char acsr;
        unsigned int irqVec;

    public:
        HWAcomp(AvrDevice *core, HWIrqSystem *irqsys, PinAtPort ain0, PinAtPort ain1, unsigned int irqVec);
        unsigned char GetAcsr();
        void SetAcsr(unsigned char val);
        void Reset(); 
        //bool IsIrqFlagSet(unsigned int vec);
        void ClearIrqFlag(unsigned int vec);
        void PinStateHasChanged(Pin *);
	IOReg<HWAcomp> acsr_reg;
};

#endif

