# -*- coding: UTF-8 -*-
import sys
import os
from optparse import OptionParser
try:
  # for python3!
  from configparser import ConfigParser
except:
  from ConfigParser import ConfigParser

FILENAME = "setup.cfg"

def readArgs():
  p = OptionParser()
  p.add_option("-c", dest = "section", help = "section name in setup.cfg")
  p.add_option("-o", dest = "option", help = "option name for section in setup.cfg")
  p.add_option("-s", dest = "value", help = "value to set for section/option in setup.cfg")
  opts, args = p.parse_args()
  if len(args) > 0 : p.error("to much arguments, none expected")
  if opts.section is None or opts.option is None or opts.value is None:
    p.error("one of the arguments is missed, see help")
  return opts

def writeCfg(opts):
  cfg = ConfigParser()
  if os.path.exists(FILENAME): cfg.read(FILENAME)
  if not cfg.has_section(opts.section): cfg.add_section(opts.section)
  cfg.set(opts.section, opts.option, opts.value)
  try:
    fhdl = open(FILENAME, "w")
    cfg.write(fhdl)
    fhdl.close()
  except:
    sys.stderr.write("error: can't write to %s ..." % FILENAME)
    sys.stderr.write(os.linesep)
    sys.exit(1)

if __name__ == "__main__":

  writeCfg(readArgs())
  sys.exit(0)

# EOF
