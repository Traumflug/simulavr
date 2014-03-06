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

module SPIfrontend(nCS, CLK, MOSI, MISO, 
		   inbyte, outbyte, out_valid);
   input nCS;
   input CLK;
   input MOSI;
   output MISO;
   
   reg 	  MISO;
   input [7:0] inbyte;
   output [7:0] outbyte;
   reg [7:0] 	outbyte;
   output 	out_valid;
   reg [2:0] 	bitcnt;

   assign 	out_valid = bitcnt==0;

   always @(posedge nCS) begin // 'reset' condition
      bitcnt<=0;
      outbyte<=0;
   end
   
   always @(posedge CLK) if (!nCS) begin
      MISO<=inbyte[bitcnt];
   end // always @ (posedge CLK)
   
   always @(negedge CLK) if (!nCS) begin
      if (bitcnt)
	outbyte[bitcnt]<=MOSI;
      else
	outbyte<=MOSI;
      bitcnt<=bitcnt+1;
   end
endmodule // SPIfrontend

module SPIslave(nCS, CLK, MOSI, MISO);
   input nCS;
   input CLK;
   input MOSI;
   
   output MISO;

   reg [7:0] inbyte;
   wire [7:0] outbyte;
   wire       out_valid;
   reg [7:0]  mem[0:65535];
   reg [3:0]  state;
   reg [15:0] adr;
   
   SPIfrontend front(nCS, CLK, MOSI, MISO, 
		     inbyte, outbyte, out_valid);

   always @(posedge nCS) begin
      state<=0;
   end
   
   always @(posedge out_valid) if (!nCS) begin
      $display("Received %d", outbyte);
      case (state)
	0: begin
	   adr[7:0]<=outbyte; state<=1;
	end
	1: begin
	   adr[15:8]<=outbyte; state<=2;
	end
	2: begin
	   if (outbyte) begin
	      state<=3;
	   end else begin
	      $display("Reading location 0x%x, which contains 0x%x", adr, mem[adr]);
	      inbyte<=mem[adr];
	      state<=4;
	   end
	end
	3: begin
	   mem[adr]<=outbyte;
	   $display("Writing location 0x%x with 0x%x", adr, outbyte);
	   state<=0;
	end
	4: begin
	   state<=0; // dummy state, this could be interleaved for more performance when reading
	end
      endcase // case(state)
   end
endmodule


module test;
   reg CLK;
   wire nCS, SCK, MOSI;
   wire MISO;

   SPIslave sl(nCS, SCK, MOSI, MISO);

   wire [7:0] pb;
   wire [7:0] pc;
   wire [7:0] pd;

   assign    MISO=pb[4];
   assign    MOSI=pb[3];
   assign    SCK=pb[5];
   assign    nCS=pb[2];
   
   defparam  avr.progfile="spi.elf";
   ATmega8 avr(CLK, pb, pc, pd);
   
   initial begin
      $avr_trace("avr.trace");
      $dumpfile("spi.vcd");
      $dumpvars(0, test);
      # 1000000 $finish;
   end // initial begin
   
   always begin
      #100 CLK<=0; 
      #100 CLK<=1;
   end   
endmodule
   