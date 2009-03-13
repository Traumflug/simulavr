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
/*
 * All here defined types are used to simulate the 
 * read write address space. This means also registers
 * io-data space, internal and external sram
 */

#include <iostream>
#include "trace.h"
#include "rwmem.h"
#include "helper.h" //HexShort...
#include "avrdevice.h" //we need that only for all "core" references here
//allways they are only here for tracing because access to not own variables/registers inside the AvrDevice
//is needed. This is not intentional and must be redesigned ! TODO XXX

using namespace std;

#include "memory.h"

void RWMemoryMembers::operator=(const RWMemoryMembers &mm) {
    *this=(char)mm;
}

unsigned char RWMemoryWithOwnMemory::operator=(unsigned char val) {
    value=val;
    return val;
}

RWMemoryWithOwnMemory::operator unsigned char() const {
    return value;
}

unsigned char IRam::operator=(unsigned char val) {
    value=val;
    if (core->trace_on==1) traceOut << "IRAM["<<HexShort(myAddress) <<","<< core->data->GetSymbolAtAddress(myAddress)<<"]="<<HexChar(val)<<dec<<" ";
    return val;
}
IRam::operator unsigned char() const {
    if (core->trace_on==1) traceOut << "IRAM["<<HexShort(myAddress) <<","<< core->data->GetSymbolAtAddress(myAddress)<<"]-->"<<HexChar(value)<<dec<<"--> ";
    return value;
}

unsigned char ERam::operator=(unsigned char val) {
    value=val;
    if (core->trace_on==1) traceOut << "ERAM[0x"<<hex<<myAddress<<"]=0x"<<hex<<(unsigned int)val<<dec<<" ";
    return val;
}

unsigned char NotAvailableIo::operator=(unsigned char val) {
    if (core->trace_on==1) traceOut << "NOT AVAILABLE RAM[0x"<<hex<<myAddress<<"]=0x"<<hex<<(unsigned int)val<<dec<<" ";
    return val;
}

NotAvailableIo::operator unsigned char() const {
    if (core->trace_on==1) traceOut << "NOT AVAILABLE RAM[0x"<<hex<<myAddress<<"] accessed ERROR!"<<dec;
    return 0;
}



RWMemoryMembers &MemoryOffsets::operator[](unsigned int externOffset) const{
    return *rwHandler[myOffset+externOffset];
}




unsigned char CPURegister::operator=(unsigned char val) {
    value=val;
    if (core->trace_on==1) {
        traceOut << "R" << dec<< myNumber << "=" << HexChar(val) << " ";

        switch (myNumber) {
            case 26:
            case 27:
                traceOut << "X=" << HexShort(((*(core->R))[27]<<8) + (*(core->R))[26]) << " " ;
                break;
            case 28:
            case 29:
                traceOut << "Y=" << HexShort(((*(core->R))[29]<<8) + (*(core->R))[28]) << " " ;
                break;
            case 30:
            case 31:
                traceOut << "Z=" << HexShort(((*(core->R))[31]<<8) + (*(core->R))[30]) << " " ;
                break;
        } //end of switch
    }

    return val;
}

CPURegister::operator unsigned char() const {
    return value;
}


unsigned char RWReserved::operator=(unsigned char val) { 
    if (core->trace_on) {
        trioaccess("Reserved",val);
    }
    return val;
}

RWReserved::operator unsigned char() const {
    return 0;
}

//---------------------------------------------------------

unsigned char RWWriteToPipe::operator=(unsigned char val) { os << val; os.flush(); return val; } 
RWWriteToPipe::operator unsigned char() const { return 0; } 

unsigned char RWReadFromPipe::operator=(unsigned char val) { return 0; } 
RWReadFromPipe::operator unsigned char() const{ 
    char val;
    is.get(val);
    return val; 
} 

// Exit the simulator magic address support
unsigned char RWExit::operator=(unsigned char c)
{
  cerr << "Exiting at simulated program request" << endl;
  exit((int) c); 
}

RWExit::operator unsigned char() const 
{
  cerr << "Exiting at simulated program request" << endl;
  exit(0);
  return 0;
}

// Abort the simulator magic address support
unsigned char RWAbort::operator=(unsigned char c)
{
  cerr << "Aborting at simulated program request" << endl;
  abort();
}

RWAbort::operator unsigned char() const 
{
  cerr << "Aborting at simulated program request" << endl;
  abort();
  return 0;
}

