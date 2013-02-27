from os.path import basename, splitext
from unittest import TestCase, TestLoader
import pysimulavr

class SimulavrAdapter(object):
  
  DEFAULT_CLOCK_SETTING = 250 # 250ns or 4MHz
  
  def loadDevice(self, t, e):
    pysimulavr.cvar.sysConHandler.SetUseExit(False)
    self.__sc = pysimulavr.SystemClock.Instance()
    self.__sc.ResetClock()
    dev = pysimulavr.AvrFactory.instance().makeDevice(t)
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
    os = pysimulavr.ostringstream()
    pysimulavr.DumpManager.Instance().save(os)
    return filter(None, [i.strip() for i in os.str().split("\n")])

  def dmanSingleDeviceApplication(self):
    pysimulavr.DumpManager.Instance().SetSingleDeviceApp()
    
  def dmanStart(self):
    pysimulavr.DumpManager.Instance().start()
  
  def dmanStop(self):
    pysimulavr.DumpManager.Instance().stopApplication()
  
  def setVCDDump(self, vcdname, signals, rstrobe = False, wstrobe = False):
    dman = pysimulavr.DumpManager.Instance()
    sigs = ["+ " + i for i in signals]
    dman.addDumpVCD(vcdname, "\n".join(sigs), "ns", rstrobe, wstrobe)
    
  def getWordByName(self, dev, label):
    addr = dev.data.GetAddressAtSymbol(label)
    v = dev.getRWMem(addr)
    addr += 1
    v = (dev.getRWMem(addr) << 8) + v
    return v
    
  def getLongByName(self, dev, label):
    addr = dev.data.GetAddressAtSymbol(label) + 3
    v = 0
    for i in range(4):
      v = (v << 8) + dev.getRWMem(addr)
      addr -= 1
    return v

  def getByteByName(self, dev, label):
    addr = dev.data.GetAddressAtSymbol(label)
    return dev.getRWMem(addr)
    
  def setByteByName(self, dev, label, value):
    addr = dev.data.GetAddressAtSymbol(label)
    dev.setRWMem(addr, value)
    
class SimTestLoader(TestLoader):
  
  def __init__(self, elffile):
    TestLoader.__init__(self)
    self.elffile = elffile
    
  def loadTestsFromTestCase(self, testCaseClass):
    testCaseNames = self.getTestCaseNames(testCaseClass)
    if not testCaseNames and hasattr(testCaseClass, "runTest"):
      testCaseNames = ["runTest"]
    return self.suiteClass([testCaseClass(m, self.elffile) for m in testCaseNames])

class PyTestLoader(TestLoader):
  
  def __init__(self, modname):
    TestLoader.__init__(self)
    self.modname = modname
    
  def loadTestsFromTestCase(self, testCaseClass):
    testCaseNames = self.getTestCaseNames(testCaseClass)
    if not testCaseNames and hasattr(testCaseClass, "runTest"):
      testCaseNames = ["runTest"]
    return self.suiteClass([testCaseClass(m, self.modname) for m in testCaseNames])

class SimTestCase(TestCase):
  
  def __init__(self, methodName, elfName):
    TestCase.__init__(self, methodName)
    self.elfName = elfName
    self.processorName = splitext(basename(elfName))[0].split("_")[-1]
    
  def shortDescription(self):
    md = TestCase.shortDescription(self)
    if md is None: md = self._testMethodName
    d = self.elfName.split(".")[0]
    return d + "::" + md
    
  def getDevice(self):
    self.sim = SimulavrAdapter()
    self.dev = self.sim.loadDevice(self.processorName,  self.elfName)
    
  def setUp(self):
    self.getDevice()
    self.sim.dmanStart()

  def tearDown(self):
    self.sim.dmanStop()
    del self.dev
    del self.sim
    
  def assertDevice(self):
    self.assertTrue(hasattr(self, "sim") and self.sim is not None and hasattr(self, "dev") and self.dev is not None, "Simulavr device not created")

  def assertStartTime(self):
    self.assertEqual(self.sim.getCurrentTime(), 0)
    
  def assertInitDone(self, limit = 20000, mainSymbol = "main"):
    bpaddr = self.dev.Flash.GetAddressAtSymbol(mainSymbol)
    self.dev.BP.AddBreakpoint(bpaddr)
    self.sim.doRun(limit)
    self.assertTrue(self.sim.getCurrentTime() < limit, "break point '%s' not arrived" % mainSymbol)
    self.dev.BP.RemoveBreakpoint(bpaddr)
    
  def assertWordValue(self, label, value):
    self.assertEqual(self.sim.getWordByName(self.dev, label), value)
    
class PyTestCase(TestCase):
  
  def __init__(self, methodName, modulName):
    TestCase.__init__(self, methodName)
    self.modulName = modulName
    
  def shortDescription(self):
    md = TestCase.shortDescription(self)
    if md is None: md = self._testMethodName
    return self.modulName + "::" + md
    
# EOF
