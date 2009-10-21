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

#ifndef ATMEGA16
#define ATMEGA16

#include "avrdevice.h"
#include "hardware.h"
#include "rwmem.h"
#include "timerprescaler.h"
#include "timerirq.h"
#include "hwtimer.h"
#include "hwmegaextirq.h"
#include "hwuart.h"
#include "hwspi.h"
#include "hwad.h"
#include "pin.h"

//! AVRDevice class for ATMega16
class AvrDevice_atmega16: public AvrDevice {
    
    public:
        Pin aref;                       //!< analog reference pin
        HWPort *porta;                  //!< port A
        HWPort *portb;                  //!< port B
        HWPort *portc;                  //!< port C
        HWPort *portd;                  //!< port D
        HWMegaExtIrq *extirq;           //!< external interrupt unit

        HWAdmux *admux;                 //!< adc multiplexer unit
        HWAd *ad;                       //!< adc unit

        IOSpecialReg *assr_reg;         //!< ASSR IO register
        IOSpecialReg *sfior_reg;        //!< SFIOR IO register
        HWPrescaler *prescaler01;       //!< prescaler unit for timer 0 and 1
        HWPrescalerAsync *prescaler2;   //!< prescaler unit for timer 2
        ICaptureSource *inputCapture1;  //!< input capture source for timer1
        HWTimer8_1C*   timer0;          //!< timer 0 unit
        HWTimer16_2C2* timer1;          //!< timer 1 unit
        HWTimer8_1C*   timer2;          //!< timer 2 unit
        TimerIRQRegister* timer012irq;  //!< timer interrupt unit for timer 0 to 2
        HWSpi *spi;                     //!< spi unit
        HWUsart *usart;                 //!< usart unit

        AvrDevice_atmega16();
        ~AvrDevice_atmega16(); 
};

#endif
