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
 */
#ifndef HWTIMER01IRQ
#define HWTIMER01IRQ

#include "hardware.h"

class HWIrqSystem;

class HWTimer01Irq: public Hardware {
	protected:
		HWIrqSystem *irqSystem;

		unsigned char timsk;
		unsigned char tifr;

	unsigned int vectorCapt; 
	unsigned int vectorCompa; 
	unsigned int vectorCompb; 
	unsigned int vectorOvf1; 
	unsigned int vectorOvf0; 

	public:
	HWTimer01Irq(AvrDevice *core, HWIrqSystem *, unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int v5);
		unsigned char GetTimsk();
		unsigned char GetTifr();

		void SetTimsk(unsigned char);
		void SetTifr(unsigned char);
		void AddFlagToTifr(unsigned char val); // { tifr|=val; }
		//bool IsIrqFlagSet(unsigned int vector);
		void ClearIrqFlag(unsigned int vector);
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);
};

#endif
