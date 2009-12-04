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

#include "externalirq.h"

ExternalIRQHandler::ExternalIRQHandler(AvrDevice* c,
                                       HWIrqSystem* irqsys,
                                       IOSpecialReg *mask,
                                       IOSpecialReg *flag):
    Hardware(c)
{
}

void ExternalIRQHandler::registerIrq(int vector, int irqBit, ExternalIRQ* extirq) {
}

void ExternalIRQHandler::ClearIrqFlag(unsigned int vector) {
}

void ExternalIRQHandler::Reset(void) {
}

unsigned char ExternalIRQHandler::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    return nv;
}

unsigned char ExternalIRQHandler::get_from_client(const IOSpecialReg* reg, unsigned char v) {
    return v;
}

ExternalIRQ::ExternalIRQ(IOSpecialReg *ctrl, int ctrlOffset, int ctrlBits) {
}

unsigned char ExternalIRQ::set_from_reg(const IOSpecialReg* reg, unsigned char nv) {
    return nv;
}

unsigned char ExternalIRQ::get_from_client(const IOSpecialReg* reg, unsigned char v) {
    return v;
}

// EOF
