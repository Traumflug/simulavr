# Python Script
from sys import argv
from os.path import splitext, basename

import pysimulavr
from ex_utils import SimulavrAdapter

class XPin(pysimulavr.Pin):
  
  def __init__(self, name):
    pysimulavr.Pin.__init__(self)
    self.name = name
    
  def SetInState(self, pin):
    pysimulavr.Pin.SetInState(self, pin)
    print "%s set to '%s' (t=%dns)" % (self.name, pin.toChar(), sim.getCurrentTime())

if __name__ == "__main__":

  doVCD = False
  
  proc, elffile = argv[1].split(":")
  
  sim = SimulavrAdapter()
  sim.dmanSingleDeviceApplication()
  dev = sim.loadDevice(proc, elffile)
  #dev.SetClockFreq(100)
  
  if doVCD:
    print "all registrered trace values:\n ",
    print "\n  ".join(sim.getAllRegisteredTraceValues())
    sigs = ("IRQ.VECTOR9", "PORTA.PORT")
    sim.setVCDDump(splitext(basename(argv[0]))[0] + ".vcd", sigs)
    print "-" * 20
    
  xpin = XPin("port A.0")
  # watch out, that this Net instance will not be deleted until simulation is
  # done (for example, if you create this in a subfunction and do not save
  # this instance too, before you leave this subfunction)
  net = pysimulavr.Net()
  net.Add(xpin)
  net.Add(dev.GetPin("A0"))
    
  sim.dmanStart()
  
  print "simulation start: (t=%dns)" % sim.getCurrentTime()
  sim.doRun(15000000)
  print "simulation end: (t=%dns)" % sim.getCurrentTime()

  print "value 'timer2_ticks'=%d" % sim.getWordByName(dev, "timer2_ticks")
  
  sim.dmanStop()
  del dev
  
# EOF
