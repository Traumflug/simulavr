## Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
##  
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
##  

#for $Letter in $io_ports
#set $letter=$Letter.lower    
#set $port=$io_ports[$Letter]
##    
#if ("PIN"+$Letter) in $io
    port$letter=new HWPort(this, "$Letter");
#else
    // output-only port
    port$letter=new HWPort(this, "$Letter", 0xff);
#end if
##    
#if ("PORT"+$Letter) in $io
    rw[$io["PORT"+$Letter].addr]=& port$letter->port_reg;
#end if
##    
## some ports are output only (e.g. mega103/portc)
## some other ports are input only (e.g. mega103/portf)
#if ("PIN"+$Letter) in $io
    rw[$io["PIN"+$Letter].addr]=& port$letter->pin_reg;
    #if ("PORT"+$Letter) in $io
    rw[$io["DDR"+$Letter].addr]=& port$letter->ddr_reg;
    #else
    // ^^ input-only port
    #end if
  #end if

#end for
