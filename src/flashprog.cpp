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

#include "flashprog.h"
#include "avrdevice.h"
#include "systemclock.h"
#include "avrmalloc.h"
#include "flash.h"

//#include <iostream>
//using namespace std;

void FlashProgramming::ClearOperationBits(void) {
    spmcr_val &= ~spmcr_opr_bits;
    action = SPM_ACTION_NOOP;
    spm_opr = SPM_OPS_NOOP;
}

void FlashProgramming::SetRWWLock(unsigned int addr) {
    // no op, if not in ATMega mode
    if(!isATMega)
        return;
    // set lock, if addr in RWW area
    if(addr < (nrww_addr * 2)) {
        spmcr_val |= 0x40;
        core->Flash->SetRWWLock(nrww_addr * 2);
    }
}

FlashProgramming::FlashProgramming(AvrDevice *c,
                                   unsigned int pgsz,
                                   unsigned int nrww,
                                   int mode):
    Hardware(c),
    pageSize(pgsz),
    nrww_addr(nrww),
    core(c),
    spmcr_reg(c, "SPMCR",
              this, &FlashProgramming::GetSpmcr, &FlashProgramming::SetSpmcr)
{
    // initialize hidden buffer
    tempBuffer = avr_new(unsigned char, pgsz * 2);
    for(unsigned int i = 0; i < (pageSize * 2); i++)
        tempBuffer[i] = 0xff;
    
    // set masks and modes
    isATMega = (mode & SPM_MEGA_MODE) == SPM_MEGA_MODE;
    spmcr_opr_bits = 0x1f;
    if((mode & SPM_SIG_OPR) == SPM_SIG_OPR)
        // extra operation on bit 5 available
        spmcr_opr_bits |= 0x20;
    spmcr_valid_bits = spmcr_opr_bits;
    if(isATMega)
        // ATMega support SPMIE bit
        spmcr_valid_bits |= 0x80;
    
    // reset processing engine
    Reset();
    
    // add to cycle list
    core->AddToCycleList(this);
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
    // process CPU lock
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

unsigned char FlashProgramming::LPM_action(unsigned int xaddr, unsigned int addr) {
    return 0;
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
        if(spm_opr == SPM_OPS_UNLOCKRWW) {
            ClearOperationBits();
            spmcr_val &= ~0x40;
            core->Flash->SetRWWLock(0);
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
            // calculate system time, where operation is finished
            timeout = SystemClock::Instance().GetCurrentTime() + FlashProgramming::SPM_TIMEOUT;
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
            for(unsigned int i = 0; i < (pageSize * 2); i++)
                tempBuffer[i] = 0xff;
            core->Flash->WriteMem(tempBuffer, addr, pageSize * 2);
            // calculate system time, where operation is finished
            timeout = SystemClock::Instance().GetCurrentTime() + FlashProgramming::SPM_TIMEOUT;
            // lock cpu while erasing flash
            action = SPM_ACTION_LOCKCPU;
            // lock RWW, if necessary
            SetRWWLock(addr);
            //cout << "erase page: [0x" << hex << addr << "]" << endl;
            return 0; // cpu clocks will be extended by CpuCycle calls
        }
        //cout << "unhandled spm-action(0x" << hex << data << ",0x" << hex << addr << ")" << endl;
        ClearOperationBits();
    }
    return 0;
}

void FlashProgramming::SetSpmcr(unsigned char v) {
    spmcr_val = (spmcr_val & ~spmcr_valid_bits) + (v & spmcr_valid_bits);
    
    // calculate operation
    if(action == SPM_ACTION_NOOP) {
        opr_enable_count = 4;
        action = SPM_ACTION_PREPARE;
        switch(spmcr_val & spmcr_opr_bits) {
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
                spm_opr = SPM_OPS_LOCKBITS;
                break;
                
            case 0x11:
                if(isATMega)
                    spm_opr = SPM_OPS_UNLOCKRWW;
                else
                    spm_opr = SPM_OPS_CLEARBUFFER;
                break;
                
            case 0x21:
                spm_opr = SPM_OPS_READSIG;
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

AvrFuses::AvrFuses(void):
    fuseBitsSize(2),
    fuseBits(0xfffffffd),
    nrwwAddr(0),
    nrwwSize(0),
    bitPosBOOTSZ(-1),
    bitPosBOOTRST(-1),
    flagBOOTRST(true),
    valueBOOTSZ(0)
{
    // do nothing!
}

void AvrFuses::SetFuseConfiguration(int size, unsigned long defvalue) {
    fuseBitsSize = size;
    fuseBits = defvalue;
}

bool AvrFuses::LoadFuses(const unsigned char *buffer, int size) {
    int fSize = ((fuseBitsSize - 1) / 8) + 1;

    // check buffer size
    if(fSize != size)
        return false;

    // store fuse values
    fuseBits = 0;
    for(int i = (fSize - 1); i >= 0; --i) {
        fuseBits <<= 8;
        fuseBits |= buffer[i];
    }

    // update fuse values for some fuse bits
    if(bitPosBOOTRST != -1 && bitPosBOOTRST < fuseBitsSize)
        flagBOOTRST = ((fuseBits >> bitPosBOOTRST) & 0x1) == 0x1;
    if(bitPosBOOTSZ != -1 && bitPosBOOTSZ < fuseBitsSize)
        valueBOOTSZ = (fuseBits >> bitPosBOOTSZ) & 0x3;

    return true;
}

void AvrFuses::SetBootloaderConfig(unsigned addr, int size, int bPosBOOTSZ, int bPosBOOTRST) {
    nrwwAddr = addr;
    nrwwSize = size;
    bitPosBOOTSZ = bPosBOOTSZ;
    bitPosBOOTRST = bPosBOOTRST;
}

unsigned int AvrFuses::GetBLSStart(void) {
    unsigned int addr = nrwwAddr;
    unsigned int size = nrwwSize;

    if(addr == 0)
        // if SPM functionality enabled and no rww functionality available, full flash is
        // used as nrww area, so "BLS" starts from flash start
        return 0;
    if(valueBOOTSZ == 0)
        return addr;
    size >>= 1;
    addr += size;
    if(valueBOOTSZ == 1)
        return addr;
    size >>= 1;
    addr += size;
    if(valueBOOTSZ == 2)
        return addr;
    size >>= 1;
    return addr + size;
}

unsigned int AvrFuses::GetResetAddr(void) {
    if(flagBOOTRST)
        return 0;
    else
        return GetBLSStart();
}

AvrLockBits::AvrLockBits(void):
    lockBitsSize(2),
    lockBits(0xff)
{
    // do nothing!
}

void AvrLockBits::SetLockBitsConfiguration(int size) {
    lockBitsSize = size;
}

bool AvrLockBits::LoadLockBits(const unsigned char *buffer, int size) {
    int lBSize = 1; // current not more than 8 bit!
    // check buffer size
    if(lBSize != size)
        return false;
    // load lock bits
    lockBits = buffer[0];
    return true;
}

void AvrLockBits::SetLockBits(unsigned char bits) {
    // bits can only set to 0, not back to 1 by this operation! Unused bits are set to 1.
    lockBits = (lockBits & bits) | ~((1 << lockBitsSize) - 1);
}

// EOF
