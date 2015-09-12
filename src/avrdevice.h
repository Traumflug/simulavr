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

#ifndef AVRDEVICE
#define AVRDEVICE

#include "systemclocktypes.h"
#include "simulationmember.h"
#include "pin.h"
#include "net.h"
#include "traceval.h"
#include "flashprog.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "types.h" // for dword

// transfered from global.h
#define BREAK_POINT    -2
#define INVALID_OPCODE -1

// transfered from breakpoint.h
class Breakpoints: public std::vector<dword> { };
class Exitpoints: public std::vector<dword> { };

// from hwsreg.h, but not included, because of circular include with this header
class HWSreg;
class RWSreg;

class AvrFlash;
class HWEeprom;
class HWStack;
class HWWado;
class Data;
class HWIrqSystem;
class RWMemoryMember;
class Hardware;
class DumpManager;
class AddressExtensionRegister;

//! Basic AVR device, contains the core functionality
class AvrDevice: public SimulationMember, public TraceValueRegister {
    
    private:
        RWMemoryMember **invalidRW; //!< hold invalid RW memory cells created by device
        const unsigned int ioSpaceSize;
        static const unsigned int totalIoSpace;
        static const unsigned int registerSpaceSize;
        const unsigned int iRamSize;
        const unsigned int eRamSize;
        unsigned int devSignature; //!< hold the device signature for this core
        std::string devName; //!< hold the device name, which this core simulate

        friend class DumpManager;
        void detachDumpManager() { dumpManager = NULL; }

        bool opIsCli(unsigned opcode);

    protected:
        SystemClockOffset clockFreq;  ///< Period of a tick (1/F_OSC) in [ns]
        std::map < std::string, Pin *> allPins;
        std::string actualFilename;
        
        /// Count of cycles before next instruction is executed (i.e. countdown)
        int cpuCycles;

    public:
        int trace_on;
        Breakpoints BP;
        Exitpoints EP;
        word PC;  ///< Next/current instruction index. Multiply by 2 to get an address. This will not be enough for ATmega2560
        /// When mupti-cycle instruction is "processed" this holds its address, PC holds the next instruction.
        word cPC;
        int PC_size;
        AvrFlash *Flash;
        FlashProgramming *spmRegister;
        AvrFuses *fuses;
        AvrLockBits *lockbits;
        HWEeprom *eeprom;
        Data *data;  ///< a hack for symbol look-up
        HWIrqSystem *irqSystem;
        AddressExtensionRegister *rampz; //!< RAMPZ address extension register
        AddressExtensionRegister *eind; //!< EIND address extension register
        bool abortOnInvalidAccess; //!< Flag, that simulation abort if an invalid access occured, default is false
        TraceValueCoreRegister coreTraceGroup;
        bool deferIrq;  ///< Almost always false.
        unsigned int newIrqPc;
        unsigned int actualIrqVector; 
        Pin v_supply; //!< represents supply voltage level, needed for analog peripherals
        Pin v_bandgap; //!< represents bandgap (ref) voltage level, needed for analog peripherals

        bool flagIWInstructions; //!< ADIW and SBIW instructions are available (not on most tiny's!)
        bool flagJMPInstructions; //!< CALL and JMP instructions are available (only on devices with bigger flash)
        bool flagIJMPInstructions; //!< ICALL and IJMP instructions are available (not on attiny1x devices)
        bool flagEIJMPInstructions; //!< EICALL and EIJMP instructions are available (only on some devices with bigger flash)
        bool flagLPMInstructions; //!< LPM and SPM instructions are available (not on some tiny devices)
        bool flagELPMInstructions; //!< ELPM instructions are available (only on devices with bigger flash)
        bool flagMULInstructions; //!< (F)MULxx instructions are available
        bool flagMOVWInstruction; //!< MOVW instruction is available
        bool flagTiny10; //!< core is a tiny4/5/9/10, change used clocks on some instructions and disables instructions
        bool flagTiny1x; //!< core is a tiny1x (but not tiny10!), change used clocks on some instructions and disables instructions
        bool flagXMega; //!< core is a XMEGA device, change used clocks on some instructions
        int DebugRecentJumps[20];  ///< Addresses of last few 'call' and 'jump' executed. For debugging.
        int DebugRecentJumpsIndex;  ///< Index to address of the most recent jump

        RWMemoryMember **rw;  ///< The whole memory: R0-R31, IO, Internal RAM.

        HWStack *stack;
        HWSreg *status;           //!< the status register itself
        RWSreg *statusRegister;   //!< the memory interface for status
        HWWado *wado;  ///< WDT timer

        std::vector<Hardware *> hwResetList; 
        std::vector<Hardware *> hwCycleList; 

        DumpManager *dumpManager;
    
        AvrDevice(unsigned int ioSpaceSize, unsigned int IRamSize, unsigned int ERamSize, unsigned int flashSize);
        virtual ~AvrDevice();

        /*! Adds to the list of parts to reset. If already in that list, does
          nothing. */
        void AddToResetList(Hardware *hw);

        /*! Adds to the list of parts to cycle per clock tick. If already in that list, does
          nothing. */
        void AddToCycleList(Hardware *hw);

        //! Removes from the cycle list, if possible.
        /*! Does nothing if the part is not in the cycle list. */
        void RemoveFromCycleList(Hardware *hw);
    
        void Load(const char* n); //!< Load flash, eeprom, signature, fuses from elf file, wrapper for LoadBFD or LoadSimpleELF
        void ReplaceIoRegister(unsigned int offset, RWMemoryMember *);
        bool ReplaceMemRegister(unsigned int offset, RWMemoryMember *);
        RWMemoryMember *GetMemRegisterInstance(unsigned int offset);
        void RegisterTerminationSymbol(const char *symbol);

        Pin *GetPin(const char *name);
        /*! Steps the AVR core.
          \param untilCoreStepFinished iff true, steps a core step and not a
          single clock cycle. */
        int Step(bool &untilCoreStepFinished, SystemClockOffset *nextStepIn_ns =0);
        void Reset();
        void SetClockFreq(SystemClockOffset f);
        SystemClockOffset GetClockFreq();

        void RegisterPin(const std::string &name, Pin *p) {
            allPins.insert(std::pair<std::string, Pin*>(name, p));
        }

        //! Clear all breakpoints in device
        void DeleteAllBreakpoints(void);

        //! Return filename from loaded program
        const std::string &GetFname(void) { return actualFilename; }
        //! Return device name
        const std::string &GetDeviceName(void) { return devName; }
        //! Return device signature
        unsigned int GetDeviceSignature(void) { return devSignature; }
        //! Set device signature and name
        void SetDeviceNameAndSignature(const std::string &name, unsigned int signature);

        //! Get configured total memory space size
        unsigned int GetMemTotalSize(void) { return totalIoSpace; }
        //! Get configured IO memory space size
        unsigned int GetMemIOSize(void) { return ioSpaceSize; }
        //! Get configured register space size
        unsigned int GetMemRegisterSize(void) { return registerSpaceSize; }
        //! Get configured internal RAM size
        unsigned int GetMemIRamSize(void) { return iRamSize; }
        //! Get configured external RAM size
        unsigned int GetMemERamSize(void) { return eRamSize; }
        
        //! Get a value of RW memory cell
        unsigned char GetRWMem(unsigned addr);
        //! Set a value to RW memory cell
        bool SetRWMem(unsigned addr, unsigned char val);
        //! Get a value from core register
        unsigned char GetCoreReg(unsigned addr);
        //! Set a value to core register
        bool SetCoreReg(unsigned addr, unsigned char val);
        //! Get a value from IO register (without offset of 0x20!)
        unsigned char GetIOReg(unsigned addr);
        //! Set a value to IO register (without offset of 0x20!)
        bool SetIOReg(unsigned addr, unsigned char val);
        //! Set a bit value to lower IO register (without offset of 0x20!)
        /*! \todo Have to be reimplemented, if IO bit set feature is implemented.
            Compare differences in datasheets, for example ATMega16 and ATMega48,
            on CBI and SBI usage at IO registers, which have special behaviour, if
            bit will be set to 1 */
        bool SetIORegBit(unsigned addr, unsigned bitaddr, bool val);
        //! Get value of X register (16bit)
        unsigned GetRegX(void);
        //! Get value of Y register (16bit)
        unsigned GetRegY(void);
        //! Get value of Z register (16bit)
        unsigned GetRegZ(void);

        //! When a call/jump/cond-jump instruction was executed. For debugging.
        void DebugOnJump();

        friend void ELFLoad(AvrDevice * core);

};

#endif
