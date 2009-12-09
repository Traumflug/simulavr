from simtestutil import SimTestLoader

import ext_int0

class TestCase(ext_int0.TestCaseBase):
  
  portmask = {
    "atmega48":   0x7f, # only 7 bit at port C
    "attiny2313": 0xff,
  }
  bit0name = {
    "atmega48":   "C0",
    "attiny2313": "B0",
  }
  bit3name = {
    "atmega48":   "C3",
    "attiny2313": "B3",
  }
  
  def setControl(self, ctrl):
    self.handshake(4, ctrl)
    
  def assertPortValue(self, val):
    val = val & self.portmask[self.processorName]
    v = self.handshake(5)
    self.assertEqual(v, val, "port input value wrong: got=%d, exp=%d" % (v, val))
    
  def test_00(self):
    """check interrupt on any change"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p = ext_int0.XPin(self.dev, self.sim, self.bit0name[self.processorName])
    # check port value
    self.assertPortValue(0xff)
    # ext pin = low
    p.SetPin("L")
    # mask only bit 0
    self.setControl(1)
    self.resetFlag()
    self.enableIRQ()
    # counter is 0?
    self.assertCounter(0, 10000)
    # ext pin = high
    p.SetPin("H")
    # counter is 1?
    self.assertCounter(1, 20000)
    # ext pin = low
    p.SetPin("L")
    # counter is 2?
    self.assertCounter(2, 20000)
    # ext pin = high
    p.SetPin("H")
    # counter is 3?
    self.assertCounter(3, 20000)
    
  def test_01(self):
    """check masking input bits"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p0 = ext_int0.XPin(self.dev, self.sim, self.bit0name[self.processorName])
    p3 = ext_int0.XPin(self.dev, self.sim, self.bit3name[self.processorName])
    # check port value
    self.assertPortValue(0xff)
    # ext pin 0 = low, pin 3 = high
    p0.SetPin("L")
    p3.SetPin("H")
    # mask only bit 0
    self.setControl(1)
    self.resetFlag()
    self.enableIRQ()
    # counter is 0?
    self.assertCounter(0, 10000)
    # ext pin 3 = low
    p3.SetPin("L")
    # counter is 0?
    self.assertCounter(0, 20000)
    # ext pin 0 = high
    p0.SetPin("H")
    # counter is 1?
    self.assertCounter(1, 20000)
    # mask bit 0 and 3
    self.setControl(9)
    # ext pin 3 = high
    p3.SetPin("H")
    # counter is 3?
    self.assertCounter(2, 20000)
    
  def test_02(self):
    """check set and reset flag bit"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p0 = ext_int0.XPin(self.dev, self.sim, self.bit0name[self.processorName])
    p3 = ext_int0.XPin(self.dev, self.sim, self.bit3name[self.processorName])
    # check port value
    self.assertPortValue(0xff)
    # ext pin 0 = low, pin 3 = high
    p0.SetPin("L")
    p3.SetPin("H")
    # check port value
    self.assertPortValue(0xfe)
    # mask bit 0 and 3
    self.setControl(9)
    self.resetFlag()
    # check flag
    self.assertFlagReg(False)
    # ext pin 3 = low
    p3.SetPin("L")
    # check port value
    self.assertPortValue(0xf6)
    # check flag
    self.assertFlagReg(True)
    # reset flag
    self.resetFlag()
    self.assertFlagReg(False)
    # mask bit 3
    self.setControl(8)
    # ext pin 0 = high
    p0.SetPin("H")
    # check port value
    self.assertPortValue(0xf7)
    # check flag
    self.assertFlagReg(False)
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("ext_int2_atmega16.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF