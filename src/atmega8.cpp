/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 *
 * atmega8.cpp
 *
 *  Created on: 15.10.2010
 *      Author: ivica
 */

#include "atmega8.h"

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

AVR_REGISTER(atmega8, AvrDevice_atmega8)

AvrDevice_atmega8::AvrDevice_atmega8() :
    AvrDevice(64, // I/O space above General Purpose Registers
            1024, // RAM size
            0, // External RAM size
            8 * 1024), // Flash Size
    adc6(),
    adc7()
{
    fuses->SetFuseConfiguration(16, 0xd9e1);
    irqSystem = new HWIrqSystem(this, 2, 19); //2 bytes per vector, 19 vectors
    eeprom = new HWEeprom(this, irqSystem, 512, 15);
    HWStackSram * stack_ram = new HWStackSram(this, 11); // Stack Pointer data space used 11 Bit wide.
    stack = stack_ram;
    osccal_reg = new OSCCALRegister(this, &coreTraceGroup, OSCCALRegister::OSCCAL_V3);
    portb = new HWPort(this, "B");
    portc = new HWPort(this, "C", false, 7);
    portd = new HWPort(this, "D");

    spmRegister = new FlashProgramming(this, // defined device
            32, // 32 words per page * 2 bytes per page * 128 pages = 8192 Bytes
            0xC000, // No Read-While-Write section starts at 0xC00
            FlashProgramming::SPM_MEGA_MODE); //

    sfior_reg = new IOSpecialReg(&coreTraceGroup, "SFIOR");

    admux = new HWAdmuxM8(this, &portc->GetPin(0), // ADC0
                                &portc->GetPin(1), // ADC1
                                &portc->GetPin(2), // ADC2
                                &portc->GetPin(3), // ADC3
                                &portc->GetPin(4), // ADC4
                                &portc->GetPin(5), // ADC5
                                &adc6,             // ADC6 only TQFP version
                                &adc7);            // ADC7 only TQFP version

    aref = new HWARef4(this, HWARef4::REFTYPE_NOBG);

    ad = new HWAd(this, HWAd::AD_M8, irqSystem, 14, admux, aref); // Interrupt Vector ADC Conversion Complete

    spi = new HWSpi(this,
            irqSystem,
            PinAtPort(portb, 3), // MOSI
            PinAtPort(portb, 4), // MISO
            PinAtPort(portb, 5), // SCK
            PinAtPort(portb, 2), // SS
            10, // Interrupt Vector Serial Transfer Complete
            true);

    gicr_reg = new IOSpecialReg(&coreTraceGroup,
            "GICR");

    gifr_reg = new IOSpecialReg(&coreTraceGroup,
            "GIFR");

    mcucr_reg = new IOSpecialReg(&coreTraceGroup,
            "MCUCR");

    mcucsr_reg = new IOSpecialReg(&coreTraceGroup,
            "MCUCSR");

    extirq = new ExternalIRQHandler(this,
            irqSystem,
            gicr_reg,
            gifr_reg);

    extirq->registerIrq(1, // INT0 External Interrupt Request 0
            6, // GICR Bit 6 - INT0: External Interrupt Request 0 Enable
            new ExternalIRQSingle(mcucr_reg,0, 2, GetPin("D2"))); // INT0

    extirq->registerIrq(2, // INT1 External Interrupt Request 1
            7, // GICR Bit 7 - INT1: External Interrupt Request 1 Enable
            new ExternalIRQSingle(mcucr_reg, 2, 2, GetPin("D3"))); // INT1

    assr_reg = new IOSpecialReg(&coreTraceGroup,
            "ASSR");

    prescaler01 = new HWPrescaler(this,
            "01",
            sfior_reg,
            0);

    prescaler2 = new HWPrescalerAsync(this,
            "2",
            PinAtPort(portb, 6),
            assr_reg,
            3,
            sfior_reg,
            1);

    wado = new HWWado(this);

    usart = new HWUsart(this,
            irqSystem,
            PinAtPort(portd, 1), // TX
            PinAtPort(portd, 0), // RX
            PinAtPort(portd, 4), // XCK
            11, // USART, RX Complete
            12, // USART Data Register Empty
            13); // USART, TX Complete

    timer012irq = new TimerIRQRegister(this, irqSystem);

    timer012irq->registerLine(0, new IRQLine("TOV0", 9));

    timer012irq->registerLine(2, new IRQLine("TOV1", 8));

    timer012irq->registerLine(3, new IRQLine("OCF1B", 7));

    timer012irq->registerLine(4, new IRQLine("OCF1A", 6));

    timer012irq->registerLine(5, new IRQLine("ICF1", 5));

    timer012irq->registerLine(6, new IRQLine("TOV2", 4));

    timer012irq->registerLine(7, new IRQLine("OCF2", 3));

    timer0 = new HWTimer8_0C(this,
            new PrescalerMultiplexerExt(prescaler01,
            PinAtPort(portd, 4)), // T0
            0,
            timer012irq->getLine("TOV0"));

    inputCapture1 = new ICaptureSource(PinAtPort(portb, 0)); // ICP1

    timer1 = new HWTimer16_2C2(this,
            new PrescalerMultiplexerExt(prescaler01,
            PinAtPort(portd, 5)), // T1
            1,
            timer012irq->getLine("TOV1"),
            timer012irq->getLine("OCF1A"),
            new PinAtPort(portb, 1), // OC1A
            timer012irq->getLine("OCF1B"),
            new PinAtPort(portb, 2), // OC1B
            timer012irq->getLine("ICF1"),
            inputCapture1, false);

    timer2 = new HWTimer8_1C(this,
            new PrescalerMultiplexer(prescaler2),
            2,
            timer012irq->getLine("TOV2"),
            timer012irq->getLine("OCF2"),
            new PinAtPort(portb, 3)); // OC2

    acomp = new HWAcomp(this, irqSystem, PinAtPort(portd, 6), PinAtPort(portd, 7), 16, ad, timer1, sfior_reg);

    rw[0x5f] = statusRegister;
    rw[0x5e] = &stack_ram->sph_reg;
    rw[0x5d] = &stack_ram->spl_reg;
//  rw[0x5c] Reserved
    rw[0x5b] = gicr_reg;
    rw[0x5a] = gifr_reg;
    rw[0x59] = &timer012irq->timsk_reg;
    rw[0x58] = &timer012irq->tifr_reg;
    rw[0x57] = &spmRegister->spmcr_reg;
//  rw[0x56] TWCR
    rw[0x55] = mcucr_reg;
    rw[0x54] = mcucsr_reg;
    rw[0x53] = &timer0->tccr_reg;
    rw[0x52] = &timer0->tcnt_reg;
    rw[0x51] = osccal_reg;
    rw[0x50] = sfior_reg;
    rw[0x4f] = &timer1->tccra_reg;
    rw[0x4e] = &timer1->tccrb_reg;
    rw[0x4d] = &timer1->tcnt_h_reg;
    rw[0x4c] = &timer1->tcnt_l_reg;
    rw[0x4b] = &timer1->ocra_h_reg;
    rw[0x4a] = &timer1->ocra_l_reg;
    rw[0x49] = &timer1->ocrb_h_reg;
    rw[0x48] = &timer1->ocrb_l_reg;
    rw[0x47] = &timer1->icr_h_reg;
    rw[0x46] = &timer1->icr_l_reg;
    rw[0x45] = &timer2->tccr_reg;
    rw[0x44] = &timer2->tcnt_reg;
    rw[0x43] = &timer2->ocra_reg;
    rw[0x42] = assr_reg;
    rw[0x41] = &wado->wdtcr_reg;
    rw[0x40] = &usart->ucsrc_ubrrh_reg;
    rw[0x3f] = &eeprom->eearh_reg;
    rw[0x3e] = &eeprom->eearl_reg;
    rw[0x3d] = &eeprom->eedr_reg;
    rw[0x3c] = &eeprom->eecr_reg;
//  rw[0x3b] Reserved
//  rw[0x3a] Reserved
//  rw[0x39] Reserved
    rw[0x38] = &portb->port_reg;
    rw[0x37] = &portb->ddr_reg;
    rw[0x36] = &portb->pin_reg;
    rw[0x35] = &portc->port_reg;
    rw[0x34] = &portc->ddr_reg;
    rw[0x33] = &portc->pin_reg;
    rw[0x32] = &portd->port_reg;
    rw[0x31] = &portd->ddr_reg;
    rw[0x30] = &portd->pin_reg;
    rw[0x2f] = &spi->spdr_reg;
    rw[0x2e] = &spi->spsr_reg;
    rw[0x2d] = &spi->spcr_reg;
    rw[0x2c] = &usart->udr_reg;
    rw[0x2b] = &usart->ucsra_reg;
    rw[0x2a] = &usart->ucsrb_reg;
    rw[0x29] = &usart->ubrr_reg;
    rw[0x28] = &acomp->acsr_reg;
    rw[0x27] = &ad->admux_reg;
    rw[0x26] = &ad->adcsra_reg;
    rw[0x25] = &ad->adch_reg;
    rw[0x24] = &ad->adcl_reg;
//  rw[0x23] TWDR
//  rw[0x22] TWAR
//  rw[0x21] TWSR
//  rw[0x20] TWBR

    Reset();
}

AvrDevice_atmega8::~AvrDevice_atmega8() {
    delete acomp;
    delete timer2;
    delete timer1;
    delete inputCapture1;
    delete timer0;
    delete usart;
    delete wado;
    delete prescaler2;
    delete prescaler01;
    delete assr_reg;
    delete extirq;
    delete mcucsr_reg;
    delete mcucr_reg;
    delete gifr_reg;
    delete gicr_reg;
    delete spi;
    delete ad;
    delete aref;
    delete admux;
    delete sfior_reg;
    delete spmRegister;
    delete portd;
    delete portc;
    delete portb;
    delete osccal_reg;
    delete stack;
    delete eeprom;
    delete irqSystem;
}
