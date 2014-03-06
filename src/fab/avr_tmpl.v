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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
 *  
 * THIS FILE HAS BEEN AUTOMATICALLY GENERATED FROM avr_tmpl.v.
 *                     --- DO NOT EDIT MANUALLY! ---
 */

module $(part)(clk,
#set exclast=sorted($io_ports)[:-1]
#set onlylast=sorted($io_ports)[-1:][0]
#for $Letter in $exclast
	     P$Letter,
#end for
	     P$onlylast
#slurp	     
	     );
   parameter progfile="UNSPECIFIED";
   input     clk;
#for $Letter in $io_ports
##FIXME: Only specify needed bits!  
   inout [7:0] P$Letter;
#end for   
   integer 	handle;

   defparam 	core.progfile=progfile;
   defparam 	core.name="$part";
   AVRCORE core(clk);

#for $Letter in $io_ports
#set $letter=$Letter.lower
#for $bit in range(8)
   avr_pin #("$(Letter)$(bit)") p$(letter)$(bit)(P$(Letter)[$(bit)]);
#end for   
#end for   
endmodule
	       