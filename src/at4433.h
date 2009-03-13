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
class HWPort;
class HWSpi;
class HWUart;
class HWAcomp;
class HWPrescaler;
class HWTimer0;
class HWTimer1;
class HWMcucr;
class HWExtIrq;
class HWTimer01Irq;
class HWAdmux;
class HWAd;

class AvrDevice_at90s4433:public AvrDevice {
	public:
		~AvrDevice_at90s4433();
		HWPort *portb;
		HWPort *portc;
		HWPort *portd;
        HWPort *portx; //TODO XXX there is no need for portx in 4433, but we
                        //have not rewritten the timer1 now, so please do this later
        HWPort *porty; //we need an analog pin (aref) but we would remove
                        //portx later so another port (y) is used
        HWAdmux *admux;
        HWAd *ad;
		HWSpi *spi;
		HWUart *uart;
		HWAcomp *acomp;
		HWPrescaler *prescaler;
		HWTimer0 *timer0;
		HWTimer1 *timer1;
		HWMcucr *mcucr;
		HWExtIrq *extirq;
		HWTimer01Irq *timer01irq;
		AvrDevice_at90s4433();
		unsigned char GetRampz();
		void SetRampz(unsigned char);
};
#endif

