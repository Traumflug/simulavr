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
#include "at8515.h"

#include "hweeprom.h"
#include "irqsystem.h"
#include "hwstack.h"
#include "hwport.h"
#include "hwtimer01irq.h"
#include "hwwado.h"




AvrDevice_at90s8515::AvrDevice_at90s8515():
AvrDevice(64, 512, 0xfda0, 8192) { 
    eeprom= new HWEeprom(this, 512);

	irqSystem = new HWIrqSystem(this, 2);
	stack = new HWStack(this, Sram, 0xffff);

	porta= new HWPort(this, "A");
	portb= new HWPort(this, "B");
	portc= new HWPort(this, "C");
	portd= new HWPort(this, "D");
	portx= new HWPort(this, "X"); //only used for ocr1b, this pin is not a gpio!

	//	irqSystem = new HWIrqSystem;
	spi= new HWSpi(this, irqSystem, PinAtPort( portb, 5), PinAtPort( portb, 6), PinAtPort( portb, 7), PinAtPort(portb, 4),/*irqvec*/ 8) ;
	uart= new HWUart( this, irqSystem, PinAtPort(portd,1), PinAtPort(portd, 0),9,10,11) ;
	acomp= new HWAcomp(this, irqSystem, PinAtPort(portb,2), PinAtPort(portb, 3),12);
	timer01irq= new HWTimer01Irq( this, irqSystem, 3,4,5,6,7);
	wado= new HWWado(this);
	prescaler = new HWPrescaler(this);
	timer0= new HWTimer0(this, prescaler, timer01irq, PinAtPort(portb, 0));
	timer1= new HWTimer1(this, prescaler, timer01irq, PinAtPort(portb, 1), PinAtPort(portd, 5), PinAtPort(portx, 0));
	extirq= new HWExtIrq( this, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3), 1,2);
	mcucr= new HWMcucr(this); //, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3));

	rw[0x5f]= new RWSreg(this, status);
	rw[0x5e]= new RWSph(this, stack);
	rw[0x5d]= new RWSpl(this, stack);
	rw[0x5c]= new RWReserved(this);
	rw[0x5b]= new RWGimsk(this, extirq, portd);
	rw[0x5a]= new RWGifr(this, extirq, portd);
	rw[0x59]= new RWTimsk(this, timer01irq);
	rw[0x58]= new RWTifr(this, timer01irq);

	rw[0x57]= new RWReserved(this);
	rw[0x56]= new RWReserved(this);

	rw[0x55]= new RWMcucr(this, mcucr, extirq);

	rw[0x54]= new RWReserved(this);

	rw[0x53]= new RWTccr(this, timer0);	
	rw[0x52]= new RWTcnt(this, timer0);	
	rw[0x51]= new RWReserved(this);
	rw[0x50]= new RWReserved(this);

	rw[0x4f]= new RWTccra(this, timer1);
	rw[0x4e]= new RWTccrb(this, timer1);
	rw[0x4d]= new RWTcnth(this, timer1);
	rw[0x4c]= new RWTcntl(this, timer1);
	rw[0x4b]= new RWOcrah(this, timer1);
	rw[0x4a]= new RWOcral(this, timer1);
	rw[0x49]= new RWOcrbh(this, timer1);
	rw[0x48]= new RWOcrbl(this, timer1);

	rw[0x47]= new RWReserved(this);
	rw[0x46]= new RWReserved(this);

	rw[0x45]= new RWIcrh(this, timer1);
	rw[0x44]= new RWIcrl(this, timer1);

	rw[0x43]= new RWReserved(this);
	rw[0x42]= new RWReserved(this);

	rw[0x41]= new RWWdtcr(this, wado);

	rw[0x40]= new RWReserved(this);

	rw[0x3f] = new RWEearh(this, eeprom);
	rw[0x3e] = new RWEearl(this, eeprom);
	rw[0x3d] = new RWEedr(this, eeprom);
	rw[0x3c] = new RWEecr(this, eeprom);

	rw[0x3b]= new RWPort(this, porta);
	rw[0x3a]= new RWDdr(this, porta);
	rw[0x39]= new RWPin(this, porta);

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

	rw[0x27]= new RWReserved(this);
	rw[0x26]= new RWReserved(this);
	rw[0x25]= new RWReserved(this);
	rw[0x24]= new RWReserved(this);
	rw[0x23]= new RWReserved(this);
	rw[0x22]= new RWReserved(this);
	rw[0x21]= new RWReserved(this);
	rw[0x20]= new RWReserved(this);
	Reset();
}

AvrDevice_at90s8515::~AvrDevice_at90s8515() {}
unsigned char AvrDevice_at90s8515::GetRampz() {
	cerr << "Rampz is not a valid Register in at8515!" ;
	return 0;
}

void AvrDevice_at90s8515::SetRampz(unsigned char val) {
	cerr << "Illegal Rampz operation in at8515 core";
}
