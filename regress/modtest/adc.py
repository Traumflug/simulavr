from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class TestCase(SimTestCase):
  
  ADC_CLOCK = 8000 # ADC clock is 125kHz

  adc0_pin = {
    "at90s4433": "C0",
    "atmega8":   "C0",
    "attiny25":  "B5",
    "at90can32": "F0",
    "atmega644": "A0",
    "atmega16":  "A0",
    "atmega128": "F0",
    "atmega48":  "C0",
  }

  aref_pin = {
    "at90s4433": "AREF",
    "atmega8":   "AREF",
    "attiny25":  "B0",
    "at90can32": "AREF",
    "atmega644": "AREF",
    "atmega16":  "AREF",
    "atmega128": "AREF",
    "atmega48":  "AREF",
  }

  def assertComplete(self, pValue, refValue):
    v = self.sim.getWordByName(self.dev, "adc_value")
    c = self.sim.getByteByName(self.dev, "complete")
    e = int((pValue / refValue) * 1024) & 0x3ff
    self.assertEqual(v, e, "expected adc value is 0x%x, got 0x%x" % (e, v))
    self.assertEqual(c, 1, "conversion is completed")

  def test_00(self):
    """check adc conversion"""
    self.assertDevice()
    self.assertStartTime()
    # create analog pin as injector and connect to ADC0 input
    apin = pysimulavr.Pin(1.0) # set to 1V level
    net1 = pysimulavr.Net()
    net1.Add(apin)
    net1.Add(self.dev.GetPin(self.adc0_pin[self.processorName])) # connect to ADC0
    # create analog pin as injector and connect to AREF
    rpin = pysimulavr.Pin(2.5) # set to 2.5V level
    net2 = pysimulavr.Net()
    net2.Add(rpin)
    net2.Add(self.dev.GetPin(self.aref_pin[self.processorName])) # connect to AREF
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 0, "not in idle loop")
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    # check, that we are now in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 1, "in idle loop")
    c = self.sim.getByteByName(self.dev, "complete")
    self.assertEqual(c, 0, "conversion is not completed")
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (12 * self.ADC_CLOCK))
    # get ADC value
    self.assertComplete(1.0, 2.5)
    # start next conversion
    self.sim.setByteByName(self.dev, "complete", 2)
    apin.SetAnalogValue(1.65)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    # get ADC value
    self.assertComplete(1.65, 2.5)

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("adc_at90s4433.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
