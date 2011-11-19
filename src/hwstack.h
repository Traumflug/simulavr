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

/** A thread automatically detected in simulated program.
* We keep track of them in core->stack.m_ThreadList.m_threads[] and
* report them to GDB.
*/
class Thread
{
public:
    /// Stack Pointer. Address 0x0000 is invalid; used for running thread. GDB never sees the 0.
    int m_sp;
    int m_ip;  ///< address (in bytes, not index)
    bool m_alive;
    /** State of R0 - R31 registers at last call-site. GDB's prologue analyzer is weak
    * and would not unwind the stack at "switch-site" - but it would on call-site. */
    unsigned char registers[32];
    int m_created_by_thread;
};

/** List of auto-detected threads. See my Google Docs notes.
Stack address 0x0000 is invalid (see datasheet).
*/
class ThreadList
{
    /// List of known threads. First addition (of main) is special.
    std::vector<Thread*> m_threads;
    enum {eNormal, eReaded, eWritten, eWritten2} m_phase_of_switch;
    int m_last_SP_read;
    int m_last_SP_writen;
    int m_on_call_sp;
    int m_on_call_ip;  ///< Address in a routine calling the context-switch
    /// Currently running thread. (Thread index used for querying by GDB is in GdbServer.)
    int m_cur_thread;
    AvrDevice & m_core;

    ThreadList& operator=(const ThreadList&);  // not assignable
public:

    ThreadList(AvrDevice & core);
    ~ThreadList();
    void OnReset();
    void OnCall();
    void OnSPRead(int SP_value);
    void OnSPWrite(int new_SP);
    void OnPush();
    void OnPop();
    int GetThreadBySP(int SP) const;  ///< Search threads

    int GetCurrentThreadForGDB() const;  ///< Get GDB-style thread ID (the first is 1)
    const Thread * GetThreadFromGDB(int thread_id) const;
    bool IsGDBThreadAlive(int thread_id) const;  ///< GDB-style thread ID (the first is 1)
    int GetCount() const;
};

//! Implements a stack register with stack logic
/*! This is the base class for all 2 different stack types. It holds the interface
    for pushing and poping bytes and addresses from stack by core and for interrupt */
class HWStack {
    
    protected:
        AvrDevice *core; //!< Link to device
        uint32_t stackPointer; //!< current value of stack pointer
        uint32_t lowestStackPointer; //!< marker: lowest stackpointer used by program
        std::multimap<unsigned long, Funktor*> returnPointList; //!< Maps adresses to listeners for return addresses

        /// Run functions registered for current stack address and delete them
        void CheckReturnPoints();
        
    public:
        ThreadList m_ThreadList;  ///< List of known threads created within target.
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

//! Implements a stack with stack register using RAM as stackarea
class HWStackSram: public HWStack, public TraceValueRegister {
    
    protected:
        unsigned long stackCeil;
        bool initRAMEND;
        
        void SetSpl(unsigned char);
        void SetSph(unsigned char);
        unsigned char GetSpl();
        unsigned char GetSph();
        void OnSPReadByTarget();
        
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

//! Implements a stack with 3 levels deep (used as returnstack by ATtiny15 an other)
class ThreeLevelStack: public HWStack, public TraceValueRegister {
    
    protected:
        unsigned long *stackArea;
        
    public:
        ThreeLevelStack(AvrDevice *core);
        ~ThreeLevelStack();
        
        virtual void Push(unsigned char val);
        virtual unsigned char Pop();
        virtual void PushAddr(unsigned long addr);
        virtual unsigned long PopAddr();

        virtual void Reset();
};

#endif
