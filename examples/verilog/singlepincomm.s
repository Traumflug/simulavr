//-----------------------------------------------------------------------------
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
//-----------------------------------------------------------------------------
//
// IMPORTANT NOTE: This file is only to illustrate the simulavrxx<->verilog
// interface and is by no means any reference for anything whatsoever!  It
// probably contains lots of bugs! As already stated above, there is no
// warranty!
//
//-----------------------------------------------------------------------------
.arch ATTiny15
#define	__AVR_ATtiny15__	1
#define __SFR_OFFSET 0
#include <avr/io.h>
#include "singlepincomm.h"

#if SPC_DEBUG

// some random TRX debugging...	;-)
.macro dbg1on
  sbi PORTB, PB3
  sbi DDRB, PB3
.endm

.macro dbg1off
  cbi PORTB, PB3
  sbi DDRB, PB3
.endm

.macro dbg2on
  sbi PORTB, PB4
  sbi DDRB, PB4
.endm

.macro dbg2off
  cbi PORTB, PB4
  sbi DDRB, PB4
.endm
#else

.macro dbg1on
.endm
.macro dbg1off
.endm
.macro dbg2on
.endm
.macro dbg2off
.endm
										
#endif

#include "singlepincomm.h"
					
#if SPC_CLISEI
#define CLI		cli
#define SEI		sei
#else
#define CLI		
#define SEI		
#endif

/* Receive one 'bit':	
	0. Switch to input	
	1. Wait for high state (no timeout)
	2. Wait for low state, count this as highcnt
	3. Wait for high state again, count this as lowcnt
	4. Result is:
		C=0 for a received zero (low phase longer than high phase)
		C=1 otherwise
		T=0 for everything ok
		T=1 for timeout

	*/
.global spc_recv_bit
spc_rx_bit:
	dbg1on
	clt
	cbi SPC_DDR, SPC_BIT

#if SPC_RX_WAIT_INITIAL_HIGH
#if SPC_16BIT_TIME	
	clr lowcntl
#endif	
	clr lowcnth

	;;  wait for high (timeout when line is shorted to ground)
srb_waitinit:
	sbic SPC_PIN, SPC_BIT
	rjmp srb_waitinit_ready
#if SPC_16BIT_TIME	
	inc lowcntl
	brne srb_waitinit
#endif	
	inc lowcnth
	cpi lowcnth, SPC_RX_TIMEOUT
	breq srb_timeout
	rjmp srb_waitinit
	
srb_waitinit_ready:
#endif
#if SPC_16BIT_TIME		
	clr lowcntl
#endif	
	clr lowcnth
#if SPC_16BIT_TIME	
	clr highcntl
#endif	
	clr highcnth

	;; wait for low 
srb_wait0:
	sbis SPC_PIN, SPC_BIT
	rjmp srb_wait0_ready
#if SPC_16BIT_TIME	
	inc highcntl
	brne srb_wait0
#endif	
	inc highcnth
	cpi highcnth, SPC_RX_TIMEOUT
	breq srb_timeout
	rjmp srb_wait0
srb_wait0_ready:
	
	;; wait for high
srb_wait1:
	sbic SPC_PIN, SPC_BIT
	rjmp srb_wait1_ready
#if SPC_16BIT_TIME	
	inc lowcntl
	brne srb_wait1
#endif	
	inc lowcnth
	cpi lowcnth, SPC_RX_TIMEOUT
	breq srb_timeout
	rjmp srb_wait1
srb_wait1_ready:	
	;; compare both times
	cp lowcnth, highcnth
#if SPC16BIT_TIME	
	breq  srb_compare_low
#endif	
	dbg1off
	ret
#if SPC16BIT_TIME	
srb_compare_low:
	cp lowcntl, highcntl
#endif	
	dbg1off
	ret
srb_timeout:
	dbg1off
	set
	ret

.macro spc_short_delay
#if SPC_SWDELAY
	mov temp0, spc_delay
#else
	ldi temp0, SPC_TSVALUE
#endif
ll\@:
	dec temp0	
 	brne  ll\@
.endm
		
/* Send one bit from carry. */
.global spc_tx_bit
spc_tx_bit:
	dbg2on
	;; start with transmission of high state
	sbi SPC_PORT, SPC_BIT
	sbi SPC_DDR, SPC_BIT
	brcs tx1
tx0:
	spc_short_delay
	cbi SPC_PORT, SPC_BIT
	ldi temp1, SPC_N
tx0l:	
	spc_short_delay
	dec temp1
	brne tx1l
	;; high again - and input
spc_tx_end:	
	sbi SPC_PORT, SPC_BIT
	cbi SPC_DDR, SPC_BIT
	dbg2off
	ret
tx1:
	ldi temp1, SPC_N
tx1l:
	spc_short_delay
	dec temp1
	brne tx1l
	;; low
	cbi SPC_PORT, SPC_BIT
	spc_short_delay
	;; and high again - and input
	rjmp spc_tx_end


// FIXME: Combine spc_trx_master and spc_trx_slave into one procedure!
	
/* Do a complete transmit/receive for one byte as the master. This means
   synchronizing with the slave (sending and receiving one start-zero) and
	transmitting/receiving the content from/into trxbyte. */
.section .text.spc_trx_master
.global spc_trx_master
spc_trx_master:
	CLI

	;; send start zero
	clc
	rcall spc_tx_bit

	;; receive start zero
	rcall spc_rx_bit
	brts mspc_trx_end	; T=1 -> timeout ->failure

	ldi bitcnt, 8

mtrxloop:
	;; fetch next bit from trxbyte and send it
	lsr trxbyte
	rcall spc_tx_bit
	rcall spc_rx_bit
	brts mspc_trx_end		; handle timeouts
	brcc mtrx_looptail	; zero received -> nothing to do
	ori trxbyte, 0x80	; push one into highest bit.. will rotate through to lower bits
mtrx_looptail:	
	dec bitcnt
	brne mtrxloop
mspc_trx_end:	
	SEI
	ret

/* Do a complete transmit/receive for one byte as the slaver. This means
   synchronizing with the master (receiving and sending one start-zero) and
	transmitting/receiving the content from/into trxbyte. */
.section .text.spc_trx_slave
.global spc_trx_slave
spc_trx_slave:
	CLI

	;; receive start zero
	rcall spc_rx_bit
	brts sspc_trx_end	; -> failure
	
	;; send start zero
	clc
	rcall spc_tx_bit

	ldi bitcnt, 8
strxloop:
	rcall spc_rx_bit
	brts sspc_trx_end	; handle timeouts
	ror trxbyte		; stuff it into the trxbyte
	rcall spc_tx_bit	; and transmit what fell out of the register at the other end

	dec bitcnt
	brne strxloop
sspc_trx_end:	
	SEI
	ret
