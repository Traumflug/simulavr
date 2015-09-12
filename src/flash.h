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

#ifndef FLASH_H_INCLUDED
#define FLASH_H_INCLUDED
#include <string>
#include <map>
#include <vector>

#include "decoder.h"
#include "memory.h"

class DecodedInstruction;

//! Holds AVR flash content and symbol informations.
class AvrFlash: public Memory {
  
    protected:
        AvrDevice *core;
        std::vector <DecodedInstruction*> DecodedMem;
        unsigned int rww_lock; //!< When Flash write is in progress then addresses below this are inaccesible, otherwise 0.
        bool flashLoaded; //!< Flag, true if there was a write to Flash after constructor call (program load)
        
        friend int avr_op_CPSE::operator()();
        friend int avr_op_SBIC::operator()();
        friend int avr_op_SBIS::operator()();
        friend int avr_op_SBRC::operator()();
        friend int avr_op_SBRS::operator()();

    public:
      
        AvrFlash(AvrDevice *c, int size);
        ~AvrFlash();
        
        void Decode(); /*!< Decode/create all instructions */
        
        /*! Decode/create instruction at address 'addr'. */
        void Decode(unsigned int addr);
        
        /*! Decode memory block with offset and size
          @param offset data offset in memory block, beginning from start of THIS memory block!
          @param secSize count of available data (bytes) in src */
        void Decode(unsigned int addr, int secSize);
        
        /*! Write `secSize' bytes from `src' data to byte address `addr'.
          @param src binary c-string with data to write in
          @param secSize count of available data (bytes) in src */
        void WriteMem(const unsigned char* src, unsigned int addr, unsigned int secSize);
        
        /*! Write byte `val' at `address' (in bytes). Caller must call Decode() later. */
        void WriteMemByte(unsigned char val, unsigned int address);
        
        /*! True if flash was written, i.e. a program was loaded */
        bool IsProgramLoaded(void) { return flashLoaded; }
        
        /*! True if simulated Flash write is in progress and the address is in locked area. */
        bool IsRWWLock(unsigned int addr) { return (addr < rww_lock);}
        
        /*! Sets/Resets RWW lock address
          @param addr address, below flash is locked, 0 to disable lock */
        void SetRWWLock(unsigned int addr) { rww_lock = addr;}
        
        /*! Returns instruction at pointer PC. Aborts if Flash write is in progress. */
        DecodedInstruction* GetInstruction(unsigned int pc);

        /*! Returns opcode at PC. Aborts if Flash write is in progress. */
        unsigned int GetOpcode(unsigned int pc);
        
        /*! Returns byte at flash address. Works even during flash writing. */
        unsigned char ReadMemRaw(unsigned int addr) { return myMemory[addr]; }
        
        /*! Returns byte at flash address. Aborts if Flash write is in progress. */
        unsigned char ReadMem(unsigned int addr);
        
        /*! Returns 16bits at flash address. Works even during flash writing. */
        unsigned int ReadMemRawWord(unsigned int addr) { return (myMemory[addr] << 8) + myMemory[addr + 1]; }
        
        /*! Returns 16bits at flash address. Aborts if Flash write is in progress. */
        unsigned int ReadMemWord(unsigned int addr);

        bool LooksLikeContextSwitch(unsigned int addr) const;
};

#endif
