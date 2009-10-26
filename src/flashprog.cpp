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
#include "systemclock.h"
#include "avrmalloc.h"
#include "flash.h"

//#include <iostream>
//using namespace std;

void FlashProgramming::ClearOperationBits(void) {
    spmcr_val &= 0xe0;
    action = SPM_ACTION_NOOP;
    spm_opr = SPM_OPS_NOOP;
}

void FlashProgramming::SetRWWLock(unsigned int addr) {
    if(addr < (nrww_addr * 2))
        spmcr_val |= 0x40;
}

FlashProgramming::FlashProgramming(AvrDevice *c,
                                   unsigned int pgsz,
                                   unsigned int nrww):
    Hardware(c),
    core(c),
    pageSize(pgsz),
    nrww_addr(nrww),
    spmcr_reg(c, "SPMCR",
              this, &FlashProgramming::GetSpmcr, &FlashProgramming::SetSpmcr)
{
    tempBuffer = avr_new(unsigned char, pgsz * 2);
    core->AddToCycleList(this);
    Reset();
}

FlashProgramming::~FlashProgramming() {
    avr_free(tempBuffer);
}

unsigned int FlashProgramming::CpuCycle() {
    // SPM or LPM enable timeout
    if(opr_enable_count > 0) {
        opr_enable_count--;
        if(opr_enable_count == 0)
            ClearOperationBits();
    }
    if(action == SPM_ACTION_LOCKCPU) {
        if(SystemClock::Instance().GetCurrentTime() < timeout)
            return 1;
        ClearOperationBits();
    }
    return 0;
}

void FlashProgramming::Reset() {
    spmcr_val = 0;
    opr_enable_count = 0;
    action = SPM_ACTION_NOOP;
    spm_opr = SPM_OPS_NOOP;
    timeout = 0;
}

int FlashProgramming::SPM_action(unsigned int data, unsigned int xaddr, unsigned int addr) {
  
    // do nothing, if called from RWW section
    unsigned int pc = core->PC;
    if(pc < nrww_addr)
        return 0; // SPM operation is disabled, if executed from RWW section
      
    // calculate full address (RAMPZ:Z)
    addr = (addr & 0xffff) + (xaddr << 16);
    
    // process/start prepared operation
    if(action == SPM_ACTION_PREPARE) {
        opr_enable_count = 0;
        if(spm_opr == SPM_OPS_NOOP) {
            ClearOperationBits();
            //cout << "no opr: [0x" << hex << addr << "]" << endl;
            return 0;
        }
        if(spm_opr == SPM_OPS_UNLOCKRWW) {
            ClearOperationBits();
            spmcr_val &= ~0x40;
            //cout << "unlock rww: [0x" << hex << addr << "]" << endl;
            return 0; // is this right, 1 cpu clock for this operation?
        }
        if(spm_opr == SPM_OPS_STOREBUFFER) {
            // calculate page offset
            addr = addr & 0xfffe; // ignore LSB
            addr &= (pageSize * 2) - 1;
            // store data to buffer at offset
            tempBuffer[addr] = data & 0xff;
            tempBuffer[addr + 1] = (data >> 8) & 0xff;
            // signal: operation done.
            ClearOperationBits();
            //cout << "store buffer: [0x" << hex << addr << "]=0x" << hex << data << endl;
            return 2; // is this right, 3 cpu clocks for this operation?
        }
        if(spm_opr == SPM_OPS_WRITEBUFFER) {
            // calculate page address
            addr &= ~((pageSize * 2) - 1);
            // store temp buffer to flash
            core->Flash->WriteMem(tempBuffer, addr, pageSize * 2);
            // calculate system time, where operation is finished (+4ms)
            timeout = SystemClock::Instance().GetCurrentTime() + 4000000;
            // lock cpu while writing flash
            action = SPM_ACTION_LOCKCPU;
            // lock RWW, if necessary
            SetRWWLock(addr);
            //cout << "write buffer: [0x" << hex << addr << "]" << endl;
            return 0; // cpu clocks will be extended by CpuCycle calls
        }
        if(spm_opr == SPM_OPS_ERASE) {
            // calculate page address
            addr &= ~((pageSize * 2) - 1);
            // erase temp. buffer and store to flash
            for(int i = 0; i < (pageSize * 2); i++) tempBuffer[i] = 0xff;
            core->Flash->WriteMem(tempBuffer, addr, pageSize * 2);
            // calculate system time, where operation is finished (+4ms)
            timeout = SystemClock::Instance().GetCurrentTime() + 4000000;
            // lock cpu while erasing flash
            action = SPM_ACTION_LOCKCPU;
            // lock RWW, if necessary
            SetRWWLock(addr);
            //cout << "erase page: [0x" << hex << addr << "]" << endl;
            return 0; // cpu clocks will be extended by CpuCycle calls
        }
        //cout << "unknown spm-action(0x" << hex << data << ",0x" << hex << addr << ")" << endl;
    }
    return 0;
}

void FlashProgramming::SetSpmcr(unsigned char v) {
    spmcr_val = (spmcr_val & 0x60) + (v & 0x9f);
    
    // calculate operation
    if(action == SPM_ACTION_NOOP) {
        opr_enable_count = 4;
        action = SPM_ACTION_PREPARE;
        switch(spmcr_val & 0x1f) {
            case 0x1:
                spm_opr = SPM_OPS_STOREBUFFER;
                break;
                
            case 0x3:
                spm_opr = SPM_OPS_ERASE;
                break;
                
            case 0x5:
                spm_opr = SPM_OPS_WRITEBUFFER;
                break;
                
            case 0x9:
                spm_opr = SPM_OPS_NOOP;
                break;
                
            case 0x11:
                spm_opr = SPM_OPS_UNLOCKRWW;
                break;
                
            default:
                spm_opr = SPM_OPS_NOOP;
                if(!(spmcr_val & 0x1)) {
                    opr_enable_count = 0;
                    action = SPM_ACTION_NOOP;
                }
                break;
        }
    }
    //cout << "spmcr=0x" << hex << (unsigned int)spmcr_val << "," << action << "," << spm_opr << endl;
}

unsigned char FlashProgramming::GetSpmcr() {
    //cout << "spmcr:0x" << hex << (unsigned int)spmcr_val << endl;
    return spmcr_val;
}

// EOF
