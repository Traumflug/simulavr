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

#ifndef FLASHPROG
#define FLASHPROG

#include "rwmem.h"
#include "hardware.h"

class AvrDevice;

//! Provides the programming engine for flash self programming
class FlashProgramming: public Hardware {
  
    protected:
        unsigned int pageSize;  //!< page size in words
        unsigned int nrww_addr; //!< start address of non RWW area of flash
        unsigned char spmcr_val; //!< holds the register value
        int opr_enable_count; //!< enable counter for SPM operation
        
    public:
        //! Create a instance of FlashProgramming class
        FlashProgramming(AvrDevice *c, unsigned int pgsz, unsigned int nrww);
        
        unsigned int CpuCycle();
        void Reset();
        
        int SPM_action(unsigned int data, unsigned int xaddr, unsigned int addr);
        void SetSpmcr(unsigned char v);
        unsigned char GetSpmcr();

        IOReg<FlashProgramming> spmcr_reg;
        
};

#endif
