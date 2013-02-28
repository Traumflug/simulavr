from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class TestCase(SimTestCase):
  
  data_load  = ( 0x00, 0x33)
  data_write = ( 0x00, 0x66)

  DELAY = 5000 # run 5 microseconds
  WDELAY_L = 9000000 # run 9 milliseconds
  WDELAY_M = 4500000 # run 4.5 milliseconds
  WDELAY_S = 3900000 # run 3.9 milliseconds

  def assertComplete(self):
    c = self.sim.getByteByName(self.dev, "complete")
    self.assertEqual(c, 1, "function isn't complete (complete=%d)" % c)

  def assertEepromData(self, data):
    for i in range(len(data)):
      d = data[i]
      v = self.dev.eeprom.ReadFromAddress(i)
      self.assertEqual(v, d, "data in EEPROM[%d]: value=0x%x, expected=0x%x" % (i, v, d))

  def test_00(self):
    """check write eeprom data with interrupt"""
    if self.processorName in ("at90s4433", "at90s8515"):
      self.WDELAY = self.WDELAY_M
    elif self.processorName in ("atmega16", "atmega128", "atmega8"):
      self.WDELAY = self.WDELAY_L
    else:
      self.WDELAY = self.WDELAY_S
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 0, "not in idle loop")
    # run till in idle loop
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    # check, that we are now in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 1, "in idle loop")
    # check EEPROM content
    self.assertEepromData(self.data_load)
    # write byte
    self.sim.setByteByName(self.dev, "complete", 2)
    self.sim.doRun(self.sim.getCurrentTime() + self.WDELAY)
    self.assertComplete()
    # check modified EEPROM content
    self.assertEepromData(self.data_write)

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("eeprom_int_at90s4433.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
