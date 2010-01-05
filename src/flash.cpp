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

#include <iostream>
#include <fstream>
#include <sstream>

extern "C" {
#include <unistd.h>
}


#include "flash.h"
#include "helper.h"
#include "memory.h"
#include "avrerror.h"

void AvrFlash::Decode(){
    for(unsigned int addr = 0; addr < size ; addr += 2)
        Decode(addr);
}

AvrFlash::AvrFlash(AvrDevice *c, int _size): Memory(_size), core(c), DecodedMem(_size) {
    // initialize memory block
    for(unsigned int tt = 0; tt < size; tt++)
        myMemory[tt] = 0xff;
    // reset RWW lock
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
#ifndef WORDS_BIGENDIAN
            *(myMemory + tt + offset) = src[tt + 1];
            *(myMemory + tt + 1 + offset) = src[tt];
#else
            *(myMemory + tt + offset) = src[tt];
            *(myMemory + tt + 1 + offset) = src[tt + 1];
#endif
        } 
    }
    Decode(offset, secSize);
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
    addr >>= 1; //PC runs per word

    word *MemPtr = (word *)(myMemory);
    word opcode;

    opcode = ((MemPtr[addr]) >> 8) + ((MemPtr[addr] & 0xff) << 8);
    if(DecodedMem[addr] != NULL)
        delete DecodedMem[addr];                     //delete old Instruction here 
    DecodedMem[addr] = lookup_opcode(opcode, core);  //and set new one
}
