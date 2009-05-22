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


//#include "avrdevice_impl.h"
AvrDevice_atmega128::~AvrDevice_atmega128() {}
AvrDevice_atmega128::AvrDevice_atmega128():
AvrDevice(224, 4096, 0xef00, 256*1024),
aref()
{
	irqSystem = new HWIrqSystem(this, 4); //4 bytes per vector
	eeprom = new HWMegaEeprom(this, irqSystem, 4096, 22); 
	stack = new HWStack(this, Sram, 0xffff);
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

    spi= new HWMegaSpi(this, irqSystem,
            PinAtPort(portb, 2), PinAtPort(portb, 3), PinAtPort(portb, 1),
            PinAtPort(portb, 0),/*irqvec*/ 17);

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
           18, 19, 20);
    usart1=new HWUsart(this, irqSystem,
           PinAtPort(portd,3), PinAtPort(portd,2), PinAtPort(portd, 5),
           30, 31, 32);

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


	for (int ii=0x9e; ii<=0xff; ii++) { rw[ii]=new RWReserved(this, ii); }
	rw[0x9d]= new RWUcsrc(this, usart1); //RWucsrc;
	rw[0x9c]= new RWUdr(this, usart1); //RWUdr;
	rw[0x9b]= new RWUcsra(this, usart1); //RWUcsra;
	rw[0x9a]= new RWUcsrb(this, usart1); //RWUcsrb;
	rw[0x99]= new RWUbrr(this, usart1); //RWUbbrl;
	rw[0x98]= new RWUbrrhi(this, usart1); //RWUbbrh;
	rw[0x97]= new RWReserved(this, 0x97);
	rw[0x96]= new RWReserved(this, 0x96);
    rw[0x95]= new RWUcsrc(this, usart0); //RWucsrc;
	rw[0x94]= new RWReserved(this, 0x94);
	rw[0x93]= new RWReserved(this, 0x93);
	rw[0x92]= new RWReserved(this, 0x92);
	rw[0x91]= new RWReserved(this, 0x91);
	rw[0x90]= new RWUbrrhi(this, usart0); //RWUbrrh;
	rw[0x8f]= new RWReserved(this, 0x8f); //RWReserved;
	rw[0x8e]= new RWReserved(this, 0x8e); //RWReserved;
	rw[0x8d]= new RWReserved(this, 0x8d); //RWReserved;
	// timer 3
	rw[0x8c]= new RWTccrcM(this, timer3);
	rw[0x8b]= new RWTccraM(this, timer3);
	rw[0x8a]= new RWTccrbM(this, timer3);
	rw[0x89]= new RWTcnthM(this, timer3);
	rw[0x88]= new RWTcntlM(this, timer3);
	rw[0x87]= new RWOcrahM(this, timer3);
	rw[0x86]= new RWOcralM(this, timer3);
	rw[0x85]= new RWOcrbhM(this, timer3);
	rw[0x84]= new RWOcrblM(this, timer3);
	rw[0x83]= new RWOcrchM(this, timer3);
	rw[0x82]= new RWOcrclM(this, timer3);
	rw[0x81]= new RWIcrhM(this, timer3);
	rw[0x80]= new RWIcrlM(this, timer3);

	rw[0x7f]= new RWReserved(this, 0x7f); 
	rw[0x7e]= new RWReserved(this, 0x7e); 
	rw[0x7d]= new RWEtimskM(this, timer0123irq);
	rw[0x7c]= new RWEtifrM(this, timer0123irq);
	rw[0x7b]= new RWReserved(this, 0x7b); 
	rw[0x7a]= new RWTccrcM(this, timer1);
	rw[0x79]= new RWOcrchM(this, timer1);
	rw[0x78]= new RWOcrclM(this, timer1);
	rw[0x77]= new RWReserved(this, 0x77); 
	rw[0x76]= new RWReserved(this, 0x76);
	rw[0x75]= new RWReserved(this, 0x75); 
	rw[0x74]= new RWReserved(this, 0x74); //RWTwcr;
	rw[0x73]= new RWReserved(this, 0x73); //RWWtdr;
	rw[0x72]= new RWReserved(this, 0x72); //RWTwar;
	rw[0x71]= new RWReserved(this, 0x71); //RWTwsr;
	rw[0x70]= new RWReserved(this, 0x70); //RWTwbr;
	rw[0x6f]= new RWReserved(this, 0x6f); //RWOsccal;
	rw[0x6e]= new RWReserved(this, 0x6e);
	rw[0x6d]= new RWReserved(this, 0x6d); //RWXmcra;
	rw[0x6c]= new RWReserved(this, 0x6c); //RWXmcrb;
	rw[0x6b]= new RWReserved(this, 0x6b);
	rw[0x6a]= new RWEicra(this,  extirq); //RWEicra;

	rw[0x69]= new RWReserved(this, 0x69);
	rw[0x68]= new RWReserved(this, 0x68); //RWSpmcsr;
	rw[0x67]= new RWReserved(this, 0x67);
	rw[0x66]= new RWReserved(this, 0x66);

	rw[0x65]= new RWPort(this, portg);
	rw[0x64]= new RWDdr(this, portg);
	rw[0x63]= new RWPin(this, portg);

	rw[0x62]= new RWPort(this, portf);
	rw[0x61]= new RWDdr(this, portf);

	rw[0x60]= new RWReserved(this, 0x60);
	rw[0x5f]= new RWSreg(this , status);
	rw[0x5e]= new RWSph(this , stack);
	rw[0x5d]= new RWSpl(this , stack);
	rw[0x5c]= new RWReserved(this, 0x5c); //RWXdiv(this, );
	rw[0x5b]= new RWRampz(this, rampz);

	rw[0x5a]= new RWEicrb(this,  extirq); //RWEicrb();
	rw[0x59]= new RWEimsk(this,  extirq); //RWEimsk();
	rw[0x58]= new RWEifr(this,  extirq); //RWEifr();

	rw[0x57]= new RWTimskM(this, timer0123irq);
	rw[0x56]= new RWTifrM(this, timer0123irq);
	rw[0x55]= new RWReserved(this, 0x55); //RWMcucr(this, );
	rw[0x54]= new RWReserved(this, 0x54); //RWMcucsr(this, );
	//Timer0
	rw[0x53]= new RWTccr0(this, timer0);
	rw[0x52]= new RWTcnt0(this, timer0);
	rw[0x51]= new RWOcr0(this, timer0);

	rw[0x50]= new RWReserved(this, 0x50); //RWAssr(this, );
	//Timer1
	rw[0x4f]= new RWTccraM(this, timer1);
	rw[0x4e]= new RWTccrbM(this, timer1);
	rw[0x4d]= new RWTcnthM(this, timer1);
	rw[0x4c]= new RWTcntlM(this, timer1);
	rw[0x4b]= new RWOcrahM(this, timer1);
	rw[0x4a]= new RWOcralM(this, timer1);
	rw[0x49]= new RWOcrbhM(this, timer1);
	rw[0x48]= new RWOcrblM(this, timer1);
	rw[0x47]= new RWIcrhM(this, timer1);
	rw[0x46]= new RWIcrlM(this, timer1);

	//Timer 2
	rw[0x45]= new RWTccr2(this, timer2);
	rw[0x44]= new RWTcnt2(this, timer2);
	rw[0x43]= new RWOcr2(this, timer2);

	//on chip debug
	rw[0x42]= new RWReserved(this, 0x42); //RWOcdr(this, );


	rw[0x41]= new RWReserved(this, 0x41); //RWWdtcr(this, );
	rw[0x40]= new RWReserved(this, 0x40); //RWSfior(this, );

	rw[0x3f]= new RWEearh(this, eeprom);
	rw[0x3e]= new RWEearl(this, eeprom);
	rw[0x3d]= new RWEedr(this, eeprom);
	rw[0x3c]= new RWEecr(this, eeprom);

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

	rw[0x2f]= new RWSpdr(this, spi); //RWSpdr();
	rw[0x2e]= new RWSpcr(this, spi); //RWSpcr();
	rw[0x2d]= new RWSpsr(this, spi); //RWSpsr();

	rw[0x2c]= new RWUdr(this, usart0); //RWUdr0(this, );
	rw[0x2b]= new RWUcsra(this, usart0); //RWUcsra();
	rw[0x2a]= new RWUcsrb(this, usart0); //RWUcsrb();
	rw[0x29]= new RWUbrr(this, usart0); //RWUbrrl();
	rw[0x28]= new RWReserved(this, 0x28); //RWAcsr(this, );
	rw[0x27]= new RWAdmux(this, admux);
	rw[0x26]= new RWAdcsr(this, ad);

	rw[0x25]= new RWAdch(this, ad);
	rw[0x24]= new RWAdcl(this, ad);

	rw[0x23]= new RWPort(this, porte);
	rw[0x22]= new RWDdr(this, porte);
	rw[0x21]= new RWPin(this, porte);

	rw[0x20]= new RWPin(this, portf);

	Reset();
}

unsigned char AvrDevice_atmega128::GetRampz() {
	return rampz->GetRampz();
}

void AvrDevice_atmega128::SetRampz(unsigned char val) {
	rampz->SetRampz(val);
}
