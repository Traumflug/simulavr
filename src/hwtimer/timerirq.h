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

#ifndef TIMERIRQ
#define TIMERIRQ
#include <string>
#include <vector>
#include "hardware.h"
#include "irqsystem.h"
#include "avrdevice.h"
#include "rwmem.h"
#include "traceval.h"

class TimerIRQRegister;

//! Represents a timer interrupt line, Frontend for timer interrupts.
/*! This class represents a interrupt line and holds the connection to interrupt system
  and mask/flag register. It handles set and clear functionality for the registers,
  takes respect of mask bits and allows to fire a interrupt, if necessary. */
class IRQLine {
    
    protected:
        friend class TimerIRQRegister;
        
        int irqvector; //!< the IRQ vector number in interrupt table, starting with 0
        std::string name; //!< name of this IRQ line
        TimerIRQRegister *irqreg; //!< pointer to irq registers, where this line is hold
        
    public:
        //! Creates a IRQLine instance, to use in connection with TimerIRQRegister and timers
        IRQLine(const std::string& name, int irqvector);
        //! inform interrupt system, that an interrupt occured
        void fireInterrupt(void);
};

//! Provices flag and mask register for timer interrupts and connects irq lines to irqsystem
class TimerIRQRegister: public Hardware, public IOSpecialRegClient, public TraceValueRegister {
    
    private:
        HWIrqSystem* irqsystem; //!< pointer to irq system
        AvrDevice* core; //!< pointer to device
        std::vector<IRQLine*> lines; //!< list with IRQ lines
        std::map<std::string, int> name2line; //!< mapping IRQ line name to index
        std::map<int, int> vector2line; //!< mapping IRQ vector to index
        unsigned char irqmask; //!< mask register value;
        unsigned char irqflags; //!< flag register value;
        unsigned char bitmask; //!< mask for used bits in registers
        
    public:
        IOSpecialReg timsk_reg; //!< the TIMSKx register
        IOSpecialReg tifr_reg; //!< the TIFRx register
        
        TimerIRQRegister(AvrDevice* core, HWIrqSystem* irqsys, int regidx = -1);
        void registerLine(int idx, IRQLine* irq);
        IRQLine* getLine(const std::string& name);
        void fireInterrupt(int irqvector);
        
        virtual void ClearIrqFlag(unsigned int vector);
        virtual void Reset(void);
        
        virtual unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        virtual unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v);
};

#endif // TIMERIRQ
