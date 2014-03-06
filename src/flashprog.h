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

#ifndef FLASHPROG
#define FLASHPROG

#include "rwmem.h"
#include "hardware.h"
#include "systemclocktypes.h"

class AvrDevice;

//! Provides the programming engine for flash self programming
/*! \todo not implemented yet: SPM interrupt. Support of LPM operation.
   Setting boot lock bits. Read-While-Write, if run code in NRWW section. */
class FlashProgramming: public Hardware {
  
    protected:
        //! states of processing engine
        enum SPM_ACTIONtype {
            SPM_ACTION_NOOP = 0,
            SPM_ACTION_PREPARE,
            SPM_ACTION_LOCKCPU,
            SPM_ACTION_WAIT
        };
        //! SPM operations
        enum SPM_OPStype {
            SPM_OPS_NOOP = 0,
            SPM_OPS_STOREBUFFER,
            SPM_OPS_WRITEBUFFER,
            SPM_OPS_ERASE,
            SPM_OPS_LOCKBITS,
            SPM_OPS_UNLOCKRWW,
            SPM_OPS_CLEARBUFFER,
            SPM_OPS_READSIG
        };
        unsigned int pageSize;  //!< page size in words
        unsigned int nrww_addr; //!< start address of non RWW area of flash (word address)
        unsigned char spmcr_val; //!< holds the register value
        unsigned char spmcr_opr_bits; //!< mask for operation bits, including SPMEN bit
        unsigned char spmcr_valid_bits; //!< mask for valid bits
        int opr_enable_count; //!< enable counter for SPM operation
        SPM_ACTIONtype action; //!< state of the processing engine
        SPM_OPStype spm_opr; //!< selected SPM operation
        AvrDevice *core; //!< link to AvrDevice
        SystemClockOffset timeout; //!< system time till operation run
        unsigned char *tempBuffer; //!< hidden buffer for flash page operations
        bool isATMega; //!< Flag: true, if in ATMega mode, if false, it's ATTiny mode
        
        void ClearOperationBits(void);
        void SetRWWLock(unsigned int addr);
        
    public:
        enum {
            SPM_TINY_MODE = 0,
            SPM_MEGA_MODE = 1,
            SPM_SIG_OPR = 2,
            SPM_TIMEOUT = 4000000
        };
        
        //! Create a instance of FlashProgramming class
        FlashProgramming(AvrDevice *c, unsigned int pgsz, unsigned int nrww, int mode);
        ~FlashProgramming();
        
        unsigned int CpuCycle();
        void Reset();
        
        unsigned char LPM_action(unsigned int xaddr, unsigned int addr);
        int SPM_action(unsigned int data, unsigned int xaddr, unsigned int addr);
        void SetSpmcr(unsigned char v);
        unsigned char GetSpmcr() { return spmcr_val; }

        IOReg<FlashProgramming> spmcr_reg;
        
};

//! Support for fuse bits
class AvrFuses {

    private:
        int fuseBitsSize;       //!< count of bits in fuses
        unsigned long fuseBits; //!< fuse data
        unsigned int nrwwAddr;  //!< start address NRWW section
        unsigned int nrwwSize;  //!< size of NRWW section in byte
        int bitPosBOOTSZ;       //!< bit position BOOTSZ fuses (2 Bit) in fuseBits
        int bitPosBOOTRST;      //!< bit position BOOTRST fuses (1 Bit) in fuseBits
        bool flagBOOTRST;       //!< value of BOOTRST fuse bit
        int valueBOOTSZ;        //!< value of BOOTSZ fuse bits

    public:
        enum {
            FB_CKDIV8 = 7   //!< lfuse: CKDIV8 bit
        };

        AvrFuses(void);
        //! Configure fuses
        void SetFuseConfiguration(int size, unsigned long defvalue);
        //! Initialize fuses from elf, checks proper size
        bool LoadFuses(const unsigned char *buffer, int size);
        //! Get fuse byte by index
        unsigned char GetFuseByte(int index) { return (fuseBits >> (index * 8)) & 0xff; }
        //! Get fuse bit by bit index, starts with 0 on lfuse bit 0, bit = 0 means true!
        bool GetFuseBit(int index) { return !(bool)((fuseBits >> index) & 0x1); }
        //! Get count of fuse bytes available
        int GetFuseByteSize(void) { return (fuseBitsSize / 8) + 1; }
        //! Set bootloader support configuration
        void SetBootloaderConfig(unsigned addr, int size, int bPosBOOTSZ, int bPosBOOTRST);
        //! Get start address of bootloader section
        unsigned int GetBLSStart(void);
        //! Get reset address
        unsigned int GetResetAddr(void);

};

//! Support for lock bits
class AvrLockBits {

    private:
        int lockBitsSize;       //!< count of lock bits
        unsigned char lockBits; //!< lock bits data

    public:
        AvrLockBits(void);
        //! Configure lock bits
        void SetLockBitsConfiguration(int size);
        //! Initialize lock bits from elf, checks proper size
        bool LoadLockBits(const unsigned char *buffer, int size);
        //! Get lock bits (for LPM instruction)
        unsigned char GetLockByte(void) { return lockBits; }
        //! Set lock bits (from a SPM instruction)
        void SetLockBits(unsigned char bits);

};

#endif
