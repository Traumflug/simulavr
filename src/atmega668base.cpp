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
#include "atmega668base.h"
#include "hardware.h"

#include "irqsystem.h"
#include "hwport.h"
#include "hwstack.h"
#include "hwspi.h"
#include "hweeprom.h"
#include "hwwado.h"
#include "hwmega48extirq.h"

AvrDevice_atmega668base::~AvrDevice_atmega668base() {}
AvrDevice_atmega668base::AvrDevice_atmega668base(
    unsigned ram_bytes, unsigned flash_bytes, unsigned ee_bytes ):
AvrDevice(	224,	// I/O space above General Purpose Registers
			ram_bytes,	// RAM size
			0,	// External RAM size
			flash_bytes //4*1024	// Flash Size
			),
aref(),
adc6(),
adc7(),
portb(this,"B"),
portc(this,"C"),
portd(this,"D"),
prescaler(this),
admux(	this,
		&portc.GetPin(0),
		&portc.GetPin(1),
		&portc.GetPin(2),
		&portc.GetPin(3),
		&portc.GetPin(4),
		&portc.GetPin(5),
		&adc6,
		&adc7
		)
{ 
	irqSystem	= new HWIrqSystem(this,(flash_bytes> 8U*1024U)?4:2);

    eeprom = new HWMegaEeprom( this, irqSystem, ee_bytes, 23); 
	stack = new HWStack(this, Sram, 0xffff);

	RegisterPin("AREF", &aref);
	RegisterPin("ADC6", &adc6);
	RegisterPin("ADC7", &adc7);

	extirq	= new HWMega48ExtIrq(	this,
								irqSystem, 
								PinAtPort(&portd, 0),
								PinAtPort(&portd, 1),
								1,
								2
								);

	
	timerIrq0	= new HWMegaX8TimerIrq( this, irqSystem, 16, 14, 15);
	timer0		= new HWMegaX8Timer0( this, &prescaler,timerIrq0,PinAtPort(&portd, 6),PinAtPort(&portd, 5));

	ad	= new HWAd( this, &admux, irqSystem, aref, 21);
	spi	= new HWMegaSpi(	this,
							irqSystem,
							PinAtPort(&portb, 3),	// MOSI
							PinAtPort(&portb, 4),	// MISO
							PinAtPort(&portb, 5),	// SCK
							PinAtPort(&portb, 2),	// /SS
							/*irqvec*/ 17
							);
    
	wado= new HWWado(this);

    usart0=new HWUsart(	this,
						irqSystem,
						PinAtPort(&portd,1),	// TXD
						PinAtPort(&portd,0),	// RXD
						PinAtPort(&portd, 4),	// XCK
						19,	// (18) RX complete vector
						20,	// (19) UDRE vector
						21	// (20) TX complete vector
						);

	for (int ii=0xE7; ii<=0xff; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0xE6]= new RWUdr(this, usart0);
	rw[0xE4]= new RWUbrr(this, usart0);

	for (int ii=0xE2; ii<=0xE3; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0xE1]= new RWUcsrb(this, usart0);
	rw[0xE0]= new RWUcsra(this, usart0);

	for (int ii=0xC6; ii<=0xDF; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0xC5]= new RWUbrrhi(this, usart0);

	for (int ii=0xC3; ii<=0xC4; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0xC2]= new RWUcsrc(this, usart0);

	for (int ii=0x7D; ii<=0xC1; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x7C]= new RWAdmux(this,&admux);
	rw[0x7B]= new RWReserved(this, 0x7B);
	rw[0x7A]= new RWAdcsr(this,ad);
	rw[0x79]= new RWAdch(this,ad);
	rw[0x78]= new RWAdcl(this,ad);

	for (int ii=0x6F; ii<=0x77; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x6E]= new RWTimskMx8(this, timerIrq0);

	for (int ii=0x6A; ii<=0x6D; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x69]= new RWEicra48(this,  extirq); //RWEicra;

	for (int ii=0x60; ii<=0x68; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x5f]= new RWSreg(this , status);
	rw[0x5e]= new RWSph(this , stack);
	rw[0x5d]= new RWSpl(this , stack);

	for (int ii=0x58; ii<=0x5C; ii++) { rw[ii]=new RWReserved(this, ii); }

	for (int ii=0x4F; ii<=0x55; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x4E]= new RWSpdr(this, spi);
	rw[0x4D]= new RWSpsr(this, spi);
	rw[0x4C]= new RWSpcr(this, spi);

	for (int ii=0x49; ii<=0x4B; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x48]= new RWOcrb0x8(this, timer0);
	rw[0x47]= new RWOcra0x8(this, timer0);
	rw[0x46]= new RWTcnt0x8(this, timer0);
	rw[0x45]= new RWTccrb0x8(this, timer0);
	rw[0x44]= new RWTccra0x8(this, timer0);

	for (int ii=0x43; ii<=0x43; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x42]= new RWEearh(this, eeprom);
	rw[0x41]= new RWEearl(this, eeprom);
	rw[0x40]= new RWEedr(this, eeprom);
	rw[0x3F]= new RWEecr(this, eeprom);
	rw[0x3E]= new RWReserved(this, 0x3E);
	rw[0x3D]= new RWEimsk48(this,  extirq);
	rw[0x3C]= new RWEifr48(this,  extirq);

	for (int ii=0x36; ii<=0x3B; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x35]= new RWTifrMx8(this, timerIrq0);

	for (int ii=0x2C; ii<=0x34; ii++) { rw[ii]=new RWReserved(this, ii); }

	rw[0x2B]= new RWPort(this, &portd);
	rw[0x2A]= new RWDdr(this, &portd);
	rw[0x29]= new RWPin(this, &portd);
	rw[0x28]= new RWPort(this, &portc);
	rw[0x27]= new RWDdr(this, &portc);
	rw[0x26]= new RWPin(this, &portc);
	rw[0x25]= new RWPort(this, &portb);
	rw[0x24]= new RWDdr(this, &portb);
	rw[0x23]= new RWPin(this, &portb);

	for (int ii=0x20; ii<=0x22; ii++) { rw[ii]=new RWReserved(this, ii); }

	Reset();
}

unsigned char AvrDevice_atmega668base::GetRampz() {
	cerr << "Rampz is not a valid Register in ATMega668!" ;
	return 0;
}

void AvrDevice_atmega668base::SetRampz(unsigned char val) {
    cerr << "Illegal Rampz operation in ATMega668 core";
}


