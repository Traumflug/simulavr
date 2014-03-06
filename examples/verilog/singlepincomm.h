#ifndef SINGLEPINCOMM_REGISTERS
#define SINGLEPINCOMM_REGISTERS
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//-----------------------------------------------------------------------------
//
// IMPORTANT NOTE: This file is only to illustrate the simulavrxx<->verilog
// interface and is by no means any reference for anything whatsoever!  It
// probably contains lots of bugs! As already stated above, there is no
// warranty!
//
//-----------------------------------------------------------------------------
; used by the low level parts
#if SPC_16BIT_TIME
#define lowcntl		r0
#define highcntl	r1
#endif

#define lowcnth		r20
#define highcnth	r21
	
#define temp0		r16
#define temp1		r17
#define trxbyte		r18
#define bitcnt		r19

// Register which may be used for the configurable software delay
#define	spc_delay	r2
//-----------------------------------------------------------------------------
// Port definition for SPC
#define SPC_PORT	PORTB
#define SPC_DDR		DDRB
#define SPC_PIN		PINB
#define SPC_BIT		PB2

// Debugging?
#define SPC_DEBUG			0

// Iff true, take the software-configurable delay from register spc_delay
#define SPC_SWDELAY			1
	
// Constant to delay the short delay (t_s) */
#define SPC_TSVALUE			150
// Multiplier N to go from t_s to t_l:	 t_l=N*t_s */
#define SPC_N				3
	
// Iff true, enclose all timing sensitive parts in CLI/SEI pairs.
#define	SPC_CLISEI			0

// Iff true, implement reading of the flash space through command 0x03 in the menu
#define SPC_FLASH_READ			1

// timeout in high-byte loop cycles for RX bit (0=> 256*256 cycles)
#define SPC_RX_TIMEOUT			0

// Iff true, wait for the line to be high first for each bit
#define SPC_RX_WAIT_INITIAL_HIGH	0

/// Iff true, use 16bit registers for timing
#define SPC_16BIT_TIME			0	
//-----------------------------------------------------------------------------
/* Simple menu for the slave.
   Call it to allow the master to read/write the RAM. */

.macro spc_slave_menu
	clr trxbyte
	rcall spc_trx_slave
	brts ssm_end		; timeout!
	cpi trxbyte, 0x01	; ram read?
	breq ssm_read
	cpi trxbyte, 0x02	; ram write?
	breq ssm_write
#if SPC_FLASH_READ
	cpi trxbyte, 0x03	; flash read?
	breq ssm_flash_read
#endif
	brne ssm_end		; something unknown...
ssm_read:
	rcall spc_trx_slave	; read low byte for location
	brts ssm_end
	mov r30, trxbyte

	rcall spc_trx_slave	; and high byte
	brts ssm_end
	mov r31, trxbyte

	ld trxbyte, Z		; load byte to transmit
	rcall spc_trx_slave	; and send it!
	rjmp ssm_end

ssm_write:
	rcall spc_trx_slave	; read low byte for location
	brts ssm_end
	mov r30, trxbyte

	rcall spc_trx_slave	; and high byte
	brts ssm_end
	mov r31, trxbyte

	rcall spc_trx_slave	; and byte to write!
	brts ssm_end
	st Z, trxbyte
#if SPC_FLASH_READ
ssm_flash_read:
	rcall spc_trx_slave	; read low byte for location
	brts ssm_end
	mov r30, trxbyte

	rcall spc_trx_slave	; and high byte
	brts ssm_end
	mov r31, trxbyte

	lpm			; load byte to transmit!
	mov trxbyte, r0		; r0 can be used, is lowcnt
	rcall spc_trx_slave	; and send it!
#endif
ssm_end:
.endm
//-----------------------------------------------------------------------------
#endif
