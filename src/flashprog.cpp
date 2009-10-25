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

#include "flashprog.h"
#include "avrdevice.h"

//#include <iostream>
//using namespace std;

FlashProgramming::FlashProgramming(AvrDevice *c,
                                   unsigned int pgsz,
                                   unsigned int nrww):
    Hardware(c),
    pageSize(pgsz),
    nrww_addr(nrww),
    spmcr_reg(c, "SPMCR",
              this, &FlashProgramming::GetSpmcr, &FlashProgramming::SetSpmcr)
{
    c->AddToCycleList(this);
    Reset();
}

unsigned int FlashProgramming::CpuCycle() {
    if(opr_enable_count > 0)
        opr_enable_count--;
    return 0;
}

void FlashProgramming::Reset() {
    spmcr_val = 0;
    opr_enable_count = 0;
}

int FlashProgramming::SPM_action(unsigned int data, unsigned int xaddr, unsigned int addr) {
    int res = 0;
    if(opr_enable_count > 0)
        res = 1;
    //cout << "spm-action(0x" << hex << data << ",0x" << hex << addr << ",0x" << hex << xaddr << ")=" << res << endl;
    return res;
}

void FlashProgramming::SetSpmcr(unsigned char v) {
    spmcr_val = v & 0x80;
    //cout << "spmcr=0x" << hex << (unsigned int)v << endl;
    if(v & 0x1)
        opr_enable_count = 4;
}

unsigned char FlashProgramming::GetSpmcr() {
    //cout << "spmcr:0x" << hex << (unsigned int)spmcr_val << endl;
    return spmcr_val;
}

// EOF
