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

class IRQVector:
    def __init__(self, xml, n):
        path="INTERRUPT_VECTOR/VECTOR%d/" % (n+1)
        self.addr=navnum(xml, path+"PROGRAM_ADDRESS")
        self.src=navtxt(xml, path+"SOURCE") # source of interrupt

        # the EEPROM ready interrupt seems to be named different for each device...
        if (self.src=="EEPROM Ready" or
            self.src=="EE READY" or
            self.src=="EE_READY"):
            self.src="EE_RDY"
            
        self.desc=navtxt(xml,  path+"DEFINITION") # description

class IRQTable(Converter):
    def doit(self, xml, tmpl):
        tmpl.irq_num=navnum(xml, "INTERRUPT_VECTOR/NMB_VECTORS")
        tmpl.irq_vec_size=2 # FIXME!!
        tmpl.irqs=[]
        for i in range(tmpl.irq_num):
            # FIXME: for some reason, irq_num can be different to the number of declared vectors.
            # example is the AT90USB162.
            if navtxt(xml, "INTERRUPT_VECTOR/VECTOR%d/PROGRAM_ADDRESS" % (i+1)):
                tmpl.irqs.append(IRQVector(xml, i))

        # map source to interrupts
        tmpl.irq_bysrc={}
        for i in tmpl.irqs:
            if i.src in tmpl.irq_bysrc:
                raise Exception, ("At least two interrupts with the same source %s." % i.src)
            tmpl.irq_bysrc[i.src]=i
        
