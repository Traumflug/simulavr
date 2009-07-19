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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 *
 *  $Id$
 */

#ifndef RWMEM
#define RWMEM

#include "traceval.h"
#include "avrerror.h"
#include <string>       // std::string
#include <vector>

class TraceValue;
class AvrDevice;

//!Member of any memory area in an AVR device.
/*! Allows to be read and written byte-wise.
  Accesses can be traced if necessary. */
class RWMemoryMember {
    
    public:
        /*! Constructs a new memory member cell
          with the given trace value name. Index is used
          for memory-like structures which have indices.
          @param core pointer to AVRDevice instance
          @param tracename name of this memory cell, used by TraceValue
          @param index (optional) index to identify a cell in a group of memory cells*/
        RWMemoryMember(
            AvrDevice *core,
            const std::string &tracename="",
            const int index=-1);
#ifndef SWIG
        //! Read access on memory
        operator unsigned char() const;
#endif
        //! Write access on memory
        unsigned char operator=(unsigned char val);
        //! Write access on memory
        unsigned char operator=(const RWMemoryMember &mm);
        virtual ~RWMemoryMember();
        
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
        AvrDevice *core;
};

//! One byte in any AVR RAM
/*! Allows clean read and write accesses and simply has one stored byte. */
class RAM : public RWMemoryMember {
    
    public:
        RAM(AvrDevice *core,
            const std::string &tracename,
            const size_t number);
        
    protected:
        unsigned char get() const;
        void set(unsigned char);
        
    private:
        unsigned char value;
};

//! Memory on which access should be avoided! :-)
/*! All accesses to this type of memory will produce an error. */
class InvalidMem : public RWMemoryMember {
    public:
        InvalidMem(
            AvrDevice *core,
            const std::string &tracename,
            const size_t number);
        
    protected:
        unsigned char get() const;
        void set(unsigned char);
};

//! Memory blocks of RWMemoryMembers
/*! Memory offsets are used to represent a given memory area
  inside a block of RWMemoryMember objects. Used for RAM blocks and
  similar things. */
class MemoryOffsets {
    
    private:
        unsigned int myOffset;
        
    protected:
        RWMemoryMember **rwHandler;
        
    public:
        MemoryOffsets(unsigned int offset, RWMemoryMember **rw):rwHandler(rw){
            myOffset=offset;
        }
        unsigned getOffset() const { return myOffset; }
#ifndef SWIG
        RWMemoryMember &operator[](unsigned int externOffset) const;
#endif
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
        IOReg(AvrDevice *core,
              const std::string &tracename,
              P *_p,
              getter_t _g=0,
              setter_t _s=0):
            RWMemoryMember(core, tracename),
            p(_p),
            g(_g),
            s(_s)
        {
            // 'undefined state' doesn't really make sense for IO registers 
            if (tv)
                tv->set_written();
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
        //! @param nv the value, which is written to IO register (but maybe changed by other clients)
        //! @return nv, if nothing is changed or your changed value
        virtual unsigned char set_from_reg(unsigned char nv)=0;
        
        //! Informs your class, that a read access from IO register happens
        //! @param v the internal saved register value (but maybe changed by other clients)
        //! @return v, if nothing is changed or your changed value
        virtual unsigned char get_from_client(unsigned char v)=0;
};

//! IO register, which holds configuration for more than one hardware unit
//! \todo Set method could modify value, how to reflect it on TraceValue mechanism?
//! Same problem for the get method.
class IOSpecialReg: public RWMemoryMember {
    
    public:
        //! Creates a IOSpecialReg instance, see RWMemoryMember for more info
        IOSpecialReg(AvrDevice *core, const std::string &tracename);
        
        //! Registers a client to this IO register to inform this client on read or write access
        void connectSRegClient(IOSpecialRegClient *c) { clients.push_back(c); }
        
        //! Register reset functionality, sets internal register value to 0.
        void Reset(void) { value = 0; }
        
    protected:
        std::vector<IOSpecialRegClient*> clients; //!< clients-list with registered clients
        
        unsigned char get() const; //!< Get value method, see RWMemoryMember
        void set(unsigned char);   //!< Set value method, see RWMemoryMember
        
    private:
        unsigned char value; //!< Internal register value
};

#endif
