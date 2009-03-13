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

#ifndef HWAD
#define HWAD

#include "hardware.h"
#include "avrdevice.h"
#include "pinatport.h"
#include "rwmem.h"

class HWAdmux: public Hardware {
    protected:
        unsigned char admux;
        PinAtPort ad[6]; //6 pins selectable from the mux
        AvrDevice *core;

    public:
        HWAdmux(AvrDevice *c, PinAtPort ad0, PinAtPort ad1, PinAtPort ad2, PinAtPort ad3, PinAtPort ad4, PinAtPort ad5);
        unsigned char GetAdmux();
        void SetAdmux(unsigned char);
        //unsigned int CpuCycle(); //not used!
        void Reset();
        int GetMuxOutput(); //give the analog from the selcted pin
};

class HWAd: public Hardware {
    protected:
        unsigned char adch;
        bool adchLocked;
        unsigned char adcl;
        int adSample;
        unsigned char adcsr;
        AvrDevice *core;
        HWAdmux *admux;
        HWIrqSystem *irqSystem;
        PinAtPort aref;
        unsigned int irqVec;

        bool usedBefore; //adc must initialzed ?
        unsigned char prescaler;
        unsigned char clk;



        enum T_State {
            IDLE,
            INIT,
            RUNNING,
        } state;


        


    public:
        HWAd(AvrDevice *core, HWAdmux *m, HWIrqSystem *, PinAtPort aref, unsigned int irqVec) ;
        unsigned int CpuCycle();

        unsigned char GetAdch();
        unsigned char GetAdcl();
        unsigned char GetAdcsr();
        void Reset();
        void SetAdcsr(unsigned char);
        //bool IsIrqFlagSet(unsigned int vec);
        void ClearIrqFlag(unsigned int vec);

};

class RWAdmux: public RWMemoryMembers {
    protected:
        HWAdmux *admux;

    public:
        RWAdmux(AvrDevice *c, HWAdmux *a): RWMemoryMembers(c), admux(a) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWAdch: public RWMemoryMembers {
    protected:
        HWAd *ad;

    public:
        RWAdch(AvrDevice *c,  HWAd *a): RWMemoryMembers(c), ad(a) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWAdcl: public RWMemoryMembers {
    protected:
        HWAd *ad;

    public:
        RWAdcl(AvrDevice *c,  HWAd *a): RWMemoryMembers(c), ad(a) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

class RWAdcsr: public RWMemoryMembers {
    protected:
        HWAd *ad;

    public:
        RWAdcsr(AvrDevice *c,  HWAd *a):RWMemoryMembers(c), ad(a) {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};

#endif

