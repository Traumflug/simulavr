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

/* Simple input-output combined test. */

`timescale 1ns / 1ns

module test;

   wire clk;
   wire [7:0] pa, pb, pd;

   defparam  avr.progfile="loop.elf";
   ATtiny2313 avr(clk, pa, pb, pd);
   avr_clock clock(clk);

   initial begin   
      $dumpfile("loop.vcd");
      $dumpvars(0, test);
      #100_000 $finish;
   end
endmodule