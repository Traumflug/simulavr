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

void AvrFlash::Decode(){
    for (unsigned int addr=0; addr<size ; addr+=2) {
        Decode(addr);
    }
}

void AvrFlash::Decode(int addr) {
    decode_single_instruction( myMemory, DecodedMem, /*currentSize,*/ addr);
}

AvrFlash::AvrFlash(int _size):Memory(_size) {
    DecodedMem=(DecodedEntry*) malloc((size>>1)*sizeof(DecodedEntry));
    if (DecodedMem==0 ) {
        cerr << "Could not allocate Flash memory" << endl;
        exit(0);
    }
    for (unsigned int tt=0; tt<size; tt++) { 
        myMemory[tt]=0xff;
    }

    Decode();

}

unsigned int AvrFlash::GetSize() { return size; }

void AvrFlash::WriteMem( unsigned char *src, unsigned int offset, unsigned int secSize) {
    for (unsigned tt=0; tt<secSize; tt+=2) { 
        if (tt+offset<size) {
            *(myMemory+tt+offset)=src[tt+1];
            *(myMemory+tt+1+offset)=src[tt];
        } 
    }
    Decode();
}
