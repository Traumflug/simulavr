# Example code for multicore example with python interface

# import python interface
import pysimulavr

if __name__ == "__main__":

  # get systemclock instance
  print "multicore example:"
  sc = pysimulavr.SystemClock.Instance()
  
  # create core A: clock generator 250Hz with device clock 4MHz
  print "  create core A ..."
  devA = pysimulavr.AvrFactory.instance().makeDevice("atmega16")
  devA.Load("multicore_a.elf")
  devA.SetClockFreq(250) # clock period in ns!
  sc.Add(devA)
  
  # create core B: count rising edges, device clock 10MHz, calculated cnt_res = 156,25 nominal!
  print "  create core B ..."
  devB = pysimulavr.AvrFactory.instance().makeDevice("atmega16")
  devB.Load("multicore_b.elf")
  devB.SetClockFreq(100) # clock period in ns!
  sc.Add(devB)

  # create net: connect core A, Port B3 to core B, Port D2
  print "  connect core A with core B ..."
  n = pysimulavr.Net()
  n.Add(devA.GetPin("B3"))
  n.Add(devB.GetPin("D2"))

  # get addresses for cnt_res and cnt_irq variables
  a = devB.data.GetAddressAtSymbol("cnt_irq")
  print "  core B: address(cnt_irq)=0x%x" % a
  b = devB.data.GetAddressAtSymbol("cnt_res")
  print "  core B: address(cnt_res)=0x%x" % a
  
  # run simulation, stop after given time and check values
  print "  run simulation ..."
  sc.RunTimeRange(4000000)
  print "  t= 4ms, cnt_irq=%d, cnt_res=%3d" % (devB.getRWMem(a), devB.getRWMem(b))
  sc.RunTimeRange(4000000)
  print "  t= 8ms, cnt_irq=%d, cnt_res=%3d" % (devB.getRWMem(a), devB.getRWMem(b))
  sc.RunTimeRange(12000000)
  print "  t=20ms, cnt_irq=%d, cnt_res=%3d" % (devB.getRWMem(a), devB.getRWMem(b))
  sc.RunTimeRange(12000000)
  print "  t=32ms, cnt_irq=%d, cnt_res=%3d" % (devB.getRWMem(a), devB.getRWMem(b))
  
# EOF
