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
#include "avrfactory.h"
#include "hwsreg.h"

AVR_REGISTER(at90s4433, AvrDevice_at90s4433);


AvrDevice_at90s4433::AvrDevice_at90s4433():
AvrDevice(64, 128, 0, 4*1024) { 

	irqSystem = new HWIrqSystem(this, 2);
        eeprom= new HWMegaEeprom(this, irqSystem, 256, 12); //we use a eeprom with irq here
	stack = new HWStack(this, Sram, 0x0100);

	portb= new HWPort(this, "B");
	portc= new HWPort(this, "C");
	portd= new HWPort(this, "D");

	portx= new HWPort(this, "X");	//TODO we have no portx for oc1b from timer1 here but 
					//we have no time to rewrite the timer logic now :-) TODO XXX
	porty= new HWPort(this, "Y");	//AREF on pin 0 ("Y0") 

	// irqSystem = new HWIrqSystem;
	admux= new HWAdmux(this,
          &portc->GetPin(0), &portc->GetPin(1), &portc->GetPin(2),
          &portc->GetPin(3), &portc->GetPin(4), &portc->GetPin(5),0,0);
	ad= new HWAd( this, admux, irqSystem, porty->GetPin(0), 11); //vec 11 ADConversion Complete
	spi= new HWSpi(this, irqSystem, PinAtPort( portb, 3), PinAtPort( portb, 4), PinAtPort( portb, 5), PinAtPort(portb, 2),/*irqvec*/ 7, false);
	uart= new HWUart( this, irqSystem, PinAtPort(portd,1), PinAtPort(portd, 0),8,9,10) ;
	acomp= new HWAcomp(this, irqSystem, PinAtPort(portd, 6), PinAtPort(portd, 7), 13);
	timer01irq= new HWTimer01Irq( this, irqSystem, 3,4,0,5,6); //overflow B not available
	wado= new HWWado(this);
	prescaler = new HWPrescaler(this);
	timer0= new HWTimer0(this, prescaler, timer01irq, PinAtPort(portd, 4), 0);
	timer1= new HWTimer1(this, prescaler, timer01irq, PinAtPort(portd, 5),
                         PinAtPort(portb, 1), PinAtPort(portx, 0),
                         PinAtPort(portx, 0), 1);
	extirq= new HWExtIrq( this, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3), 1,2);
	mcucr= new HWMcucr(this); //, irqSystem, PinAtPort(portd, 2), PinAtPort(portd, 3));

	rw[0x5f]= new RWSreg(this, status);
	rw[0x5e]= & stack->sph_reg;
	rw[0x5d]= & stack->spl_reg;
    
    rw[0x5b]= & extirq->gimsk_reg;
	rw[0x5a]= & extirq->gifr_reg;
	rw[0x59]= & timer01irq->timsk_reg;
	rw[0x58]= & timer01irq->tifr_reg;

	rw[0x55]= & mcucr->mcucr_reg;

	//0x54: MCUSR reset status flag (reset, wado, brown out...) //TODO XXX

	rw[0x53]= & timer0->tccr_reg;
	rw[0x52]= & timer0->tcnt_reg;

	rw[0x4f]= & timer1->tccr1a_reg;
	rw[0x4e]= & timer1->tccr1b_reg;
	rw[0x4d]= & timer1->tcnt1h_reg;
	rw[0x4c]= & timer1->tcnt1l_reg;
	rw[0x4b]= & timer1->ocr1ah_reg;
	rw[0x4a]= & timer1->ocr1al_reg;
    
	//Attention, we copied the complete timer from 8515 device, but there are
	//some differces between them! TODO    
	//0x49: now comp B here RWOcrbh(this, timer1);
	//0x48: now comp B here Ocrbl(timer1);

    rw[0x47]= & timer1->icr1h_reg;
	rw[0x46]= & timer1->icr1l_reg;

	rw[0x41]= & wado->wdtcr_reg;

	// 0x3f: only 256 bytes EEProm here :-) [RWEearh(this, eeprom)]
	rw[0x3e] = & eeprom->eearl_reg;
	rw[0x3d] = & eeprom->eedr_reg;
	rw[0x3c] = & eeprom->eecr_reg;

    // 0x3b-0x39: no port a here

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

	rw[0x27]= & admux->admux_reg;
	rw[0x26]= & ad->adcsr_reg;
	rw[0x25]= & ad->adch_reg;
	rw[0x24]= & ad->adcl_reg;

	rw[0x23]= & uart->ubrrhi_reg;

    Reset();
}

AvrDevice_at90s4433::~AvrDevice_at90s4433() {}
unsigned char AvrDevice_at90s4433::GetRampz() {
    std::cerr << "RAMPZ is not a valid register in at90s4433!" ;
	return 0;
}

void AvrDevice_at90s4433::SetRampz(unsigned char val) {
    std::cerr << "Illegal RAMPZ operation in at90s4433 core";
}

