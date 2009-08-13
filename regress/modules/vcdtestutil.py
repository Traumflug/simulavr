from os.path import basename, splitext
from unittest import TestCase, TestLoader
from sys import stderr

from vcdreader import VCD, VCDError

# time factors, time base is 1ns!
uSec = 1000
mSec = 1000000
Sec =  1000000000

__vcds = dict()

def getVCD(name):
  global __vcds
  if not name in __vcds: __vcds[name] = loadVCDFromName(name)
  return __vcds[name]

def loadVCDFromName(name):
  print >> stderr, "load %s ... " % name,
  try:
    return VCD(name)
  except VCDError:
    return None
  
class VCDTestLoader(TestLoader):
  
  def __init__(self, vcdfile):
    TestLoader.__init__(self)
    self.vcdfile = vcdfile
    
  def loadTestsFromTestCase(self, testCaseClass):
    testCaseNames = self.getTestCaseNames(testCaseClass)
    if not testCaseNames and hasattr(testCaseClass, "runTest"):
      testCaseNames = ["runTest"]
    return self.suiteClass([testCaseClass(m, self.vcdfile) for m in testCaseNames])

class VCDTestCase(TestCase):
  
  def __init__(self, methodName, vcdName):
    TestCase.__init__(self, methodName)
    self.vcdName = vcdName
    
  def getVCD(self):
    self.vcd = getVCD(self.vcdName)
    
  def getProcessorType(self):
    """This parse self.vcdname and give back the processor part of vcd filename
    
    The vcd filename format is: "<testname>_<processor>.vcd" where "testname"
    could have also "_"!
    """
    return splitext(basename(self.vcdName))[0].split("_")[-1]
    
  def getVariable(self, name): return self.vcd.getVariable(name)
  
  def shortDescription(self):
    md = TestCase.shortDescription(self)
    if md is None: md = self._testMethodName
    d = self.vcdName.split(".")[0]
    return d + "::" + md
    
  def setClock(self, freq):
    self.fClock = freq
    self.tClock = int(1000000000 / freq)
      
  def assertVCD(self):
    self.assertTrue(hasattr(self, "vcd") and self.vcd is not None, "vcd file not loaded")

# EOF
