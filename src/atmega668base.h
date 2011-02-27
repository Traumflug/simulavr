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
#ifndef ATMEGA668
#define ATMEGA668

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

/*! AVRDevice class for ATMega48/88/168/328
  \todo This device isn't completely implemented. There is no
  boot loader section support for >= ATMega88, only normal interrupt vector
  start address supported, incomplete usart registers (and maybe more ...) */
class AvrDevice_atmega668base: public AvrDevice {
    
    protected:
        Pin                 aref;        //!< analog reference pin
        Pin                 adc6;        //!< adc channel 6 input pin
        Pin                 adc7;        //!< adc channel 7 input pin
        HWPort              portb;       //!< port B
        HWPort              portc;       //!< port C
        HWPort              portd;       //!< port D
        IOSpecialReg        gtccr_reg;   //!< GTCCR IO register
        IOSpecialReg        assr_reg;    //!< ASSR IO register
        HWPrescaler         prescaler01; //!< prescaler unit for timer 0 and 1
        HWPrescalerAsync    prescaler2;  //!< prescaler unit for timer 2
        ExternalIRQHandler* extirq01;    //!< external interrupt support for INT0, INT1
        IOSpecialReg*       eicra_reg;   //!< EICRA IO register
        IOSpecialReg*       eimsk_reg;   //!< EIMSK IO register
        IOSpecialReg*       eifr_reg;    //!< EIFR IO register
        ExternalIRQHandler* extirqpc;    //!< external interrupt support for PCINT[0-2]
        IOSpecialReg*       pcicr_reg;   //!< PCICR IO register
        IOSpecialReg*       pcifr_reg;   //!< PCIFR IO register
        IOSpecialReg*       pcmsk0_reg;  //!< PCIMSK0 IO register
        IOSpecialReg*       pcmsk1_reg;  //!< PCIMSK1 IO register
        IOSpecialReg*       pcmsk2_reg;  //!< PCIMSK2 IO register
        HWAdmux             admux;       //!< adc multiplexer unit
        HWAd*               ad;          //!< adc unit
        HWSpi*              spi;         //!< spi unit
        HWUsart*            usart0;      //!< usart 0 unit
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
        
        /*! Creates the device for ATMega48/88/168/328
          @param ram_bytes how much SRAM does the device own
          @param flash_bytes how much flash memory does the device own
          @param ee_bytes how much EEPROM space does the device own */
        AvrDevice_atmega668base(unsigned ram_bytes, unsigned flash_bytes,
                                unsigned ee_bytes );
        
        ~AvrDevice_atmega668base();
        
};

//! AVR device class for ATMega328, see AvrDevice_atmega668base.
class AvrDevice_atmega328: public AvrDevice_atmega668base {
    public:
        //! Creates the device for ATMega328, see AvrDevice_atmega668base.
        AvrDevice_atmega328() : AvrDevice_atmega668base(2 * 1024, 32 * 1024, 1024) {}
};

//! AVR device class for ATMega168, see AvrDevice_atmega668base.
class AvrDevice_atmega168: public AvrDevice_atmega668base {
    public:
        //! Creates the device for ATMega168, see AvrDevice_atmega668base.
        AvrDevice_atmega168() : AvrDevice_atmega668base(1024, 16 * 1024, 512) {}
};

//! AVR device class for ATMega88, see AvrDevice_atmega668base.
class AvrDevice_atmega88:public AvrDevice_atmega668base {
    public:
        //! Creates the device for ATMega88, see AvrDevice_atmega668base.
        AvrDevice_atmega88() : AvrDevice_atmega668base(1024, 8 * 1024, 512) {}
};

//! AVR device class for ATMega48, see AvrDevice_atmega668base.
class AvrDevice_atmega48:public AvrDevice_atmega668base {
    public:
        //! Creates the device for ATMega48, see AvrDevice_atmega668base.
        AvrDevice_atmega48() : AvrDevice_atmega668base(512, 4 * 1024, 256) {}
};

#endif
