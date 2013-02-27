/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 *
 * atmega8.h
 *
 *  Created on: 15.10.2010
 *      Author: ivica
 */

#ifndef ATMEGA8_H_
#define ATMEGA8_H_

#include "avrdevice.h"
#include "hardware.h"
#include "rwmem.h"
#include "hwtimer/timerprescaler.h"
#include "hwtimer/timerirq.h"
#include "hwtimer/hwtimer.h"
#include "externalirq.h"
#include "hwuart.h"
#include "hwspi.h"
#include "hwad.h"
#include "hwacomp.h"
#include "pin.h"

//! AVRDevice class for ATMega8
class AvrDevice_atmega8: public AvrDevice {

    public:
        Pin adc6;                       //!< adc channel 6 input pin
        Pin adc7;                       //!< adc channel 7 input pin
        HWPort *portb;                  //!< port B
        HWPort *portc;                  //!< port C
        HWPort *portd;                  //!< port D
        ExternalIRQHandler *extirq;     //!< external interrupt support
        IOSpecialReg *gicr_reg;         //!< GICR IO register
        IOSpecialReg *gifr_reg;         //!< GIFR IO register
        IOSpecialReg *mcucr_reg;        //!< MCUCR IO register
        IOSpecialReg *mcucsr_reg;       //!< MCUCSR IO register
        OSCCALRegister *osccal_reg;     //!< OSCCAL IO register

        HWAdmux *admux;                 //!< ADC multiplexer unit
        HWARef *aref;                   //!< ADC reference unit
        HWAd *ad;                       //!< ADC unit
        HWAcomp *acomp;                 //!< analog compare unit

        IOSpecialReg *assr_reg;         //!< ASSR IO register
        IOSpecialReg *sfior_reg;        //!< SFIOR IO register
        HWPrescaler *prescaler01;       //!< prescaler unit for timer 0 and 1
        HWPrescalerAsync *prescaler2;   //!< prescaler unit for timer 2
        ICaptureSource *inputCapture1;  //!< input capture source for timer1
        HWTimer8_0C*   timer0;          //!< timer 0 unit
        HWTimer16_2C2* timer1;          //!< timer 1 unit
        HWTimer8_1C*   timer2;          //!< timer 2 unit
        TimerIRQRegister* timer012irq;  //!< timer interrupt unit for timer 0 to 2
        HWSpi *spi;                     //!< SPI unit
        HWUsart *usart;                 //!< USART unit

        AvrDevice_atmega8();
        virtual ~AvrDevice_atmega8();
};

#endif /* ATMEGA8_H_ */
