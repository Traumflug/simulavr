/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph       
 * Copyright (C) 2009 Onno Kortmann
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

#ifndef RWMEM
#define RWMEM

#include <string>       // std::string
#include <vector>

#include "traceval.h"
#include "avrerror.h"
#include "hardware.h"

class TraceValue;

//!Member of any memory area in an AVR device.
/*! Allows to be read and written byte-wise.
  Accesses can be traced if necessary. */
class RWMemoryMember {
    
    public:
        /*! Constructs a new memory member cell
          with the given trace value name. Index is used
          for memory-like structures which have indices.
          @param registry pointer to TraceValueRegister instance
          @param tracename name of this memory cell, used by TraceValue
          @param index (optional) index to identify a cell in a group of memory cells*/
        RWMemoryMember(
            TraceValueRegister *registry,
            const std::string &tracename="",
            const int index=-1);
        /*! Constructs a new memory member cell
          with no trace value name. (for InvalidRam) */
        RWMemoryMember(void);
#ifndef SWIG
        //! Read access on memory
        operator unsigned char() const;
        //! Write access on memory
        unsigned char operator=(unsigned char val);
        //! Write access on memory
        unsigned char operator=(const RWMemoryMember &mm);
#endif
        virtual ~RWMemoryMember();
        const std::string &GetTraceName(void) { return tracename; }
        bool IsInvalid(void) const { return isInvalid; } 

    protected:
        /*! This function is the function which will
          be called by the above access operators and
          is expected to do the real work when writing a byte. */
        virtual void set(unsigned char nv)=0;
        /*! This function as the oppposite to get() is
          expected to read the real byte. */
        virtual unsigned char get() const=0;
    
        /*! If non-null, this is the tracing value
          bound to this memory member. All read/write
          operators on the contents of a memory member
          will inform the tracing value of changes and
          accesses, if applicable. */
        mutable TraceValue *tv;
        TraceValueRegister *registry;
        const std::string tracename;
        const bool isInvalid;
};

//! A register in IO register space unrelated to any peripheral. "GPIORx" in datasheets.
/*! Allows clean read and write accesses and simply has one stored byte. */
class GPIORegister: public RWMemoryMember, public Hardware {
    
    public:
        GPIORegister(AvrDevice *core,
                     TraceValueRegister *registry,
                     const std::string &tracename):
            RWMemoryMember(registry, tracename),
            Hardware(core)
    { value = 0; }
        
        // from Hardware
        void Reset(void) { value = 0; }
        
    protected:
        unsigned char get() const { return value; }
        void set(unsigned char v) { value = v; }
        
    private:
        unsigned char value;
};

//! Implement CLKPR register
class CLKPRRegister: public RWMemoryMember, public Hardware {

    public:
        CLKPRRegister(AvrDevice *core,
                      TraceValueRegister *registry);

        // from Hardware
        void Reset(void);
        unsigned int CpuCycle(void);

    protected:
        unsigned char get() const { return value; }
        void set(unsigned char v);

    private:
        AvrDevice *_core;
        unsigned char value;
        unsigned char activate;
};

//! Implement XDIV register
class XDIVRegister: public RWMemoryMember, public Hardware {

    public:
        XDIVRegister(AvrDevice *core,
                     TraceValueRegister *registry);

        // from Hardware
        void Reset(void) { value = 0; }

    protected:
        unsigned char get() const { return value; }
        void set(unsigned char v);

    private:
        unsigned char value;
};

//! Implement OSCCAL register
class OSCCALRegister: public RWMemoryMember, public Hardware {

    public:
        // select type of oscillator, see AVR053 application note!
        enum {
            OSCCAL_V3 = 0, //!< oscillator version 3.x and older, 8bit, one range
            OSCCAL_V4 = 1, //!< oscillator version 4.x, 7bit, one range
            OSCCAL_V5 = 2  //!< oscillator version 5.x, 8bit, two ranges
        };

        OSCCALRegister(AvrDevice *core,
                       TraceValueRegister *registry,
                       int cal);

        // from Hardware
        void Reset(void);

    protected:
        unsigned char get() const { return value; }
        void set(unsigned char v);

    private:
        unsigned char value;
        int cal_type;
};

//! One byte in any AVR RAM
/*! Allows clean read and write accesses and simply has one stored byte. */
class RAM : public RWMemoryMember {
    
    public:
        RAM(TraceValueCoreRegister *registry,
            const std::string &tracename,
            const size_t number,
            const size_t maxsize);
        
    protected:
        unsigned char get() const;
        void set(unsigned char);
        
    private:
        unsigned char value;
        TraceValueCoreRegister *corereg;
};

//! Memory on which access should be avoided! :-)
/*! All accesses to this type of memory will produce an error. */
class InvalidMem : public RWMemoryMember {
    private:
        AvrDevice* core;
        int addr;
        
    public:
        InvalidMem(AvrDevice *core, int addr);
        
    protected:
        unsigned char get() const;
        void set(unsigned char);
};

//! An IO register which is not simulated because programmers are lazy.
/*! Reads and writes are ignored and produce warning. */
class NotSimulatedRegister : public RWMemoryMember {
    private:
        const char * message_on_access;

    public:
        NotSimulatedRegister(const char * message_on_access);

    protected:
        unsigned char get() const;
        void set(unsigned char);
};

//! IO register to be specialized for a certain class/hardware
/*! The template parameter class P specifies the class type in which
  the io register resides. */
template<class P>
class IOReg: public RWMemoryMember {
    
    public:
        typedef unsigned char(P::*getter_t)();
        typedef void (P::*setter_t)(unsigned char);
        /*! Creates an IO control register for controlling hardware units
          \param _p: pointer to object this will be part of
          \param _g: pointer to get method
          \param _s: pointer to set method */
        IOReg(TraceValueRegister *registry,
              const std::string &tracename,
              P *_p,
              getter_t _g=0,
              setter_t _s=0):
            RWMemoryMember(registry, tracename),
            p(_p),
            g(_g),
            s(_s)
        {
            // 'undefined state' doesn't really make sense for IO registers 
            if (tv)
                tv->set_written();
        }
        
        /*! Reflects a value change from hardware (for example timer count occured)
          @param val the new register value */
        void hardwareChange(unsigned char val) { if(tv) tv->change(val); }
        /*! Releases the TraceValue to hide this IOReg from registry */
        void releaseTraceValue(void) {
            if(tv) {
                registry->UnregisterTraceValue(tv);
                delete tv;
                tv = NULL;
            }
        }
        
    protected:
        unsigned char get() const {
            if (g)
                return (p->*g)();
            else if (tv) {
                avr_warning("Reading of '%s' is not supported.", tv->name().c_str());
            }
            return 0;
        }
        void set(unsigned char val) {
            if (s)
                (p->*s)(val);
            else if (tv) {
                avr_warning("Writing of '%s' (with %d) is not supported.", tv->name().c_str(), val);
            }
        }
        
    private:
        P *p;
        getter_t g;
        setter_t s;
};

class IOSpecialReg;

//! Interface class to connect hardware units to control registers
/*! This interface gives hardware, theres functionality depends on IO registers,
  which are not special for this hardware (maybe only a reset bit for a prescaler)
  the possibillity to react on write access to such register and to reflect some
  internal states to bits of such register, like async state on some timers,
  which are set to be clocked from external clock.
  
  To use this interface, let your hardware class inherit from this interface and
  implement set_from_reg and get_from_client. The simplest body for both
  functions would be "return nv;" and "return v;", means to change or reflect
  nothing. But in every case your hardware is informed on reading or writing
  to that IO register.*/
class IOSpecialRegClient {
    
    protected:
        friend class IOSpecialReg;
        
        //! Informs your class, that a write access to IO register is happen
        //! @param reg caller register instance
        //! @param nv the value, which is written to IO register (but maybe changed by other clients)
        //! @return nv, if nothing is changed or your changed value
        virtual unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv)=0;
        
        //! Informs your class, that a read access from IO register happens
        //! @param v the internal saved register value (but maybe changed by other clients)
        //! @return v, if nothing is changed or your changed value
        virtual unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v)=0;

    public:
        virtual ~IOSpecialRegClient() {}

};

//! IO register, which holds configuration for more than one hardware unit
//! \todo Set method could modify value, how to reflect it on TraceValue mechanism?
//! Same problem for the get method.
class IOSpecialReg: public RWMemoryMember {
    
    public:
        //! Creates a IOSpecialReg instance, see RWMemoryMember for more info
        IOSpecialReg(TraceValueRegister *registry, const std::string &tracename);
        
        //! Registers a client to this IO register to inform this client on read or write access
        void connectSRegClient(IOSpecialRegClient *c) { clients.push_back(c); }
        
        //! Register reset functionality, sets internal register value to 0.
        void Reset(void) { Reset(0); }
        //! Register reset functionality, sets internal register value to val.
        //! @param val the reset value
        void Reset(unsigned char val) { value = 0; if(tv) tv->set_written(val); }
        
        /*! Reflects a value change from hardware (for example timer count occured)
          @param val the new register value */
        void hardwareChange(unsigned char val) { if(tv) tv->change(val); }
        
        /*! Reflects a value change from hardware (for example timer count occured), but with bitmask
          @param val the new register value
          @param mask the bitmask for val */
        void hardwareChangeMask(unsigned char val, unsigned char mask) { if(tv) tv->change(val, mask); }
        
    protected:
        std::vector<IOSpecialRegClient*> clients; //!< clients-list with registered clients
        
        unsigned char get() const; //!< Get value method, see RWMemoryMember
        void set(unsigned char);   //!< Set value method, see RWMemoryMember
        
    private:
        unsigned char value; //!< Internal register value
};

#endif
