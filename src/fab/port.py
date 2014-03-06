# Copyright (C) 2009 Onno Kortmann <onno@gmx.net>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
#  
from converter import *

class Port:
    def __init__(self, xml, letter):
        self.letter=letter # upper case port letter (PORTB -> 'B', etc.)
        pre="IO_MODULE/PORT%s/" % letter

        for s, us in [("port_", "PORT%s/"),
                      ("pin_", "PIN%s/"),
                      ("ddr_", "DDR%s/")]:
            for e in ["mask", "wmask", "imask"]:
                self.__dict__[s+e]=0x00

            for i in range(8):
                if navdir(xml, (pre+us+"BIT%d") % (letter, i)):
                    self.__dict__[s+"mask"]|=1<<i
                    assert(navtxt(xml,    (pre+us+"BIT%d/ACCESS") % (letter, i)) in ["R", "RW"])
                
                    if "W" in navtxt(xml,    (pre+us+"BIT%d/ACCESS") % (letter, i)):
                        self.__dict__[s+"wmask"]|=1<<i
                    if navtxt(xml,   (pre+us+"BIT%d/INIT_VAL") % (letter, i)):
                        self.__dict__[s+"imask"]|=1<<i
        
    
class Ports(Converter):
    def doit(self, xml, tmpl):
        tmpl.io_ports={}
        for l in "ABCDEF":
            if navdir(xml, "IO_MODULE/PORT%s" % l):
                tmpl.io_ports[l]=self.__dict__["port"+l.lower()]=Port(xml, l)

