// Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//  
//-----------------------------------------------------------------------------
//
// IMPORTANT NOTE: This file is only to illustrate the simulavrxx<->verilog
// interface and is by no means any reference for anything whatsoever!  It
// probably contains lots of bugs! As already stated above, there is no
// warranty!
//
//-----------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include "csinglepincomm.h"
#define F_CPU 12000000
// Port definition for SPC
// NOTE: DO NOT CHANGE THESE EASILY, THE TIMER OUTPUT PINS ARE USED
// SO CHANGING THE PINS IS USUALLY NOT AN OPTION!
#define SPC_PORT	PORTB
#define SPC_DDR		DDRB
#define SPC_PIN		PINB
#define SPC_BIT		PB3

// Constant to delay the short delay (t_s), initial value. In clock ticks */
#define SPC_INITIAL_DELAY	1500

// Initial multiplier N to go from t_s to t_l:	 t_l=N*t_s */
 #define SPC_LONG_DELAY		3

// loop cycles to wait for receive signal
#define SPC_RX_TIMEOUT		20000

// minimum distance between the high and the low count, in number of counts of
// the smaller value
#define SPC_MINIMUM_DISTANCE	2

// Interrupt blocking?
#if SPC_CLISEI
#define NCLI			cli()
#define NSEI			sei()
#else
#define NCLI			
#define NSEI
#endif

uint16_t spc_delay=SPC_INITIAL_DELAY;
uint8_t spc_multiplier=SPC_LONG_DELAY;
uint8_t spc_mindistance=SPC_MINIMUM_DISTANCE;

void spc_init() {
    // enable pull up
    SPC_PORT|=_BV(SPC_BIT);
}

uint8_t spc_trx_bit(uint8_t bit) {
    uint16_t timeout, high_time, low_time;
    uint16_t spc_long_delay=spc_delay * spc_multiplier;
    // ------------------------------------------------------------
    // --- TRANSMIT                                             ---
    // ------------------------------------------------------------
    NCLI;
    TCCR1B=0; // stop counter
    // timer preparation
    
    // force compare-match into high state
    TCCR1A=_BV(COM1A1)|_BV(COM1A0); // set PB3 on compare match
    TCCR1C=_BV(FOC1A); // force output compare
    // set high state of comp.match on output (should be just pullup before)
    SPC_DDR|=_BV(SPC_BIT);
    TCCR1A=_BV(COM1A1); // clear PB3 on compare match
    TIFR|=_BV(OCF1A); // clear comp.match
    
    // set time until zero
    if (bit)
	OCR1A=spc_long_delay;
    else
	OCR1A=spc_delay;
	    
    // reset counter
    TCNT1=0;

     // and apply full system clock timer1
    TCCR1B=_BV(CS10);
    NSEI;
    
    // wait until high part has been sent
    while (! (TIFR & _BV(OCF1A)));

    NCLI;
    TCCR1B=0; // stop  counter
    TCCR1A=_BV(COM1A1)|_BV(COM1A0); // set PB3 on compare match
    TIFR|=_BV(OCF1A); // clear comp.match

    // set time until one
    if (bit)
	OCR1A=spc_delay;
    else
	OCR1A=spc_long_delay;

    // reset counter
    TCNT1=0;

    TCCR1B=_BV(CS10); // start counter
    NSEI;
    
    // wait until low part has been sent
    while (! (TIFR & _BV(OCF1A)));
    NCLI;
    //TIFR|=_BV(OCF1A); // clear comp.match
    // ------------------------------------------------------------
    // --- RECEIVE                                              ---
    // ------------------------------------------------------------
    // go back to normal port mode on PB3
    TCCR1A=0;
    SPC_DDR&=~_BV(SPC_BIT);

    OCR1A=0x0000; // 'disable' OCR1A

    NSEI;
    
    /* stop and clear counter, set noise canceler bit,
       get ready for input capture. */
    TCNT1=0;
    TIFR|=_BV(ICF1);
    TCCR1B=_BV(CS10)|_BV(ICNC1); 
    NSEI;

    // wait for falling edge
    timeout=0;
    while (! (TIFR & _BV(ICF1))) {
	if (timeout == SPC_RX_TIMEOUT)
	    return 2;
	timeout++;
    }
    NCLI;
    high_time=ICR1; // read that value
    // get ready for another round, raising edge now
    TCCR1B=_BV(CS10)|_BV(ICNC1)|_BV(ICES1);
    TIFR|=_BV(ICF1); 
    NSEI;

    // wait for rising edge
    timeout=0;
    while (! (TIFR & _BV(ICF1))) {
	if (timeout == SPC_RX_TIMEOUT)
	    return 2;
	timeout++;
    }

    NCLI;
    low_time=ICR1-high_time; // read that value, too
    NSEI;

    if (high_time>low_time) {
	if (high_time<low_time * spc_mindistance)
	    return 2;
	else return 1;
    } else {
	if (low_time<high_time * spc_mindistance)
	    return 2;
	else return 0;
    }
}

    
uint16_t spc_trx(uint8_t val) {
    uint8_t res;
    uint8_t i;
    uint8_t rxbit;
    
    // send start bit
    i=spc_trx_bit(0);

    if (i) // timeout or 'one' bit?
	return 0x100 * i;

    // main TRX loop
    res=0;
    for (i=0; i < 8; i++) {
	rxbit=spc_trx_bit(val & 0x01);
	if (rxbit>1)
	    return 0x100 * rxbit; // timeout or framing error!
	if (rxbit) res|=1<<i;
	val>>=1;
    }
    return res;
}
