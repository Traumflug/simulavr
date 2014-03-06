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
class Converter:
    """ A converter converting parts of the XML description into settings for a template. """
    def __call__(self, xcfg, tmpl):
        """
        xcfg: Atmel configuration XML object (DOM tree)
        tmpl: output template
        """
        self.xcfg=xcfg
        self.tmpl=tmpl

        self.doit(xcfg, tmpl)

def nav(xml, path):
    """ Navigate in XML object xml. Path is a list of
    element names to navigate into. Yields a list of all matching elements.
    """
#    print "Path:", path
    if not len(path):
        return xml.childNodes
    else:
        res=[]
        for c in xml.childNodes:
#            print "Looking at", c
            if path[0]=="*" or c.localName==path[0]:
#                print "Navigating to ", c
                res.extend(nav(c, path[1:]))
#                print "done."
        return res
    
def navdir(xml, path, default=True):
    """ Same as nav, but for a /-separated path. Includes the initial AVRPART/ automatically iff def is true. """
    if default:
        p="AVRPART/"+path
    else:
        p=path
    return nav(xml, p.split("/"))

def navls(xml, path, default=True):
    """ Do a 'list directory' for /-separated path path. """
    for i in navdir(xml, path, default):
        if i.localName:
            print i.localName

def navcat(xml, path, default=True):
    """ Print texts of an XML element. """
    for i in navdir(xml, path, default):
        try:
            print i.wholeText
        except AttributeError:
            print "<no attribute wholeText>"
            
def navtxt(xml, path, default=True):
    """ Gives texts of XML element(s). """
    t=""
    try:
        for i in navdir(xml, path, default):
            t+=i.wholeText
    except AttributeError, e:
        raise AttributeError, ("XML-Path:"+str(path)+" : "+str(e))
    return t
    
def avrnum(n):
    """ Converts a integer number from an AVR XML file.
    $-numbers and 0x-numbers are treated as hex, ones starting with digits as dec. """
    if n[0]=="$":
        return int(n[1:], 16)
    else:
        return int(n, 0)

def navnum(xml, path, default=True):
    """ avrnum(navtxt(xml, path) """
    txt=navtxt(xml, path, default)
    try:
        return avrnum(txt)
    except IndexError:
        raise IndexError, ("String index out of range in XML tree at '%s'." % path)
    except ValueError:
        raise ValueError, ("Value '%s' can't be converted into an integer, (sub-)tree position '%s', element %s." % (txt, path, xml))

