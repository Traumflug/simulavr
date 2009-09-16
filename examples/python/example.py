# -*- coding: UTF-8 -*-
# Python test script as demonstration of using pysimulavr in unit tests
from unittest import TestSuite, TextTestRunner, TestCase, defaultTestLoader
from sys import argv

import pysimulavr
from ex_utils import SimulavrAdapter

class TestBaseClass(TestCase, SimulavrAdapter):
  
  def setUp(self):
    proc, elffile = argv[1].split(":")
    self.device = self.loadDevice(proc, elffile)
    
  def tearDown(self):
    del self.device
    
  def test_01(self):
    "just run 3000 ns + 250 ns"
    n = 3000
    self.doRun(n)
    self.assertEqual(self.getCurrentTime(), n)
    self.doStep()
    self.assertEqual(self.getCurrentTime(), n + self.device.GetClockFreq())
    
  def test_02(self):
    "just run 2 steps"
    self.doStep()
    self.assertEqual(self.getCurrentTime(), 0)
    self.doStep()
    self.assertEqual(self.getCurrentTime(), self.device.GetClockFreq())
    
  def test_03(self):
    "check PC and PC size"
    self.assertEqual(self.device.PC_size, 2)
    self.doStep()
    self.doStep()
    self.assertEqual(self.device.PC, 0x8c / 2)
    
  def test_04(self):
    "check flash and data symbols"
    self.assertEqual(self.device.Flash.GetAddressAtSymbol("main"), 0xfc / 2)
    self.assertEqual(self.device.data.GetAddressAtSymbol("timer2_ticks"), 0x100)
    
  def addr2word(self, addr):
    d1 = self.device.getRWMem(addr + 1)
    d2 = self.device.getRWMem(addr)
    return d2 + (d1 << 8)
    
  def test_05(self):
    "access to data by symbol"
    addr = self.device.data.GetAddressAtSymbol("timer2_ticks")
    o = 10000   # duration of interrupt function, about 10us
    d = 2000000 # timer period 2ms
    self.doRun(o * 2) # skip initialisation
    self.assertEqual(self.addr2word(addr), 0)
    self.doRun(d + o)
    self.assertEqual(self.addr2word(addr), 1)
    self.doRun((d * 3) + o)
    self.assertEqual(self.addr2word(addr), 3)
    
  def test_06(self):
    "write access to data by symbol"
    addr = self.device.data.GetAddressAtSymbol("timer2_ticks")
    o = 10000   # duration of interrupt function, about 10us
    d = 2000000 # timer period 2ms
    self.doRun(o * 2) # skip initialisation
    self.assertEqual(self.addr2word(addr), 0)
    self.device.setRWMem(addr, 2)
    self.doRun(d)
    self.assertEqual(self.addr2word(addr), 2)
    self.doRun(d + o)
    self.assertEqual(self.addr2word(addr), 3)
    
  def test_07(self):
    "test toggle output pin"
    o = 10000   # duration of interrupt function, about 10us
    d = 2000000 # timer period 2ms
    self.assertEqual(self.device.GetPin("A0").toChar(), "t")
    self.doRun(o * 2) # skip initialisation
    # now output should be set to LOW
    self.assertEqual(self.device.GetPin("A0").toChar(), "L")
    self.doRun(d + o * 2) # reaction to timer interrupt about 20us after!
    self.assertEqual(self.device.GetPin("A0").toChar(), "H")
    self.doRun(d * 2 + o * 2)
    self.assertEqual(self.device.GetPin("A0").toChar(), "L")
    
  def test_08(self):
    "work with breakpoints"
    bpaddr = self.device.Flash.GetAddressAtSymbol("main")
    self.device.BP.AddBreakpoint(bpaddr)
    self.doRun(10000)
    self.assertEqual(self.device.PC, 0xc2 / 2)
    self.doStep(4) # call to main
    self.assertEqual(self.device.PC, bpaddr)
    self.doStep(4) # 4 steps more, do nothing because of breakpoint
    self.assertEqual(self.device.PC, bpaddr)
    self.device.BP.RemoveBreakpoint(bpaddr)
    self.doStep(2) # push needs 2 steps
    self.assertEqual(self.device.PC, bpaddr + 1)
        
if __name__ == "__main__":
  
  allTestsFrom = defaultTestLoader.loadTestsFromTestCase
  suite = TestSuite()
  suite.addTests(allTestsFrom(TestBaseClass))
  TextTestRunner(verbosity = 2).run(suite)
  
# EOF
