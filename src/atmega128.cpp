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


//#include "avrdevice_impl.h"
AvrDevice_atmega128::~AvrDevice_atmega128() {}
AvrDevice_atmega128::AvrDevice_atmega128():
AvrDevice(224, 4096, 0xef00, 128*1024) { 
	irqSystem = new HWIrqSystem(4); //4 bytes per vector
    eeprom = new HWMegaEeprom( this, irqSystem, 4096, 22); 
	stack = new HWStack(this, Sram, 0xffff);
	porta= new HWPort(this, "A");
	portb= new HWPort(this, "B");
	portc= new HWPort(this, "C");
	portd= new HWPort(this, "D");
	porte= new HWPort(this, "E");
	portf= new HWPort(this, "F");
	portg= new HWPort(this, "G");

	portx= new HWPort(this, "X"); //could be used if there are pins which are not GPIO
	rampz= new HWRampz(this);

    spi= new HWMegaSpi(this, irqSystem, PinAtPort( portb, 2), PinAtPort( portb, 3), PinAtPort( portb, 1), PinAtPort(portb, 0),/*irqvec*/ 17);
    
    extirq= new HWMegaExtIrq( this, irqSystem, 
            PinAtPort(portd, 0), PinAtPort(portd, 1), PinAtPort(portd, 2),PinAtPort(portd, 3),
            PinAtPort(porte, 4), PinAtPort(porte, 5), PinAtPort(porte, 6),PinAtPort(porte, 7),
            1,2,3,4,5,6,7,8);
    
	prescaler123=new HWPrescaler(this);
	prescaler0=new HWPrescaler(this);



	timer0123irq= new HWMegaTimer0123Irq( this, irqSystem,
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
				PinAtPort(portd, 6), PinAtPort(portb, 5), PinAtPort (portb, 6), PinAtPort (portb , 7));

			timer2=new HWMegaTimer2(this, prescaler123, timer0123irq, PinAtPort(portd, 7), PinAtPort(portb, 7));

			timer3=new HWMegaTimer1(this, prescaler123, timer0123irq,  0, //is not timer1
				PinAtPort(porte, 6), PinAtPort(porte, 3), PinAtPort (porte, 4), PinAtPort (porte, 5));



			for (int ii=0x9e; ii<=0xff; ii++) { rw[ii]=new RWReserved; }
			rw[0x9d]= new RWReserved; //RWucsrc;
			rw[0x9c]= new RWReserved; //RWUdr;
			rw[0x9b]= new RWReserved; //RWUcsra;
			rw[0x9a]= new RWReserved; //RWUcsrb;
			rw[0x99]= new RWReserved; //RWUbbrl;
			rw[0x98]= new RWReserved; //RWUbbrh;
			rw[0x97]= new RWReserved;
			rw[0x96]= new RWReserved;
			rw[0x95]= new RWReserved; //RWUcsrc
			rw[0x94]= new RWReserved;
			rw[0x93]= new RWReserved;
			rw[0x92]= new RWReserved;
			rw[0x91]= new RWReserved;
			rw[0x90]= new RWReserved; //RWUbrrh;
			rw[0x8f]= new RWReserved; //RWReserved;
			rw[0x8e]= new RWReserved; //RWReserved;
			rw[0x8d]= new RWReserved; //RWReserved;
			// timer 3
			rw[0x8c]= new RWTccrcM(timer3);
			rw[0x8b]= new RWTccraM(timer3);
			rw[0x8a]= new RWTccrbM(timer3);
			rw[0x89]= new RWTcnthM(timer3);
			rw[0x88]= new RWTcntlM(timer3);
			rw[0x87]= new RWOcrahM(timer3);
			rw[0x86]= new RWOcralM(timer3);
			rw[0x85]= new RWOcrbhM(timer3);
			rw[0x84]= new RWOcrblM(timer3);
			rw[0x83]= new RWOcrchM(timer3);
			rw[0x82]= new RWOcrclM(timer3);
			rw[0x81]= new RWIcrhM(timer3);
			rw[0x80]= new RWIcrlM(timer3);

			rw[0x7f]= new RWReserved; 
			rw[0x7e]= new RWReserved; 
			rw[0x7d]= new RWEtimskM(timer0123irq);
			rw[0x7c]= new RWEtifrM(timer0123irq);
			rw[0x7b]= new RWReserved; 
			rw[0x7a]= new RWTccrcM(timer1);
			rw[0x79]= new RWOcrchM(timer1);
			rw[0x78]= new RWOcrclM(timer1);
			rw[0x77]= new RWReserved; 
			rw[0x76]= new RWReserved;
			rw[0x75]= new RWReserved; 
			rw[0x74]= new RWReserved; //RWTwcr;
			rw[0x73]= new RWReserved; //RWWtdr;
			rw[0x72]= new RWReserved; //RWTwar;
			rw[0x71]= new RWReserved; //RWTwsr;
			rw[0x70]= new RWReserved; //RWTwbr;
			rw[0x6f]= new RWReserved; //RWOsccal;
			rw[0x6e]= new RWReserved;
			rw[0x6d]= new RWReserved; //RWXmcra;
			rw[0x6c]= new RWReserved; //RWXmcrb;
			rw[0x6b]= new RWReserved;
			rw[0x6a]= new RWEicra( extirq); //RWEicra;

			rw[0x69]= new RWReserved;
			rw[0x68]= new RWReserved; //RWSpmcsr;
			rw[0x67]= new RWReserved;
			rw[0x66]= new RWReserved;

			rw[0x65]= new RWPort(portg);
			rw[0x64]= new RWDdr(portg);
			rw[0x63]= new RWPin(portg);

			rw[0x62]= new RWPort(portf);
			rw[0x61]= new RWDdr(portf);

			rw[0x60]= new RWReserved;
			rw[0x5f]= new RWSreg(status);
			rw[0x5e]= new RWSph(stack);
			rw[0x5d]= new RWSpl(stack);
			rw[0x5c]= new RWReserved; //RWXdiv();
			rw[0x5b]= new RWRampz(rampz);

			rw[0x5a]= new RWEicrb( extirq); //RWEicrb();
			rw[0x59]= new RWEimsk( extirq); //RWEimsk();
			rw[0x58]= new RWEifr( extirq); //RWEifr();

			rw[0x57]= new RWTimskM(timer0123irq);
			rw[0x56]= new RWTifrM(timer0123irq);
			rw[0x55]= new RWReserved; //RWMcucr();
			rw[0x54]= new RWReserved; //RWMcucsr();
			//Timer0
			rw[0x53]= new RWTccr0(timer0);
			rw[0x52]= new RWTcnt0(timer0);
			rw[0x51]= new RWOcr0(timer0);

			rw[0x50]= new RWReserved; //RWAssr();
			//Timer1
			rw[0x4f]= new RWTccraM(timer1);
			rw[0x4e]= new RWTccrbM(timer1);
			rw[0x4d]= new RWTcnthM(timer1);
			rw[0x4c]= new RWTcntlM(timer1);
			rw[0x4b]= new RWOcrahM(timer1);
			rw[0x4a]= new RWOcralM(timer1);
			rw[0x49]= new RWOcrbhM(timer1);
			rw[0x48]= new RWOcrblM(timer1);
			rw[0x47]= new RWIcrhM(timer1);
			rw[0x46]= new RWIcrlM(timer1);

			//Timer 2
			rw[0x45]= new RWTccr2(timer2);
			rw[0x44]= new RWTcnt2(timer2);
			rw[0x43]= new RWOcr2(timer2);

			//on chip debug
			rw[0x42]= new RWReserved; //RWOcdr();


			rw[0x41]= new RWReserved; //RWWdtcr();
			rw[0x40]= new RWReserved; //RWSfior();

			rw[0x3f]= new RWEearh(eeprom);
			rw[0x3e]= new RWEearl(eeprom);
			rw[0x3d]= new RWEedr(eeprom);
			rw[0x3c]= new RWEecr(eeprom);

			rw[0x3b]= new RWPort(porta);
			rw[0x3a]= new RWDdr(porta);
			rw[0x39]= new RWPin(porta);

			rw[0x38]= new RWPort(portb);
			rw[0x37]= new RWDdr(portb);
			rw[0x36]= new RWPin(portb);

			rw[0x35]= new RWPort(portc);
			rw[0x34]= new RWDdr(portc);
			rw[0x33]= new RWPin(portc);

			rw[0x32]= new RWPort(portd);
			rw[0x31]= new RWDdr(portd);
			rw[0x30]= new RWPin(portd);

			rw[0x2f]= new RWSpdr(spi); //RWSpdr();
			rw[0x2e]= new RWSpcr(spi); //RWSpcr();
			rw[0x2d]= new RWSpsr(spi); //RWSpsr();

			rw[0x2c]= new RWReserved; //RWUdr0();
			rw[0x2b]= new RWReserved; //RWUcsra();
			rw[0x2a]= new RWReserved; //RWUcsrb();
			rw[0x29]= new RWReserved; //RWUbrrl();
			rw[0x28]= new RWReserved; //RWAcsr();
			rw[0x27]= new RWReserved; //RWAdmux();
			rw[0x26]= new RWReserved; //RWAdcsra();

			rw[0x25]= new RWReserved; //RWAdch();
			rw[0x24]= new RWReserved; //RWAdcl();

			rw[0x23]= new RWPort(porte);
			rw[0x22]= new RWDdr(porte);
			rw[0x21]= new RWPin(porte);

			rw[0x20]= new RWPin(portf);

			Reset();


}

unsigned char AvrDevice_atmega128::GetRampz() {
	return rampz->GetRampz();
}

void AvrDevice_atmega128::SetRampz(unsigned char val) {
	rampz->SetRampz(val);
}
