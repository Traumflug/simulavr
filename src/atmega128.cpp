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
#include "hweeprom.h"
#include "hwwado.h"
#include "hwsreg.h"

#include "avrfactory.h"

AVR_REGISTER(atmega128, AvrDevice_atmega128);

AvrDevice_atmega128::~AvrDevice_atmega128() {
    delete timer3;
    delete inputCapture3;
    delete timer2;
    delete timer1;
    delete inputCapture1;
    delete timer0;
    delete timer3irq;
    delete timer012irq;
    delete usart1;
    delete usart0;
    delete wado;
    delete prescaler123;
    delete prescaler0;
    delete assr_reg;
    delete sfior_reg;
    delete extirq;
    delete eifr_reg;
    delete eimsk_reg;
    delete eicrb_reg;
    delete eicra_reg;
    delete spi;
    delete ad;
    delete admux;
    delete rampz;
    delete portg;
    delete portf;
    delete porte;
    delete portd;
    delete portc;
    delete portb;
    delete porta;
    delete stack;
    delete eeprom;
    delete irqSystem;
}

AvrDevice_atmega128::AvrDevice_atmega128():
    AvrDevice(224, 4096, 0xef00, 256*1024),
    aref()
{
    flagELPMInstructions = true;
    irqSystem = new HWIrqSystem(this, 4, 35); //4 bytes per vector, 35 vectors
    eeprom = new HWEeprom( this, irqSystem, 4096, 22); 
    stack = new HWStackSram(this, 16);
    porta = new HWPort(this, "A");
    portb = new HWPort(this, "B");
    portc = new HWPort(this, "C");
    portd = new HWPort(this, "D");
    porte = new HWPort(this, "E");
    portf = new HWPort(this, "F");
    portg = new HWPort(this, "G");

    RegisterPin("AREF", &aref);
    rampz = new AddressExtensionRegister(this, "RAMPZ", 1);

    admux = new HWAdmux(this,
          &portf->GetPin(0), &portf->GetPin(1), &portf->GetPin(2),
          &portf->GetPin(3), &portf->GetPin(4), &portf->GetPin(5),
          &portf->GetPin(6), &portf->GetPin(7));

    // vector 21 ADConversion Complete
    ad = new HWAd(this, admux, irqSystem, aref, 21);

    spi = new HWSpi(this, irqSystem,
            PinAtPort(portb, 2), PinAtPort(portb, 3), PinAtPort(portb, 1),
            PinAtPort(portb, 0),/*irqvec*/ 17, true);

    eicra_reg = new IOSpecialReg(&coreTraceGroup, "EICRA");
    eicrb_reg = new IOSpecialReg(&coreTraceGroup, "EICRB");
    eimsk_reg = new IOSpecialReg(&coreTraceGroup, "EIMSK");
    eifr_reg = new IOSpecialReg(&coreTraceGroup, "EIFR");
    extirq = new ExternalIRQHandler(this, irqSystem, eimsk_reg, eifr_reg);
    extirq->registerIrq(1, 0, new ExternalIRQSingle(eicra_reg, 0, 2, GetPin("D0")));
    extirq->registerIrq(2, 1, new ExternalIRQSingle(eicra_reg, 2, 2, GetPin("D1")));
    extirq->registerIrq(3, 2, new ExternalIRQSingle(eicra_reg, 4, 2, GetPin("D2")));
    extirq->registerIrq(4, 3, new ExternalIRQSingle(eicra_reg, 6, 2, GetPin("D3")));
    extirq->registerIrq(5, 4, new ExternalIRQSingle(eicrb_reg, 0, 2, GetPin("E4")));
    extirq->registerIrq(6, 5, new ExternalIRQSingle(eicrb_reg, 2, 2, GetPin("E5")));
    extirq->registerIrq(7, 6, new ExternalIRQSingle(eicrb_reg, 4, 2, GetPin("E6")));
    extirq->registerIrq(8, 7, new ExternalIRQSingle(eicrb_reg, 6, 2, GetPin("E7")));
  
    sfior_reg = new IOSpecialReg(&coreTraceGroup, "SFIOR");
    assr_reg = new IOSpecialReg(&coreTraceGroup, "ASSR");
    prescaler0 = new HWPrescalerAsync(this, "0", PinAtPort(portg, 4), assr_reg, 3, sfior_reg, 1, 7);
    prescaler123 = new HWPrescaler(this, "123", sfior_reg, 0, 7);
    
    wado = new HWWado(this);

    usart0 = new HWUsart(this, irqSystem,
               PinAtPort(porte,1), PinAtPort(porte,0), PinAtPort(porte, 2),
               18, 19, 20, 0);
    usart1 = new HWUsart(this, irqSystem,
               PinAtPort(portd,3), PinAtPort(portd,2), PinAtPort(portd, 5),
               30, 31, 32, 1);

    timer012irq = new TimerIRQRegister(this, irqSystem);
    timer012irq->registerLine(0, new IRQLine("TOV0",  16));
    timer012irq->registerLine(1, new IRQLine("OCF0",  15));
    timer012irq->registerLine(2, new IRQLine("TOV1",  14));
    timer012irq->registerLine(3, new IRQLine("OCF1B", 13));
    timer012irq->registerLine(4, new IRQLine("OCF1A", 12));
    timer012irq->registerLine(5, new IRQLine("ICF1",  11));
    timer012irq->registerLine(6, new IRQLine("TOV2",  10));
    timer012irq->registerLine(7, new IRQLine("OCF2",   9));
    
    timer3irq = new TimerIRQRegister(this, irqSystem, -2);
    timer3irq->registerLine(0, new IRQLine("OCF1C", 24));
    timer3irq->registerLine(1, new IRQLine("OCF3C", 28));
    timer3irq->registerLine(2, new IRQLine("TOV3",  29));
    timer3irq->registerLine(3, new IRQLine("OCF3B", 27));
    timer3irq->registerLine(4, new IRQLine("OCF3A", 26));
    timer3irq->registerLine(5, new IRQLine("ICF3",  25));
    
    timer0 = new HWTimer8_1C(this,
                           new PrescalerMultiplexer(prescaler0),
                           0,
                           timer012irq->getLine("TOV0"),
                           timer012irq->getLine("OCF0"),
                           new PinAtPort(portb, 4));
    inputCapture1 = new ICaptureSource(PinAtPort(portd, 4));
    timer1 = new HWTimer16_3C(this,
                            new PrescalerMultiplexerExt(prescaler123, PinAtPort(portd, 6)),
                            1,
                            timer012irq->getLine("TOV1"),
                            timer012irq->getLine("OCF1A"),
                            new PinAtPort(portb, 5),
                            timer012irq->getLine("OCF1B"),
                            new PinAtPort(portb, 6),
                            timer3irq->getLine("OCF1C"),
                            new PinAtPort(portb, 7),
                            timer012irq->getLine("ICF1"),
                            inputCapture1);
    timer2 = new HWTimer8_1C(this,
                           new PrescalerMultiplexerExt(prescaler123, PinAtPort(portd, 7)),
                           2,
                           timer012irq->getLine("TOV2"),
                           timer012irq->getLine("OCF2"),
                           new PinAtPort(portb, 7));
    inputCapture3 = new ICaptureSource(PinAtPort(porte, 7));
    timer3 = new HWTimer16_3C(this,
                            new PrescalerMultiplexerExt(prescaler123, PinAtPort(porte, 6)),
                            3,
                            timer3irq->getLine("TOV3"),
                            timer3irq->getLine("OCF3A"),
                            new PinAtPort(porte, 3),
                            timer3irq->getLine("OCF3B"),
                            new PinAtPort(porte, 4),
                            timer3irq->getLine("OCF3C"),
                            new PinAtPort(porte, 5),
                            timer3irq->getLine("ICF3"),
                            inputCapture3);
  
    rw[0x9d]= & usart1->ucsrc_reg;
    rw[0x9c]= & usart1->udr_reg;
    rw[0x9b]= & usart1->ucsra_reg;
    rw[0x9a]= & usart1->ucsrb_reg;
    rw[0x99]= & usart1->ubrr_reg;
    rw[0x98]= & usart1->ubrrhi_reg;
    // 0x97, 0x96 reserved
    rw[0x95]= & usart0->ucsrc_reg;
    // 0x94 - 0x91 reserved
    rw[0x90]= & usart0->ubrrhi_reg;
    // 0x8f - 0x8d reserved
    rw[0x8c]= & timer3->tccrc_reg;
    rw[0x8b]= & timer3->tccra_reg;
    rw[0x8a]= & timer3->tccrb_reg;
    rw[0x89]= & timer3->tcnt_h_reg;
    rw[0x88]= & timer3->tcnt_l_reg;
    rw[0x87]= & timer3->ocra_h_reg;
    rw[0x86]= & timer3->ocra_l_reg;
    rw[0x85]= & timer3->ocrb_h_reg;
    rw[0x84]= & timer3->ocrb_l_reg;
    rw[0x83]= & timer3->ocrc_h_reg;
    rw[0x82]= & timer3->ocrc_l_reg;
    rw[0x81]= & timer3->icr_h_reg;
    rw[0x80]= & timer3->icr_l_reg;
    // 0x7f, 0x7e reserved
    rw[0x7d]= & timer3irq->timsk_reg;
    rw[0x7c]= & timer3irq->tifr_reg;
    // 0x7b reserved
    rw[0x7a]= & timer1->tccrc_reg;
    rw[0x79]= & timer1->ocrc_h_reg;
    rw[0x78]= & timer1->ocrc_l_reg;
    
    rw[0x6a]= eicra_reg;
    
    rw[0x65]= & portg->port_reg;
    rw[0x64]= & portg->ddr_reg;
    rw[0x63]= & portg->pin_reg;
    
    rw[0x62]= & portf->port_reg;
    rw[0x61]= & portf->ddr_reg;
    
    rw[0x5f]= statusRegister;
    rw[0x5e]= & ((HWStackSram *)stack)->sph_reg;
    rw[0x5d]= & ((HWStackSram *)stack)->spl_reg;
    
    rw[0x5b]= & rampz->ext_reg;

    rw[0x5a]= eicrb_reg;
    rw[0x59]= eimsk_reg;
    rw[0x58]= eifr_reg;
    rw[0x57]= & timer012irq->timsk_reg;
    rw[0x56]= & timer012irq->tifr_reg;
    
    rw[0x53]= & timer0->tccr_reg;
    rw[0x52]= & timer0->tcnt_reg;
    rw[0x51]= & timer0->ocra_reg;
    rw[0x50]= assr_reg;
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

    //0x42: on chip debug

    rw[0x40]= sfior_reg;
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

