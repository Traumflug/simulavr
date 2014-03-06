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

#include "hweeprom.h"
#include "avrdevice.h"
#include "systemclock.h"
#include "irqsystem.h"
#include "avrerror.h"
#include <assert.h>

using namespace std;

HWEeprom::HWEeprom(AvrDevice *_core,
                   HWIrqSystem *_irqSystem,
                   unsigned int size,
                   unsigned int irqVec,
                   int devMode):
    Hardware(_core),
    Memory(size),
    TraceValueRegister(_core, "EEPROM"),
    core(_core),
    irqSystem(_irqSystem),
    irqVectorNo(irqVec),
    eearh_reg(this, "EEARH",
              this, &HWEeprom::GetEearh, &HWEeprom::SetEearh),
    eearl_reg(this, "EEARL",
              this, &HWEeprom::GetEearl, &HWEeprom::SetEearl),
    eedr_reg(this, "EEDR",
             this, &HWEeprom::GetEedr, &HWEeprom::SetEedr),
    eecr_reg(this, "EECR",
             this, &HWEeprom::GetEecr, &HWEeprom::SetEecr)
{
    if(irqSystem)
        irqSystem->DebugVerifyInterruptVector(irqVectorNo, this);

    // in a "fresh" device eeprom is initialized to 0xff like flash too
    for(unsigned int tt = 0; tt < size; tt++)
        myMemory[tt] = 0xff;

    // operation duration, see datasheets for time!
    if(devMode == DEVMODE_NORMAL) {
        eraseWriteDelayTime = 8500000LL; // 8.5ms
        eraseDelayTime = 0LL; // isn't available in this op mode!
        writeDelayTime = 0LL; // isn't available in this op mode!
    } else if(devMode == DEVMODE_AT90S) {
        eraseWriteDelayTime = 4000000LL; // 4.0ms
        eraseDelayTime = 0LL; // isn't available in this op mode!
        writeDelayTime = 0LL; // isn't available in this op mode!
    } else {
        eraseWriteDelayTime = 3400000LL; // 3.4ms
        eraseDelayTime = 1800000LL; // 1.8ms
        writeDelayTime = 1800000LL; // 1.8ms
    }

    // in normal mode only erase + write in one operation is available
    if((devMode == DEVMODE_NORMAL) || (devMode == DEVMODE_AT90S)) {
        if(irqSystem == NULL)
            eecr_mask = 0x07; // without operation mode bits and irq enable
        else
            eecr_mask = 0x0f; // without operation mode bits
    } else
        eecr_mask = 0x3f; // with operation mode bits
    eecr = 0;
    
    eear_mask = (size - 1); // mask out all not significant MSB's, assumes that
                            // size is a 2^n value! This limits also access to
                            // wrong myMemory places.
    eear = 0;
    
    opState = OPSTATE_READY;
    
    Reset();
}

void HWEeprom::Reset() {
    eecr &= 0x32; // bit 1 reflect EEPROM statemachine state before reset!
                  // bit 4 and bit 5 are operation modes, which are hold over reset
    eedr = 0;
    
    opEnableCycles = 0;
    cpuHoldCycles = 0;
}


HWEeprom::~HWEeprom() {
    avr_free(myMemory);
    myMemory = NULL;    // to prevent a double-free error!
}

void HWEeprom::SetEearl(unsigned char val) {
    eear = ((eear & 0xff00) + val) & eear_mask;
    if(core->trace_on == 1)
        traceOut << "EEAR=0x" << hex << eear << dec;
}

void HWEeprom::SetEearh(unsigned char val) {
    if((GetSize() <= 256) && (val != 0))
        avr_warning("invalid write access: EEARH=0x%02x, EEPROM size <= 256 byte", val);
    eear = ((eear & 0x00ff) + (val << 8)) & eear_mask;
    if(core->trace_on == 1)
        traceOut << "EEAR=0x" << hex << eear << dec;
}

void HWEeprom::SetEedr(unsigned char val) {
    eedr = val;
    if(core->trace_on == 1)
        traceOut << "EEDR=0x"<< hex << (unsigned int)eedr << dec;
}

void HWEeprom::SetEecr(unsigned char newval) {
    if(core->trace_on == 1)
        traceOut << "EECR=0x" << hex << (unsigned int)newval << dec;
    
    eecr = newval & eecr_mask;

    switch(opState) {
        
        default:
        case OPSTATE_READY:
            // enable write mode
            if((eecr & CTRL_ENABLE) == CTRL_ENABLE) {
                opState = OPSTATE_ENABLED;
                opEnableCycles = 4;
                core->AddToCycleList(this);
            }
            // read will be processed immediately
            if((eecr & CTRL_READ) == CTRL_READ) {
                cpuHoldCycles = 4;
                assert(eear < size);
                eedr = myMemory[eear];
                eecr &= ~CTRL_READ; // reset read bit isn't described in document!
                core->AddToCycleList(this);
                if(core->trace_on == 1)
                    traceOut << " EEPROM: Read = 0x" << hex << (unsigned int)eedr << dec;
            }
            // write will not processed
            eecr &= ~CTRL_WRITE;
            break;
            
        case OPSTATE_ENABLED:
            // enable bit will be hold in this state
            eecr |= CTRL_ENABLE;
            // read will be processed immediately
            if((eecr & CTRL_READ) == CTRL_READ) {
                cpuHoldCycles = 4;  // Datasheet: "When the EEPROM is read, the CPU is halted for four cycles"
                assert(eear < size);
                eedr = myMemory[eear];
                eecr &= ~CTRL_READ; // reset read bit isn't described in document!
                if(core->trace_on == 1)
                    traceOut << " EEPROM: Read = 0x" << hex << (unsigned int)eedr << dec;
                break; // to ignore possible write request!
            }
            // start write operation
            if((eecr & CTRL_WRITE) == CTRL_WRITE) {
                cpuHoldCycles = 2;  // Datasheet: "When EEWE has been set, the CPU is halted for two cycles"
                // abort enable state, switch to write state
                opMode = eecr & CTRL_MODES;
                opAddr = eear;
                assert(opAddr < size);
                opState = OPSTATE_WRITE;
                opEnableCycles = 0;
                eecr &= ~CTRL_ENABLE;
                // start timer ...
                SystemClockOffset t;
                switch(opMode & CTRL_MODES) {
                    default:
                    case CTRL_MODE_ERASEWRITE:
                        t = eraseWriteDelayTime;
                        break;
                    case CTRL_MODE_ERASE:
                        t = eraseDelayTime;
                        break;
                    case CTRL_MODE_WRITE:
                        t = writeDelayTime;
                        break;
                }
                writeDoneTime = SystemClock::Instance().GetCurrentTime() + t;
                if(core->trace_on == 1)
                    traceOut << " EEPROM: Write start";
            }
            break;
            
        case OPSTATE_WRITE:
            // enable write mode, mode change will not happen!
            if((eecr & CTRL_ENABLE) == CTRL_ENABLE) {
                opEnableCycles = 4;
            }
            // read is ignored here
            eecr &= ~CTRL_READ;
            // write is hold
            eecr |= CTRL_WRITE;
            break;
            
    }
}

unsigned int HWEeprom::CpuCycle() {
    
    // handle enable state and fallback to ready
    if(opEnableCycles > 0) {
        opEnableCycles--;
        if(opEnableCycles == 0) {
            eecr &= ~CTRL_ENABLE;
            if(opState == OPSTATE_ENABLED)
                opState = OPSTATE_READY;
            if(core->trace_on == 1)
                traceOut << " EEPROM: WriteEnable cleared";
        }
    }
    
    // handle write state
    if(opState == OPSTATE_WRITE) {
        if(SystemClock::Instance().GetCurrentTime() >= writeDoneTime) {
            // go to ready state
            opState = OPSTATE_READY;
            // reset write enable bit
            eecr &= ~CTRL_WRITE;
            assert(opAddr < size);
            // process operation
            switch(opMode & CTRL_MODES) {
                default:
                case CTRL_MODE_ERASEWRITE:
                    myMemory[opAddr] = eedr;
                    break;
                case CTRL_MODE_ERASE:
                    myMemory[opAddr] = 0xff;
                    break;
                case CTRL_MODE_WRITE:
                    myMemory[opAddr] = eedr & myMemory[opAddr];
                    break;
            }
            if(core->trace_on == 1)
                traceOut << " EEPROM: Write done";
            // now raise irq if enabled and available
            if((irqSystem != NULL) && ((eecr & CTRL_IRQ) == CTRL_IRQ))
                irqSystem->SetIrqFlag(this, irqVectorNo);
        }
    }
    
    // deactivate engine, if not used
    if((opState == OPSTATE_READY) && (cpuHoldCycles == 0) && (opEnableCycles == 0))
        core->RemoveFromCycleList(this);
    
    // handle cpu hold state
    if(cpuHoldCycles > 0) {
        cpuHoldCycles--;
        return 1;
    } else
        return 0;
      
}

void HWEeprom::ClearIrqFlag(unsigned int vector) {
    if(vector == irqVectorNo)
        irqSystem->ClearIrqFlag(irqVectorNo);
}

void HWEeprom::WriteAtAddress(unsigned int addr, unsigned char val) {
    myMemory[addr] = val;
}

unsigned char HWEeprom::ReadFromAddress( unsigned int addr) {
    return myMemory[addr];
}

void HWEeprom::WriteMem(const unsigned char *src, unsigned int offset, unsigned int secSize) {
    for(unsigned int tt = 0; tt < secSize; tt++) { 
        if(tt + offset < size) {
            *(myMemory + tt + offset) = src[tt];
        }
    }
}

