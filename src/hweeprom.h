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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#include "irqsystem.h"

class HWEeprom: public Hardware, public Memory, public TraceValueRegister {
    protected:
        AvrDevice *core;
        unsigned int eear;
        unsigned int eear_mask;
        unsigned char eecr;
        unsigned char eecr_mask;
        unsigned char eedr;
        HWIrqSystem* irqSystem;
        unsigned int irqVectorNo;
        int opEnableCycles;
        int cpuHoldCycles;
        int opState;
        int opMode;
        unsigned int opAddr;
        SystemClockOffset eraseWriteDelayTime;
        SystemClockOffset eraseDelayTime;
        SystemClockOffset writeDelayTime;
        SystemClockOffset writeDoneTime;
        
    public:
        enum {
          DEVMODE_NORMAL = 0,
          DEVMODE_AT90S,
          DEVMODE_EXTENDED
        };
        
        enum {
          OPSTATE_READY,
          OPSTATE_ENABLED,
          OPSTATE_WRITE
        };
        
        enum {
          CTRL_MODE_ERASEWRITE = 0,
          CTRL_READ = 1,
          CTRL_WRITE = 2,
          CTRL_ENABLE = 4,
          CTRL_IRQ = 8,
          CTRL_MODE_ERASE = 16,
          CTRL_MODE_WRITE = 32,
          CTRL_MODES = 48,
        };
        
        HWEeprom(AvrDevice *core, HWIrqSystem *irqs, unsigned int size, unsigned int irqVec, int devMode = DEVMODE_NORMAL);
        virtual ~HWEeprom();

        virtual unsigned int CpuCycle();
        void Reset();
        void ClearIrqFlag(unsigned int vector);

        void WriteMem(const unsigned char *, unsigned int offset, unsigned int size);
        void WriteAtAddress(unsigned int, unsigned char);
        unsigned char ReadFromAddress(unsigned int);

        void SetEearl(unsigned char);
        void SetEearh(unsigned char);
        void SetEedr(unsigned char);
        void SetEecr(unsigned char);

        unsigned char GetEearl() {return eear & 0xff; }
        unsigned char GetEearh() {return (eear >> 8) & 0xff; }
        unsigned char GetEecr() { return eecr; }
        unsigned char GetEedr() { return eedr; }

        IOReg<HWEeprom>
            eearh_reg,
            eearl_reg,
            eedr_reg,
            eecr_reg;
};

#endif
