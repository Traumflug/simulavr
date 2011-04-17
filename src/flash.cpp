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

#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>



#include "flash.h"
#include "helper.h"
#include "memory.h"
#include "avrerror.h"

void AvrFlash::Decode(){
    for(unsigned int addr = 0; addr < size ; addr += 2)
        Decode(addr);
}

AvrFlash::AvrFlash(AvrDevice *c, int _size):
    Memory(_size),
    core(c),
    DecodedMem(_size),
    flashLoaded(false) {
    for(unsigned int tt = 0; tt < size; tt++)
        myMemory[tt] = 0xff;  // Safeguard, will be decoded as avr_op_ILLEGAL
    rww_lock = 0;
    // initialize DecodedMem
    Decode();
}

AvrFlash::~AvrFlash() {
    for(unsigned int i = 0; i < size; i++) {
       if(DecodedMem[i] != NULL)
          delete DecodedMem[i]; // delete Instruction
    }
}

void AvrFlash::WriteMem(unsigned char *src, unsigned int offset, unsigned int secSize) {
    for(unsigned tt = 0; tt < secSize; tt += 2) { 
        if(tt + offset < size) {
            assert(tt+offset+1<size);
            *(myMemory + tt + offset) = src[tt + 1];
            *(myMemory + tt + 1 + offset) = src[tt];
        } 
    }
    Decode(offset, secSize);
    flashLoaded = true;
}

void AvrFlash::WriteMemByte(unsigned char val, unsigned int offset) {
    assert(offset < size);  // in bytes
    *(myMemory + offset) = val;
    flashLoaded = true;
}

DecodedInstruction* AvrFlash::GetInstruction(unsigned int pc) {
    if(IsRWWLock(pc * 2))
        avr_error("flash is locked (RWW lock)");
    return DecodedMem[pc];
}

unsigned char AvrFlash::ReadMem(unsigned int offset) {
    if(IsRWWLock(offset)) {
        avr_warning("flash is locked (RWW lock)");
        return 0;
    }
    return myMemory[offset];
}

unsigned int AvrFlash::ReadMemWord(unsigned int offset) {
    // example: "lds r24, 0x013B" is 91 80 01 3b, we return 0x013B.
    assert(offset < size);  // in bytes
    if(IsRWWLock(offset)) {
        avr_warning("flash is locked (RWW lock)");
        return 0;
    }
    return (myMemory[offset] << 8) + myMemory[offset + 1];
}

void AvrFlash::Decode(unsigned int offset, int secSize) {
    for(; (offset < size) && (secSize > 0); offset += 2, secSize -= 2)
        Decode(offset);
}

void AvrFlash::Decode(unsigned int addr) {
    assert(0 <= addr && (unsigned)addr < size);
    assert((addr % 2) == 0);
    word opcode = (myMemory[addr] << 8) + myMemory[addr + 1];
    unsigned int index = addr / 2;
    if(DecodedMem[index] != NULL)
        delete DecodedMem[index];                     //delete old Instruction here 
    DecodedMem[index] = lookup_opcode(opcode, core);  //and set new one
}
