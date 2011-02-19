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
#ifndef AT90CAN
#define AT90CAN

#include "avrdevice.h"
#include "hardware.h"
#include "rwmem.h"
#include "externalirq.h"
#include "hwuart.h"
#include "hwad.h"
#include "hwacomp.h"
#include "hwport.h"
#include "hwspi.h"
#include "hwtimer/timerprescaler.h"
#include "hwtimer/hwtimer.h"
#include "ioregs.h" //only for rampz here

#include <memory>

using std::auto_ptr;

/*! AVRDevice class for AT90CAN/32/64/128
  \todo This device isn't completely implemented. There is no
  boot loader section support for >= ATMega88, only normal interrupt vector
  start address supported, incomplete usart registers (and maybe more ...) 
  \todo Timer2 needs the external clocking bits
  */
class AvrDevice_at90canbase: public AvrDevice {
    
    protected:
        Pin                 aref;        //!< analog reference pin
        HWPort              porta;       //!< port B
        HWPort              portb;       //!< port B
        HWPort              portc;       //!< port C
        HWPort              portd;       //!< port D
        HWPort              porte;       //!< port E
        HWPort              portf;       //!< port F
        HWPort              portg;       //!< port G
        IOSpecialReg        gtccr_reg;   //!< GTCCR IO register
        IOSpecialReg        assr_reg;    //!< ASSR IO register
        HWPrescaler         prescaler013; //!< prescaler unit for timer 0 and 1
        HWPrescalerAsync    prescaler2;  //!< prescaler unit for timer 2
        ExternalIRQHandler* extirq01;    //!< external interrupt support for INT0, INT1, INT2, INT3, INT4, INT5, INT6, INT7
        IOSpecialReg*       eicra_reg;   //!< EICRA IO register
        IOSpecialReg*       eicrb_reg;   //!< EICRA IO register
        IOSpecialReg*       eimsk_reg;   //!< EIMSK IO register
        IOSpecialReg*       eifr_reg;    //!< EIFR IO register
        HWAdmux             admux;       //!< adc multiplexer unit
        HWAd*               ad;          //!< adc unit
        HWSpi*              spi;         //!< spi unit
        HWAcomp*            acomp;       //!< analog compare unit
        HWUsart*            usart0;      //!< usart 0 unit
        HWUsart*            usart1;      //!< usart 1 unit
        TimerIRQRegister*   timerIrq0;   //!< timer interrupt unit for timer 0
        HWTimer8_1C*        timer0;      //!< timer 0 unit
        ICaptureSource*     inputCapture1; //!< input capture source for timer1
        TimerIRQRegister*   timerIrq1;   //!< timer interrupt unit for timer 1
        HWTimer16_3C*       timer1;      //!< timer 1 unit
        TimerIRQRegister*   timerIrq2;   //!< timer interrupt unit for timer 2
        HWTimer8_1C*        timer2;      //!< timer 2 unit

        ICaptureSource*     inputCapture3; //!< input capture source for timer3
        TimerIRQRegister*   timerIrq3;   //!< timer interrupt unit for timer 3
        HWTimer16_3C*       timer3;      //!< timer 3 unit

        GPIORegister* gpior0_reg;
        GPIORegister* gpior1_reg;
        GPIORegister* gpior2_reg;
        
    public:
        
        /*! Creates the device for ATMega48/88/168/328
          @param ram_bytes how much SRAM does the device own
          @param flash_bytes how much flash memory does the device own
          @param ee_bytes how much EEPROM space does the device own */
        AvrDevice_at90canbase(unsigned ram_bytes, unsigned flash_bytes,
                                unsigned ee_bytes );
        
        ~AvrDevice_at90canbase();
        
};

//! AVR device class for AT90CAN32, see AvrDevice_at90canbase
class AvrDevice_at90can32: public AvrDevice_at90canbase {
    public:
        //! Creates the device for AT90CAN32, see AvrDevice_atcan90base.
        AvrDevice_at90can32() : AvrDevice_at90canbase(2 * 1024, 32 * 1024, 1024) {}
};

//! AVR device class for AT90CAN64, see AvrDevice_at90canbase
class AvrDevice_at90can64: public AvrDevice_at90canbase {
    public:
        //! Creates the device for AT90CAN64, see AvrDevice_atcan90base.
        AvrDevice_at90can64() : AvrDevice_at90canbase(4 * 1024, 64 * 1024, 2 * 1024) {}
};

//! AVR device class for AT90CAN128, see AvrDevice_at90canbase
class AvrDevice_at90can128: public AvrDevice_at90canbase {
    public:
        //! Creates the device for AT90CAN128, see AvrDevice_atcan90base.
        AvrDevice_at90can128() : AvrDevice_at90canbase(4 * 1024, 128 * 1024, 4 * 1024) {}
};

#endif
