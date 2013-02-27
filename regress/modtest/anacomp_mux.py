from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class TestCase(SimTestCase):
  
  DELAY = 6000 # run 6 microseconds

  adc1_pin = {
    "at90can32": "F1",
    "attiny25":  "B2",
    "atmega644": "A1",
    "atmega16":  "A1",
    "atmega128": "F1",
    "atmega48":  "C1",
    "atmega8":   "C1",
  }

  ain0_pin = {
    "at90can32": "E2",
    "attiny25":  "B0",
    "atmega644": "B2",
    "atmega16":  "B2",
    "atmega128": "E2",
    "atmega48":  "D6",
    "atmega8":   "D6",
  }

  def test_00(self):
    """check state of analog comparator"""
    self.assertDevice()
    self.assertStartTime()
    # create analog pin as injector and connect to F1 (AIN1 over ADC1)
    a1pin = pysimulavr.Pin(1.0)
    net1 = pysimulavr.Net()
    net1.Add(a1pin)
    net1.Add(self.dev.GetPin(self.adc1_pin[self.processorName])) # connect to AIN1
    # create analog pin as injector and connect to E2 (AIN0)
    a0pin = pysimulavr.Pin(1.8)
    net0 = pysimulavr.Net()
    net0.Add(a0pin)
    net0.Add(self.dev.GetPin(self.ain0_pin[self.processorName])) # connect to AIN0
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 0, "not in idle loop")
    # get a output pin
    opin = self.dev.GetPin("B1")
    # run till in idle loop
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    # check, that we are now in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 1, "in idle loop")
    # check output state
    self.assertEqual(opin.toChar(), "H", "output value wrong: got=%s, exp=H" % opin.toChar())
    # set voltage on a0pin == AIN0 to 0.3V
    a0pin.SetAnalogValue(0.3)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    # check output state
    self.assertEqual(opin.toChar(), "L", "output value wrong: got=%s, exp=L" % opin.toChar())

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("anacomp_mux_at90s4433.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
