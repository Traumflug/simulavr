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
#include "trace.h"
#include "hwstack.h"



unsigned char RWSph::operator=(unsigned char val) { hwstack->SetSph(val); return val;} 
unsigned char RWSpl::operator=(unsigned char val) { hwstack->SetSpl(val); return val;} 
RWSpl::operator unsigned char() const { return hwstack->GetSpl();  } 
RWSph::operator unsigned char() const { return hwstack->GetSph();  } 
RWSph::RWSph(HWStack *stack) { hwstack=stack;}
RWSpl::RWSpl(HWStack *stack) { hwstack = stack; }



HWStack::HWStack(AvrDevice *core, MemoryOffsets *sr, unsigned int mask):Hardware(core) {
    stackMask=mask;
	mem=sr;
	Reset();
}

void HWStack::Reset() {
	stackPointer=0;
}

void HWStack::CheckBreakPoints() {
    typedef multimap<unsigned int, Funktor *>::iterator I;
    pair<I,I> l= breakPointList.equal_range(stackPointer);
    for (I i=l.first; i!=l.second; i++) {
        (*(i->second))(); //execute Funktor
        delete i->second; //and delete it
    }
    breakPointList.erase(l.first, l.second);
}

void HWStack::Push(unsigned char val){
	(*mem)[stackPointer]=val;
	stackPointer--;
    stackPointer&=stackMask;
	if (trace_on==1) traceOut << "SP=0x" << hex << stackPointer << dec << " " ;
    CheckBreakPoints();
}
unsigned char HWStack::Pop(){
	stackPointer++;
    stackPointer&=stackMask;
	if (trace_on==1) traceOut << "SP=0x" << hex << stackPointer << dec << " " ;
    CheckBreakPoints();
	return (*mem)[stackPointer];
}

void HWStack::SetSpl(unsigned char val) {
	stackPointer=stackPointer&0xffff00;
	stackPointer+=val;
    stackPointer&=stackMask;
	if (trace_on==1) traceOut << "SP=0x" << hex << stackPointer << dec << " " ; 
    CheckBreakPoints();
}

void HWStack::SetSph(unsigned char val) {
	stackPointer=stackPointer&0xff00ff;
	stackPointer+=(val<<8);
    stackPointer&=stackMask;
	if (trace_on==1) traceOut << "SP=0x" << hex << stackPointer << dec << " " ; 
    CheckBreakPoints();
}

unsigned char HWStack::GetSph() {
	return (stackPointer&0xff00)>>8;
}

unsigned char HWStack::GetSpl() {
	return (stackPointer&0xff);
}

//Attention! SetBreakPoint must get a copy!! of a Funktor because he selft destroy it after usage!!!
void HWStack::SetBreakPoint(unsigned int stackPointer, Funktor *f) {
    breakPointList.insert(make_pair(stackPointer,f));
}
