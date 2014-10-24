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
// IMPORTANT NOTE: This file is only to illustrate the simulavrxx<->verilog
// interface and is by no means any reference for anything whatsoever!  It
// probably contains lots of bugs! As already stated above, there is no
// warranty!
//-----------------------------------------------------------------------------
/* Simple test with two units communicating with each other.
 One unit is named 'left', the other unit is named right.
 The left unit controls the right unit. The left unit runs at
 12MHz the right one only at 1.6MHz.
 */

`timescale 1ns / 1ns

module test;
   wire lclk, rclk;
   wire [7:0] leftpba, leftpb, leftpd;
   wire [7:0] rightpb;

   defparam   lavr.progfile="left-unit.elf";
   ATtiny2313 lavr(lclk, leftpba, leftpb, leftpd);
   
   defparam   ravr.progfile="right-unit.elf";
   ATtiny25   ravr(rclk, rightpb);

   defparam  lclock.FREQ=12_000_000;
   defparam  rclock.FREQ=1_600_000;
   avr_clock lclock(lclk), rclock(rclk);


   /* The following is the complete wiring between the two AVRs. Connects
    the output-compare pin as well as the input-capture pin of the tiny2313
    to PB2 on the right tiny15l, and then puts a pullup on that connection. */
   wire      lrconn;
   assign     lrconn=leftpb[3];
   assign     lrconn=rightpb[2];
   assign     lrconn=leftpd[6];
     
   // put pull-up on single wire comm link
   // note that this is slightly inaccurate as the pullups are formed
   // from the switched AVR ones!
   assign     (strong0, weak1) lrconn=1; 

   initial begin   
      $dumpfile("spc.vcd");
      $dumpvars(0, test);

      // enable this for too much output :)
      //$avr_trace("spc.trace");

      #40_000_000 ;
      $finish;
   end


   reg[7:0] l_trxbyte;
   reg[7:0] r_trxbyte;

   reg[15:0] l_lowcnt;
   reg[15:0] r_lowcnt;

   reg[15:0] l_highcnt;
   reg[15:0] r_highcnt;

   reg [7:0] simsend;
   reg [7:0] simsend_clk;
   reg [7:0] tcntl;
   reg [7:0] tcnth;

   always @(lrconn) begin
      $display("LRCONN %d", lrconn);
   end
   
   always @(negedge lclk) begin
      /* This connects directly to RAM location 0x70 in the left AVR to read
       some useful information from that unit. */
      simsend=$avr_get_rw(lavr.core.handle, 'h0070);
      // WARNING: Reading these registers would count as a 16bit read from the AVR!!!
      //tcntl=$avr_get_rw(lavr.core.handle, 76);
      //tcnth=$avr_get_rw(lavr.core.handle, 77);
   end
   always @(negedge rclk) begin
      r_trxbyte=$avr_get_rw(ravr.core.handle, 18);
      r_lowcnt=$avr_get_rw(ravr.core.handle, 0)+256*$avr_get_rw(ravr.core.handle, 20);
      r_highcnt=$avr_get_rw(ravr.core.handle, 1)+256*$avr_get_rw(ravr.core.handle, 21);
   end
endmodule // test
