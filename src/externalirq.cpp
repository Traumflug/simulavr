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

#include "externalirq.h"
#include "avrerror.h"

ExternalIRQHandler::ExternalIRQHandler(AvrDevice* c,
                                       HWIrqSystem* irqsys,
                                       IOSpecialReg *mask,
                                       IOSpecialReg *flag):
    Hardware(c),
    irqsystem(irqsys)
{
    // connect mask and flag register to getter and setter method
    mask_reg = mask;
    mask_reg->connectSRegClient(this);
    flag_reg = flag;
    flag_reg->connectSRegClient(this);
    
    // set mask and flag values
    reg_mask = 0;
    
    Reset();
}

ExternalIRQHandler::~ExternalIRQHandler(void) {
    for(unsigned int idx = 0; idx < extirqs.size(); idx++)
        delete extirqs[idx];
}

void ExternalIRQHandler::registerIrq(int vector, int irqBit, ExternalIRQ* extirq) {
    irqsystem->DebugVerifyInterruptVector(vector, this);
    // set mask for relevant bits
    reg_mask |= 1 << irqBit;
    // register IRQ, mask bit and vector
    extirqs.push_back(extirq);
    vectors.push_back(vector);
    irqbits.push_back(irqBit);
    int idx = extirqs.size() - 1;
    vector2idx[vector] = idx;
    // communicate index to IRQ
    extirq->setHandlerIndex(this, idx);
}

void ExternalIRQHandler::fireInterrupt(int idx) {
    int irqBit = irqbits[idx];
    if(extirqs[idx]->mustSetFlagOnFire())
        irq_flag |= (1 << irqBit); // must not set in case of level interrupt
    flag_reg->hardwareChangeMask(irq_flag, reg_mask);
    if(irq_mask & (1 << irqBit)) // check irq mask and trigger interrupt
        irqsystem->SetIrqFlag(this, vectors[idx]);
}

void ExternalIRQHandler::ClearIrqFlag(unsigned int vector) {
    // get index from vector
    int idx = vector2idx[vector];
    // reset flag
    irq_flag &= ~(1 << irqbits[idx]);
    flag_reg->hardwareChangeMask(irq_flag, reg_mask);
    // signal: irq done!
    irqsystem->ClearIrqFlag(vector);
    // have to trigger again?
    if(extirqs[idx]->fireAgain()) {
        if(irq_mask & (1 << irqbits[idx])) // check irq mask and trigger interrupt
            irqsystem->SetIrqFlag(this, vectors[idx]);
    }
}

bool ExternalIRQHandler::IsLevelInterrupt(unsigned int vector) {
    // get index from vector
    int idx = vector2idx[vector];
    return !extirqs[idx]->mustSetFlagOnFire();
}

bool ExternalIRQHandler::LevelInterruptPending(unsigned int vector) {
    // get index from vector
    int idx = vector2idx[vector];
    return extirqs[idx]->fireAgain() && irq_mask & (1 << irqbits[idx]);
}

void ExternalIRQHandler::Reset(void) {
    irq_mask = 0;
    irq_flag = 0;
    for(unsigned int idx = 0; idx < extirqs.size(); idx++)
        extirqs[idx]->ResetMode();
}

unsigned char ExternalIRQHandler::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    if(reg == mask_reg) {
        // mask register: trigger interrupt, if mask bit is new set and flag is true or fireAgain()
        for(unsigned int idx = 0; idx < irqbits.size(); idx++) {
            unsigned char m = 1 << irqbits[idx];
            if(((nv & m) != 0) &&
               ((irq_mask & m) == 0) &&
               (((irq_flag & m) != 0) || extirqs[idx]->fireAgain()))
                irqsystem->SetIrqFlag(this, vectors[idx]);
        }
        // store value
        irq_mask = nv & reg_mask;
    } else {
        // flag register
        irq_flag ^= nv & reg_mask & irq_flag;
        // change value
        nv = (nv & ~reg_mask) | irq_flag;
    }
    return nv;
}

unsigned char ExternalIRQHandler::get_from_client(const IOSpecialReg* reg, unsigned char v) {
    if(reg == mask_reg)
        // mask register
        v = (v & ~reg_mask) | irq_mask;
    else
        // flag register
        v = (v & ~reg_mask) | irq_flag;
    return v;
}

ExternalIRQ::ExternalIRQ(IOSpecialReg *ctrl, int ctrlOffset, int ctrlBits) {
    handlerIndex = -1;
    handler = NULL;
    bitshift = ctrlOffset;
    mask = ((1 << ctrlBits) - 1) << bitshift;
    
    ctrl->connectSRegClient(this);
}

unsigned char ExternalIRQ::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    ChangeMode((nv & mask) >> bitshift);
    return nv;
}

unsigned char ExternalIRQ::get_from_client(const IOSpecialReg* reg, unsigned char v) {
    return (v & ~mask) | (mode << bitshift);
}

ExternalIRQSingle::ExternalIRQSingle(IOSpecialReg *ctrl, int ctrlOffset, int ctrlBits, Pin *pin, bool _8515mode):
    ExternalIRQ(ctrl, ctrlOffset, ctrlBits)
{
    state = (bool)*pin;
    twoBitMode = (ctrlBits == 2);
    mode8515 = _8515mode;
    pin->RegisterCallback(this);
    ResetMode();
}

void ExternalIRQSingle::PinStateHasChanged(Pin *pin) {
    // new state
    bool s = (bool)*pin;
    
    // handle changes
    switch(mode) {
        case MODE_LEVEL_LOW:
            // interrupt on low level
            if(s == false)
                fireInterrupt();
            break;
        case MODE_EDGE_ALL:
            // interrupt on any logical change
            if(mode8515)
                break; // do nothing, because at90s8515 don't support this mode
            if(s != state)
                fireInterrupt();
            break;
        case MODE_EDGE_FALL:
            // interrupt on falling edge
            if((s == false) && (state == true))
                fireInterrupt();
            break;
        case MODE_EDGE_RISE:
            // interrupt on rising edge
            if((s == true) && (state == false))
                fireInterrupt();
            break;
    }
    
    // store state
    state = s;
}

void ExternalIRQSingle::ChangeMode(unsigned char m) {
    if(twoBitMode)
        mode = m;
    else
        mode = m + MODE_EDGE_FALL;
    if(mode8515 && mode == MODE_EDGE_ALL)
        avr_warning("External irq mode ISCx1:ISCx0 = 0:1 isn't supported here");
}

bool ExternalIRQSingle::fireAgain(void) {
    return (mode == MODE_LEVEL_LOW) && (state == false);
}

bool ExternalIRQSingle::mustSetFlagOnFire(void) {
    return mode != MODE_LEVEL_LOW;
}

ExternalIRQPort::ExternalIRQPort(IOSpecialReg *ctrl, HWPort *port):
    ExternalIRQ(ctrl, 0, port->GetPortSize())
{
    portSize = port->GetPortSize();
    for(unsigned int idx = 0; idx < 8; idx++) {
        if(idx < portSize) {
            Pin *p = &port->GetPin((unsigned char)idx);
            pins[idx] = p;
            state[idx] = (bool)*p;
            p->RegisterCallback(this);
        } else {
            pins[idx] = NULL;
            state[idx] = false;
        }
    }
    ResetMode();
}

void ExternalIRQPort::PinStateHasChanged(Pin *pin) {
    // new state
    bool s = (bool)*pin;
    
    // get bit of port
    unsigned int idx = 0;
    unsigned char m = 1;
    for(; idx < portSize; idx++, m <<= 1) {
        if(pin == pins[idx]) {
            // is mask bit is set and logical change?
            if(((m & mode) != 0) && (s != state[idx]))
                fireInterrupt();
            // store new state
            state[idx] = s;
            break;
        }
    }
}

// EOF
