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

#ifndef FLASH
#define FLASH
#include <string>
#include <map>
#include <vector>

#include "decoder.h"
#include "memory.h"

class DecodedInstruction;

//! Hold AVR flash content and symbol informations.
class AvrFlash: public Memory {
  
    protected:
        AvrDevice *core; //!< ptr to connected device core
        std::vector <DecodedInstruction*> DecodedMem; //!< list of decoded ops
        unsigned int rww_lock; //!< RWW flash lock address, the area below is locked
        
        friend int avr_op_CPSE::operator()();
        friend int avr_op_SBIC::operator()();
        friend int avr_op_SBIS::operator()();
        friend int avr_op_SBRC::operator()();
        friend int avr_op_SBRS::operator()();
        friend int AvrDevice::Step(bool &, SystemClockOffset *);

    public:
      
        /*! Creates the AVR Flash block.
          @param c pointer to connected device
          @param size the memory block size */
        AvrFlash(AvrDevice *c, int size);
        ~AvrFlash();
        
        void Decode(); /*!< Decode complete memory block */
        
        /*! Decode only operation at address
          @param addr address, where operation have to be decoded */
        void Decode(unsigned int addr);
        
        /*! Decode memory block with offset and size
          @param offset data offset in memory block, beginning from start of THIS memory block!
          @param secSize count of available data (bytes) in src */
        void Decode(unsigned int offset, int secSize);
        
        /*! Write memory data to memory block.
          @param src binary c-string with data to write in
          @param offset data offset in memory block, beginning from start of THIS memory block!
          @param secSize count of available data (bytes) in src */
        void WriteMem(unsigned char* src, unsigned int offset, unsigned int secSize);
        
        /*! Returns true, if lock is set for address
          @param addr address to check
          @return true, if address is locked, false, if not */
        bool IsRWWLock(unsigned int addr) { return (addr < rww_lock);}
        
        /*! Sets/Resets RWW lock address
          @param addr address, below flash is locked, 0 to disable lock */
        void SetRWWLock(unsigned int addr) { rww_lock = addr;}
        
        /*! Returns instruction at address
          @param pc pc word address
          @return instruction at pc word address */
        DecodedInstruction* GetInstruction(unsigned int pc);
        
        /*! Returns byte at flash address
          @param offset data offset in memory block, beginning from start of THIS memory block!
          @return byte at offset */
        unsigned char ReadMem(unsigned int offset);
        
        /*! Returns word at flash address
          @param offset data offset in memory block, beginning from start of THIS memory block!
          @return word at offset */
        unsigned int ReadMemWord(unsigned int offset);
};

#endif
