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
#include "irqsystem.h"
//a map is allways sorted, so the priority of the irq vector is known and handled correctly
unsigned int HWIrqSystem::GetNewPc() {
    unsigned int newPC=0xffffffff;

    static map<unsigned int, Hardware *>::iterator ii;
    static map<unsigned int, Hardware *>::iterator end;
    end= irqPartnerList.end();

    for (ii=irqPartnerList.begin(); ii!=end; ii++) {
        Hardware* second= ii->second;
        unsigned int first= ii->first;

        //if (second->IsIrqFlagSet(first)) {
            second->ClearIrqFlag(first);
            return first*(bytesPerVector/2);
        //}
    }		


    return newPC;
}

void HWIrqSystem::RegisterIrqPartner(Hardware *hwp, unsigned int vector) {
    irqPartnerList[vector]=hwp;
}

void HWIrqSystem::SetIrqFlag(Hardware *hwp, unsigned int vector) {
    irqPartnerList[vector]=hwp;
}

void HWIrqSystem::ClearIrqFlag(unsigned int vector) {
    irqPartnerList.erase(vector);
}



