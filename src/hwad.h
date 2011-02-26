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
#include "traceval.h"

/** ADC multiplexer. This version does not handle differential inputs. */
class HWAdmux: public Hardware, public TraceValueRegister {
    protected:
        unsigned char admux;
        Pin* ad[8]; //8 pins selectable from the mux
        AvrDevice *core;

    public:
        HWAdmux(
           AvrDevice* c,
           Pin*  _ad0,
           Pin*  _ad1,
           Pin*  _ad2, 
           Pin*  _ad3,
           Pin*  _ad4,
           Pin*  _ad5,
           Pin*  _ad6,
           Pin*  _ad7
        );
        unsigned char GetAdmux();
        void SetAdmux(unsigned char);
        //unsigned int CpuCycle(); //not used!
        void Reset();
		/// Get analog voltage (0..INT_MAX=Vcc) of the selected pin
        int GetMuxOutput();
	IOReg<HWAdmux> admux_reg;
};

/** Analog-digital converter (ADC) */
class HWAd: public Hardware, public TraceValueRegister {
    protected:
        unsigned char adch;
        bool adchLocked;
        unsigned char adcl;
        int adSample;
        unsigned char adcsr;
        AvrDevice *core;
        HWAdmux *admux;
        HWIrqSystem *irqSystem;
        Pin& aref;
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
        HWAd(AvrDevice *core, HWAdmux *m, HWIrqSystem *, Pin& aref, unsigned int irqVec) ;
        unsigned int CpuCycle();

        unsigned char GetAdch();
        unsigned char GetAdcl();
        unsigned char GetAdcsr();
        void Reset();
        void SetAdcsr(unsigned char);
        //bool IsIrqFlagSet(unsigned int vec);
        void ClearIrqFlag(unsigned int vec);
	IOReg<HWAd>
	    adch_reg,
	    adcl_reg,
	    adcsr_reg;
};
#endif

