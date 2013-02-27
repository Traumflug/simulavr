from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class TestCase(SimTestCase):
  
  ADC_CLOCK = 8000 # ADC clock is 125kHz

  adc0_pin = {
    "atmega16":  "A0",
    "atmega644": "A0",
    "at90can32": "F0",
    "atmega128": "F0",
  }

  adc1_pin = {
    "atmega16":  "A1",
    "atmega644": "A1",
    "at90can32": "F1",
    "atmega128": "F1",
  }

  def assertComplete(self, pValue, nValue, refValue):
    v = self.sim.getWordByName(self.dev, "adc_value")
    c = self.sim.getByteByName(self.dev, "complete")
    e = int((((pValue - nValue) * 200) / refValue) * 512) & 0x3ff
    self.assertEqual(v, e, "expected adc value is 0x%x, got 0x%x" % (e, v))
    self.assertEqual(c, 1, "conversion is completed")

  def test_00(self):
    """check adc conversion, differential channel with gain 200"""
    self.assertDevice()
    self.assertStartTime()
    # create analog pin as injector and connect to ADC1
    a1pin = pysimulavr.Pin(1.3305) # set to 1.3305V level
    net1 = pysimulavr.Net()
    net1.Add(a1pin)
    net1.Add(self.dev.GetPin(self.adc1_pin[self.processorName])) # connect to ADC1
    # create analog pin as injector and connect to ADC0
    a2pin = pysimulavr.Pin(1.32) # set to 1.32V level
    net2 = pysimulavr.Net()
    net2.Add(a2pin)
    net2.Add(self.dev.GetPin(self.adc0_pin[self.processorName])) # connect to ADC0
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
    # run, first conversion, channel A1 / A0
    self.sim.doRun(self.sim.getCurrentTime() + (12 * self.ADC_CLOCK))
    # get ADC value
    self.assertComplete(1.32, 1.32, 2.56)
    # start next conversion
    self.sim.setByteByName(self.dev, "complete", 2)
    # run, further conversion, channel A3 / A1
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    # get ADC value
    self.assertComplete(1.3305, 1.32, 2.56)

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("adc_diff_atmega16.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
