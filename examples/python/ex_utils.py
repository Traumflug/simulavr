# Python Script
import pyavrs

class SimulavrAdapter(object):
  
  DEFAULT_CLOCK_SETTING = 250 # 250ns or 4MHz
  
  def loadDevice(self, t, e):
    self.__sc = pyavrs.SystemClock.Instance()
    self.__sc.ResetClock()
    dev = pyavrs.AvrFactory.instance().makeDevice(t)
    dev.Load(e)
    dev.SetClockFreq(self.DEFAULT_CLOCK_SETTING)
    self.__sc.Add(dev)
    return dev
    
  def doRun(self, n):
    ct = self.__sc.GetCurrentTime
    while ct() < n:
      res = self.__sc.Step()
      if res is not 0: return res
    return 0
      
  def doStep(self, stepcount = 1):
    while stepcount > 0:
      res = self.__sc.Step()
      if res is not 0: return res
      stepcount -= 1
    return 0
    
  def getCurrentTime(self):
    return self.__sc.GetCurrentTime()
    
  def getAllRegisteredTraceValues(self):
    os = pyavrs.ostringstream()
    pyavrs.DumpManager.Instance().save(os)
    return filter(None, [i.strip() for i in os.str().split("\n")])

  def dmanSingleDeviceApplication(self):
    pyavrs.DumpManager.Instance().SetSingleDeviceApp()
    
  def dmanStart(self):
    pyavrs.DumpManager.Instance().start()
  
  def dmanStop(self):
    pyavrs.DumpManager.Instance().stopApplication()
  
  def setVCDDump(self, vcdname, signals, rstrobe = False, wstrobe = False):
    dman = pyavrs.DumpManager.Instance()
    sigs = ["+ " + i for i in signals]
    dman.addDumpVCD(vcdname, "\n".join(sigs), "ns", rstrobe, wstrobe)
    
  def getWordByName(self, dev, label):
    addr = dev.data.GetAddressAtSymbol(label)
    v = dev.getRWMem(addr)
    addr += 1
    v = (dev.getRWMem(addr) << 8) + v
    return v
    
# EOF
