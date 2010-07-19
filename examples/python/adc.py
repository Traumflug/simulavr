# Python Script
from sys import argv

import pysimulavr
from ex_utils import SimulavrAdapter

class XPin(pysimulavr.Pin):
  
  def __init__(self, dev, name, state = None):
    pysimulavr.Pin.__init__(self)
    self.dev=dev
    self.name = name
    devpin = dev.GetPin(name)
    if state is not None: self.SetPin(state)
    # hold the connecting net here, it have not be destroyed, if we leave this method
    self.__net = pysimulavr.Net()
    self.__net.Add(self)
    self.__net.Add(devpin)

if __name__ == "__main__":

  proc, elffile = argv[1].split(":")
  
  sim = SimulavrAdapter()
  sim.dmanSingleDeviceApplication()
  dev = sim.loadDevice(proc, elffile)
  #dev.SetClockFreq(250) # clock frequency is 4MHz by default
  print "before simulation start:"
  print "  value 'adc_value'=%d (before init)" % sim.getWordByName(dev, "adc_value")
  
  a0 = XPin(dev, "A0", 'a')
  aref = XPin(dev, "AREF", 'a')
  
  INT_MAX = 2**31 - 1
  aref.SetAnalog(INT_MAX)
  # hwad.cpp: adSample= (int)((float)adSample/(float)adref*INT_MAX);
  
  sim.dmanStart()
  print "simulation start: (t=%dns)" % sim.getCurrentTime()
  a0.SetAnalog(123)
  
  print "run till main function ..."
  bpaddr = dev.Flash.GetAddressAtSymbol("main")
  dev.BP.AddBreakpoint(bpaddr)
  sim.doRun(30000)
  if not dev.PC == bpaddr:
      print "error: main function not arrived!"
  dev.BP.RemoveBreakpoint(bpaddr)
  print "simulation main entrance: (t=%dns)" % sim.getCurrentTime()
  print "  value 'adc_value'=%d (after init)" % sim.getWordByName(dev, "adc_value")
  
  sim.doRun(sim.getCurrentTime() + 120000)
  print "simulation break: (t=%dns)" % sim.getCurrentTime()  
  print "  value 'conversions'=%d" % sim.getWordByName(dev, "conversions")
  print "  value 'adc_value'=%d (simulation break)" % sim.getWordByName(dev, "adc_value")
  
  sim.doRun(sim.getCurrentTime() + 330000)
  print "simulation end: (t=%dns)" % sim.getCurrentTime()  
  print "  value 'conversions'=%d" % sim.getWordByName(dev, "conversions")
  print "  value 'adc_value'=%d (simulation end)" % sim.getWordByName(dev, "adc_value")
  
  sim.dmanStop()
  del dev
  
# EOF
