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

#include "atmega16_32.h"

#include "hardware.h"
#include "irqsystem.h"
#include "hwport.h"
#include "hwstack.h"
#include "hwspi.h"
#include "hweeprom.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "flashprog.h"

#include "avrfactory.h"

AVR_REGISTER(atmega16, AvrDevice_atmega16);
AVR_REGISTER(atmega32, AvrDevice_atmega32);

AvrDevice_atmega16_32::~AvrDevice_atmega16_32() {
    delete timer2;
    delete timer1;
    delete inputCapture1;
    delete timer0;
    delete usart;
    delete wado;
    delete prescaler2;
    delete prescaler01;
    delete assr_reg;
    delete sfior_reg;
    delete extirq;
    delete mcucsr_reg;
    delete mcucr_reg;
    delete gifr_reg;
    delete gicr_reg;
    delete spi;
    delete ad;
    delete admux;
    delete spmRegister;
    delete portd;
    delete portc;
    delete portb;
    delete porta;
    delete stack;
    delete eeprom;
    delete irqSystem;
}

AvrDevice_atmega16_32::AvrDevice_atmega16_32(unsigned ram_bytes,
                                             unsigned flash_bytes,
                                             unsigned ee_bytes,
                                             unsigned nrww_start,
											 bool atmega16):
    AvrDevice(64 ,          // I/O space above General Purpose Registers
              ram_bytes,    // RAM size
              0,            // External RAM size
              flash_bytes), // Flash Size
    aref()
{
    irqSystem = new HWIrqSystem(this, 4, 21); //4 bytes per vector, 21 vectors
    eeprom = new HWEeprom(this, irqSystem, ee_bytes, atmega16 ? 15 : 17);
    stack = new HWStackSram(this, atmega16 ? 11 : 12);
    porta = new HWPort(this, "A");
    portb = new HWPort(this, "B");
    portc = new HWPort(this, "C");
    portd = new HWPort(this, "D");

    spmRegister = new FlashProgramming(this, 64, nrww_start, FlashProgramming::SPM_MEGA_MODE);

    // TWI/I2C not implemented yet, vectors 17/19
    // Analog Comparator not implemented yet, vectors 16/18
    // Store Program Memory Ready not implemented yet, vectors 21/21

    RegisterPin("AREF", &aref);

    admux = new HWAdmux(this,
                        &porta->GetPin(0), &porta->GetPin(1), &porta->GetPin(2),
                        &porta->GetPin(3), &porta->GetPin(4), &porta->GetPin(5),
                        &porta->GetPin(6), &porta->GetPin(7));

    ad = new HWAd(this, admux, irqSystem, aref, atmega16 ? 14 : 16);

    spi = new HWSpi(this, irqSystem,
                    PinAtPort(portb, 5), PinAtPort(portb, 6), PinAtPort(portb, 7),
                    PinAtPort(portb, 4),/*irqvec*/ atmega16 ? 10 : 12, true);

    gicr_reg = new IOSpecialReg(&coreTraceGroup, "GICR");
    gifr_reg = new IOSpecialReg(&coreTraceGroup, "GIFR");
    mcucr_reg = new IOSpecialReg(&coreTraceGroup, "MCUCR");
    mcucsr_reg = new IOSpecialReg(&coreTraceGroup, "MCUCSR");
    extirq = new ExternalIRQHandler(this, irqSystem, gicr_reg, gifr_reg);
    extirq->registerIrq(1, 6, new ExternalIRQSingle(mcucr_reg, 0, 2, GetPin("D2")));  // INT0
    extirq->registerIrq(2, 7, new ExternalIRQSingle(mcucr_reg, 2, 2, GetPin("D3")));  // INT1
    extirq->registerIrq(atmega16 ? 18 : 3, 5, new ExternalIRQSingle(mcucsr_reg, 6, 1, GetPin("B2")));  // INT2
    
    sfior_reg = new IOSpecialReg(&coreTraceGroup, "SFIOR");
    assr_reg = new IOSpecialReg(&coreTraceGroup, "ASSR");
    prescaler01 = new HWPrescaler(this, "01", sfior_reg, 0);
    prescaler2 = new HWPrescalerAsync(this, "2", PinAtPort(portc, 6),
                                      assr_reg, 3, sfior_reg, 1);
    
    wado = new HWWado(this);

    usart = new HWUsart(this, irqSystem,
                        PinAtPort(portd,1), PinAtPort(portd,0), PinAtPort(portb, 0),
                        atmega16 ? 11 : 13, atmega16 ? 12 : 14, atmega16 ? 13 : 15);

    timer012irq = new TimerIRQRegister(this, irqSystem);
    timer012irq->registerLine(0, new IRQLine("TOV0",  atmega16 ? 9 : 11));  // Timer/Counter0 Overflow
    timer012irq->registerLine(1, new IRQLine("OCF0",  atmega16 ? 19 : 10));  // Timer/Counter0 Compare Match
    timer012irq->registerLine(2, new IRQLine("TOV1",  atmega16 ? 8 : 9));  // Timer/Counter1 Overflow
    timer012irq->registerLine(3, new IRQLine("OCF1B", atmega16 ? 7 : 8));  // Timer/Counter1 Compare Match B
    timer012irq->registerLine(4, new IRQLine("OCF1A", atmega16 ? 6 : 7));  // Timer/Counter1 Compare Match A
    timer012irq->registerLine(5, new IRQLine("ICF1",  atmega16 ? 5 : 6));  // Timer/Counter1 Capture Even
    timer012irq->registerLine(6, new IRQLine("TOV2",  atmega16 ? 4 : 5));  // Timer/Counter2 Overflow
    timer012irq->registerLine(7, new IRQLine("OCF2",  atmega16 ? 3 : 4));  // Timer/Counter2 Compare Match
    
    timer0 = new HWTimer8_1C(this,
                             new PrescalerMultiplexerExt(prescaler01, PinAtPort(portb, 0)),
                             0,
                             timer012irq->getLine("TOV0"),
                             timer012irq->getLine("OCF0"),
                             new PinAtPort(portb, 3));
    inputCapture1 = new ICaptureSource(PinAtPort(portd, 6));
    timer1 = new HWTimer16_2C2(this,
                               new PrescalerMultiplexerExt(prescaler01, PinAtPort(portb, 1)),
                               1,
                               timer012irq->getLine("TOV1"),
                               timer012irq->getLine("OCF1A"),
                               new PinAtPort(portd, 5),
                               timer012irq->getLine("OCF1B"),
                               new PinAtPort(portd, 4),
                               timer012irq->getLine("ICF1"),
                               inputCapture1,
                               false);
    timer2 = new HWTimer8_1C(this,
                             new PrescalerMultiplexer(prescaler2),
                             2,
                             timer012irq->getLine("TOV2"),
                             timer012irq->getLine("OCF2"),
                             new PinAtPort(portd, 7));
    
    rw[0x5f]= statusRegister;
    rw[0x5e]= & ((HWStackSram *)stack)->sph_reg;
    rw[0x5d]= & ((HWStackSram *)stack)->spl_reg;
    rw[0x5c]= & timer0->ocra_reg;
    rw[0x5b]= gicr_reg;
    rw[0x5a]= gifr_reg;
    rw[0x59]= & timer012irq->timsk_reg;
    rw[0x58]= & timer012irq->tifr_reg;
    rw[0x57]= & spmRegister->spmcr_reg;
    //rw[0x56] TWCR
    rw[0x55] = mcucr_reg;
    rw[0x54] = mcucsr_reg;
    rw[0x53]= & timer0->tccr_reg;
    rw[0x52]= & timer0->tcnt_reg;
    //rw[0x51] OSCCAL/OCDR
    rw[0x50]= sfior_reg;
    rw[0x4f]= & timer1->tccra_reg; 
    rw[0x4e]= & timer1->tccrb_reg;
    rw[0x4d]= & timer1->tcnt_h_reg;
    rw[0x4c]= & timer1->tcnt_l_reg;
    rw[0x4b]= & timer1->ocra_h_reg;
    rw[0x4a]= & timer1->ocra_l_reg;
    rw[0x49]= & timer1->ocrb_h_reg;
    rw[0x48]= & timer1->ocrb_l_reg;
    rw[0x47]= & timer1->icr_h_reg;
    rw[0x46]= & timer1->icr_l_reg;
    rw[0x45]= & timer2->tccr_reg;
    rw[0x44]= & timer2->tcnt_reg;
    rw[0x43]= & timer2->ocra_reg;
    rw[0x42]= assr_reg;
    rw[0x41]= & wado->wdtcr_reg;
    rw[0x40]= & usart->ucsrc_ubrrh_reg;
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
    rw[0x2c]= & usart->udr_reg;
    rw[0x2b]= & usart->ucsra_reg;
    rw[0x2a]= & usart->ucsrb_reg;
    rw[0x29]= & usart->ubrr_reg;
    //rw[0x28] ACSR
    rw[0x27]= & admux->admux_reg;
    rw[0x26]= & ad->adcsr_reg;
    rw[0x25]= & ad->adch_reg;
    rw[0x24]= & ad->adcl_reg;
    //rw[0x23] TWDR
    //rw[0x22] TWAR
    //rw[0x21] TWSR
    //rw[0x20] TWBR
    
    Reset();
}

/* EOF */
