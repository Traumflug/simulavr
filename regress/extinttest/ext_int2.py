from simtestutil import SimTestLoader

import ext_int0

class TestCase(ext_int0.TestCaseBase):
  
  ctrlshift = 6
  
  def test_00(self):
    """check interrupt on rising edge"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p = ext_int0.XPin(self.dev, self.sim, "B2")
    # check port value
    self.assertPortValue(0xff)
    # ext pin = low
    p.SetPin("L")
    # init for rising edge
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
    # counter is 1?
    self.assertCounter(1, 20000)
    # ext pin = high
    p.SetPin("H")
    # counter is 2?
    self.assertCounter(2, 20000)
    
  def test_01(self):
    """check interrupt on falling edge"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p = ext_int0.XPin(self.dev, self.sim, "B2")
    # check port value
    self.assertPortValue(0xff)
    # ext pin = high
    p.SetPin("H")
    # init for rising edge
    self.setControl(0)
    self.resetFlag()
    self.enableIRQ()
    # counter is 0?
    self.assertCounter(0, 10000)
    # ext pin = low
    p.SetPin("L")
    # counter is 1?
    self.assertCounter(1, 20000)
    # ext pin = high
    p.SetPin("H")
    # counter is 1?
    self.assertCounter(1, 20000)
    # ext pin = low
    p.SetPin("L")
    # counter is 2?
    self.assertCounter(2, 20000)
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("ext_int2_atmega16.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF