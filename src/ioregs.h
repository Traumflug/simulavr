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
#ifndef IOREGS
#define IOREGS

#include "hwextirq.h"

class HWRampz: public Hardware {
    protected:
        unsigned char rampz;

    public:
        HWRampz(AvrDevice *core):Hardware(core) { Reset(); }
        void Reset(){rampz=0;}
        unsigned char GetRampz() { return rampz; }
        void SetRampz(unsigned char val) { rampz=val; }
};


class RWRampz: public RWMemoryMembers {
    protected:
        HWRampz* ad;
    public:
        RWRampz(HWRampz *a) { ad=a; }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

//----------------------------------------------------------------

class HWMcucr: public Hardware {
    protected:
        unsigned char mcucr;

    public:
        HWMcucr(AvrDevice *core ): Hardware(core) {
            mcucr=0;
        }
        unsigned char GetMcucr() { return mcucr; }
        void SetMcucr( unsigned char val) { mcucr=val; }
        virtual unsigned int CpuCycle();
};

class RWMcucr: public RWMemoryMembers {
    protected:
        HWMcucr *hwMcucr;
        HWExtIrq *hwExtIrq;
    public:
        RWMcucr(HWMcucr *m, HWExtIrq *e) { hwMcucr=m; hwExtIrq=e;}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};
#endif
