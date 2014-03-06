#!/usr/bin/python
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
from xml.dom.minidom import parse

from Cheetah.Template import Template

from core import Core
from port import Ports
from irq import IRQTable
from timer import Timer
from pins import Pins
from spi import SPI
from usart import USART

converters=[
    Core(),
    IRQTable(),
    Ports(),
    Timer(),
    Pins(),
    SPI(),
    USART()
    ]


from os import listdir

# for fn in listdir("atmel-xml/"):
#     if (fn.find(".xml")<0
#         or fn.find("ATxmega")>=0
#         or fn.find("AT89")>=0
#         or fn.find("AT86")>=0
#         or fn.find("ATtiny10")>=0
#         or fn.find("ATtiny28")>=0 # <- has weird port logic. FIXME: support this!
#         or fn.find("HV")>=0 # 'smart battery devices' also have some weird port logic.
#         or fn.find("CAN")>=0 # CAN devices are not implemented either
#         or fn.find("USB")>=0 # USB devices are weird in general.
#         or fn.find("M1")>=0 or fn.find("C1")>=0 # automotive stuff
#         ):
#         continue
    

for fn in ["ATtiny15.xml",
           "ATtiny2313.xml",
           "ATmega8.xml"
           ]:
    print "Parsing %s" % fn
    p=parse("atmel-xml/%s" % fn)

    CPPtmpl=Template(file="avr_tmpl.cpp")
    Htmpl=Template(file="avr_tmpl.h")
    Vtmpl=Template(file="avr_tmpl.v")
    
    for c in converters:
        print "Extracting using %s." % c
        c(p, CPPtmpl)
        c(p, Htmpl)
        c(p, Vtmpl)
        
    cpp_f=open("../avr_"+fn[:-4]+".cpp", "w")
    h_f=open("../avr_"+fn[:-4]+".h", "w")
    v_f=open("../verilog/avr_"+fn[:-4]+".v", "w")
    
    print "Writing files for %s" % fn[:-4]
    print>>cpp_f, CPPtmpl
    print>>h_f, Htmpl
    print>>v_f, Vtmpl
    
    cpp_f.close()
    h_f.close()
    v_f.close()
    
    
