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
//  
//-----------------------------------------------------------------------------
//
// IMPORTANT NOTE: This file is only to illustrate the simulavrxx<->verilog
// interface and is by no means any reference for anything whatsoever!  It
// probably contains lots of bugs! As already stated above, there is no
// warranty!
//
//-----------------------------------------------------------------------------
#ifndef __SINGLEPINCOMM_H
#define __SINGLEPINCOMM_H
//-----------------------------------------------------------------------------
//! Initialize SPC interface
/* The SPC interface uses the 16bit timer counter1 */
void spc_init();

//! TRX one byte
/* Does not block any interrupts and works through the timer/counter1
   interface and its interrupts.

   Returns the received byte in the low byte.
   If the high byte is non-zero, an error has occured.
   */
uint16_t spc_trx(uint8_t val);

//! Send/receive single bits
uint8_t spc_trx_bit(uint8_t bit);

//! Value for producing delays
extern uint16_t spc_delay;
//! Multiplier value
extern uint8_t spc_multiplier;
/*! Minimum distance (in smaller counts) to the larger plateau before
  signalling an error. */
extern uint8_t spc_mindistance;
//-----------------------------------------------------------------------------
#endif
