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

#ifndef AVRDEVICE
#define AVRDEVICE

#include "systemclocktypes.h"
#include "simulationmember.h"
#include "pin.h"
#include "net.h"
#include "breakpoint.h"
#include "traceval.h"
#include "flashprog.h"

#include <string>
#include <map>

// from hwsreg.h, but not included, because of circular include with this header
class HWSreg;
class RWSreg;

class AvrFlash;
class HWEeprom;
class HWStack;
class HWWado;
class Data;
class HWIrqSystem;
class MemoryOffsets;
class RWMemoryMember;
class Hardware;
class DumpManager;
class HWRampz;

//! Basic AVR device, contains the core functionality
class AvrDevice: public SimulationMember, public TraceValueRegister {
    
    protected:
        SystemClockOffset clockFreq;
        std::map < std::string, Pin *> allPins; 
        const unsigned int ioSpaceSize;
        const unsigned int totalIoSpace;
        const unsigned int registerSpaceSize;
        std::string actualFilename;
        
        //old static vars for Step()
        int cpuCycles;
        unsigned int newIrqPc;
        unsigned int actualIrqVector;
        int noDirectIrqJump;
        
    public:
        Breakpoints BP;
        Exitpoints EP;
        word PC;
        int PC_size;
        AvrFlash *Flash;
        FlashProgramming * spmRegister;
        HWEeprom *eeprom;
        Data *data;
        HWIrqSystem *irqSystem;
        HWRampz *rampz;
        bool abortOnInvalidAccess; //!< Flag, that simulation abort if an invalid access occured, default is false
        TraceValueCoreRegister coreTraceGroup;

        //RWMemory *rwmem;
        MemoryOffsets *Sram;
        MemoryOffsets *R;
        MemoryOffsets *ioreg;
        RWMemoryMember **rw;

        HWStack *stack;
        HWSreg *status;           //!< the status register itself
        RWSreg *statusRegister;   //!< the memory interface for status
        HWWado *wado;

        std::vector<Hardware *> hwResetList; 
        std::vector<Hardware *> hwCycleList; 

        DumpManager *dump_manager;
    
        /*! Adds to the list of parts to reset. If already in that list, does
          nothing. */
        void AddToResetList(Hardware *hw);

        /*! Adds to the list of parts to cycle per clock tick. If already in that list, does
          nothing. */
        void AddToCycleList(Hardware *hw);

        //! Removes from the cycle list, if possible.
        /*! Does nothing if the part is not in the cycle list. */
        void RemoveFromCycleList(Hardware *hw);
    
        void Load(const char* n);
        void ReplaceIoRegister(unsigned int offset, RWMemoryMember *);
        void RegisterTerminationSymbol(const char *symbol);

        Pin *GetPin(const char *name);
#ifndef SWIG
        MemoryOffsets &operator->*(MemoryOffsets *m) { return *m;}
#endif

        AvrDevice(unsigned int ioSpaceSize, unsigned int IRamSize, unsigned int ERamSize, unsigned int flashSize);
        /*! Steps the AVR core.
          \param untilCoreStepFinished iff true, steps a core step and not a
          single clock cycle. */
        int Step(bool &untilCoreStepFinished, SystemClockOffset *nextStepIn_ns =0);
        void Reset();
        void SetClockFreq(SystemClockOffset f);
        SystemClockOffset GetClockFreq();

        virtual ~AvrDevice();
        virtual unsigned char GetRampz();
        virtual void SetRampz(unsigned char);

        void RegisterPin(const std::string &name, Pin *p) {
            allPins.insert(std::pair<std::string, Pin*>(name, p));
        }

        //! Clear all breakpoints in device
        void DeleteAllBreakpoints(void);

        //! Return filename from loaded program
        const std::string &GetFname(void) { return actualFilename; }
        
        //! Get configured total memory space size
        unsigned int GetMemTotalSize(void) { return totalIoSpace; }
        //! Get configured IO memory space size
        unsigned int GetMemIOSize(void) { return ioSpaceSize; }
        //! Get configured register space size
        unsigned int GetMemRegisterSize(void) { return registerSpaceSize; }
};

#endif
