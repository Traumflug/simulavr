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

#include "at8515.h"

#include "hweeprom.h"
#include "irqsystem.h"
#include "hwstack.h"
#include "hwport.h"
#include "hwtimer01irq.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "avrfactory.h"

AVR_REGISTER(at90s8515, AvrDevice_at90s8515);




AvrDevice_at90s8515::AvrDevice_at90s8515():
AvrDevice(64, 512, 0xfda0, 8192) { 
    eeprom= new HWEeprom(this, 512);

	irqSystem = new HWIrqSystem(this, 2);
	stack = new HWStack(this, Sram, 0x10000);


    
    porta= new HWPort(this, "A");
	portb= new HWPort(this, "B");
	portc= new HWPort(this, "C");
	portd= new HWPort(this, "D");
	portx= new HWPort(this, "X"); //only used for ocr1b, this pin is not a gpio!

	//	irqSystem = new HWIrqSystem;
	spi= new HWSpi(this, irqSystem, PinAtPort( portb, 5), PinAtPort( portb, 6), PinAtPort( portb, 7), PinAtPort(portb, 4),/*irqvec*/ 8, false);
	uart= new HWUart( this, irqSystem, PinAtPort(portd,1), PinAtPort(portd, 0),9,10,11) ;
	acomp= new HWAcomp(this, irqSystem, PinAtPort(portb,2), PinAtPort(portb, 3),12);
	timer01irq= new HWTimer01Irq( this, irqSystem, 3,4,5,6,7);
	wado= new HWWado(this);
	prescaler = new HWPrescaler(this);
	timer0= new HWTimer0(this, prescaler, timer01irq, PinAtPort(portb, 0));
	timer1= new HWTimer1(this, prescaler, timer01irq, PinAtPort(portb, 1), PinAtPort(portd, 5), PinAtPort(portx, 0), PinAtPort(portx, 0));
	extirq= new HWExtIrq( this, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3), 1,2);
	mcucr= new HWMcucr(this); //, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3));

	rw[0x5f]= new RWSreg(this, status);
	rw[0x5e]= & stack->sph_reg;
	rw[0x5d]= & stack->spl_reg;
	// 0x5c: reserved
    
	rw[0x5b]= & extirq->gimsk_reg;
	rw[0x5a]= & extirq->gifr_reg;
	rw[0x59]= & timer01irq->timsk_reg;
	rw[0x58]= & timer01irq->tifr_reg;

	rw[0x55]= & mcucr->mcucr_reg;

	rw[0x53]= & timer0->tccr_reg;
	rw[0x52]= & timer0->tcnt_reg;

	rw[0x4f]= & timer1->tccr1a_reg;
	rw[0x4e]= & timer1->tccr1b_reg;
	rw[0x4d]= & timer1->tcnt1h_reg;
	rw[0x4c]= & timer1->tcnt1l_reg;
	rw[0x4b]= & timer1->ocr1ah_reg;
	rw[0x4a]= & timer1->ocr1al_reg;
	rw[0x49]= & timer1->ocr1bh_reg;
	rw[0x48]= & timer1->ocr1bl_reg;

	rw[0x45]= & timer1->icr1h_reg;
	rw[0x44]= & timer1->icr1l_reg;

	rw[0x41]= & wado->wdtcr_reg;

	rw[0x3f]= & eeprom->eearh_reg;
	rw[0x3e]= & eeprom->eearl_reg;
	rw[0x3d]= & eeprom->eedr_reg;
	rw[0x3c]= & eeprom->eecr_reg;

	rw[0x3b]= & porta->port_reg;
	rw[0x3a]= & porta->ddr_reg;
	rw[0x39]= & porta->pin_reg;

    rw[0x38]= & portb->port_reg;
	rw[0x37]= & portb->ddr_reg;
	rw[0x36]= & portb->pin_reg;

	rw[0x35]= & portc->port_reg;
	rw[0x34]= & portc->ddr_reg;
	rw[0x33]= & portc->pin_reg;

	rw[0x32]= & portd->port_reg;
	rw[0x31]= & portd->ddr_reg;
	rw[0x30]= & portd->pin_reg;

	rw[0x2f]= & spi->spdr_reg;
	rw[0x2e]= & spi->spsr_reg;
	rw[0x2d]= & spi->spcr_reg;

	rw[0x2c]= & uart->udr_reg;
	rw[0x2b]= & uart->usr_reg;
	rw[0x2a]= & uart->ucr_reg;
	rw[0x29]= & uart->ubrr_reg;

	rw[0x28]= & acomp->acsr_reg;

	Reset();
}

AvrDevice_at90s8515::~AvrDevice_at90s8515() {}
unsigned char AvrDevice_at90s8515::GetRampz() {
    std::cerr << "RAMPZ is not a valid register in at90s8515!" ;
    return 0;
}

void AvrDevice_at90s8515::SetRampz(unsigned char val) {
    std::cerr << "Illegal RAMPZ operation in at90s8515 core";
}
