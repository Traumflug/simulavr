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

  def assertComplete(self, pValue, refValue, iCnt):
    v = self.sim.getWordByName(self.dev, "adc_value")
    c = self.sim.getByteByName(self.dev, "isr_count")
    e = int((pValue / refValue) * 1024) & 0x3ff
    self.assertEqual(v, e, "expected adc value is 0x%x, got 0x%x" % (e, v))
    self.assertEqual(c, iCnt, "expected isr_count is %d, got %d" % (iCnt, c))

  def test_00(self):
    """check adc conversion on free running mode with interrupt"""
    self.assertDevice()
    self.assertStartTime()
    # create analog pin as injector and connect to ADC0
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
    self.sim.doRun(self.sim.getCurrentTime() + (16 * self.ADC_CLOCK))
    # check, that we are now in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 1, "in idle loop")
    # check adc_value and isr_count to be (0x5555, 0)
    v = self.sim.getWordByName(self.dev, "adc_value")
    c = self.sim.getByteByName(self.dev, "isr_count")
    self.assertEqual(v, 0x5555, "expected adc value is 0x0, got 0x%x" % v)
    self.assertEqual(c, 0, "expected isr_count is 0, got %d" % c)
    # change input value for next conversion
    apin.SetAnalogValue(1.35)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (14 * self.ADC_CLOCK))
    # check adc_value and isr_count
    self.assertComplete(1.0, 2.5, 1)
    # change input value for next conversion
    apin.SetAnalogValue(0.7)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (14 * self.ADC_CLOCK))
    # check adc_value and isr_count
    self.assertComplete(1.35, 2.5, 2)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (14 * self.ADC_CLOCK))
    # check adc_value and isr_count
    self.assertComplete(0.7, 2.5, 3)

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("adc_fr_at90s4433.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
