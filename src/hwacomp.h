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

#ifndef HWACOMP
#define HWACOMP

#include "hardware.h"
#include "rwmem.h"
#include "irqsystem.h"
#include "pinatport.h"
#include "pinnotify.h"
#include "traceval.h"

// support input configuration for analog comparator and input capture event for timer,
// if it's possible by hardware
class HWAd;
class BasicTimerUnit;

//! Analog comparator peripheral
class HWAcomp: public Hardware,
               public HasPinNotifyFunction,
               public TraceValueRegister,
               public IOSpecialRegClient,
               public AnalogSignalChange {

    protected:
        HWIrqSystem *irqSystem; //!< connection to IRQ controller
        PinAtPort pinAin0;      //!< port pin AIN0
        PinAtPort pinAin1;      //!< port pin AIN1
        Pin *v_bg;              //!< get access to bandgap reference
        Pin *v_cc;              //!< get access to voltage supply level
        bool useBG;             //!< has ADBG register bit and can switch In0 to bandgap ref
        bool acme_sfior;        //!< ACME flag in SFIOR register is set
        bool enabled;           //!< analog comparator is enabled
        unsigned char acsr;     //!< ACSR register value
        unsigned int irqVec;    //!< stores the IRQ vector number
        BasicTimerUnit *timerA; //!< connection to timerA for input capture event. NULL, if not available
        BasicTimerUnit *timerB; //!< connection to timerB for input capture event. NULL, if not available
        HWAd *ad;               //!< connection to ADC for analog input mux. NULL, if not available
        IOSpecialReg *sfior;    //!< connection to SFIOR register, if necessary. NULL, if not used

        unsigned char set_from_reg(const IOSpecialReg* reg, unsigned char nv);
        unsigned char get_from_client(const IOSpecialReg* reg, unsigned char v) { return v; }

        //! Check, if ACME flag is set (from ADC or SFIOR register)
        bool isSetACME(void);

    public:
        enum {
            ACD   = 0x80,
            ACBG  = 0x40,
            ACO   = 0x20,
            ACI   = 0x10,
            ACIE  = 0x08,
            ACIC  = 0x04,
            ACIS1 = 0x02,
            ACIS0 = 0x01
        };

        IOReg<HWAcomp> acsr_reg; //!< ACSR IO register

        //! constructor to instantiate a analog comparator peripheral
        HWAcomp(AvrDevice *core,
                HWIrqSystem *irqsys,
                PinAtPort ain0,
                PinAtPort ain1,
                unsigned int irqVec,
                HWAd *_ad,
                BasicTimerUnit *_timerA,
                IOSpecialReg *_sfior = NULL,
                BasicTimerUnit *_timerB = NULL,
                bool _useBG = true);
        ~HWAcomp();

        //! Get method for ACSR register
        unsigned char GetAcsr() { return acsr; }
        //! Set method for ACSR register
        void SetAcsr(unsigned char val);
        //! Reset the unit
        void Reset();
        //! Reflect irq processing, reset interrupt source
        void ClearIrqFlag(unsigned int vec);
        //! Get informed about input pin change
        void PinStateHasChanged(Pin *);
        //! Get analog value for comparator input 0
        float GetIn0(void);
        //! Get analog value for comparator input 1
        float GetIn1(void);
        //! Return last state of analog comparator (assume, that this is correct, if unit is disabled)
        bool GetACO(void) { return ((acsr & ACO) == ACO); }
        // Interface for notify signal change in ADC multiplexer
        void NotifySignalChanged(void);

};

#endif

