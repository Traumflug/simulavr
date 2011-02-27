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
#include "atmega1284abase.h"

#include "irqsystem.h"
#include "hwstack.h"
#include "hweeprom.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "avrerror.h"
#include "avrfactory.h"

AVR_REGISTER(atmega164A, AvrDevice_atmega164A);
AVR_REGISTER(atmega324A, AvrDevice_atmega324A);
AVR_REGISTER(atmega644A, AvrDevice_atmega644A);
AVR_REGISTER(atmega1284A, AvrDevice_atmega1284A);

AvrDevice_atmega1284Abase::~AvrDevice_atmega1284Abase() {
    delete usart1;
    delete usart0;
    delete wado;
    delete spi;
    delete gpior2_reg;
    delete gpior1_reg;
    delete gpior0_reg;
    delete ad;
    delete timer2;
    delete timerIrq2;
    delete timer1;
    delete inputCapture1;
    delete timerIrq1;
    delete timer0;
    delete timerIrq0;
    delete extirqpc;
    delete pcmsk3_reg;
    delete pcmsk2_reg;
    delete pcmsk1_reg;
    delete pcmsk0_reg;
    delete pcifr_reg;
    delete pcicr_reg;
    delete extirq012;
    delete eifr_reg;
    delete eimsk_reg;
    delete eicra_reg;
    delete stack;
    delete eeprom;
    delete irqSystem;
}

AvrDevice_atmega1284Abase::AvrDevice_atmega1284Abase(unsigned ram_bytes,
                                                     unsigned flash_bytes,
                                                     unsigned ee_bytes ):
    AvrDevice(256-32,       // I/O space size (above ALU registers)
              ram_bytes,    // RAM size
              0,            // External RAM size
              flash_bytes), // Flash Size
    aref(),
    porta(this, "A", true),
    portb(this, "B", true),
    portc(this, "C", true),
    portd(this, "D", true),
    assr_reg(&coreTraceGroup, "ASSR"),
    gtccr_reg(&coreTraceGroup, "GTCCR"),
    prescaler01(this, "01", &gtccr_reg, 0, 7),
    prescaler2(this, "2", PinAtPort(&portb, 6), &assr_reg, 5, &gtccr_reg, 1, 7),
    admux(this,
          &porta.GetPin(0),
          &porta.GetPin(1),
          &porta.GetPin(2),
          &porta.GetPin(3),
          &porta.GetPin(4),
          &porta.GetPin(5),
          &porta.GetPin(6),
          &porta.GetPin(7)) // TODO: Differential inputs, 1.1V ref, GND input
{ 
    irqSystem = new HWIrqSystem(this, 4, 31);

    eeprom = new HWEeprom(this, irqSystem, ee_bytes, 25, HWEeprom::DEVMODE_EXTENDED); 
    stack = new HWStackSram(this, 16);

    RegisterPin("AREF", &aref);

    eicra_reg = new IOSpecialReg(&coreTraceGroup, "EICRA");
    eimsk_reg = new IOSpecialReg(&coreTraceGroup, "EIMSK");
    eifr_reg = new IOSpecialReg(&coreTraceGroup, "EIFR");
    extirq012 = new ExternalIRQHandler(this, irqSystem, eimsk_reg, eifr_reg);
    extirq012->registerIrq(1, 0, new ExternalIRQSingle(eicra_reg, 0, 2, GetPin("D2")));
    extirq012->registerIrq(2, 1, new ExternalIRQSingle(eicra_reg, 2, 2, GetPin("D3")));
    extirq012->registerIrq(3, 2, new ExternalIRQSingle(eicra_reg, 4, 2, GetPin("B2")));

    pcicr_reg = new IOSpecialReg(&coreTraceGroup, "PCICR");
    pcifr_reg = new IOSpecialReg(&coreTraceGroup, "PCIFR");
    pcmsk0_reg = new IOSpecialReg(&coreTraceGroup, "PCMSK0");
    pcmsk1_reg = new IOSpecialReg(&coreTraceGroup, "PCMSK1");
    pcmsk2_reg = new IOSpecialReg(&coreTraceGroup, "PCMSK2");
    pcmsk3_reg = new IOSpecialReg(&coreTraceGroup, "PCMSK3");
    extirqpc = new ExternalIRQHandler(this, irqSystem, pcicr_reg, pcifr_reg);
    extirqpc->registerIrq(4, 0, new ExternalIRQPort(pcmsk0_reg, &porta));
    extirqpc->registerIrq(5, 1, new ExternalIRQPort(pcmsk1_reg, &portb));
    extirqpc->registerIrq(6, 2, new ExternalIRQPort(pcmsk2_reg, &portc));
    extirqpc->registerIrq(7, 3, new ExternalIRQPort(pcmsk3_reg, &portd));

    timerIrq0 = new TimerIRQRegister(this, irqSystem, 0);
    timerIrq0->registerLine(0, new IRQLine("TOV0",  18));
    timerIrq0->registerLine(1, new IRQLine("OCF0A", 16));
    timerIrq0->registerLine(2, new IRQLine("OCF0B", 17));

    timer0 = new HWTimer8_2C(this,
                             new PrescalerMultiplexerExt(&prescaler01, PinAtPort(&portd, 4)),
                             0,
                             timerIrq0->getLine("TOV0"),
                             timerIrq0->getLine("OCF0A"),
                             new PinAtPort(&portb, 3),
                             timerIrq0->getLine("OCF0B"),
                             new PinAtPort(&portb, 4));

    timerIrq1 = new TimerIRQRegister(this, irqSystem, 1);
    timerIrq1->registerLine(0, new IRQLine("TOV1",  15));
    timerIrq1->registerLine(1, new IRQLine("OCF1A", 13));
    timerIrq1->registerLine(2, new IRQLine("OCF1B", 14));
    timerIrq1->registerLine(5, new IRQLine("ICF1",  12));

    inputCapture1 = new ICaptureSource(PinAtPort(&portb, 0));
    timer1 = new HWTimer16_2C3(this,
                               new PrescalerMultiplexerExt(&prescaler01, PinAtPort(&portd, 5)),
                               1,
                               timerIrq1->getLine("TOV1"),
                               timerIrq1->getLine("OCF1A"),
                               new PinAtPort(&portd, 5),
                               timerIrq1->getLine("OCF1B"),
                               new PinAtPort(&portd, 4),
                               timerIrq1->getLine("ICF1"),
                               inputCapture1);

    timerIrq2 = new TimerIRQRegister(this, irqSystem, 2);
    timerIrq2->registerLine(0, new IRQLine("TOV2",  11));
    timerIrq2->registerLine(1, new IRQLine("OCF2A", 9));
    timerIrq2->registerLine(2, new IRQLine("OCF2B", 10));

    timer2 = new HWTimer8_2C(this,
                             new PrescalerMultiplexer(&prescaler2),
                             2,
                             timerIrq2->getLine("TOV2"),
                             timerIrq2->getLine("OCF2A"),
                             new PinAtPort(&portd, 7),
                             timerIrq2->getLine("OCF2B"),
                             new PinAtPort(&portd, 6));

	gpior0_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR0");
	gpior1_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR1");
	gpior2_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR2");

    ad = new HWAd(this, &admux, irqSystem, aref, 24);
    spi = new HWSpi(this,
                    irqSystem,
                    PinAtPort(&portb, 5),   // MOSI
                    PinAtPort(&portb, 6),   // MISO
                    PinAtPort(&portb, 7),   // SCK
                    PinAtPort(&portb, 4),   // /SS
                    19,                     // irqvec
                    true);
    
    wado = new HWWado(this);

    usart0 = new HWUsart(this,
                         irqSystem,
                         PinAtPort(&portd, 1),    // TXD0
                         PinAtPort(&portd, 0),    // RXD0
                         PinAtPort(&portb, 0),    // XCK0
                         20,   // (21) RX complete vector
                         21,   // (22) UDRE vector
                         22);  // (23) TX complete vector

    usart1 = new HWUsart(this,
                         irqSystem,
                         PinAtPort(&portd, 3),    // TXD1
                         PinAtPort(&portd, 2),    // RXD1
                         PinAtPort(&portd, 4),    // XCK1
                         28,   // (29) RX complete vector
                         29,   // (30) UDRE vector
                         30,   // (31) TX complete vector
                         1);   // instance_id for tracking in UI

    // 0xCF - 0xFF reserved

    rw[0xCE]= & usart1->udr_reg;
    rw[0xCD]= & usart1->ubrrhi_reg;
    rw[0xCC]= & usart1->ubrr_reg;
    // 0xCB reserved
    rw[0xCA]= & usart1->ucsrc_reg;
    rw[0xC9]= & usart1->ucsrb_reg;
    rw[0xC8]= & usart1->ucsra_reg;
    // 0xC7 reserved
    rw[0xC6]= & usart0->udr_reg;
    rw[0xC5]= & usart0->ubrrhi_reg;
    rw[0xC4]= & usart0->ubrr_reg;
    // 0xC3 reserved
    rw[0xC2]= & usart0->ucsrc_reg;
    rw[0xC1]= & usart0->ucsrb_reg;
    rw[0xC0]= & usart0->ucsra_reg;
    // 0xBF reserved
    // 0xBE reserved
    rw[0xBD]= new NotSimulatedRegister("TWI register TWAMR not simulated");
    rw[0xBC]= new NotSimulatedRegister("TWI register TWCR not simulated");
    rw[0xBB]= new NotSimulatedRegister("TWI register TWDR not simulated");
    rw[0xBA]= new NotSimulatedRegister("TWI register TWAR not simulated");
    rw[0xB9]= new NotSimulatedRegister("TWI register TWSR not simulated");
    rw[0xB8]= new NotSimulatedRegister("TWI register TWBR not simulated");
    // 0xB7 reserved
    rw[0xb6]= & assr_reg;
    // 0xb5 reserved
    rw[0xb4]= & timer2->ocrb_reg;
    rw[0xb3]= & timer2->ocra_reg;
    rw[0xb2]= & timer2->tcnt_reg;
    rw[0xb1]= & timer2->tccrb_reg;
    rw[0xb0]= & timer2->tccra_reg;
    // 0x8c - 0xaf reserved
    rw[0x8b]= & timer1->ocrb_h_reg;
    rw[0x8a]= & timer1->ocrb_l_reg;
    rw[0x89]= & timer1->ocra_h_reg;
    rw[0x88]= & timer1->ocra_l_reg;
    rw[0x87]= & timer1->icr_h_reg;
    rw[0x86]= & timer1->icr_l_reg;
    rw[0x85]= & timer1->tcnt_h_reg;
    rw[0x84]= & timer1->tcnt_l_reg;
    // 0x83 reserved
    rw[0x82]= & timer1->tccrc_reg;
    rw[0x81]= & timer1->tccrb_reg;
    rw[0x80]= & timer1->tccra_reg;
    rw[0x7F]= new NotSimulatedRegister("ADC register DIDR1 not simulated");
    rw[0x7E]= new NotSimulatedRegister("ADC register DIDR0 not simulated");
    // 0x7D reserved
    rw[0x7C]= & admux.admux_reg;
    rw[0x7B]= new NotSimulatedRegister("ADC register ADCSRB not simulated");
    rw[0x7A]= & ad->adcsr_reg;
    rw[0x79]= & ad->adch_reg;
    rw[0x78]= & ad->adcl_reg;
    // 0x74, 0x75, 0x76, 0x77 reserved
    rw[0x73]= pcmsk3_reg;
    // 0x72 reserved
    // 0x71 reserved
    rw[0x70]= & timerIrq2->timsk_reg;
    rw[0x6F]= & timerIrq1->timsk_reg;
    rw[0x6E]= & timerIrq0->timsk_reg;
    rw[0x6d]= pcmsk2_reg;
    rw[0x6c]= pcmsk1_reg;
    rw[0x6b]= pcmsk0_reg;
    // 0x6A reserved
    rw[0x69]= eicra_reg;
    rw[0x68]= pcicr_reg;
    // 0x67 reserved
    rw[0x66]= new NotSimulatedRegister("MCU register OSCCAL not simulated");
    // 0x65 reserved
    rw[0x64]= new NotSimulatedRegister("MCU register PRR not simulated");
    // 0x63 reserved
    // 0x62 reserved
    rw[0x61]= new NotSimulatedRegister("MCU register CLKPR not simulated");
    rw[0x60]= new NotSimulatedRegister("MCU register WDTCSR not simulated");
    rw[0x5f]= statusRegister;
    rw[0x5e]= & ((HWStackSram *)stack)->sph_reg;
    rw[0x5d]= & ((HWStackSram *)stack)->spl_reg;
    // 0x58 - 0x5c reserved
    rw[0x57]= new NotSimulatedRegister("Self-programming register SPMCSR not simulated");
    // 0x56 reserved
    rw[0x55]= new NotSimulatedRegister("MCU register MCUCR not simulated");
	rw[0x54]= new NotSimulatedRegister("MCU register MCUSR not simulated");
    rw[0x53]= new NotSimulatedRegister("MCU register SMCR not simulated");
	// 0x52 reserved
	rw[0x51]= new NotSimulatedRegister("On-chip debug register OCDR not simulated");
    rw[0x50]= new NotSimulatedRegister("ADC register ADCSRA not simulated");
    // 0x4F reserved
    rw[0x4E]= & spi->spdr_reg;
    rw[0x4D]= & spi->spsr_reg;
    rw[0x4C]= & spi->spcr_reg;
	rw[0x4B]= gpior2_reg;
	rw[0x4A]= gpior1_reg;
    // 0x49 reserved
    rw[0x48]= & timer0->ocrb_reg;
    rw[0x47]= & timer0->ocra_reg;
    rw[0x46]= & timer0->tcnt_reg;
    rw[0x45]= & timer0->tccrb_reg;
    rw[0x44]= & timer0->tccra_reg;
    rw[0x43]= & gtccr_reg;
    rw[0x42]= & eeprom->eearh_reg;
    rw[0x41]= & eeprom->eearl_reg;
    rw[0x40]= & eeprom->eedr_reg;
    rw[0x3F]= & eeprom->eecr_reg;
    rw[0x3E]= gpior0_reg;
    rw[0x3D]= eimsk_reg;
    rw[0x3C]= eifr_reg;
    rw[0x3b]= pcifr_reg;
    // 0x38, 0x39, 0x3A reserved
    rw[0x37]= & timerIrq2->tifr_reg;
    rw[0x36]= & timerIrq1->tifr_reg;
    rw[0x35]= & timerIrq0->tifr_reg;
    // 0x2C - 0x34 reserved
    rw[0x2B]= & portd.port_reg;
    rw[0x2A]= & portd.ddr_reg;
    rw[0x29]= & portd.pin_reg;
    rw[0x28]= & portc.port_reg;
    rw[0x27]= & portc.ddr_reg;
    rw[0x26]= & portc.pin_reg;
    rw[0x25]= & portb.port_reg;
    rw[0x24]= & portb.ddr_reg;
    rw[0x23]= & portb.pin_reg;
    rw[0x25]= & porta.port_reg;
    rw[0x24]= & porta.ddr_reg;
    rw[0x23]= & porta.pin_reg;

    Reset();
}

