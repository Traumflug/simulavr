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

#ifndef EXTERNALIRQ_INCLUDED
#define EXTERNALIRQ_INCLUDED

#include <vector>

#include "hardware.h"
#include "irqsystem.h"
#include "rwmem.h"
#include "avrdevice.h"
#include "pinnotify.h"
#include "hwport.h"

class ExternalIRQ;

//! Handler for external IRQ's to communicate with IRQ system and mask/flag registers
class ExternalIRQHandler: public Hardware, public IOSpecialRegClient {
    
    protected:
        HWIrqSystem *irqsystem; //!< pointer to irq system
        IOSpecialReg *mask_reg; //!< the interrupt mask register
        IOSpecialReg *flag_reg; //!< the interrupt flag register
        std::vector<ExternalIRQ*> extirqs; //!< list with external IRQ's
        unsigned char irq_mask; //!< mask register value for registered IRQ's
        unsigned char irq_flag; //!< flag register value for registered IRQ's
        unsigned char reg_mask; //!< mask for relevant bits in flag and mask register
        std::vector<int> vectors; //!< mapping index to vector
        std::vector<int> irqbits; //!< mapping index to mask bit
        std::map<int, int> vector2idx; //!< mapping irq vector to index
        
        void fireInterrupt(int idx); //!< fire a interupt from IRQ with index
        
        friend class ExternalIRQ;
        
    public:
        ExternalIRQHandler(AvrDevice* core, HWIrqSystem* irqsys, IOSpecialReg *mask, IOSpecialReg *flag);
        ~ExternalIRQHandler();
        void registerIrq(int vector, int irqBit, ExternalIRQ* extirq);
        
        // from Hardware
        virtual void ClearIrqFlag(unsigned int vector);
        virtual void Reset(void);
        virtual bool IsLevelInterrupt(unsigned int vector);
        virtual bool LevelInterruptPending(unsigned int vector);
        
        // from IOSpecialRegClient
        virtual unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        virtual unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v);
        
};

//! Basic handler for one external IRQ, handles control register
class ExternalIRQ: public IOSpecialRegClient {
    
    protected:
        int handlerIndex; //!< my own index on handler instance
        ExternalIRQHandler *handler; //!< reference to IRQ handler
        int bitshift; //!< how many bits to shift to get mode from control register
        unsigned char mask; //!< mask for extract mode from control register
        unsigned char mode; //!< control mode from control register
        
        //! register handler and index for signaling interrupt
        void setHandlerIndex(ExternalIRQHandler *h, int idx) { handler = h; handlerIndex = idx; }
        //! fire a interrupt
        void fireInterrupt(void) { handler->fireInterrupt(handlerIndex); }
        //! Reset mode
        virtual void ResetMode(void) { mode = 0; }
        //! Handle change of control register
        virtual void ChangeMode(unsigned char m) { mode = m; }
        //! does the interrupt source fire again? (for interrupt on level)
        virtual bool fireAgain(void) { return false; }
        //! does fire interrupt set the interrupt flag? (level interrupt does this not!)
        virtual bool mustSetFlagOnFire(void) { return true; }
        
        friend class ExternalIRQHandler;
        
    public:
        ExternalIRQ(IOSpecialReg *ctrl, int ctrlOffset, int ctrlBits);
        
        // from IOSpecialRegClient
        virtual unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        virtual unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v);
};

//! External interrupt (INT0, INT1...) on a single pin, one and 2 bit configuration
class ExternalIRQSingle: public ExternalIRQ, public HasPinNotifyFunction {
    
    protected:
        bool state; //!< saved state from pin
        bool twoBitMode; //!< IRQ is controlled by 2 mode bits
        bool mode8515; //!< at90s8515 don't support MODE_EDGE_ALL
        
        enum {
            MODE_LEVEL_LOW = 0, //!< Fire interrupt on low level
            MODE_EDGE_ALL =  1, //!< Fire interrupt on any logical change
            MODE_EDGE_FALL = 2, //!< Fire interrupt on falling edge
            MODE_EDGE_RISE = 3  //!< Fire interrupt on rising edge
        };
        
    public:
        ExternalIRQSingle(IOSpecialReg *ctrl, int ctrlOffset, int ctrlBits, Pin *pin, bool _8515mode = false);
        
        // from ExternalIRQ
        void ChangeMode(unsigned char m);
        bool fireAgain(void);
        bool mustSetFlagOnFire(void);
        
        // from HasPinNotifyFunction
        void PinStateHasChanged(Pin *pin);
};

//! Pin-change interrupt on all pins of a port
class ExternalIRQPort: public ExternalIRQ, public HasPinNotifyFunction {
    
    protected:
        bool state[8]; //!< saved states from all pins
        Pin* pins[8]; //!< pins of port for identifying, which bit is changed
        unsigned int portSize; //!< how much pins the port controls
        
    public:
        ExternalIRQPort(IOSpecialReg *ctrl, HWPort *port);
        
        // from HasPinNotifyFunction
        void PinStateHasChanged(Pin *pin);
};

#endif
