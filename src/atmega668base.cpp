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

#include "irqsystem.h"
#include "hwstack.h"
#include "hweeprom.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "avrerror.h"
#include "avrfactory.h"

AVR_REGISTER(atmega48, AvrDevice_atmega48);
AVR_REGISTER(atmega88, AvrDevice_atmega88);
AVR_REGISTER(atmega168, AvrDevice_atmega168);
AVR_REGISTER(atmega328, AvrDevice_atmega328);

AvrDevice_atmega668base::~AvrDevice_atmega668base() {
    delete rw[0x5f]; // RWSreg
    delete usart0;
    delete wado;
    delete spi;
    delete ad;
    delete timer2;
    delete timerIrq2;
    delete timer1;
    delete inputCapture1;
    delete timerIrq1;
    delete timer0;
    delete timerIrq0;
    delete extirq;
    delete stack;
    delete eeprom;
    delete irqSystem;
}

AvrDevice_atmega668base::AvrDevice_atmega668base(unsigned ram_bytes,
                                                 unsigned flash_bytes,
                                                 unsigned ee_bytes ):
    AvrDevice(224,          // I/O space above General Purpose Registers
              ram_bytes,    // RAM size
              0,            // External RAM size
              flash_bytes), // Flash Size
    aref(),
    adc6(),
    adc7(),
    portb(this, "B"),
    portc(this, "C"),
    portd(this, "D"),
    assr_reg(&coreTraceGroup, "ASSR"),
    gtccr_reg(&coreTraceGroup, "GTCCR"),
    prescaler01(this, "01", &gtccr_reg, 0, 7),
    prescaler2(this, "2", PinAtPort(&portb, 6), &assr_reg, 5, &gtccr_reg, 1, 7),
    admux(this,
          &portc.GetPin(0),
          &portc.GetPin(1),
          &portc.GetPin(2),
          &portc.GetPin(3),
          &portc.GetPin(4),
          &portc.GetPin(5),
          &adc6,
          &adc7)
{ 
    irqSystem = new HWIrqSystem(this, (flash_bytes > 8U * 1024U) ? 4 : 2, 26);

    eeprom = new HWEeprom(this, irqSystem, ee_bytes, 23, HWEeprom::DEVMODE_EXTENDED); 
    stack = new HWStack(this, Sram, 0x10000);

    RegisterPin("AREF", &aref);
    RegisterPin("ADC6", &adc6);
    RegisterPin("ADC7", &adc7);

    extirq = new HWMega48ExtIrq(this,
                                irqSystem, 
                                PinAtPort(&portd, 0),
                                PinAtPort(&portd, 1),
                                1,
                                2);

    timerIrq0 = new TimerIRQRegister(this, irqSystem, 0);
    timerIrq0->registerLine(0, new IRQLine("TOV0",  16));
    timerIrq0->registerLine(1, new IRQLine("OCF0A", 14));
    timerIrq0->registerLine(2, new IRQLine("OCF0B", 15));
    
    timer0 = new HWTimer8_2C(this,
                             new PrescalerMultiplexerExt(&prescaler01, PinAtPort(&portd, 4)),
                             0,
                             timerIrq0->getLine("TOV0"),
                             timerIrq0->getLine("OCF0A"),
                             new PinAtPort(&portd, 6),
                             timerIrq0->getLine("OCF0B"),
                             new PinAtPort(&portd, 5));

    timerIrq1 = new TimerIRQRegister(this, irqSystem, 1);
    timerIrq1->registerLine(0, new IRQLine("TOV1",  13));
    timerIrq1->registerLine(1, new IRQLine("OCF1A", 11));
    timerIrq1->registerLine(2, new IRQLine("OCF1B", 12));
    timerIrq1->registerLine(5, new IRQLine("ICF1",  10));
    
    inputCapture1 = new ICaptureSource(PinAtPort(&portb, 0));
    timer1 = new HWTimer16_2C3(this,
                               new PrescalerMultiplexerExt(&prescaler01, PinAtPort(&portd, 5)),
                               1,
                               timerIrq1->getLine("TOV1"),
                               timerIrq1->getLine("OCF1A"),
                               new PinAtPort(&portb, 1),
                               timerIrq1->getLine("OCF1B"),
                               new PinAtPort(&portb, 2),
                               timerIrq1->getLine("ICF1"),
                               inputCapture1);
    
    timerIrq2 = new TimerIRQRegister(this, irqSystem, 2);
    timerIrq2->registerLine(0, new IRQLine("TOV2",  9));
    timerIrq2->registerLine(1, new IRQLine("OCF2A", 7));
    timerIrq2->registerLine(2, new IRQLine("OCF2B", 8));
    
    timer2 = new HWTimer8_2C(this,
                             new PrescalerMultiplexer(&prescaler2),
                             2,
                             timerIrq2->getLine("TOV2"),
                             timerIrq2->getLine("OCF2A"),
                             new PinAtPort(&portb, 3),
                             timerIrq2->getLine("OCF2B"),
                             new PinAtPort(&portd, 3));

    ad = new HWAd(this, &admux, irqSystem, aref, 21);
    spi = new HWSpi(this,
                    irqSystem,
                    PinAtPort(&portb, 3),   // MOSI
                    PinAtPort(&portb, 4),   // MISO
                    PinAtPort(&portb, 5),   // SCK
                    PinAtPort(&portb, 2),   // /SS
                    17,                     // irqvec
                    true);
    
    wado = new HWWado(this);

    usart0 = new HWUsart(this,
                         irqSystem,
                         PinAtPort(&portd,1),    // TXD
                         PinAtPort(&portd,0),    // RXD
                         PinAtPort(&portd, 4),   // XCK
                         19,   // (18) RX complete vector
                         20,   // (19) UDRE vector
                         21);  // (20) TX complete vector

    rw[0xE6]= & usart0->udr_reg;

    rw[0xE4]= & usart0->ubrr_reg;

    rw[0xE1]= & usart0->ucsrb_reg;
    rw[0xE0]= & usart0->ucsra_reg;

    rw[0xC5]= & usart0->ubrrhi_reg;

    rw[0xC2]= & usart0->ucsrc_reg;

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
    
    rw[0x7C]= & admux.admux_reg;

    rw[0x7A]= & ad->adcsr_reg;
    rw[0x79]= & ad->adch_reg;
    rw[0x78]= & ad->adcl_reg;

    rw[0x70]= & timerIrq2->timsk_reg;
    rw[0x6F]= & timerIrq1->timsk_reg;
    rw[0x6E]= & timerIrq0->timsk_reg;

    rw[0x69]= & extirq->eicra_reg;

    rw[0x5f]= statusRegister;
    rw[0x5e]= & stack->sph_reg;
    rw[0x5d]= & stack->spl_reg;

    rw[0x4E]= & spi->spdr_reg;
    rw[0x4D]= & spi->spsr_reg;
    rw[0x4C]= & spi->spcr_reg;

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

    rw[0x3D]= & extirq->eimsk_reg;

    rw[0x3C]= & extirq->eifr_reg;

    rw[0x37]= & timerIrq2->tifr_reg;
    rw[0x36]= & timerIrq1->tifr_reg;
    rw[0x35]= & timerIrq0->tifr_reg;

    rw[0x2B]= & portd.port_reg;
    rw[0x2A]= & portd.ddr_reg;
    rw[0x29]= & portd.pin_reg;
    
    rw[0x28]= & portc.port_reg;
    rw[0x27]= & portc.ddr_reg;
    rw[0x26]= & portc.pin_reg;
    
    rw[0x25]= & portb.port_reg;
    rw[0x24]= & portb.ddr_reg;
    rw[0x23]= & portb.pin_reg;

    Reset();
}

