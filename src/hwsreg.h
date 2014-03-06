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

#ifndef HWSREG
#define HWSREG
#include <string>

#include "rwmem.h"
#include "traceval.h"

/* this is the fastes solution, tested on Ubuntu and WinXP */
class HWSreg_bool {
    
    public:
        bool    I;
        bool    T;
        bool    H;
        bool    S;
        bool    V;
        bool    N;
        bool    Z;
        bool    C;
#ifndef SWIG
        operator int();
#endif
        HWSreg_bool(const int i);
        HWSreg_bool();
};

class HWSreg: public HWSreg_bool {
    
    public:
#ifndef SWIG
        operator std::string();
        HWSreg operator =(const int );
#endif
};

/*! SREG - ALU status register in IO space
  \todo Replace the status register with an ordinary byte somewhere and simple
  inline access functions sN(), gN() to get/set flags. This should also make
  accesses faster. */
class RWSreg: public RWMemoryMember {
    
    public:
        RWSreg(TraceValueRegister *registry, HWSreg *s): RWMemoryMember(registry, "SREG"), status(s) {}
        //! reflect a change, which comes from CPU core
        void trigger_change(void) { tv->change((int)*status); }

    protected:
        HWSreg *status;
        unsigned char get() const;
        void set(unsigned char);
};

#endif
