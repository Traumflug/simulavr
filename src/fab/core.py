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

class IOReg:
    def __init__(self, xe):
        if navtxt(xe, "MEM_ADDR", False)!="NA":
            self.addr=navnum(xe, "MEM_ADDR", False)
        else:
            # FIXME: 0x20 always right?
            self.addr=0x20+navnum(xe, "IO_ADDR", False)
            
        self.name=xe.localName
        if navtxt(xe, "IO_ADDR", False)!="NA":
            assert(self.addr==0x20+navnum(xe, "IO_ADDR", False))
            
    def __call__(self):
        return self.addr
    
class Core(Converter):
    def doit(self, xml, tmpl):
        """ Sets some core parameters. """


        regf="CORE/GP_REG_FILE/"
        (tmpl.reg_num,
         tmpl.reg_xl, tmpl.reg_xh,
         tmpl.reg_yl, tmpl.reg_yh,
         tmpl.reg_zl, tmpl.reg_zh,
         tmpl.reg_start)=[navnum(xml, s) for s in [
            regf+"NMB_REG",
            regf+"X_REG_LOW",
            regf+"X_REG_HIGH",
            regf+"Y_REG_LOW",
            regf+"Y_REG_HIGH",
            regf+"Z_REG_LOW",
            regf+"Z_REG_HIGH",
            regf+"START_ADDR"
            ]]

        tmpl.part=navtxt(xml, "ADMIN/PART_NAME")

        # in bytes
        (tmpl.flash_size,
         tmpl.eeprom_size,
         tmpl.iram_size) = (
            navnum(xml, "MEMORY/PROG_FLASH"),
            navnum(xml, "MEMORY/EEPROM"),
            navnum(xml, "MEMORY/INT_SRAM/SIZE"))

        if navtxt(xml, "MEMORY/EXT_SRAM/SIZE")!="NA":
            tmpl.eram_size=navnum(xml, "MEMORY/EXT_SRAM/SIZE")
        else:
            tmpl.eram_size=0
            
        if tmpl.iram_size:
            tmpl.iram_start=navnum(xml, "MEMORY/INT_SRAM/START_ADDR")
        if tmpl.eram_size:
            tmpl.eram_start=navnum(xml, "MEMORY/EXT_SRAM/START_ADDR")

        tmpl.io_start, tmpl.io_stop=(
            navnum(xml, "MEMORY/IO_MEMORY/IO_START_ADDR"),
            navnum(xml, "MEMORY/IO_MEMORY/IO_STOP_ADDR"))

        tmpl.io_size=1+tmpl.io_stop-tmpl.io_start

        # read ioregisters
        tmpl.io={}
        for io in navdir(xml, "MEMORY/IO_MEMORY"):
            if io.nodeType==io.ELEMENT_NODE and io.getElementsByTagName("MEM_ADDR"):
                if io.localName in tmpl.io:
                    raise Exception, ("I/O-Register '%s' defined twice." % io.localName)
                tmpl.io[io.localName]=IOReg(io)

        tmpl.stack={}
        if "SPH" in tmpl.io:
            # FIXME: This is surely not 100% right on all devices!!
            tmpl.stack["ceil"]=0x10000
        else:
            tmpl.stack["ceil"]=0x100
        
        self.check(xml, tmpl)
        
    def check(self, xml, tmpl):
        # Some simple checks
        assert(tmpl.reg_num==32)
        assert(tmpl.reg_start==0)

        assert(tmpl.io_start==0)
        assert(tmpl.io_stop==0x3f)

        # some things which should be constant
        for c, p in [
            (0x3f, "MEMORY/IO_MEMORY/SREG/IO_ADDR"),
            (0x01, "MEMORY/IO_MEMORY/SREG/C_MASK"),
            (0x02, "MEMORY/IO_MEMORY/SREG/Z_MASK"),
            (0x04, "MEMORY/IO_MEMORY/SREG/N_MASK"),
            (0x08, "MEMORY/IO_MEMORY/SREG/V_MASK"),
            (0x10, "MEMORY/IO_MEMORY/SREG/S_MASK"),
            (0x20, "MEMORY/IO_MEMORY/SREG/H_MASK"),
            (0x40, "MEMORY/IO_MEMORY/SREG/T_MASK"),
            (0x80, "MEMORY/IO_MEMORY/SREG/I_MASK")
            ]:
            assert(c==navnum(xml, p))
