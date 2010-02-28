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

#include "hwstack.h"
#include "avrerror.h"
#include "avrmalloc.h"

using namespace std;

HWStack::HWStack(AvrDevice *c):
    core(c) {
    Reset();
}

void HWStack::Reset(void) {
    returnPointList.clear();
    stackPointer = 0;
    lowestStackPointer = 0;
}

void HWStack::CheckReturnPoints() {
    typedef multimap<unsigned long, Funktor *>::iterator I;
    pair<I,I> l = returnPointList.equal_range(stackPointer);
    
    for(I i = l.first; i != l.second; i++) {
        (*(i->second))(); //execute Funktor
        delete i->second; //and delete it
    }
    returnPointList.erase(l.first, l.second);
}

void HWStack::SetReturnPoint(unsigned long stackPointer, Funktor *f) {
    returnPointList.insert(make_pair(stackPointer, f));
}

HWStackSram::HWStackSram(AvrDevice *c, int bs, bool initRE):
    HWStack(c),
    TraceValueRegister(c, "STACK"),
    sph_reg(this, "SPH",
            this, &HWStackSram::GetSph, &HWStackSram::SetSph),
    spl_reg(this, "SPL",
            this, &HWStackSram::GetSpl, &HWStackSram::SetSpl),
    initRAMEND(initRE) {
    stackCeil = 1 << bs;
    mem = c->Sram;
    Reset();
}

void HWStackSram::Reset() {
    returnPointList.clear();
    if(initRAMEND)
        stackPointer = core->GetMemIRamSize() +
                       core->GetMemIOSize() +
                       core->GetMemRegisterSize() - 1;
    else
        stackPointer = 0;
    lowestStackPointer = stackPointer;
}

void HWStackSram::Push(unsigned char val) {
    (*mem)[stackPointer] = val;
    stackPointer--;
    stackPointer %= stackCeil;

    spl_reg.hardwareChange(stackPointer & 0x0000ff);
    sph_reg.hardwareChange((stackPointer & 0x00ff00)>>8);
    
    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << " 0x" << int(val) << dec << " ";
    CheckReturnPoints();
    
    // measure stack usage, calculate lowest stack pointer
    if(lowestStackPointer > stackPointer)
        lowestStackPointer = stackPointer;
}

unsigned char HWStackSram::Pop() {
    stackPointer++;
    stackPointer %= stackCeil;

    spl_reg.hardwareChange(stackPointer & 0x0000ff);
    sph_reg.hardwareChange((stackPointer & 0x00ff00)>>8);
    
    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << " 0x" << int((*mem)[stackPointer]) << dec << " ";
    CheckReturnPoints();
    return (*mem)[stackPointer];
}

void HWStackSram::PushAddr(unsigned long addr) {
    // low byte first, then high byte
    Push(addr & 0xff);
    addr >>= 8;
    Push(addr & 0xff);
    if(core->PC_size == 3) {
        addr >>= 8;
        Push(addr & 0xff);
    }
}

unsigned long HWStackSram::PopAddr() {
    // high byte first, then low byte
    unsigned long val = Pop();
    val <<= 8;
    val += Pop();
    if(core->PC_size == 3) {
        val <<= 8;
        val += Pop();
    }
    return val;
}

void HWStackSram::SetSpl(unsigned char val) {
    stackPointer &= ~0xff;
    stackPointer += val;
    stackPointer %= stackCeil; // zero the not used bits

    spl_reg.hardwareChange(stackPointer & 0x0000ff);
    
    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << dec << " " ; 
    CheckReturnPoints();
}

void HWStackSram::SetSph(unsigned char val) {
    if(stackCeil <= 0x100)
        avr_warning("assignment to non existent SPH (value=0x%x)", (unsigned int)val);
    stackPointer &= ~0xff00;
    stackPointer += val << 8;
    stackPointer %= stackCeil; // zero the not used bits

    sph_reg.hardwareChange((stackPointer & 0x00ff00)>>8);

    if(core->trace_on == 1)
        traceOut << "SP=0x" << hex << stackPointer << dec << " " ; 
    CheckReturnPoints();
}

unsigned char HWStackSram::GetSph() {
    return (stackPointer & 0xff00) >> 8;
}

unsigned char HWStackSram::GetSpl() {
    return stackPointer & 0xff;
}

ThreeLevelStack::ThreeLevelStack(AvrDevice *c):
    HWStack(c),
    TraceValueRegister(c, "STACK") {
    stackArea = avr_new(unsigned long, 3);
    trace_direct(this, "PTR", &stackPointer);
    Reset();
}

ThreeLevelStack::~ThreeLevelStack() {
    avr_free(stackArea);
}

void ThreeLevelStack::Reset(void) {
    returnPointList.clear();
    stackPointer = 3;
    lowestStackPointer = stackPointer;
}

void ThreeLevelStack::Push(unsigned char val) {
    avr_error("Push method isn't available on TreeLevelStack");
}

unsigned char ThreeLevelStack::Pop() {
    avr_error("Pop method isn't available on TreeLevelStack");
    return 0;
}

void ThreeLevelStack::PushAddr(unsigned long addr) {
    stackArea[2] = stackArea[1];
    stackArea[1] = stackArea[0];
    stackArea[0] = addr;
    if(stackPointer > 0)
        stackPointer--;
    if(lowestStackPointer > stackPointer)
        lowestStackPointer = stackPointer;
    if(stackPointer == 0)
        avr_warning("stack overflow");
}

unsigned long ThreeLevelStack::PopAddr() {
    unsigned long val = stackArea[0];
    stackArea[0] = stackArea[1];
    stackArea[1] = stackArea[2];
    stackPointer++;
    if(stackPointer > 3) {
        stackPointer = 3;
        avr_warning("stack underflow");
    }
    return val;
}

/* EOF */
