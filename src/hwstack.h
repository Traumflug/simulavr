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

#ifndef HWSTACK
#define HWSTACK

#include "rwmem.h"
#include "funktor.h"
#include "avrdevice.h"
#include "traceval.h"

#include <map>

//! Implements a stack register with stack logic
/*! This is the base class for all 2 different stack types. It holds the interface
    for pushing and poping bytes and addresses from stack by core and for interrupt */
class HWStack {
    
    protected:
        AvrDevice *core; //!< Link to device
        unsigned long stackPointer; //!< current value of stack pointer
        unsigned long lowestStackPointer; //!< marker: lowest stackpointer used by program
        std::multimap<unsigned long, Funktor*> returnPointList; //!< Maps adresses to listeners for return addresses

        void CheckReturnPoints(); //!< Checks PC to inform listeners, that a special address is arrived
        
    public:
        //! Creates a stack instance
        HWStack(AvrDevice *core);
        ~HWStack() {}

        virtual void Push(unsigned char val)=0; //!< Pushs one byte to stack
        virtual unsigned char Pop()=0; //!< Pops one byte from stack
        virtual void PushAddr(unsigned long addr)=0; //!< Pushs a address to stack
        virtual unsigned long PopAddr()=0; //!< Pops a address from stack

        virtual void Reset(); //!< Resets stack pointer and listener table

        //! Returns current stack pointer value
        unsigned long GetStackPointer() const { return stackPointer; }
        //! Sets current stack pointer value (used by GDB interface)
        void SetStackPointer(unsigned long val) { stackPointer = val; }
        //! Subscribes a Listener for a return address
        /*! Attention! SetReturnPoint must get a COPY of a Funktor because it
            self destroy this functor after usage! */
        void SetReturnPoint(unsigned long stackPointer, Funktor *listener);
        
        //! Sets lowest stack marker back to current stackpointer
        void ResetLowestStackpointer(void) { lowestStackPointer = stackPointer; }
        //! Gets back the lowest stack pointer (for measuring stack usage)
        unsigned long GetLowestStackpointer(void) { return lowestStackPointer; }
};

//! Implements a stack register with stack logic
class HWStackSram: public HWStack, public TraceValueRegister {
    
    protected:
        MemoryOffsets *mem;
        unsigned long stackCeil;
        bool initRAMEND;
        
        void SetSpl(unsigned char);
        void SetSph(unsigned char);
        unsigned char GetSpl();
        unsigned char GetSph();
        
    public:
        //! Creates a stack instance
        HWStackSram(AvrDevice *core, int bitsize, bool initRAMEND = false);

        virtual void Push(unsigned char val);
        virtual unsigned char Pop();
        virtual void PushAddr(unsigned long addr);
        virtual unsigned long PopAddr();

        virtual void Reset();
        
        IOReg<HWStackSram> sph_reg;
        IOReg<HWStackSram> spl_reg;
};

#if 0
/*! Implementation of the special three-level deep hardware stack which is
  not accessible in any memory space on the devices which have this, for
  example the ATTiny15L or the good old AT90S1200. */
class ThreeLevelStack : public MemoryOffsets {
  public:
    ThreeLevelStack(AvrDevice *core);
    ~ThreeLevelStack();
};
#endif

#endif
