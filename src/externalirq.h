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

#ifndef EXTERNALIRQ
#define EXTERNALIRQ

#include <vector>
#include "hardware.h"
#include "irqsystem.h"
#include "rwmem.h"
#include "avrdevice.h"

class ExternalIRQ;

class ExternalIRQHandler: public Hardware, public IOSpecialRegClient {
    
    protected:
        HWIrqSystem *irqsystem; //!< pointer to irq system
        AvrDevice* core; //!< pointer to device
        IOSpecialReg *mask_reg; //!< the interrupt mask register
        IOSpecialReg *flag_reg; //!< the interrupt flag register
        std::vector<ExternalIRQ*> extirqs; //!< list with external IRQ's
        
    public:
        ExternalIRQHandler(AvrDevice* core, HWIrqSystem* irqsys, IOSpecialReg *mask, IOSpecialReg *flag);
        void registerIrq(int vector, int irqBit, ExternalIRQ* extirq);
        
        // from Hardware
        virtual void ClearIrqFlag(unsigned int vector);
        virtual void Reset(void);
        
        // from IOSpecialRegClient
        virtual unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        virtual unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v);
};

class ExternalIRQ: public IOSpecialRegClient {
  
    public:
        ExternalIRQ(IOSpecialReg *ctrl, int ctrlOffset, int ctrlBits);
        
        // from IOSpecialRegClient
        virtual unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        virtual unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v);
};

#endif
