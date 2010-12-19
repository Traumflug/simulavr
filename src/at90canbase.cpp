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
#include "at90canbase.h"

#include "irqsystem.h"
#include "hwstack.h"
#include "hweeprom.h"
#include "hwwado.h"
#include "hwsreg.h"
#include "avrerror.h"
#include "avrfactory.h"

AVR_REGISTER(at90can32, AvrDevice_at90can32);
AVR_REGISTER(at90can64, AvrDevice_at90can64);
AVR_REGISTER(at90can128, AvrDevice_at90can128);

AvrDevice_at90canbase::~AvrDevice_at90canbase() {
    delete extirq01;
    delete eicra_reg;
    delete eicrb_reg;
    delete eimsk_reg;
    delete eifr_reg;
    delete ad;
    delete spi;
    delete acomp;
    delete usart0;
    delete usart1;
    delete timerIrq0;
    delete timer0;
    delete inputCapture1;
    delete timerIrq1;
    delete timer1;
    delete timerIrq2;
    delete timer2;

    delete inputCapture3;
    delete timerIrq3;
    delete timer3;

    delete gpior0_reg;
    delete gpior1_reg;
    delete gpior2_reg;
}

AvrDevice_at90canbase::AvrDevice_at90canbase(unsigned ram_bytes,
                                                 unsigned flash_bytes,
                                                 unsigned ee_bytes ):
    AvrDevice(224,          // I/O space above General Purpose Registers
              ram_bytes,    // RAM size
              0,            // External RAM size
              flash_bytes), // Flash Size
    aref(),
    porta(this, "A", true),
    portb(this, "B", true),
    portc(this, "C", true, 7),
    portd(this, "D", true),
    porte(this, "E", true),
    portf(this, "F", true),
    portg(this, "G", true),
    assr_reg(&coreTraceGroup, "ASSR"),
    gtccr_reg(&coreTraceGroup, "GTCCR"),
    prescaler013(this, "01", &gtccr_reg, 0, 7),
    prescaler2(this, "2", PinAtPort(&portc, 7), &assr_reg, 5, &gtccr_reg, 1, 7),

    admux(this,
          &portf.GetPin(0),
          &portf.GetPin(1),
          &portf.GetPin(2),
          &portf.GetPin(3),
          &portf.GetPin(4),
          &portf.GetPin(5),
          &portf.GetPin(6),
          &portf.GetPin(7))
{
    flagJMPInstructions = (flash_bytes > 8U * 1024U) ? true : false;
    irqSystem = new HWIrqSystem(this, (flash_bytes > 8U * 1024U) ? 4 : 2, 26);

    eeprom = new HWEeprom(this, irqSystem, ee_bytes, 23, HWEeprom::DEVMODE_EXTENDED); 
    stack = new HWStackSram(this, 16);

    RegisterPin("AREF", &aref);
    rampz = new AddressExtensionRegister(this, "RAMPZ", 1);

    eicra_reg = new IOSpecialReg(&coreTraceGroup, "EICRA");
    eicrb_reg = new IOSpecialReg(&coreTraceGroup, "EICRB");
    eimsk_reg = new IOSpecialReg(&coreTraceGroup, "EIMSK");
    eifr_reg =  new IOSpecialReg(&coreTraceGroup, "EIFR");
    extirq01 = new ExternalIRQHandler(this, irqSystem, eimsk_reg, eifr_reg);
    extirq01->registerIrq(1, 0, new ExternalIRQSingle(eicra_reg, 0, 2, GetPin("D0")));
    extirq01->registerIrq(2, 1, new ExternalIRQSingle(eicra_reg, 2, 2, GetPin("D1")));
    extirq01->registerIrq(3, 2, new ExternalIRQSingle(eicra_reg, 4, 2, GetPin("D2")));
    extirq01->registerIrq(4, 3, new ExternalIRQSingle(eicra_reg, 6, 2, GetPin("D3")));
    extirq01->registerIrq(5, 4, new ExternalIRQSingle(eicrb_reg, 0, 2, GetPin("E4")));
    extirq01->registerIrq(6, 5, new ExternalIRQSingle(eicrb_reg, 2, 2, GetPin("E5")));
    extirq01->registerIrq(7, 6, new ExternalIRQSingle(eicrb_reg, 4, 2, GetPin("E6")));
    extirq01->registerIrq(8, 7, new ExternalIRQSingle(eicrb_reg, 6, 2, GetPin("E7")));
    
    timerIrq0 = new TimerIRQRegister(this, irqSystem, 0);
    timerIrq0->registerLine(0, new IRQLine("TOV0",  16));
    timerIrq0->registerLine(1, new IRQLine("OCF0A", 15));
    
    timer0 = new HWTimer8_1C(this,
                             new PrescalerMultiplexerExt(&prescaler013, PinAtPort(&portd, 7)),
                             0,
                             timerIrq0->getLine("TOV0"),
                             timerIrq0->getLine("OCF0A"),
                             new PinAtPort(&portb, 7));

    timerIrq1 = new TimerIRQRegister(this, irqSystem, 1);
    timerIrq1->registerLine(0, new IRQLine("TOV1",  14));
    timerIrq1->registerLine(1, new IRQLine("OCF1A", 13));
    timerIrq1->registerLine(2, new IRQLine("OCF1B", 12));
    timerIrq1->registerLine(3, new IRQLine("OCF1C", 11));
    timerIrq1->registerLine(5, new IRQLine("ICF1",  10));
    
    inputCapture1 = new ICaptureSource(PinAtPort(&portd, 4));
    timer1 = new HWTimer16_3C(this,
                               new PrescalerMultiplexer(&prescaler013),
                               1,
                               timerIrq1->getLine("TOV1"),
                               timerIrq1->getLine("OCF1A"),
                               new PinAtPort(&portb, 1),
                               timerIrq1->getLine("OCF1B"),
                               new PinAtPort(&portb, 2),
                               timerIrq1->getLine("OCF1C"),
                               new PinAtPort(&portb, 3),
                               timerIrq1->getLine("ICF1"),
                               inputCapture1);
    
    timerIrq2 = new TimerIRQRegister(this, irqSystem, 2);
    timerIrq2->registerLine(0, new IRQLine("TOV2",  9));
    timerIrq2->registerLine(1, new IRQLine("OCF2A", 7));
    
    timer2 = new HWTimer8_1C(this,
                             new PrescalerMultiplexer(&prescaler2),
                             2,
                             timerIrq2->getLine("TOV2"),
                             timerIrq2->getLine("OCF2A"),
                             new PinAtPort(&portb, 4));

    timerIrq3 = new TimerIRQRegister(this, irqSystem, 3);
    timerIrq3->registerLine(0, new IRQLine("TOV3",  30));
    timerIrq3->registerLine(1, new IRQLine("OCF3A", 27));
    timerIrq3->registerLine(2, new IRQLine("OCF3B", 28));
    timerIrq3->registerLine(3, new IRQLine("OCF3C", 29));
    timerIrq3->registerLine(5, new IRQLine("ICF3",  26));

    inputCapture3 = new ICaptureSource(PinAtPort(&porte, 7));
    timer3 = new HWTimer16_3C(this,
                               new PrescalerMultiplexerExt(&prescaler013, PinAtPort(&porte, 6)),
                               3,
                               timerIrq3->getLine("TOV3"),
                               timerIrq3->getLine("OCF3A"),
                               new PinAtPort(&portb, 1),
                               timerIrq3->getLine("OCF3B"),
                               new PinAtPort(&portb, 2),
                               timerIrq3->getLine("OCF3C"),
                               new PinAtPort(&portb, 3),
                               timerIrq3->getLine("ICF3"),
                               inputCapture3);

    gpior0_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR0");
    gpior1_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR1");
    gpior2_reg = new GPIORegister(this, &coreTraceGroup, "GPIOR2");

    ad = new HWAd(this, &admux, irqSystem, aref, 25);
    spi = new HWSpi(this,
                    irqSystem,
                    PinAtPort(&portb, 2),   // MOSI
                    PinAtPort(&portb, 3),   // MISO
                    PinAtPort(&portb, 1),   // SCK
                    PinAtPort(&portb, 0),   // /SS
                    20,                     // irqvec
                    true);

    wado = new HWWado(this);

    acomp = new HWAcomp(this, irqSystem, PinAtPort(&porte, 2), PinAtPort(&porte, 3), 23);

    usart0 = new HWUsart(this,
                         irqSystem,
                         PinAtPort(&porte,1),    // TXD
                         PinAtPort(&porte,0),    // RXD
                         PinAtPort(&porte,2),   // XCK
                         20,   // RX complete vector
                         21,   // UDRE vector
                         22,   // TX complete vector
                         0);

    usart1 = new HWUsart(this,
                         irqSystem,
                         PinAtPort(&portd,3),    // TXD
                         PinAtPort(&portd,2),    // RXD
                         PinAtPort(&portd,5),   // XCK
                         31,   // RX complete vector
                         32,   // UDRE vector
                         33,   // TX complete vector
                         1);


    /* 0xfb - 0xff reserved */
    /* 0xd8 - 0xfa CANBUS TODO */
    /* 0xcf - 0xd7 reserved */
    rw[0xce]= & usart1->udr_reg;
    rw[0xcd]= & usart1->ubrrhi_reg;
    /* 0xcb reserved */
    rw[0xca]= & usart1->ucsrc_reg;
    rw[0xcc]= & usart1->ubrr_reg;
    rw[0xc9]= & usart1->ucsrb_reg;
    rw[0xc8]= & usart1->ucsra_reg;
    /* 0xc7 reserved */
    rw[0xc6]= & usart0->udr_reg;
    rw[0xc5]= & usart0->ubrrhi_reg;
    rw[0xc4]= & usart0->ubrr_reg;
    /* 0xc3 reserved */
    rw[0xc2]= & usart0->ucsrc_reg;
    rw[0xc1]= & usart0->ucsrb_reg;
    rw[0xc0]= & usart0->ucsra_reg;
    /* 0xbd - 0xbf reserved */
    /* 0xb8 - 0xbc Twin Wire TODO */
    /* 0xb7 reserved */
    rw[0xb6]= & assr_reg;
    /* 0xb4 - 0xb5 reserved */
    rw[0xb3]= & timer2->ocra_reg;
    rw[0xb2]= & timer2->tcnt_reg;
    /* 0xb1 reserved */
    rw[0xb0]= & timer2->tccr_reg;
    /* 0x9e - 0xaf reserved */
    rw[0x9d]= & timer3->ocrc_h_reg;
    rw[0x9c]= & timer3->ocrc_l_reg;
    rw[0x9b]= & timer3->ocrb_h_reg;
    rw[0x9a]= & timer3->ocrb_l_reg;
    rw[0x99]= & timer3->ocra_h_reg;
    rw[0x98]= & timer3->ocra_l_reg;
    rw[0x97]= & timer3->icr_h_reg;
    rw[0x96]= & timer3->icr_l_reg;
    rw[0x95]= & timer3->tcnt_h_reg;
    rw[0x94]= & timer3->tcnt_l_reg;
    /* 0x93 reserved */
    rw[0x92]= & timer3->tccrc_reg;
    rw[0x91]= & timer3->tccrb_reg;
    rw[0x90]= & timer3->tccra_reg;
    /* 0x8e - 0x8f reserved */
    rw[0x8d]= & timer1->ocrc_h_reg;
    rw[0x8c]= & timer1->ocrc_l_reg;
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
    /* 0x7e-0x7f DIDR TODO */
    rw[0x7C]= & admux.admux_reg;
    rw[0x7A]= & ad->adcsr_reg;
    rw[0x79]= & ad->adch_reg;
    rw[0x78]= & ad->adcl_reg;
    /* 0x76-0x77 reserved */
    /* 0x74-0x75 External memory control registers TODO */
    /* 0x72-0x73 reserved */
    rw[0x70]= & timerIrq2->timsk_reg;
    rw[0x6F]= & timerIrq1->timsk_reg;
    rw[0x6E]= & timerIrq0->timsk_reg;
    /* 0x6b-0x6d Reserved */
    /* rw[0x6A]= eicrb_reg; XXX */
    rw[0x69]= eicra_reg;
    /*rw[0x68]= pcicr_reg; XXX */
    /* 0x67-0x68 Reserved */
    /* 0x66 OSCCAL TODO */
    /* 0x62-0x65 Reserved */
    /* 0x61 CLKPR TODO */
    rw[0x60]= & wado->wdtcr_reg;
    rw[0x5f]= statusRegister;
    rw[0x5e]= & ((HWStackSram *)stack)->sph_reg;
    rw[0x5d]= & ((HWStackSram *)stack)->spl_reg;
    /* 0x5c reserved */
    rw[0x5b]= & rampz->ext_reg;
    /* 0x58-0x5A Reserved */
    /* 0x57 SPMCSR TODO?? */
    /* 0x56 Reserved */
    /* 0x55 MCUCR -- Memory control TODO */
    /* 0x54 MCUSR -- Memory control TODO */
    /* 0x53 SMCR -- sleep register TODO */
    /* 0x52 Reserved */
    /* 0x51 OCDR */
    rw[0x50]= & acomp->acsr_reg;
    /* 0x4f reserved */
    rw[0x4E]= & spi->spdr_reg;
    rw[0x4D]= & spi->spsr_reg;
    rw[0x4C]= & spi->spcr_reg;
    rw[0x4B]= gpior2_reg;
    rw[0x4A]= gpior1_reg;
    /* 0x48 - 0x49 reserved */
    rw[0x47]= & timer0->ocra_reg;
    rw[0x46]= & timer0->tcnt_reg;
    /* 0x45 reserved */
    rw[0x44]= & timer0->tccr_reg;
    rw[0x43]= & gtccr_reg;
    rw[0x42]= & eeprom->eearh_reg;
    rw[0x41]= & eeprom->eearl_reg;
    rw[0x40]= & eeprom->eedr_reg;
    rw[0x3F]= & eeprom->eecr_reg;

    rw[0x3E]= gpior0_reg;
    rw[0x3D]= eimsk_reg;
    rw[0x3C]= eifr_reg;

    /* 0x39-0x3b Reserved */
    rw[0x38]= & timerIrq3->tifr_reg;
    rw[0x37]= & timerIrq2->tifr_reg;
    rw[0x36]= & timerIrq1->tifr_reg;
    rw[0x35]= & timerIrq0->tifr_reg;

    rw[0x34]= & portg.port_reg;
    rw[0x33]= & portg.ddr_reg;
    rw[0x32]= & portg.pin_reg;

    rw[0x31]= & portf.port_reg;
    rw[0x30]= & portf.ddr_reg;
    rw[0x2F]= & portf.pin_reg;

    rw[0x2E]= & porte.port_reg;
    rw[0x2D]= & porte.ddr_reg;
    rw[0x2C]= & porte.pin_reg;

    rw[0x2B]= & portd.port_reg;
    rw[0x2A]= & portd.ddr_reg;
    rw[0x29]= & portd.pin_reg;

    rw[0x28]= & portc.port_reg;
    rw[0x27]= & portc.ddr_reg;
    rw[0x26]= & portc.pin_reg;

    rw[0x25]= & portb.port_reg;
    rw[0x24]= & portb.ddr_reg;
    rw[0x23]= & portb.pin_reg;

    rw[0x22]= & porta.port_reg;
    rw[0x21]= & porta.ddr_reg;
    rw[0x20]= & porta.pin_reg;

    Reset();
}

