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

#ifndef HWEXTIRQ
#define HWEXTIRQ

#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"
#include "pinnotify.h"


class HWIrqSystem;


class HWExtIrq: public Hardware, public HasPinNotifyFunction {
	protected:
		unsigned char gimsk;
		unsigned char gifr;
		unsigned char mcucrCopy;

		HWIrqSystem *irqSystem;

		bool int0_old;
		bool int1_old;

		PinAtPort pinI0;
		PinAtPort pinI1;

		unsigned int vectorInt0;
		unsigned int vectorInt1;

	public:
		HWExtIrq(AvrDevice *core, HWIrqSystem *, PinAtPort p0, PinAtPort p2, unsigned int, unsigned int);
		unsigned char GetGimsk();
		unsigned char GetGifr();
		void SetGimsk(unsigned char val);
		void SetGifr(unsigned char val);
		void SetMcucrCopy(unsigned char val);
		//bool IsIrqFlagSet(unsigned int vector);
		void ClearIrqFlag(unsigned int vector);
		unsigned int CpuCycle();
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);

        void PinStateHasChanged(Pin *);
        void Reset();

        IOReg<HWExtIrq>
            gimsk_reg,
            gifr_reg;
};

#endif
