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

// FIXME: The following two includes are just because of the cerr stuff in IOReg!
#include "traceval.h"
#include <iostream>
#include <string>

class TraceValue;
class AvrDevice;

//!Member of any memory area in an AVR device.
/* !Allows to be read and written byte-wise.
   Accesses can be traced if wanted necessary. */
class RWMemoryMember {
 public:
    /*! Constructs a new memory member cell
      with the given trace value name. Index is used
      for memory-like structures which have indices.*/
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
/*! Allows clean read and write accesses and simply
  has one stored byte. */
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
class IOReg : public RWMemoryMember {
  public:
	typedef unsigned char(P::*getter_t)();
	typedef void (P::*setter_t)(unsigned char);
	/*!
	  \param _p: pointer to object this will be part of
	  \param _g: pointer to get method
	  \param _s: pointer to set method */
  IOReg(AvrDevice *core,
        const std::string &tracename,
        P *_p,
        getter_t _g=0, setter_t _s=0) :
    RWMemoryMember(core, tracename), p(_p), g(_g), s(_s) {
        // 'undefined state' doesn't really make sense for IO registers 
        if (tv)
            tv->set_written();
    }
  protected:
	unsigned char get() const {
	    if (g)
            return (p->*g)();
	    else if (tv)
            std::cerr << "Reading of '" << tv->name() << "' is not supported."
            << std::endl;
        return 0;
	}
	void set(unsigned char val) {
	    if (s)
            (p->*s)(val);
	    else if (tv)
            std::cerr << "Writing of '" << tv->name() << "' (with " << val << ") is not supported." << std::endl;
	}
  private:
	P *p;
	getter_t g;
	setter_t s;
}
;

#endif
