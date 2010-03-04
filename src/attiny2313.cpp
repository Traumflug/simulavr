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

#include "attiny2313.h"

#include "hardware.h"
#include "irqsystem.h"
#include "hwport.h"
#include "hwstack.h"
#include "hweeprom.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "flashprog.h"

#include "avrfactory.h"

AVR_REGISTER(attiny2313, AvrDevice_attiny2313);

AvrDevice_attiny2313::~AvrDevice_attiny2313() {
    delete timer1;
    delete inputCapture1;
    delete timer0;
    delete timer01irq;
    delete usart;
    delete pcmsk_reg;
    delete mcucr_reg;
    delete eifr_reg;
    delete gimsk_reg;
    delete gpior2_reg;
    delete gpior1_reg;
    delete gpior0_reg;
    delete prescaler01;
    delete gtccr_reg;
    delete spmRegister;
    delete portd;
    delete portb;
    delete porta;
    delete stack;
    delete eeprom;
    delete irqSystem;
}

AvrDevice_attiny2313::AvrDevice_attiny2313():
    AvrDevice(64 ,          // I/O space above General Purpose Registers
              128,          // RAM size
              0,            // External RAM size
              2 * 1024)     // Flash Size
{
    flagJMPInstructions = false;
    flagMULInstructions = false;
    irqSystem = new HWIrqSystem(this, 2, 19); //2 bytes per vector, 19 vectors
    eeprom = new HWEeprom(this, irqSystem, 128, 17, HWEeprom::DEVMODE_EXTENDED); 
    stack = new HWStackSram(this, 8);
    porta = new HWPort(this, "A", true, 3);
    portb = new HWPort(this, "B", true);
    portd = new HWPort(this, "D", true, 7);

    spmRegister = new FlashProgramming(this, 16, 0, FlashProgramming::SPM_TINY_MODE);
    
    gtccr_reg = new IOSpecialReg(&coreTraceGroup, "GTCCR");
    prescaler01 = new HWPrescaler(this, "01", gtccr_reg, 0);
    
    gpior0_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR0");
    gpior1_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR1");
    gpior2_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR2");
    
    gimsk_reg = new IOSpecialReg(&coreTraceGroup, "GIMSK");
    eifr_reg = new IOSpecialReg(&coreTraceGroup, "EIFR");
    mcucr_reg = new IOSpecialReg(&coreTraceGroup, "MCUCR");
    pcmsk_reg = new IOSpecialReg(&coreTraceGroup, "PCMSK");
    extirq = new ExternalIRQHandler(this, irqSystem, gimsk_reg, eifr_reg);
    extirq->registerIrq(1, 6, new ExternalIRQSingle(mcucr_reg, 0, 2, GetPin("D2")));
    extirq->registerIrq(2, 7, new ExternalIRQSingle(mcucr_reg, 2, 2, GetPin("D3")));
    extirq->registerIrq(11, 5, new ExternalIRQPort(pcmsk_reg, portb));
    
    //wado = new HWWado(this);

    usart = new HWUsart(this, irqSystem,
                        PinAtPort(portd,1), PinAtPort(portd,0), PinAtPort(portd, 2),
                        7, 8, 9,
                        0, false);

    timer01irq = new TimerIRQRegister(this, irqSystem);
    timer01irq->registerLine(0, new IRQLine("OCF0A", 13));
    timer01irq->registerLine(1, new IRQLine("TOV0",   6));
    timer01irq->registerLine(2, new IRQLine("OCF0B", 14));
    timer01irq->registerLine(3, new IRQLine("ICF1",   3));
    timer01irq->registerLine(5, new IRQLine("OCF1B", 12));
    timer01irq->registerLine(6, new IRQLine("OCF1A",  4));
    timer01irq->registerLine(7, new IRQLine("TOV1",   5));
    
    timer0 = new HWTimer8_2C(this,
                             new PrescalerMultiplexerExt(prescaler01, PinAtPort(portd, 4)),
                             0,
                             timer01irq->getLine("TOV0"),
                             timer01irq->getLine("OCF0A"),
                             new PinAtPort(portb, 2),
                             timer01irq->getLine("OCF0B"),
                             new PinAtPort(portd, 5));
    inputCapture1 = new ICaptureSource(PinAtPort(portd, 6));
    timer1 = new HWTimer16_2C3(this,
                               new PrescalerMultiplexerExt(prescaler01, PinAtPort(portd, 5)),
                               1,
                               timer01irq->getLine("TOV1"),
                               timer01irq->getLine("OCF1A"),
                               new PinAtPort(portb, 3),
                               timer01irq->getLine("OCF1B"),
                               new PinAtPort(portb, 4),
                               timer01irq->getLine("ICF1"),
                               inputCapture1);
    
    rw[0x5f]= statusRegister;
    rw[0x5e]= & ((HWStackSram *)stack)->sph_reg;
    rw[0x5d]= & ((HWStackSram *)stack)->spl_reg;
    rw[0x5c]= & timer0->ocrb_reg;
    rw[0x5b]= gimsk_reg;
    rw[0x5a]= eifr_reg;
    rw[0x59]= & timer01irq->timsk_reg;
    rw[0x58]= & timer01irq->tifr_reg;
    rw[0x57]= & spmRegister->spmcr_reg;
    rw[0x56]= & timer0->ocra_reg;
    rw[0x55]= mcucr_reg;
    //rw[0x54] MCUSR
    rw[0x53]= & timer0->tccrb_reg;
    rw[0x52]= & timer0->tcnt_reg;
    //rw[0x51] OSCCAL
    rw[0x50]= & timer0->tccra_reg;
    rw[0x4f]= & timer1->tccra_reg; 
    rw[0x4e]= & timer1->tccrb_reg;
    rw[0x4d]= & timer1->tcnt_h_reg;
    rw[0x4c]= & timer1->tcnt_l_reg;
    rw[0x4b]= & timer1->ocra_h_reg;
    rw[0x4a]= & timer1->ocra_l_reg;
    rw[0x49]= & timer1->ocrb_h_reg;
    rw[0x48]= & timer1->ocrb_l_reg;
    //rw[0x47] reserved
    //rw[0x46] CLKPR
    rw[0x45]= & timer1->icr_h_reg;
    rw[0x44]= & timer1->icr_l_reg;
    rw[0x43]= gtccr_reg;
    rw[0x42]= & timer1->tccrc_reg;
    //rw[0x41]= & wado->wdtcr_reg;
    rw[0x40]= pcmsk_reg;
    //rw[0x3f] reserved
    rw[0x3e]= & eeprom->eearl_reg;
    rw[0x3d]= & eeprom->eedr_reg;
    rw[0x3c]= & eeprom->eecr_reg;
    rw[0x3b]= & porta->port_reg;
    rw[0x3a]= & porta->ddr_reg;
    rw[0x39]= & porta->pin_reg;
    rw[0x38]= & portb->port_reg;
    rw[0x37]= & portb->ddr_reg;
    rw[0x36]= & portb->pin_reg;
    rw[0x35]= gpior2_reg;
    rw[0x34]= gpior1_reg;
    rw[0x33]= gpior0_reg;
    rw[0x32]= & portd->port_reg;
    rw[0x31]= & portd->ddr_reg;
    rw[0x30]= & portd->pin_reg;
    //rw[0x2f] USIDR
    //rw[0x2e] USISR
    //rw[0x2d] USICR
    rw[0x2c]= & usart->udr_reg;
    rw[0x2b]= & usart->ucsra_reg;
    rw[0x2a]= & usart->ucsrb_reg;
    rw[0x29]= & usart->ubrr_reg;
    //rw[0x28] ACSR
    //rw[0x27] reserved
    //rw[0x26] reserved
    //rw[0x25] reserved
    //rw[0x24] reserved
    rw[0x23]= & usart->ucsrc_reg;
    rw[0x22]= & usart->ubrrh_reg;
    //rw[0x21] DIDR
    //rw[0x20] reserved
    
    Reset();
}

/* EOF */
