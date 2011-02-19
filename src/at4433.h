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

#ifndef AT4433
#define AT4433
#include "avrdevice.h"
class HWAcomp;
class HWMcucr;

#include "hwuart.h"
#include "hwad.h"
#include "hwport.h"
#include "hwspi.h"
#include "hwtimer/timerprescaler.h"
#include "hwtimer/hwtimer.h"
#include "externalirq.h"

//! AVRDevice class for AT90S4433
class AvrDevice_at90s4433: public AvrDevice {
    
    private:
        HWPort *porty; //we need an analog pin (aref)
        
    public:
        HWPort *portb;                  //!< port B
        HWPort *portc;                  //!< port C
        HWPort *portd;                  //!< port D
        HWAdmux *admux;                 //!< adc multiplexer unit
        HWAd *ad;                       //!< adc unit
        HWSpi *spi;                     //!< spi unit
        HWUart *uart;                   //!< uart unit
        HWAcomp *acomp;
        HWPrescaler      *prescaler;    //!< prescaler unit for timer
        TimerIRQRegister *timer01irq;   //!< timer interrupt unit for timer
        HWTimer8_0C      *timer0;       //!< timer 0 unit
        ICaptureSource   *inputCapture1; //!< input capture source for timer1
        HWTimer16_1C     *timer1;       //!< timer 1 unit
        ExternalIRQHandler *extirq;     //!< external interrupt support
        IOSpecialReg *gimsk_reg;        //!< GIMSK IO register
        IOSpecialReg *gifr_reg;         //!< GIFR IO register
        IOSpecialReg *mcucr_reg;        //!< MCUCR IO register
        
        AvrDevice_at90s4433();
        ~AvrDevice_at90s4433();
};

#endif

