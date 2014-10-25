/*
 * Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *  
 * THIS FILE HAS BEEN AUTOMATICALLY GENERATED FROM avr_tmpl.v.
 *                     --- DO NOT EDIT MANUALLY! ---
 */

module ATtiny25(clk, PB);

   parameter progfile="UNSPECIFIED";
   input     clk;
   inout [7:0] PB;
   integer 	handle;

   defparam 	core.progfile=progfile;
   defparam 	core.name="attiny25";
   AVRCORE core(clk);

   avr_pin #("B0") pb0(PB[0]);
   avr_pin #("B1") pb1(PB[1]);
   avr_pin #("B2") pb2(PB[2]);
   avr_pin #("B3") pb3(PB[3]);
   avr_pin #("B4") pb4(PB[4]);
   avr_pin #("B5") pb5(PB[5]);

endmodule
	       
