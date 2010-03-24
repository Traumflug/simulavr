# Python Script
from sys import argv
from os.path import splitext, basename

import pysimulavr
from ex_utils import SimulavrAdapter

class XPin(pysimulavr.Pin):
  
  def __init__(self, dev, name, state = None):
    pysimulavr.Pin.__init__(self)
    self.name = name
    if state is not None: self.SetPin(state)
    # hold the connecting net here, it have not be destroyed, if we leave this method
    self.__net = pysimulavr.Net()
    self.__net.Add(self)
    self.__net.Add(dev.GetPin(name))
    
  def SetInState(self, pin):
    pysimulavr.Pin.SetInState(self, pin)
    print "%s='%s' (t=%dns)" % (self.name, pin.toChar(), sim.getCurrentTime())

if __name__ == "__main__":

  proc, elffile = argv[1].split(":")
  
  sim = SimulavrAdapter()
  sim.dmanSingleDeviceApplication()
  dev = sim.loadDevice(proc, elffile)
  
  a0 = XPin(dev, "A0")
  a1 = XPin(dev, "A1", "H")
  a7 = XPin(dev, "A7", "H")
  
  sim.dmanStart()
  print "simulation start: (t=%dns)" % sim.getCurrentTime()
  sim.doRun(sim.getCurrentTime() + 7000000)
  a1.SetPin("L")
  sim.doRun(sim.getCurrentTime() + 5000000)
  a7.SetPin("L")
  sim.doRun(sim.getCurrentTime() + 2000000)
  a1.SetPin("H")
  sim.doRun(sim.getCurrentTime() + 1000000)
  print "simulation end: (t=%dns)" % sim.getCurrentTime()
  
  print "value 'timer2_ticks'=%d" % sim.getWordByName(dev, "timer2_ticks")
  print "value 'port_val'=0x%x" % sim.getWordByName(dev, "port_val")
  print "value 'port_cnt'=%d" % sim.getWordByName(dev, "port_cnt")
  
  sim.dmanStop()
  del dev
  
# EOF
