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
#ifndef HWSTACK
#define HWSTACK

#include "rwmem.h"
#include "hardware.h"

class HWStack: public Hardware {
	protected:
		MemoryOffsets *mem;
		unsigned int stackPointer;
        unsigned int stackMask;

	public:
        //the mask give the max width of the stack pointer, in smaller devices there are not all 16 bits available!
		HWStack(AvrDevice *core, MemoryOffsets *sr, unsigned int mask);
		void Push(unsigned char val);
		unsigned char Pop();

		void SetSpl(unsigned char);
		void SetSph(unsigned char);
		unsigned char GetSpl();
		unsigned char GetSph();
		void Reset();
        ~HWStack() {}
};

class RWSph: public RWMemoryMembers {
	protected: 
		HWStack *hwstack;
	public:
		RWSph(HWStack *stack); // { hwstack=stack;}
		virtual unsigned char operator=(unsigned char) ;
		virtual operator unsigned char() const;
};

class RWSpl: public RWMemoryMembers {
	protected:
		HWStack *hwstack;
	public:
		RWSpl(HWStack *stack); // { hwstack = stack; }
		virtual unsigned char operator=(unsigned char) ;
		virtual operator unsigned char() const;
};
#endif
