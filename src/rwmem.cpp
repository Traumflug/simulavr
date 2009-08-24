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

/*
 * All here defined types are used to simulate the 
 * read write address space. This means also registers
 * io-data space, internal and external sram
 */

#include "avrerror.h"
#include "traceval.h"
#include "avrdevice.h"
#include "helper.h"
#include "rwmem.h"

using namespace std;

RWMemoryMember::RWMemoryMember(TraceValueRegister *_reg,
                               const std::string &tracename,
                               const int index):
    registry(_reg)
{
    if (tracename.size()) {
        tv = new TraceValue(8, registry->GetTraceValuePrefix() + tracename, index);
        if (!registry) {
            avr_error("registry not initialized for RWMemoryMember '%s'.", tracename.c_str());
        }
        registry->RegisterTraceValue(tv);
    } else {
        tv=0;
    }
}

RWMemoryMember::RWMemoryMember(void):
    registry(NULL),
    tv(NULL) {}

RWMemoryMember::operator unsigned char() const {
    if (tv)
        tv->read();
    return get();
}

unsigned char RWMemoryMember::operator=(unsigned char val) {
    set(val);
    if (tv)
        tv->write(val);
    return val;
}

unsigned char RWMemoryMember::operator=(const RWMemoryMember &mm) {
    if (mm.tv)
        mm.tv->read();
    unsigned char v=mm.get();
    set(v);
    if (tv)
        tv->write(v);
    return v;
}


RWMemoryMember::~RWMemoryMember() {
    if (tv)
        delete tv;
}

RWMemoryMember& MemoryOffsets::operator[](unsigned int externOffset) const {
    return *(rwHandler[myOffset+externOffset]);
}

RAM::RAM(TraceValueCoreRegister *_reg, const std::string &name, const size_t number, const size_t maxsize) {
    corereg = _reg;
    if(name.size()) {
        tv = new TraceValue(8, corereg->GetTraceValuePrefix() + name, number);
        if(!corereg) {
            avr_error("registry not initialized for RWMemoryMember '%s'.", name.c_str());
        }
        corereg->RegisterTraceSetValue(tv, name, maxsize);
    } else {
        tv = NULL;
    }
}

unsigned char RAM::get() const { return value; }

void RAM::set(unsigned char v) { value=v; }

InvalidMem::InvalidMem(AvrDevice* _c, int _a):
    RWMemoryMember(),
    core(_c),
    addr(_a) {}

unsigned char InvalidMem::get() const {
    string s = "Invalid read access to iospace address " + int2str(addr);
    if(core->abortOnInvalidAccess)
        avr_error(s.c_str());
    avr_warning(s.c_str());
    return 0;
}

void InvalidMem::set(unsigned char c) {
    string s = "Invalid write access to iospace address " + int2str(addr) + ", trying to set value=" + int2str(c);
    if(core->abortOnInvalidAccess)
        avr_error(s.c_str());
    avr_warning(s.c_str());
}

IOSpecialReg::IOSpecialReg(TraceValueRegister *registry, const std::string &name):
    RWMemoryMember(registry, name)
{
    Reset();
}

unsigned char IOSpecialReg::get() const {
    unsigned char val = value;
    for(size_t i = 0; i < clients.size(); i++)
        val = clients[i]->get_from_client(this, val);
    return val;
}

void IOSpecialReg::set(unsigned char val) {
    for(size_t i = 0; i < clients.size(); i++)
        val = clients[i]->set_from_reg(this, val);
    value = val;
}

