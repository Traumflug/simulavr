/*
** Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
**  
*/
/* SPI slave Verilog example code. */
`timescale 1ns / 1ns

module test;
   reg CLK;
   wire nCS, SCK, MOSI;
   wire MISO;

   wire [7:0] pb;
   wire [7:0] pc;
   wire [7:0] pd;

   assign    MISO=pb[4];
   assign    MOSI=pb[3];
   assign    SCK=pb[5];
   assign    nCS=pb[2];
   
   defparam  avr.progfile="spi-waveforms.elf";
   ATmega8 avr(CLK, pb, pc, pd);
   
   initial begin
      $avr_trace("spi-waveforms.trace");
      $dumpfile("spi-waveforms.vcd");
      $dumpvars(0, test);
      # 100000 $finish;
   end // initial begin
   
   always begin
      #100 CLK<=0; 
      #100 CLK<=1;
   end   
endmodule
   