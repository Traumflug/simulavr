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

#ifndef HWSTACK
#define HWSTACK

#include "rwmem.h"
#include "hardware.h"
#include "funktor.h"
#include "avrdevice.h"

#include <map>


/*! Implementation of the special three-level deep hardware stack which is
  not accessible in any memory space on the devices which have this, for
  example the ATTiny15L or the good old AT90S1200. */
class ThreeLevelStack : public MemoryOffsets {
 public:
    ThreeLevelStack() : MemoryOffsets(0, 0) {
		rwHandler=(RWMemoryMembers**)malloc(sizeof(RWMemoryMembers*) * 6);
		for (size_t i=0; i < 6; i++) {
		    rwHandler[i]=new RWMemoryWithOwnMemory(0);
		}
    }
    ~ThreeLevelStack() {
	free(rwHandler);
    }
};


class HWStack: public Hardware {
	protected:
        AvrDevice *core;
		MemoryOffsets *mem;
		unsigned int stackPointer;
        unsigned int stackCeil;
        multimap<unsigned int , Funktor* > breakPointList; //later the second parameter should be a function Pointer!

	public:
        /*!Ceil gives the maximum value (+1) for the stack pointer, in smaller devices
	  there are not all 16 bits available!
	  Ceil does not need to be a power of two.
	  */
	HWStack(AvrDevice *core, MemoryOffsets *sr, unsigned int ceil);
		void Push(unsigned char val);
		unsigned char Pop();

		void SetSpl(unsigned char);
		void SetSph(unsigned char);
		unsigned char GetSpl();
		unsigned char GetSph();
		void Reset();
        ~HWStack() {}
        unsigned int GetStackPointer() const { return stackPointer; }
        void SetBreakPoint(unsigned int stackPointer, Funktor *);
        void CheckBreakPoints();
};

class RWSph: public RWMemoryMembers {
	protected: 
		HWStack *hwstack;
	public:
		RWSph(AvrDevice *c, HWStack *stack): RWMemoryMembers(c), hwstack(stack){}
		virtual unsigned char operator=(unsigned char) ;
		virtual operator unsigned char() const;
};

class RWSphFake: public RWSph {
	public:
		RWSphFake(AvrDevice *c, HWStack *stack): RWSph(c, stack){}
		virtual unsigned char operator=(unsigned char) ;
		virtual operator unsigned char() const;
};

class RWSpl: public RWMemoryMembers {
	protected:
		HWStack *hwstack;
	public:
		RWSpl(AvrDevice *c, HWStack *stack): RWMemoryMembers(c), hwstack(stack){} 
		virtual unsigned char operator=(unsigned char) ;
		virtual operator unsigned char() const;
};
#endif
