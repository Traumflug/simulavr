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

TimerIRQRegister::TimerIRQRegister(AvrDevice* core,
                                   HWIrqSystem* irqsys,
                                   int regidx):
    Hardware(core),
    irqsystem(irqsys),
    lines(8),
    timsk(core, "TIMSK" + (regidx == -1) ? "" : int2str(regidx)),
    tifr(core, "TIFR" + (regidx == -1) ? "" : int2str(regidx))
{
    timsk.connectSRegClient(this);
    tifr.connectSRegClient(this);
    Reset();
}

void TimerIRQRegister::registerLine(int idx, IRQLine irq) {
    // no check, if idx is in right range!
    irq.irqreg = this;
    lines[idx] = &irq;
    vector2line[irq.irqvector] = idx;
    name2line[irq.name] = idx;
}

IRQLine* TimerIRQRegister::getLine(const std::string& n) {
    std::map<std::string, int>::iterator cur  = name2line.find(n);
    if(cur == name2line.end())
        avr_error("IRQ line '%s' not found", n.c_str());
    return lines[name2line[n]];
}

void TimerIRQRegister::fireInterrupt(int irqvector) {
    int idx = vector2line[irqvector];
    if(irqmask & (1 << idx)) { // check irq mask
        irqflags |= (1 << idx);
        irqsystem->SetIrqFlag(this, irqvector);
    }
}

void TimerIRQRegister::ClearIrqFlag(unsigned int vector) {
    int idx = vector2line[vector];
    irqflags &= ~(1 << idx);
}

void TimerIRQRegister::Reset(void) {
    irqmask = 0;
    timsk.Reset();
    irqflags = 0;
    tifr.Reset();
}

unsigned char TimerIRQRegister::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    if(reg == &timsk)
        irqmask = nv;
    else {
        unsigned char newflags = (irqflags ^ nv) & nv;
        unsigned char mask = irqmask;
        for(int i = 0; newflags && i < lines.size(); i++) {
            if((newflags & 0x1) && (mask & 0x1)) {
                IRQLine* irq = lines[i];
                if(irq->irqvector >= 0)
                    irqsystem->SetIrqFlag(this, irq->irqvector);
            }
            newflags >>= 1;
            mask >>= 1;
        }
        irqflags = nv;
        
    }
    return nv;
}

unsigned char TimerIRQRegister::get_from_client(const IOSpecialReg* reg, unsigned char v) {
    // don't use v in this case
    if(reg == &timsk)
        return irqmask;
    else
        return irqflags;
}

