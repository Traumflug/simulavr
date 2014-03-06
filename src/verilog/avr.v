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

/* SimulavrXX glue code on the verilog side. */

/* FIXME: Some parts are still unfinished! 
 FIXME: Output-pullups are not implemented yet - find a good way to do that!
 */

module avr_pin(conn);
   parameter name="UNSPECIFIED";
   inout conn;
   wire  out;
   
   integer val;
   
   wire    output_active;
   assign  output_active = (val<=2);
   
   assign  conn = output_active ? out : 1'bz;

   function a2v;
      input apin;
      if (apin==0) // low
	a2v=0;
      else if (apin==1) // high
	a2v=1;
      else if (apin==2) // shorted
	a2v=1'bx;
      else if (apin==3) // pull-up
	a2v=1;
      else if (apin==4) // tristate
	a2v=1'bz;
      else if (apin==5) // pull-down
	a2v=0;
      else if (apin==6) // analog
	a2v=1'bx;
      else if (apin==7) // analog, shorted
	a2v=1'bx;
   endfunction // a2v

   function v2a;
      input vpin;
      if (vpin==1'bz)
	v2a=4; // tristate
      else if (vpin==1'bx)
	v2a=2; // approximate as shorted
      else if (vpin==1)
	v2a=1; // high
      else if (vpin==0)
	v2a=0; // low
   endfunction // v2a

   assign out=a2v(val);

   always @(posedge core.clk) begin
      val<=$avr_get_pin(core.handle, name);
      $avr_set_pin(core.handle, name, v2a(conn));
   end
   
endmodule // avr_pin

module avr_clock(clk);
   output clk;
   reg 	  clk;
   parameter FREQ=4_000_000;
   initial begin
      clk<=0;
   end
   
   always @(clk) begin
      #(1_000_000_000/FREQ/2) clk<=~clk; //125000 -> 4MHz clock
   end   
endmodule // avr_clock
    
module AVRCORE(clk);
   parameter progfile="UNSPECIFIED";
   parameter name="UNSPECIFIED";
   input     clk;

   integer   handle;
   integer   PCw; // word-wise PC as it comes from simulavrxx
   wire [16:0] PCb;  // byte-wise PC as used in output from avr-objdump!
   assign  PCb=2*PCw;
   
   initial begin
      $display("Creating an AVR device.");
      handle=$avr_create(name, progfile);
      //$avr_reset(handle);
   end

   always @(posedge clk) begin
      $avr_set_time($time);
      $avr_tick(handle);      
      PCw=$avr_get_pc(handle);
   end
   
endmodule // AVRCORE

