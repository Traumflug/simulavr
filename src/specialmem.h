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

/*@file special memory types which are rarely used
  are defined here. */
#ifndef SPECIALMEM
#define SPECIALMEM

#include <fstream>
#include "rwmem.h"

//! FIFO write memory
/*! Memory register which will redirect all write
  accesses to the given (FIFO) file. The output
  format in the file is binary. */
class RWWriteToFile: public RWMemoryMember {
 public:
    /*! The output filename can be '-' which will
      make this object use cout then. */
    RWWriteToFile(TraceValueRegister *registry,
                  const std::string &tracename,
                  const std::string &filename);
 protected:
    unsigned char get() const;
    void set(unsigned char);

    std::ostream &os;
    std::ofstream ofs;
};

//! FIFO read memory
/*! Memory register which will fulfill all reads with
  a byte drawn from a given (FIFO) file. The input
  format is binary. */
class RWReadFromFile: public RWMemoryMember {
 public:
    /*! The input filename can be '-' which will
      make this object use cin then. */
    RWReadFromFile(TraceValueRegister *registry,
                   const std::string &tracename,
                   const std::string &filename);
 protected:
    unsigned char get() const;
    void set(unsigned char);

    std::istream &is;
    mutable std::ifstream ifs;
};

//! exit() on access memory
/*! Any access to this memory will exit the simulator.
  If a byte is written, it will be return code of the simulavr
  process. If a byte is being read, the exit code is 0x00. */
class RWExit: public RWMemoryMember {
 public:
    RWExit(TraceValueRegister *registry, const std::string &tracename="");
 protected:
    unsigned char get() const;
    void set(unsigned char);
};

//! abort() on access memory
/*! Any access to this memory will instantly stop simulavr. */
class RWAbort: public RWMemoryMember {
 public:
    RWAbort(TraceValueRegister *registry, const std::string &tracename="");
 protected:
    unsigned char get() const;
    void set(unsigned char);
};

#endif
