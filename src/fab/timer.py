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

class Timer(Converter):
    def doit(self, xml, tmpl):
        # there does not seem to be a timerless device
        tmpl.has_prescaler=True
        try:
            tmpl.has_timer1=navtxt(xml, "IO_MODULE/TIMER_COUNTER_1/ID")[:3]=="t16"
            if tmpl.has_timer1:
                tmpl.timer1_timsk=self.bit_meanings(xml, "IO_MODULE/TIMER_COUNTER_1/TIMSK")
                tmpl.timer1_tifr=self.bit_meanings(xml, "IO_MODULE/TIMER_COUNTER_1/TIFR")
        except:
            tmpl.has_timer1=False
            
    def bit_meanings(self, xml, path):
        """ Gives back a dictionary which mappes bit names to their values when anding/oring (already 1<< shifted!). """
        res={}
        for i in range(8):
            try:
                name=navtxt(xml, path+("/BIT%d/NAME" % i))
                if len(name):
                    res[name]=1<<i
            except:
                pass
        return res
