/*
 * Copyright (C) 2007 Onno Kortmann <onno@gmx.net>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
 *  
 */
#include <avr/io.h>
#define F_CPU	12000000
#include <util/delay.h>
#include "csinglepincomm.h"

int main() {
    // wait until other side is ready
    _delay_us(200);
    spc_init();
    spc_trx(0x03); // flash read
    spc_trx(0x01); // adr L
    spc_trx(0x00); // adr H

    /* Flash @ word 0x0001: 0xc0. This should appear in simsend in the gtkwave
       window! */
    
    /* read byte and place into memory location where it
       can be read by by the verilog code... yes, this is very crude! */
    *(unsigned char*)(0x70)=spc_trx(0x00); 
    return 0;
}
