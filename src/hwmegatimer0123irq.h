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

#ifndef HWMEGATIMER0123IRQ
#define HWMEGATIMER0123IRQ

#include "hardware.h"
class HWIrqSystem;

class HWMegaTimer0123Irq: public Hardware {
	protected:
		HWIrqSystem *irqSystem;

		unsigned char timsk;
		unsigned char tifr;
		unsigned char etimsk;
		unsigned char etifr;

	unsigned int vectorOvf0; 

	unsigned int vectorTimer0Comp;
	unsigned int vectorTimer0Ovf;

	unsigned int vectorTimer1CompA;
	unsigned int vectorTimer1CompB;
	unsigned int vectorTimer1CompC;
	unsigned int vectorTimer1Capt;
	unsigned int vectorTimer1Ovf;
	
	unsigned int vectorTimer2Comp;
	unsigned int vectorTimer2Ovf;

	unsigned int vectorTimer3CompA;
	unsigned int vectorTimer3CompB;
	unsigned int vectorTimer3CompC;
	unsigned int vectorTimer3Capt;
	unsigned int vectorTimer3Ovf;


	public:
	HWMegaTimer0123Irq(AvrDevice *core, HWIrqSystem *,
			unsigned int t0Comp,
			unsigned int t0Ovf,
			unsigned int t1compa,
			unsigned int t1compb,
			unsigned int t1compc,
			unsigned int t1capt,
			unsigned int t1ovf,
		   	unsigned int t2comp,
			unsigned int t2ovf,
			unsigned int t3compa,
			unsigned int t3compb,
			unsigned int t3compc,
			unsigned int t3capt,
			unsigned int t3ovf

			);
		unsigned char GetTimsk();
		unsigned char GetTifr();
		unsigned char GetEtimsk();
		unsigned char GetEtifr();

		void SetTimsk(unsigned char);
		void SetTifr(unsigned char);
		void SetEtimsk(unsigned char);
		void SetEtifr(unsigned char);
		
		void AddFlagToTifr(unsigned char val); //{ tifr|=val; }
		void AddFlagToEtifr(unsigned char val); //{ etifr|=val; }
		//bool IsIrqFlagSet(unsigned int vector);
		void ClearIrqFlag(unsigned int vector);
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);
        void CheckForNewSetIrqE(unsigned char);
        void CheckForNewClearIrqE(unsigned char);
};

#endif
