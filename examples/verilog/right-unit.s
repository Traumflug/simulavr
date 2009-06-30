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
.arch ATTiny15
#define __SFR_OFFSET 0
	
#define	__AVR_ATtiny15__	1
#include <avr/io.h>

#include "singlepincomm.h"
	
.section .text
.global _start
_start:	
reset_vec:
	rjmp reset
ext_int0_vec:
	reti
pin_change_vec:
	reti
tim1_cmp_vec:	
	reti
tim1_ovf_vec:
	reti
tim0_ovf_vec:
	reti
ee_rdy_vec:
	reti
ana_comp_vec:
	reti
adc_vec:
	reti
reset:
	ldi temp0, 0x10
	mov spc_delay, temp0
l:	
	spc_slave_menu
	rjmp l
