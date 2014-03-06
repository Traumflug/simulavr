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

#ifndef HWWADO
#define HWWADO

#include "hardware.h"
#include "rwmem.h"
#include "systemclocktypes.h"
#include "traceval.h"

class AvrDevice;
class HWIrqSystem;

/** Watchdog (WDT) peripheral. Interrupts are not implemented. */
class HWWado: public Hardware, public TraceValueRegister {
	protected:
	unsigned char wdtcr;
	unsigned char cntWde; //4 cycles counter for unsetting the wde
	SystemClockOffset timeOutAt; 
	AvrDevice *core;

	public:
		HWWado(AvrDevice *); // { irqSystem= s;}
		virtual unsigned int CpuCycle();

		void SetWdtcr(unsigned char val);  
		unsigned char GetWdtcr() { return wdtcr; }
		void Wdr(); //reset the wado counter
		void Reset();

        IOReg<HWWado> wdtcr_reg;
};


#endif
