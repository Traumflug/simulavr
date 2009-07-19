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

#ifndef TIMERPRESCALER
#define TIMERPRESCALER

#include "hardware.h"
#include "avrdevice.h"
#include "rwmem.h"

//! Prescaler unit for support timers with clock
/*! This is a prescaler unit without external clock input, features reset and
  reset sync bit. Size of prescaler is 10 bit.*/
class HWPrescaler: public Hardware, public IOSpecialRegClient {
    
    private:
        int _resetBit;     //!< holds bit position for reset bit on IO register
        int _resetSyncBit; //!< holds sync bit position for prescaler reset synchronisation
        bool countEnable;  //!< enables counting of prescaler (for reset sync)
        
    protected:
        unsigned short preScaleValue; //!< prescaler counter value
        //! IO register interface set method, see IOSpecialRegClient
        unsigned char set_from_reg(unsigned char nv);
        //! IO register interface get method, see IOSpecialRegClient
        unsigned char get_from_client(unsigned char v) { return v; }
        
    public:
        //! Creates HWPrescaler instance without reset feature
        HWPrescaler(AvrDevice *core, const std::string &tracename);
        //! Creates HWPrescaler instance with reset but without sync reset feature
        HWPrescaler(AvrDevice *core,
                    const std::string &tracename,
                    IOSpecialReg *ioreg,
                    int resetBit);
        //! Creates HWPrescaler instance with reset and sync reset feature
        HWPrescaler(AvrDevice *core,
                    const std::string &tracename,
                    IOSpecialReg *ioreg,
                    int resetBit,
                    int resetSyncBit);
        //! Count functionality for prescaler
        virtual unsigned int CpuCycle() {
            if(countEnable) {
              preScaleValue++;
              if(preScaleValue > 1023) preScaleValue = 0;
            }
            return 0;
        }
        //! Get method for current prescaler counter value
        unsigned short GetValue() { return preScaleValue; }
        //! Reset method, sets prescaler counter to 0
        void Reset(){ preScaleValue = 0; }
};

#endif // TIMERPRESCALER
