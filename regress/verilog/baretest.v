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

/*
 Bare simulavrxx AVR<->verilog interface 'testbench' without using any of
 the gluecode in avr.v
 */
`timescale 1ns / 1ns

module test;

   integer hd;
   reg 	   clk;


   initial begin
      $dumpfile("baretest.vcd");
      $dumpvars(0, test);
      hd=8'h01;
      #1 hd=8'h02;
      #1 hd=8'h03;
      hd=$avr_create("attiny2313", "toggle.elf");
      $avr_reset(hd);
      #100_000 $avr_destroy(hd);
      $finish;
   end

   integer val;
   // Pin state LOW is zero
   wire	   pb0=val!=1'b0;


   always @(posedge clk) begin
      #10 $avr_tick(hd);
      #10 val=$avr_get_pin(hd, "B0");
   end

   always begin
      #125 clk<=0; //125000 -> 4MHz clock
      #125 clk<=1;
   end
endmodule // test

