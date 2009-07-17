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
#ifndef HWMEGAX8TIMERIRQ
#define HWMEGAX8TIMERIRQ

#include "rwmem.h"

#include "hardware.h"
class HWIrqSystem;

class HWMegaX8TimerIrq: public Hardware {
	protected:
		HWIrqSystem *irqSystem;

		unsigned char timsk;
		unsigned char tifr;
		unsigned char etimsk;
		unsigned char etifr;

		unsigned int vectorOvf0; 

		unsigned int vectorOvf;
		unsigned int vectorCompA;
		unsigned int vectorCompB;

	public:
		HWMegaX8TimerIrq(	AvrDevice*		core,
							HWIrqSystem*	irqSys,
							unsigned int	ovfVect,
							unsigned int	compAVect,
							unsigned int	compBVect
							);
		unsigned char GetTimsk();
		unsigned char GetTifr();

		void SetTimsk(unsigned char);
		void SetTifr(unsigned char);
		
		void AddFlagToTifr(unsigned char val);

		void ClearIrqFlag(unsigned int vector);
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);

        IOReg<HWMegaX8TimerIrq>
            timsk_reg,
            tifr_reg;
};

#endif
