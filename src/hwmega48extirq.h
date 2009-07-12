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
#ifndef HWMEGA48EXTIRQ
#define HWMEGA48EXTIRQ
#include <vector>
#include "hardware.h"
#include "pinatport.h"
#include "rwmem.h"
#include "pinnotify.h"

class HWMega48ExtIrq: public Hardware, public HasPinNotifyFunction {
	protected:
        unsigned char eimsk;
        unsigned char eifr;
        unsigned char eicra;
        unsigned char eicrb;
        
		HWIrqSystem *irqSystem;

		bool int_old[2];

		//PinAtPort pinI[8];
		std::vector<PinAtPort> pinI;

		//unsigned int vectorInt[8];
		std::vector<unsigned int> vectorInt; 

	public:
		HWMega48ExtIrq(AvrDevice *core, HWIrqSystem *, 
                PinAtPort p0, PinAtPort p1,
                unsigned int, unsigned int);

		unsigned char GetEimsk();
		unsigned char GetEifr();
		void SetEimsk(unsigned char val);
		void SetEifr(unsigned char val);

		void SetEicra(unsigned char val);
		void SetEicrb(unsigned char val);
        unsigned char GetEicra();
        unsigned char GetEicrb();

		//bool IsIrqFlagSet(unsigned int vector);
		void ClearIrqFlag(unsigned int vector);
		unsigned int CpuCycle();
        void CheckForNewSetIrq(unsigned char);
        void CheckForNewClearIrq(unsigned char);
        void PinStateHasChanged(Pin*);
        void Reset();
};



class RWEicra48: public RWMemoryMembers {
    protected:
        HWMega48ExtIrq *megaextirq;

    public:
        RWEicra48(AvrDevice *c, HWMega48ExtIrq *_irq): RWMemoryMembers(c), megaextirq(_irq) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};



class RWEicrb48: public RWMemoryMembers {
    protected:
        HWMega48ExtIrq *megaextirq;

    public:
        RWEicrb48(AvrDevice *c, HWMega48ExtIrq *_irq): RWMemoryMembers(c), megaextirq(_irq) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};



class RWEimsk48: public RWMemoryMembers {
    protected:
        HWMega48ExtIrq *megaextirq;

    public:
        RWEimsk48(AvrDevice *c, HWMega48ExtIrq *_irq): RWMemoryMembers(c), megaextirq(_irq) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};



class RWEifr48: public RWMemoryMembers {
    protected:
        HWMega48ExtIrq *megaextirq;

    public:
        RWEifr48(AvrDevice *c, HWMega48ExtIrq *_irq): RWMemoryMembers(c), megaextirq(_irq) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};



#endif
