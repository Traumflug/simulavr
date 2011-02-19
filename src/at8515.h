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

#ifndef AT8515
#define AT8515
#include "avrdevice.h"

#include "hwspi.h"
#include "hwuart.h"
#include "hwacomp.h"
#include "hwtimer/timerprescaler.h"
#include "hwtimer/hwtimer.h"
#include "externalirq.h"
#include "hwport.h"

//! AVRDevice class for AT90S8515
class AvrDevice_at90s8515: public AvrDevice {
    
    private:
        HWPort portx; // only used for oc1b and icp (Timer1)
        
    public:
        HWPort *porta;                  //!< port A
        HWPort *portb;                  //!< port B
        HWPort *portc;                  //!< port C
        HWPort *portd;                  //!< port D
        Pin&   ocr1b;                   //!< output pin for output compare B on timer 1
        HWSpi *spi;                     //!< spi unit
        HWUart *uart;                   //!< uart unit
        HWAcomp *acomp;
        HWPrescaler *prescaler;         //!< prescaler unit for timer
        TimerIRQRegister *timer01irq;   //!< timer interrupt unit for timer
        HWTimer8_0C      *timer0;       //!< timer 0 unit
        ICaptureSource   *inputCapture1; //!< input capture source for timer1
        HWTimer16_2C2    *timer1;       //!< timer 1 unit
        ExternalIRQHandler *extirq;     //!< external interrupt support
        IOSpecialReg *gimsk_reg;        //!< GIMSK IO register
        IOSpecialReg *gifr_reg;         //!< GIFR IO register
        IOSpecialReg *mcucr_reg;        //!< MCUCR IO register
        
        AvrDevice_at90s8515();
        ~AvrDevice_at90s8515();
};
#endif

