#!/usr/bin/python
from optparse import OptionParser
import sys
import os
import re

HELP = """Parses the avrdude.conf file for all available parts and extracts there the device
signature to build a static map in output file. Output MUST exist and MUST be the .cpp file,
which contains this map! In this file are some markers to replace only the map content.
"""

def readArgs():
  p = OptionParser(description = HELP)
  p.add_option("-i", "--input", dest = "inputName",
                        help = "input file, have to be a avrdude.conf or avrdude.conf.in")
  p.add_option("-o", "--output", dest = "outputName",
                        help = "output file, MUST be the .cpp file, where signature map is located")
  opts, args = p.parse_args()
  if len(args) > 0 : p.error("to much arguments, none expected")
  if opts.inputName is None or not os.path.exists(opts.inputName):
    p.error("input file name not given or dosn't exist")
  if opts.outputName is None or not os.path.exists(opts.outputName):
    p.error("output file name not given or dosn't exist")
  return opts

def readInput(inputName):
  rx = re.compile("""(part)|(desc|signature)\s+=\s+("(.+)"|0[0-9a-fx\s]+);""")
  result = dict()
  def addToMap(desc, signature):
    if desc == "?" or signature == -1: return
    result[desc] = signature
  desc = "?"
  signature = -1
  f = open(inputName, "r")
  for l in f.readlines():
    mx = rx.match(l.strip())
    if mx:
      data = mx.groups()
      if data[0] == "part":
        desc = "?"
        signature = -1
      elif data[1] == "desc":
        desc = data[3]
        addToMap(desc, signature)
      elif data[1] == "signature":
        signature = int("".join(data[2].split(" 0x")), 16)
        addToMap(desc, signature)
  f.close()
  return result

def map2template(filehandle, sigmap, mode, template):
  for k, v in sigmap.items():
    filehandle.write(template % {"signature": "0x%x" % v, "name": k.lower()})

def writeOutput(outputName, sigmap):
  rx_start = re.compile("""^//\s+MARK\s+start\s+.*""")
  rx_end = re.compile("""^//\s+(MARK|MODE|TEMPLATE)(.*)""", re.DOTALL)
  # read file into RAM
  f = open(outputName, "r")
  txt = f.readlines()
  f.close()
  # open file for writing
  f = open(outputName, "w")
  state = 0
  mode = "?"
  template = "?"
  for line in txt:
    if state == 0:
      if not rx_start.match(line):
        f.write(line)
      else:
        f.write(line)
        state = 1
    elif state == 1:
      mx = rx_end.match(line)
      if mx:
        if mx.group(1) == "MARK":
          map2template(f, sigmap, mode, template)
          f.write(line)
          state = 0
        else:
          f.write(line)
          if mx.group(1) == "MODE":
            mode = mx.group(2).strip()
          elif mx.group(1) == "TEMPLATE":
            template = mx.group(2)
  f.close()
  
if __name__ == "__main__":

  opts = readArgs()
  sigmap = readInput(opts.inputName)
  #for k, v in sigmap.items(): print "%s: 0x%x" % (k, v)
  writeOutput(opts.outputName, sigmap)
  sys.exit(0)

# EOF
