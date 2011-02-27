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
 */
#ifndef ATMEGA124ABASE_INCLUDED
#define ATMEGA124ABASE_INCLUDED

#include "avrdevice.h"
#include "hardware.h"
#include "rwmem.h"
#include "externalirq.h"
#include "hwuart.h"
#include "hwad.h"
#include "hwport.h"
#include "hwspi.h"
#include "hwtimer/timerprescaler.h"
#include "hwtimer/hwtimer.h"

/*! AvrDevice class for ATmega164A/164PA/324A/324PA/644A/644PA/1284/1284P.
The only difference of ATmega164PA/324PA/644PA/1284P flavor is it has
a BODS bit in MCUCR. (We do not simulate the register anyway.)
\todo This device isn't completely implemented. */
class AvrDevice_atmega1284Abase: public AvrDevice {

protected:
    Pin                 aref;        //!< analog reference pin
    HWPort              porta;       //!< port A
    HWPort              portb;       //!< port B
    HWPort              portc;       //!< port C
    HWPort              portd;       //!< port D
    IOSpecialReg        gtccr_reg;   //!< GTCCR IO register
    IOSpecialReg        assr_reg;    //!< ASSR IO register
    HWPrescaler         prescaler01; //!< prescaler unit for timer 0 and 1
    HWPrescalerAsync    prescaler2;  //!< prescaler unit for timer 2
    ExternalIRQHandler* extirq012;   //!< external interrupt support for INT0, INT1, INT2
    IOSpecialReg*       eicra_reg;   //!< EICRA IO register
    IOSpecialReg*       eimsk_reg;   //!< EIMSK IO register
    IOSpecialReg*       eifr_reg;    //!< EIFR IO register
    ExternalIRQHandler* extirqpc;    //!< external interrupt support for PCINT[0-2]
    IOSpecialReg*       pcicr_reg;   //!< PCICR IO register
    IOSpecialReg*       pcifr_reg;   //!< PCIFR IO register
    IOSpecialReg*       pcmsk0_reg;  //!< PCIMSK0 IO register
    IOSpecialReg*       pcmsk1_reg;  //!< PCIMSK1 IO register
    IOSpecialReg*       pcmsk2_reg;  //!< PCIMSK2 IO register
    IOSpecialReg*       pcmsk3_reg;  //!< PCIMSK3 IO register
    HWAdmux             admux;       //!< adc multiplexer unit
    HWAd*               ad;          //!< adc unit
    HWSpi*              spi;         //!< spi unit
    HWUsart*            usart0;      //!< usart 0 unit
    HWUsart*            usart1;      //!< usart 1 unit
    TimerIRQRegister*   timerIrq0;   //!< timer interrupt unit for timer 0
    HWTimer8_2C*        timer0;      //!< timer 0 unit
    ICaptureSource*     inputCapture1; //!< input capture source for timer1
    TimerIRQRegister*   timerIrq1;   //!< timer interrupt unit for timer 1
    HWTimer16_2C3*      timer1;      //!< timer 1 unit
    TimerIRQRegister*   timerIrq2;   //!< timer interrupt unit for timer 2
    HWTimer8_2C*        timer2;      //!< timer 2 unit
    GPIORegister*       gpior0_reg;  //!< general purpose IO register
    GPIORegister*       gpior1_reg;  //!< general purpose IO register
    GPIORegister*       gpior2_reg;  //!< general purpose IO register

public:
    AvrDevice_atmega1284Abase(unsigned ram_bytes, unsigned flash_bytes,
                              unsigned ee_bytes );
    ~AvrDevice_atmega1284Abase();
};

class AvrDevice_atmega1284A: public AvrDevice_atmega1284Abase {
public:
    AvrDevice_atmega1284A() : AvrDevice_atmega1284Abase(16 * 1024, 128 * 1024, 4 * 1024) {}
};

class AvrDevice_atmega644A: public AvrDevice_atmega1284Abase {
public:
    AvrDevice_atmega644A() : AvrDevice_atmega1284Abase(4 * 1024, 64 * 1024, 2 * 1024) {}
};

class AvrDevice_atmega324A: public AvrDevice_atmega1284Abase {
public:
    AvrDevice_atmega324A() : AvrDevice_atmega1284Abase(2 * 1024, 32 * 1024, 1 * 1024) {}
};

class AvrDevice_atmega164A: public AvrDevice_atmega1284Abase {
public:
    AvrDevice_atmega164A() : AvrDevice_atmega1284Abase(1 * 1024, 16 * 1024,    512  ) {}
};

#endif
