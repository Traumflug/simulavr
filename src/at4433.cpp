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

    portx= new HWPort(this, "X"); //TODO we have no portx for oc1b from timer1 here but 
                                    //we have no time to rewrite the timer logic now :-) TODO XXX
    porty= new HWPort(this, "Y"); //AREF on pin 0 ("Y0") 

	//	irqSystem = new HWIrqSystem;
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

	rw[0x5f]= new RWSreg(status);
	rw[0x5e]= new RWReserved;
	rw[0x5d]= new RWSpl(stack);  //only 8 Bit Stack Pointer in 4433
	rw[0x5c]= new RWReserved;
	rw[0x5b]= new RWGimsk(extirq, portd);
	rw[0x5a]= new RWGifr(extirq, portd);
	rw[0x59]= new RWTimsk(timer01irq);
	rw[0x58]= new RWTifr(timer01irq);

	rw[0x57]= new RWReserved;
	rw[0x56]= new RWReserved;

	rw[0x55]= new RWMcucr(mcucr, extirq);

	rw[0x54]= new RWReserved; //MCUSR reset status flag (reset, wado, brown out...) //TODO XXX

	rw[0x53]= new RWTccr(timer0);	
	rw[0x52]= new RWTcnt(timer0);	
	rw[0x51]= new RWReserved;
	rw[0x50]= new RWReserved;

	rw[0x4f]= new RWTccra(timer1);
	rw[0x4e]= new RWTccrb(timer1);
	rw[0x4d]= new RWTcnth(timer1);
	rw[0x4c]= new RWTcntl(timer1);
	rw[0x4b]= new RWOcrah(timer1);
	rw[0x4a]= new RWOcral(timer1);
    //Attention, we copied the complete timer from 8515 device, but there are some differces between them! TODO
	rw[0x49]= new RWReserved; //now comp B here RWOcrbh(timer1);
	rw[0x48]= new RWReserved; //now comp B here Ocrbl(timer1);


	rw[0x47]= new RWIcrh(timer1);
	rw[0x46]= new RWIcrl(timer1);

	rw[0x45]= new RWReserved;
	rw[0x44]= new RWReserved;

	rw[0x43]= new RWReserved;
	rw[0x42]= new RWReserved;

	rw[0x41]= new RWWdtcr(wado);

	rw[0x40]= new RWReserved;

	rw[0x3f] = new RWReserved;   //only 256 bytes EEProm here :-) RWEearh(eeprom);
	rw[0x3e] = new RWEearl(eeprom);
	rw[0x3d] = new RWEedr(eeprom);
	rw[0x3c] = new RWEecr(eeprom);

	rw[0x3b]= new RWReserved; //no port a here
	rw[0x3a]= new RWReserved;
	rw[0x39]= new RWReserved;

	rw[0x38]= new RWPort(portb);
	rw[0x37]= new RWDdr(portb);
	rw[0x36]= new RWPin(portb);

	rw[0x35]= new RWPort(portc);
	rw[0x34]= new RWDdr(portc);
	rw[0x33]= new RWPin(portc);

	rw[0x32]= new RWPort(portd);
	rw[0x31]= new RWDdr(portd);
	rw[0x30]= new RWPin(portd);

	rw[0x2f]= new RWSpdr(spi);
	rw[0x2e]= new RWSpsr(spi);
	rw[0x2d]= new RWSpcr(spi);

	rw[0x2c]= new RWUdr(uart);
	rw[0x2b]= new RWUsr(uart);
	rw[0x2a]= new RWUcr(uart);
	rw[0x29]= new RWUbrr(uart);

	rw[0x28]= new RWAcsr(acomp);

	rw[0x27]= new RWAdmux(admux);
	rw[0x26]= new RWAdcsr(ad);
	rw[0x25]= new RWAdch(ad);
	rw[0x24]= new RWAdcl(ad);

	rw[0x23]= new RWUbrrhi(uart); //we have 12 bits brr 
	rw[0x22]= new RWReserved;
	rw[0x21]= new RWReserved;
	rw[0x20]= new RWReserved;
	Reset();
}

AvrDevice_at90s4433::~AvrDevice_at90s4433() {}
unsigned char AvrDevice_at90s4433::GetRampz() {
	cout << "Rampz is not a valid Register in at8515!" ;
	return 0;
}

void AvrDevice_at90s4433::SetRampz(unsigned char val) {
	cout << "Illegal Rampz operation in at8515 core";
}
