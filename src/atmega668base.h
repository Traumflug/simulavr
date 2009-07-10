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
#include "hwmega48extirq.h"
#include "hwuart.h"
#include "hwad.h"
#include "hwport.h"
#include "hwspi.h"
#include "hwtimer.h"
#include "hwmegax8timer.h"
#include "hwmegax8timerirq.h"

class AvrDevice_atmega668base:public AvrDevice {
	protected:
		Pin					aref;
		Pin					adc6;
		Pin					adc7;
	   	HWPort				portb;
	   	HWPort				portc;
	   	HWPort				portd;
		HWPrescaler			prescaler;
        HWMega48ExtIrq*		extirq;
        HWAdmux				admux;
        HWAd*				ad;
        HWMegaSpi*			spi;
        HWUsart*			usart0;
		HWMegaX8TimerIrq*	timerIrq0;
		HWMegaX8Timer0*		timer0;
	public:
		AvrDevice_atmega668base(unsigned ram_bytes, unsigned flash_bytes,
                                unsigned ee_bytes );
		~AvrDevice_atmega668base(); 
		unsigned char GetRampz();
		void SetRampz(unsigned char);
};
// AvrDevice_atmega668base
#endif
