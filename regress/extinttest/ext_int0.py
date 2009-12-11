from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class XPin(pysimulavr.Pin):
  
  def __init__(self, dev, sim, name):
    pysimulavr.Pin.__init__(self)
    self.ct = sim.getCurrentTime
    self.name = name
    # hold the connecting net here, must not be destroyed till deleting XPin
    self.__net = pysimulavr.Net()
    self.__net.Add(self)
    self.__net.Add(dev.GetPin(name))
    
class TestCaseBase(SimTestCase):
  
  extpin = dict()
  
  portin = dict()
  
  ctrlshift = 0
  
  def handshake(self, cmd, data = None, steptime = 10000):
    hs = (self.sim.getByteByName(self.dev, "hs_out") + 1) & 0xff
    self.sim.setByteByName(self.dev, "hs_cmd", cmd)
    if data is not None: self.sim.setByteByName(self.dev, "hs_data", data)
    self.sim.setByteByName(self.dev, "hs_in", hs)
    while True:
      self.sim.doRun(self.sim.getCurrentTime() + steptime)
      if self.sim.getByteByName(self.dev, "hs_out") == hs:
        break
    return self.sim.getByteByName(self.dev, "hs_data")
    
  def assertPortValue(self, val):
    v = self.handshake(5)
    self.assertEqual(v, val, "port input value wrong: got=%d, exp=%d" % (v, val))
    
  def assertCounter(self, val, timeBefore = None):
    if timeBefore is not None:
      self.sim.doRun(self.sim.getCurrentTime() + timeBefore)
    v = self.sim.getByteByName(self.dev, "cnt_irq")
    self.assertEqual(v, val, "irq counter value wrong: got=%d, exp=%d" % (v, val))
    
  def assertFlagReg(self, state):
    v = not self.getFlagReg() == 0
    if state:
      self.assertTrue(v, "irq flag isn't set")
    else:
      self.assertFalse(v, "irq flag is set")
      
  def setControl(self, ctrl):
    self.handshake(4, ctrl << self.ctrlshift)
    
  def enableIRQ(self):
    self.handshake(1, 1)
    
  def disableIRQ(self):
    self.handshake(1, 0)
    
  def resetFlag(self):
    self.handshake(3)
    
  def getFlagReg(self):
    return self.handshake(6)
    
  def getMaskReg(self):
    return self.handshake(7)
    
  def setDisableMask(self, val):
    val = bool(val)
    if val:
      self.sim.setByteByName(self.dev, "dis_mask", 1)
    else:
      self.sim.setByteByName(self.dev, "dis_mask", 0)
      
class TestCase(TestCaseBase):
  
  extpin = {
    "atmega128":  "D0",
    "atmega16":   "D2",
    "atmega48":   "D2",
    "attiny2313": "D2",
    "at90s8515":  "D2",
    "at90s4433":  "D2",
  }
  
  portin = {
    "atmega128":  0xff,
    "atmega16":   0xff,
    "atmega48":   0xff,
    "attiny2313": 0x7f, # only 7 bit at port D
    "at90s8515":  0xff,
    "at90s4433":  0xff,
  }
  
  ctrlshift = 0
  
  def test_00(self):
    """check interrupt on rising edge"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p = XPin(self.dev, self.sim, self.extpin[self.processorName])
    # check port value
    self.assertPortValue(self.portin[self.processorName])
    # ext pin = low
    p.SetPin("L")
    # init for rising edge
    self.setControl(3)
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
    p = XPin(self.dev, self.sim, self.extpin[self.processorName])
    # check port value
    self.assertPortValue(self.portin[self.processorName])
    # ext pin = high
    p.SetPin("H")
    # init for rising edge
    self.setControl(2)
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
    
  def test_02(self):
    """check interrupt on all changes"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p = XPin(self.dev, self.sim, self.extpin[self.processorName])
    # check port value
    self.assertPortValue(self.portin[self.processorName])
    # ext pin = low
    p.SetPin("L")
    # init for rising edge
    self.setControl(1) # the warning at this step for at90s8515 is right!
    self.resetFlag()
    self.enableIRQ()
    # counter is 0?
    self.assertCounter(0, 10000)
    # ext pin = high
    p.SetPin("H")
    # ok, this test can't be successfull for at90s8515, because it dosn't support this mode
    # so we abort the following steps (quick and dirty, but it works ;-) )
    if self.processorName == "at90s8515":
      # counter is 0 again?
      self.assertCounter(0, 20000)
      return
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
    
  def test_03(self):
    """check level interrupt"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create Pin
    p = XPin(self.dev, self.sim, self.extpin[self.processorName])
    # check port value
    self.assertPortValue(self.portin[self.processorName])
    # ext pin = high
    p.SetPin("H")
    # init for level interrupt
    self.setDisableMask(True)
    self.setControl(0)
    self.resetFlag()
    self.enableIRQ()
    # counter is 0?
    self.assertCounter(0, 10000)
    self.assertFalse(self.getMaskReg() == 0, "mask register is 0")
    self.assertTrue(self.getFlagReg() == 0, "flag register is not 0")
    # ext pin = low
    p.SetPin("L")
    # counter is 1?
    self.assertCounter(1, 20000)
    self.assertTrue(self.getMaskReg() == 0, "mask register is not 0")
    self.assertTrue(self.getFlagReg() == 0, "flag register is not 0")
    # counter is 2?
    self.enableIRQ()
    self.assertCounter(2, 20000)
    self.assertTrue(self.getMaskReg() == 0, "mask register is not 0")
    self.assertTrue(self.getFlagReg() == 0, "flag register is not 0")
    # ext pin = high
    p.SetPin("H")
    # counter is still 2?
    self.enableIRQ()
    self.assertCounter(2, 20000)
    self.assertFalse(self.getMaskReg() == 0, "mask register is 0")
    self.assertTrue(self.getFlagReg() == 0, "flag register is not 0")
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("ext_int0_atmega16.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF