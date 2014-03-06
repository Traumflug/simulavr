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

def sls(s):
    return s.replace("[", "").replace("]", "").split(":")
    
# FIXME: Support different package types!
class Pins(Converter):
    def doit(self, xml, tmpl):
        tmpl.pkg_types=sls(navtxt(xml, "PACKAGE/PACKAGES"))

        p=tmpl.pkg_types[0]
        tmpl.pkg_pincnt=navnum(xml, "PACKAGE/%s/NMB_PIN" % p)

        tmpl.pkg_pins=[]
        tmpl.pkg_portlet={} # mapping of pkg_pins indices to port letters (lower case)
        tmpl.pkg_portbit={} # mapping of pkg_pins indices to port bits
        
        for i in range(1, tmpl.pkg_pincnt+1):
            s=sls(navtxt(xml, "PACKAGE/%s/PIN%d/NAME" % (p, i)))
            port=False
            portlet=None
            portbit=None
            for sl in s:
                if len(sl)==3 and sl[0]=="P":
                    if port:
                        raise Exception, "Weird PACKAGE/.../PIN structure."
                    portlet=sl[1].lower()
                    portbit=int(sl[2])
                    port=True

            for sl in s:
                tmpl.pkg_portlet[sl]=portlet
                tmpl.pkg_portbit[sl]=portbit

            tmpl.pkg_pins.append(s)

