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

#ifndef HWEEPROM
#define HWEEPROM

#include "rwmem.h"
#include "hardware.h"
#include "memory.h"
#include "traceval.h"

class HWEeprom: public Hardware, public Memory, public TraceValueRegister {
    protected:
        unsigned int eear;
        unsigned char eecr;
        unsigned char eedr;

        unsigned int cpuHoldCycles;
        unsigned int uSleepForOutput;
        unsigned int state;
        unsigned int writeEnableCycles;    // let EEMWE stay only 4 cycles
        unsigned int writeStartTime;
        AvrDevice *core;

    public:
        HWEeprom(AvrDevice *core, unsigned int size);
        void WriteMem(unsigned char *, unsigned int offset, unsigned int size);
        virtual ~HWEeprom();
        void SetEearl(unsigned char);
        void SetEearh(unsigned char);
        void SetEedr(unsigned char);
        void SetEecr(unsigned char);

        unsigned char GetEearl();
        unsigned char GetEearh();
        unsigned char GetEecr();
        unsigned char GetEedr();
        void WriteAtAddress(unsigned int, unsigned char);
        unsigned char ReadFromAddress(unsigned int);

        virtual unsigned int CpuCycle();
        void Reset();

        enum {
            READY,
            READ,
            WRITE,
            WRITE_ENABLED
        } T_State;

        IOReg<HWEeprom>
            eearh_reg,
            eearl_reg,
            eedr_reg,
            eecr_reg;
};

class HWIrqSystem;

class HWMegaEeprom: public HWEeprom {
    protected:
        HWIrqSystem* irqSystem;
        unsigned int irqVectorNo;
        bool irqFlag;

    public:
        HWMegaEeprom(AvrDevice *core, HWIrqSystem *, unsigned int size, unsigned int irqVec);
        virtual unsigned int CpuCycle();
        //bool IsIrqFlagSet(unsigned int vector);
        void ClearIrqFlag(unsigned int vector);
};

#endif
