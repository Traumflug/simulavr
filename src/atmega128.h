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

#ifndef ATMEGA128
#define ATMEGA128


#include "avrdevice.h"
#include "hardware.h"
#include "hwmegatimer.h"
#include "hwmegaextirq.h"
#include "hwuart.h"
#include "hwad.h"
#include "pin.h"

#include "ioregs.h" //only for rampz here

class HWSpi;


class AvrDevice_atmega128:public AvrDevice {
 private:
    HWPort *portx;
 public:
    Pin	aref;
    HWPort *porta;
    HWPort *portb;
    HWPort *portc;
    HWPort *portd;
    HWPort *porte;
    HWPort *portf;
    HWPort *portg;
    HWRampz *rampz;
    HWMegaExtIrq *extirq;

    HWAdmux *admux;
    HWAd *ad;

    HWPrescaler *prescaler0;
    HWPrescaler *prescaler123;
    HWMegaTimer0 *timer0;
    HWMegaTimer1 *timer1;
    HWMegaTimer1 *timer3;
    HWMegaTimer2 *timer2;
    HWMegaTimer0123Irq *timer0123irq;
    HWSpi *spi;
    HWUsart *usart0;
    HWUsart *usart1;

    AvrDevice_atmega128();
    ~AvrDevice_atmega128(); 
    unsigned char GetRampz();
    void SetRampz(unsigned char);
};
#endif
