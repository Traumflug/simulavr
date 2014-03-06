#! /usr/bin/env python
# -*- coding: UTF-8 -*-
###############################################################################
#
# simulavr - A simulator for the Atmel AVR family of microcontrollers.
# Copyright (C) 2009 Thomas Klepp
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
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################
#
# $Id: $
#
from optparse import OptionParser
from ConfigParser import ConfigParser
from os.path import exists
from sys import stderr
from re import compile

rx_time = compile(r"^(?P<val>\d+)\s*(?P<unit>s|ms|us|ns)?$")

def readArgs():
  p = OptionParser()
  p.add_option("-o", action = "store", dest = "outputfile", default = None,
                     help = "output makefile path")
  p.add_option("-c", action = "store", dest = "configfile", default = None,
                     help = "config path")
  p.add_option("-t", action = "store", dest = "templatefile", default = None,
                     help = "template path")
  opts, args = p.parse_args()
  if not len(args) == 0: p.error("no arguments expected")
  
  if opts.outputfile is None: p.error("output file expected, use -h for help")
  
  if opts.configfile is None: p.error("config file expected, use -h for help")
  if not exists(opts.configfile): p.error("config file not found")
  opts.config = ConfigParser()
  try:
    opts.config.read(opts.configfile)
  except Exception, e:
    p.error("config file not readable: %s" % str(e))
  if not opts.config.has_section("_rule_") or not opts.config.has_option("_rule_", "rule"):
    p.error("config file wrong: no section '_rule_' or no option 'rule' in it")
    
  if opts.templatefile is None: p.error("template file expected, use -h for help")
  if not exists(opts.templatefile): p.error("template file not found")
  try:
    opts.template = open(opts.templatefile, "r").read()
  except Exception, e:
    p.error("template file not readable: %s" % str(e))
    
  return opts
  
def time2ns(timestr):
  m = rx_time.match(timestr)
  n = int(m.group("val"))
  u = m.group("unit")
  if u == None or u == "ns":
    f = 1
  elif u == "us":
    f = 1000
  elif u == "ms":
    f = 1000000
  else:
    f = 1000000000
  return str(n * f)
  
def create_rules(config):
  def cfg_get_default(sec, opt, default):
    if config.has_option(sec, opt):
      return config.get(sec, opt)
    else:
      return default
  rules = list()
  targets = list()
  for name in config.sections():
    if name == "_rule_": continue
    try:
      data = dict(name = cfg_get_default(name, "name", name),
                  tab = "\t",
                  ccopts = cfg_get_default(name, "ccopts", ""),
                  sources = config.get(name, "sources"),
                  simtime = time2ns(config.get(name, "simtime")),
                  signals = cfg_get_default(name, "signals", name),
                  simopts = cfg_get_default(name, "simopts", ""),
                  shellopts = cfg_get_default(name, "shellopts", ""))
      processors = config.get(name, "processors").strip()
      if len(processors) > 0:
        for p in processors.split():
          d = dict(processor = p)
          d.update(data)
          rules.append(config.get("_rule_", "rule", False, d))
          targets.append(config.get(name, "target", False, d))
      else:
          targets.append(config.get(name, "target"))
    except Exception, e:
      print >> stderr, str(e)
  return dict(rules = "\n".join(rules), targets = " ".join(targets))
  
if __name__ == "__main__":
  
  opts = readArgs()
  
  rules = create_rules(opts.config)
  
  open(opts.outputfile, "w").write(opts.template % rules)
  
# EOF
