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

module ATmega8(clk, PB, PC, PD);

   parameter progfile="UNSPECIFIED";
   input     clk;
   inout [7:0] PB;
   inout [7:0] PC;
   inout [7:0] PD;
   integer 	handle;

   defparam 	core.progfile=progfile;
   defparam 	core.name="atmega8";
   AVRCORE core(clk);

   avr_pin #("B0") pb0(PB[0]);
   avr_pin #("B1") pb1(PB[1]);
   avr_pin #("B2") pb2(PB[2]);
   avr_pin #("B3") pb3(PB[3]);
   avr_pin #("B4") pb4(PB[4]);
   avr_pin #("B5") pb5(PB[5]);
   avr_pin #("B6") pb6(PB[6]);
   avr_pin #("B7") pb7(PB[7]);

   avr_pin #("C0") pc0(PC[0]);
   avr_pin #("C1") pc1(PC[1]);
   avr_pin #("C2") pc2(PC[2]);
   avr_pin #("C3") pc3(PC[3]);
   avr_pin #("C4") pc4(PC[4]);
   avr_pin #("C5") pc5(PC[5]);
   avr_pin #("C6") pc6(PC[6]);

   avr_pin #("D0") pd0(PD[0]);
   avr_pin #("D1") pd1(PD[1]);
   avr_pin #("D2") pd2(PD[2]);
   avr_pin #("D3") pd3(PD[3]);
   avr_pin #("D4") pd4(PD[4]);
   avr_pin #("D5") pd5(PD[5]);
   avr_pin #("D6") pd6(PD[6]);
   avr_pin #("D7") pd7(PD[7]);

endmodule
	       
