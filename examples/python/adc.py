# Python Script
from sys import argv
from os.path import splitext, basename

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
  print "value 'adc_value'=%d" % sim.getWordByName(dev, "adc_value")
  
  a0 = XPin(dev, "A0", 'a')
  A0 = dev.GetPin("A0")
  aref = XPin(dev, "AREF", 'a')
  
  INT_MAX = 2**31 - 1
  aref.SetAnalog(INT_MAX)
  # hwad.cpp: adSample= (int)((float)adSample/(float)adref*INT_MAX);
  
  sim.dmanStart()
  print "simulation start: (t=%dns)" % sim.getCurrentTime()
  a0.SetAnalog(123)
  #a0.SetAnalog(int(0.1 * 2**31))
  sim.doRun(sim.getCurrentTime() + 500000)
  print "simulation end: (t=%dns)" % sim.getCurrentTime()  
  print "conversions: %d" % sim.getWordByName(dev, "conversions")
  
  print "adc_value=%d" % sim.getWordByName(dev, "adc_value")
  
  sim.dmanStop()
  del dev
  
# EOF
