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

#include "timerirq.h"
#include "helper.h"
#include "avrerror.h"

IRQLine::IRQLine(const std::string& n, int irqvec):
    irqvector(irqvec),
    name(n) {
    irqreg = NULL; // set unregistered state
}

void IRQLine::fireInterrupt(void) {
    if(irqreg) irqreg->fireInterrupt(irqvector);
}

static const std::string __hlp2name(const std::string s, int i) {
    // if i == -2, then precede name with "E", for example "ETIMSK"
    if(i == -2)
        return "E" + s;
    // if then i < 0, let the name unchanged
    if(i < 0)
        return s;
    // in all other cases append i as number
    return s + int2str(i);
}

static const std::string __hlp2scope(const std::string p, int i) {
    // if i == -2, then append "X"
    if(i == -2)
        return p + "X";
    // if then i < 0, let the name unchanged
    if(i < 0)
        return p;
    // in all other cases append i as number
    return p + int2str(i);
}

TimerIRQRegister::TimerIRQRegister(AvrDevice* c,
                                   HWIrqSystem* irqsys,
                                   int regidx):
    Hardware(c),
    TraceValueRegister(c, __hlp2scope("TMRIRQ", regidx)),
    irqsystem(irqsys),
    core(c),
    lines(8),
    timsk_reg(this, __hlp2name("TIMSK", regidx)),
    tifr_reg(this, __hlp2name("TIFR", regidx))
{
    timsk_reg.connectSRegClient(this);
    tifr_reg.connectSRegClient(this);
    bitmask = 0;
    Reset();
}

void TimerIRQRegister::registerLine(int idx, IRQLine* irq) {
    irqsystem->DebugVerifyInterruptVector(irq->irqvector, this);
    // no check, if idx is in right range!
    irq->irqreg = this;
    lines[idx] = irq;
    vector2line[irq->irqvector] = idx;
    name2line[irq->name] = idx;
    bitmask |= 1 << idx;
}

IRQLine* TimerIRQRegister::getLine(const std::string& n) {
    std::map<std::string, int>::iterator cur  = name2line.find(n);
    if(cur == name2line.end())
        avr_error("IRQ line '%s' not found", n.c_str());
    return lines[name2line[n]];
}

void TimerIRQRegister::fireInterrupt(int irqvector) {
    int idx = vector2line[irqvector];
    irqflags |= (1 << idx);
    tifr_reg.hardwareChange(irqflags);
    if(irqmask & (1 << idx)) // check irq mask
        irqsystem->SetIrqFlag(this, irqvector);
}

void TimerIRQRegister::ClearIrqFlag(unsigned int vector) {
    int idx = vector2line[vector];
    irqflags &= ~(1 << idx);
    tifr_reg.hardwareChange(irqflags);
    irqsystem->ClearIrqFlag(vector);
}

void TimerIRQRegister::Reset(void) {
    irqmask = 0;
    timsk_reg.Reset();
    irqflags = 0;
    tifr_reg.Reset();
}

unsigned char TimerIRQRegister::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    if(reg == &timsk_reg) {
        // mask register: trigger interrupt, if mask bit is new set and flag is true
        unsigned int idx = 0;
        unsigned char m = 1;
        nv &= bitmask;
        for(; idx < lines.size(); idx++, m <<= 1) {
            if(((nv & m) != 0) &&
               ((irqmask & m) == 0) &&
               ((irqflags & m) != 0) &&
               (lines[idx] != NULL))
                irqsystem->SetIrqFlag(this, lines[idx]->irqvector);
        }
        irqmask = nv;
    } else {
        // Get all interrupt flags that are actually cleared with this instruction.
        unsigned char reset = nv & bitmask & irqflags;

        // reset flag, if written with 1
        irqflags ^= reset;
        // Walk through resetting flags beginning with the LSB ...
        for(unsigned char idx = 0; idx < lines.size(); ++idx)
            if(reset & (1<<idx))
                // ... and remove there pending calls from the irq system.
                ClearIrqFlag(lines[idx]->irqvector);
    }

    return nv;
}

unsigned char TimerIRQRegister::get_from_client(const IOSpecialReg* reg, unsigned char v) {
    // don't use v in this case, all bits under control of TimerIRQRegister
    // unused bits return 0
    if(reg == &timsk_reg)
        return irqmask;
    else
        return irqflags;
}

