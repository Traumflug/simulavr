from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class TestCase(SimTestCase):
  
  DELAY = 5000 # run 30 microseconds

  def test_00(self):
    """check state of analog comparator"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # create analog pin as injector and connect to D7
    apin = pysimulavr.Pin(1.0) # below bandgap voltage level
    net = pysimulavr.Net()
    net.Add(apin)
    net.Add(self.dev.GetPin("D7")) # connect to AIN1
    # get a output pin
    opin = self.dev.GetPin("B0")
    # run
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    # check output state
    self.assertEqual(opin.toChar(), "H", "output value wrong: got=%s, exp=H" % opin.toChar())
    # set voltage on apin to 2.0V
    apin.SetAnalogValue(2.0)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    # check output state
    self.assertEqual(opin.toChar(), "L", "output value wrong: got=%s, exp=L" % opin.toChar())

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("ext_int0_atmega16.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
