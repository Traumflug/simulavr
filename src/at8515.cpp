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
#include "hwwado.h"
#include "hwsreg.h"
#include "avrfactory.h"

AVR_REGISTER(at90s8515, AvrDevice_at90s8515);

AvrDevice_at90s8515::AvrDevice_at90s8515():
    AvrDevice(64, 512, 0xfda0, 8192),
    portx(this, "X"),
    ocr1b(portx.GetPin(0))
{ 
    flagJMPInstructions = false;
    flagMULInstructions = false;
    flagMOVWInstruction = false;
    irqSystem = new HWIrqSystem(this, 2, 13);
    eeprom= new HWEeprom(this, NULL, 512, 0);
    stack = new HWStackSram(this, 16);
    
    porta= new HWPort(this, "A");
    portb= new HWPort(this, "B");
    portc= new HWPort(this, "C");
    portd= new HWPort(this, "D");

    // portx is internaly used for ocr1b and icp, this 2 pins are not gpio!
    // set DDR Bit 0 to output: ocr1b
    // set DDT Bit 1 to input: icp
    portx.SetDdr(0x01);
    
    spi= new HWSpi(this, irqSystem, PinAtPort( portb, 5), PinAtPort( portb, 6), PinAtPort( portb, 7), PinAtPort(portb, 4),/*irqvec*/ 8, false);

    uart= new HWUart( this, irqSystem, PinAtPort(portd,1), PinAtPort(portd, 0),9,10,11) ;

    acomp= new HWAcomp(this, irqSystem, PinAtPort(portb,2), PinAtPort(portb, 3),12);

    wado= new HWWado(this);

    prescaler = new HWPrescaler(this, "01");
    timer01irq = new TimerIRQRegister(this, irqSystem);
    timer01irq->registerLine(1, new IRQLine("TOV0", 7));
    timer01irq->registerLine(3, new IRQLine("ICF1", 3));
    timer01irq->registerLine(5, new IRQLine("OCF1B", 5));
    timer01irq->registerLine(6, new IRQLine("OCF1A", 4));
    timer01irq->registerLine(7, new IRQLine("TOV1", 6));
    timer0 = new HWTimer8_0C(this,
                             new PrescalerMultiplexerExt(prescaler, PinAtPort(portb, 0)),
                             0,
                             timer01irq->getLine("TOV0"));
    inputCapture1 = new ICaptureSource(PinAtPort(&portx, 1));
    timer1 = new HWTimer16_2C2(this,
                               new PrescalerMultiplexerExt(prescaler, PinAtPort(portb, 1)),
                               1,
                               timer01irq->getLine("TOV1"),
                               timer01irq->getLine("OCF1A"),
                               new PinAtPort(portd, 5),
                               timer01irq->getLine("OCF1B"),
                               new PinAtPort(&portx, 0),
                               timer01irq->getLine("ICF1"),
                               inputCapture1,
                               true);

    gimsk_reg = new IOSpecialReg(&coreTraceGroup, "GIMSK");
    gifr_reg = new IOSpecialReg(&coreTraceGroup, "GIFR");
    mcucr_reg = new IOSpecialReg(&coreTraceGroup, "MCUCR");
    extirq = new ExternalIRQHandler(this, irqSystem, gimsk_reg, gifr_reg);
    extirq->registerIrq(1, 6, new ExternalIRQSingle(mcucr_reg, 0, 2, GetPin("D2"), true));
    extirq->registerIrq(2, 7, new ExternalIRQSingle(mcucr_reg, 2, 2, GetPin("D3"), true));

    rw[0x5f]= statusRegister;
    rw[0x5e]= & ((HWStackSram *)stack)->sph_reg;
    rw[0x5d]= & ((HWStackSram *)stack)->spl_reg;
    // 0x5c reserved
    rw[0x5b]= gimsk_reg;
    rw[0x5a]= gifr_reg;
    rw[0x59]= & timer01irq->timsk_reg;
    rw[0x58]= & timer01irq->tifr_reg;

    rw[0x55]= mcucr_reg;

    rw[0x53]= & timer0->tccr_reg;
    rw[0x52]= & timer0->tcnt_reg;

    rw[0x4f]= & timer1->tccra_reg;
    rw[0x4e]= & timer1->tccrb_reg;
    rw[0x4d]= & timer1->tcnt_h_reg;
    rw[0x4c]= & timer1->tcnt_l_reg;
    rw[0x4b]= & timer1->ocra_h_reg;
    rw[0x4a]= & timer1->ocra_l_reg;
    rw[0x49]= & timer1->ocrb_h_reg;
    rw[0x48]= & timer1->ocrb_l_reg;

    rw[0x45]= & timer1->icr_h_reg;
    rw[0x44]= & timer1->icr_l_reg;

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

AvrDevice_at90s8515::~AvrDevice_at90s8515() {
    delete extirq;
    delete mcucr_reg;
    delete gifr_reg;
    delete gimsk_reg;
    delete timer1;
    delete inputCapture1;
    delete timer0;
    delete timer01irq;
    delete prescaler;
    delete wado;
    delete acomp;
    delete uart;
    delete spi;
    delete portd;
    delete portc;
    delete portb;
    delete porta;
    delete stack;
    delete irqSystem;
    delete eeprom;
}

