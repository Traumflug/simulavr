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

#include "atmega128.h"
#include "hardware.h"

#include "irqsystem.h"
#include "hwport.h"
#include "hwstack.h"
#include "hwspi.h"
#include "hwmegatimer.h"
#include "hwtimer.h"    //prescaler
#include "hweeprom.h"
#include "hwmegatimer0123irq.h"
#include "hwwado.h"
#include "hwsreg.h"

#include "avrfactory.h"

AVR_REGISTER(atmega128, AvrDevice_atmega128);


//#include "avrdevice_impl.h"
AvrDevice_atmega128::~AvrDevice_atmega128() {}
AvrDevice_atmega128::AvrDevice_atmega128():
AvrDevice(224, 4096, 0xef00, 256*1024),
aref()
{
	irqSystem = new HWIrqSystem(this, 4); //4 bytes per vector
        eeprom = new HWMegaEeprom( this, irqSystem, 4096, 22); 
	stack = new HWStack(this, Sram, 0x10000);
	porta= new HWPort(this, "A");
	portb= new HWPort(this, "B");
	portc= new HWPort(this, "C");
	portd= new HWPort(this, "D");
	porte= new HWPort(this, "E");
	portf= new HWPort(this, "F");
	portg= new HWPort(this, "G");

	RegisterPin("AREF", &aref);
	portx= new HWPort(this, "X"); //could be used if there are pins which are not GPIO
	rampz= new HWRampz(this);

	admux= new HWAdmux(this,
          &portf->GetPin(0), &portf->GetPin(1), &portf->GetPin(2),
          &portf->GetPin(3), &portf->GetPin(4), &portf->GetPin(5),
          &portf->GetPin(6), &portf->GetPin(7));

    // vector 21 ADConversion Complete
    ad= new HWAd(this, admux, irqSystem, aref, 21);

    spi= new HWSpi(this, irqSystem,
            PinAtPort(portb, 2), PinAtPort(portb, 3), PinAtPort(portb, 1),
		   PinAtPort(portb, 0),/*irqvec*/ 17, true);

	extirq= new HWMegaExtIrq(this, irqSystem, 
            PinAtPort(portd, 0), PinAtPort(portd, 1), PinAtPort(portd, 2),
            PinAtPort(portd, 3), PinAtPort(porte, 4), PinAtPort(porte, 5),
            PinAtPort(porte, 6),PinAtPort(porte, 7),
            1,2,3,4,5,6,7,8);

	prescaler123=new HWPrescaler(this);
	prescaler0=new HWPrescaler(this);

	wado = new HWWado(this);

    usart0=new HWUsart(this, irqSystem,
           PinAtPort(porte,1), PinAtPort(porte,0), PinAtPort(porte, 2),
		       18, 19, 20, 0);
    usart1=new HWUsart(this, irqSystem,
           PinAtPort(portd,3), PinAtPort(portd,2), PinAtPort(portd, 5),
		       30, 31, 32, 1);

	timer0123irq= new HWMegaTimer0123Irq(this, irqSystem,
			15 , /*tpComp*/
			16 , /*t0Ovf*/
			12 , /*t1compa*/
			13 , /*t1compb*/
			24 , /*t1compc*/
			11 , /*t1capt */
			14 , /*t1ovf*/
			9,   /*t2comp */
			10, /*t2ovf*/
			26, /*t3compa*/
			27, /*t3compb*/
			28, /*t3compc*/
			25, /*t3capt */
			29  /*t3ovf*/
			);

	timer0=new HWMegaTimer0(this, prescaler0, timer0123irq, PinAtPort(portb, 4));

	timer1= new HWMegaTimer1(this, prescaler123, timer0123irq, 1, //is timer1
            PinAtPort(portd, 6), PinAtPort(portb, 5), PinAtPort (portb, 6),
            PinAtPort (portb , 7));

    timer2=new HWMegaTimer2(this, prescaler123, timer0123irq,
            PinAtPort(portd, 7), PinAtPort(portb, 7));

    timer3=new HWMegaTimer1(this, prescaler123, timer0123irq,
           0, //is not timer1
           PinAtPort(porte, 6), PinAtPort(porte, 3), PinAtPort (porte, 4),
           PinAtPort (porte, 5));


	rw[0x9d]= & usart1->ucsrc_reg;
	rw[0x9c]= & usart1->udr_reg;
	rw[0x9b]= & usart1->ucsra_reg;
	rw[0x9a]= & usart1->ucsrb_reg;
	rw[0x99]= & usart1->ubrr_reg;
	rw[0x98]= & usart1->ubrrhi_reg;
	
	
    rw[0x95]= & usart0->ucsrc_reg;
	
	
	
	
	rw[0x90]= & usart0->ubrrhi_reg;
	
	
	
	// timer 3
	rw[0x8c]= & timer3->tccrc_reg;
	rw[0x8b]= & timer3->tccra_reg;
	rw[0x8a]= & timer3->tccrb_reg;
	rw[0x89]= & timer3->tcnth_reg;
	rw[0x88]= & timer3->tcntl_reg;
	rw[0x87]= & timer3->ocrah_reg;
	rw[0x86]= & timer3->ocral_reg;
	rw[0x85]= & timer3->ocrbh_reg;
	rw[0x84]= & timer3->ocrbl_reg;
	rw[0x83]= & timer3->ocrch_reg;
	rw[0x82]= & timer3->ocrcl_reg;
	rw[0x81]= & timer3->icrh_reg;
	rw[0x80]= & timer3->icrl_reg;

	
	
	rw[0x7d]= & timer0123irq->etimsk_reg;
	rw[0x7c]= & timer0123irq->etifr_reg;
	
	rw[0x7a]= & timer1->tccrc_reg;
	rw[0x79]= & timer1->ocrch_reg;
	rw[0x78]= & timer1->ocrcl_reg;
	
	
	
	
	
	
	
	
	
	
	
	
	
	rw[0x6a]= & extirq->eicra_reg;

	
	
	
	

	rw[0x65]= & portg->port_reg;
	rw[0x64]= & portg->ddr_reg;
	rw[0x63]= & portg->pin_reg;

	rw[0x62]= & portf->port_reg;
	rw[0x61]= & portf->ddr_reg;

	
	rw[0x5f]= new RWSreg(this, status);
	rw[0x5e]= & stack->sph_reg;
	rw[0x5d]= & stack->spl_reg;
	
	rw[0x5b]= & rampz->rampz_reg;

	rw[0x5a]= & extirq->eicrb_reg;
	rw[0x59]= & extirq->eimsk_reg;
	rw[0x58]= & extirq->eifr_reg;

	rw[0x57]= & timer0123irq->timsk_reg;
	rw[0x56]= & timer0123irq->tifr_reg;
	
	
	//Timer0
	rw[0x53]= & timer0->tccr_reg;
	rw[0x52]= & timer0->tcnt_reg;
	rw[0x51]= & timer0->ocr_reg;

	
	//Timer1
	rw[0x4f]= & timer1->tccra_reg; 
	rw[0x4e]= & timer1->tccrb_reg;
	rw[0x4d]= & timer1->tcnth_reg;
	rw[0x4c]= & timer1->tcntl_reg;
	rw[0x4b]= & timer1->ocrah_reg;
	rw[0x4a]= & timer1->ocral_reg;
	rw[0x49]= & timer1->ocrbh_reg;
	rw[0x48]= & timer1->ocrbl_reg;
	rw[0x47]= & timer1->icrh_reg;
	rw[0x46]= & timer1->icrl_reg;

	//Timer 2
	rw[0x45]= & timer2->tccr_reg;
	rw[0x44]= & timer2->tcnt_reg;
	rw[0x43]= & timer2->ocr_reg;

	//0x42: on chip debug


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
	rw[0x2e]= & spi->spcr_reg;
	rw[0x2d]= & spi->spsr_reg;

	rw[0x2c]= & usart0->udr_reg;
	rw[0x2b]= & usart0->ucsra_reg;
	rw[0x2a]= & usart0->ucsrb_reg;
	rw[0x29]= & usart0->ubrr_reg;

	rw[0x27]= & admux->admux_reg;
    
	rw[0x26]= & ad->adcsr_reg;
	rw[0x25]= & ad->adch_reg;
	rw[0x24]= & ad->adcl_reg;

	rw[0x23]= & porte->port_reg;
	rw[0x22]= & porte->ddr_reg;
	rw[0x21]= & porte->pin_reg;

	rw[0x20]= & portf->pin_reg;

	Reset();
}

unsigned char AvrDevice_atmega128::GetRampz() {
	return rampz->GetRampz();
}

void AvrDevice_atmega128::SetRampz(unsigned char val) {
	rampz->SetRampz(val);
}
