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

#include "at4433.h"

#include "irqsystem.h"
#include "hweeprom.h"
#include "hwstack.h"
#include "hwport.h"
#include "hwspi.h"
#include "pinatport.h"
#include "hwuart.h"
#include "hwtimer.h"
#include "hwacomp.h"
#include "hwwado.h"
#include "hwextirq.h"
#include "ioregs.h" //mcucr
#include "hwtimer01irq.h"
#include "hwad.h"





AvrDevice_at90s4433::AvrDevice_at90s4433():
AvrDevice(64, 128, 0, 4*1024) { 

	irqSystem = new HWIrqSystem(this, 2);
	eeprom= new HWMegaEeprom(this, irqSystem, 256, 12); //we use a eeprom with irq here
	stack = new HWStack(this, Sram, 0x00ff);

	portb= new HWPort(this, "B");
	portc= new HWPort(this, "C");
	portd= new HWPort(this, "D");

	portx= new HWPort(this, "X");	//TODO we have no portx for oc1b from timer1 here but 
					//we have no time to rewrite the timer logic now :-) TODO XXX
	porty= new HWPort(this, "Y");	//AREF on pin 0 ("Y0") 

	// irqSystem = new HWIrqSystem;
	admux= new HWAdmux(this, PinAtPort( portc, 0), PinAtPort( portc, 1), PinAtPort( portc, 2), PinAtPort( portc, 3), PinAtPort( portc, 4), PinAtPort (portc,5));
	ad= new HWAd( this, admux, irqSystem, PinAtPort( porty, 0), 11); //vec 11 ADConversion Complete
	spi= new HWSpi(this, irqSystem, PinAtPort( portb, 3), PinAtPort( portb, 4), PinAtPort( portb, 5), PinAtPort(portb, 2),/*irqvec*/ 7) ;
	uart= new HWUart( this, irqSystem, PinAtPort(portd,1), PinAtPort(portd, 0),8,9,10) ;
	acomp= new HWAcomp(this, irqSystem, PinAtPort(portd, 6), PinAtPort(portd, 7), 13);
	timer01irq= new HWTimer01Irq( this, irqSystem, 3,4,0,5,6); //overflow B not available
	wado= new HWWado(this);
	prescaler = new HWPrescaler(this);
	timer0= new HWTimer0(this, prescaler, timer01irq, PinAtPort(portd, 4));
	timer1= new HWTimer1(this, prescaler, timer01irq, PinAtPort(portd, 5), PinAtPort(portb, 1), PinAtPort(portx, 0));
	extirq= new HWExtIrq( this, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3), 1,2);
	mcucr= new HWMcucr(this); //, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3));

	rw[0x5f]= new RWSreg(this, status);
	rw[0x5e]= new RWReserved(this, 0x5e);
	rw[0x5d]= new RWSpl(this, stack);  //only 8 Bit Stack Pointer in 4433
	rw[0x5c]= new RWReserved(this, 0x5c);
	rw[0x5b]= new RWGimsk(this, extirq, portd);
	rw[0x5a]= new RWGifr(this, extirq, portd);
	rw[0x59]= new RWTimsk(this, timer01irq);
	rw[0x58]= new RWTifr(this, timer01irq);

	rw[0x57]= new RWReserved(this, 0x57);
	rw[0x56]= new RWReserved(this, 0x56);

	rw[0x55]= new RWMcucr(this, mcucr, extirq);

	rw[0x54]= new RWReserved(this, 0x54); //MCUSR reset status flag (reset, wado, brown out...) //TODO XXX

	rw[0x53]= new RWTccr(this, timer0);	
	rw[0x52]= new RWTcnt(this, timer0);	
	rw[0x51]= new RWReserved(this, 0x51);
	rw[0x50]= new RWReserved(this, 0x50);

	rw[0x4f]= new RWTccra(this, timer1);
	rw[0x4e]= new RWTccrb(this, timer1);
	rw[0x4d]= new RWTcnth(this, timer1);
	rw[0x4c]= new RWTcntl(this, timer1);
	rw[0x4b]= new RWOcrah(this, timer1);
	rw[0x4a]= new RWOcral(this, timer1);
	//Attention, we copied the complete timer from 8515 device, but there are some differces between them! TODO
	rw[0x49]= new RWReserved(this, 0x49); //now comp B here RWOcrbh(this, timer1);
	rw[0x48]= new RWReserved(this, 0x48); //now comp B here Ocrbl(timer1);


	rw[0x47]= new RWIcrh(this, timer1);
	rw[0x46]= new RWIcrl(this, timer1);

	rw[0x45]= new RWReserved(this, 0x45);
	rw[0x44]= new RWReserved(this, 0x44);

	rw[0x43]= new RWReserved(this, 0x43);
	rw[0x42]= new RWReserved(this, 0x42);

	rw[0x41]= new RWWdtcr(this, wado);

	rw[0x40]= new RWReserved(this, 0x40);

	rw[0x3f] = new RWReserved(this, 0x3f);//only 256 bytes EEProm here :-) RWEearh(this, eeprom);
	rw[0x3e] = new RWEearl(this, eeprom);
	rw[0x3d] = new RWEedr(this, eeprom);
	rw[0x3c] = new RWEecr(this, eeprom);

	rw[0x3b]= new RWReserved(this, 0x3b); //no port a here
	rw[0x3a]= new RWReserved(this, 0x3a);
	rw[0x39]= new RWReserved(this, 0x39);

	rw[0x38]= new RWPort(this, portb);
	rw[0x37]= new RWDdr(this, portb);
	rw[0x36]= new RWPin(this, portb);

	rw[0x35]= new RWPort(this, portc);
	rw[0x34]= new RWDdr(this, portc);
	rw[0x33]= new RWPin(this, portc);

	rw[0x32]= new RWPort(this, portd);
	rw[0x31]= new RWDdr(this, portd);
	rw[0x30]= new RWPin(this, portd);

	rw[0x2f]= new RWSpdr(this, spi);
	rw[0x2e]= new RWSpsr(this, spi);
	rw[0x2d]= new RWSpcr(this, spi);

	rw[0x2c]= new RWUdr(this, uart);
	rw[0x2b]= new RWUsr(this, uart);
	rw[0x2a]= new RWUcr(this, uart);
	rw[0x29]= new RWUbrr(this, uart);

	rw[0x28]= new RWAcsr(this, acomp);

	rw[0x27]= new RWAdmux(this, admux);
	rw[0x26]= new RWAdcsr(this, ad);
	rw[0x25]= new RWAdch(this, ad);
	rw[0x24]= new RWAdcl(this, ad);

	rw[0x23]= new RWUbrrhi(this, uart); //we have 12 bits brr 
	rw[0x22]= new RWReserved(this, 0x22);
	rw[0x21]= new RWReserved(this, 0x21);
	rw[0x20]= new RWReserved(this, 0x20);
	Reset();
}

AvrDevice_at90s4433::~AvrDevice_at90s4433() {}
unsigned char AvrDevice_at90s4433::GetRampz() {
	cerr << "RAMPZ is not a valid register in at90s4433!" ;
	return 0;
}

void AvrDevice_at90s4433::SetRampz(unsigned char val) {
	cerr << "Illegal RAMPZ operation in at90s4433 core";
}
