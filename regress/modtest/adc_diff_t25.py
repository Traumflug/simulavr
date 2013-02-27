from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class TestCase(SimTestCase):
  
  ADC_CLOCK = 8000 # ADC clock is 125kHz, CPU clock is 4MHz

  def setUp(self):
    SimTestCase.setUp(self)
    # create analog pin as injector and connect to ADC2
    self.a2pin = pysimulavr.Pin(1.5) # set to 1.5V level
    self.__net1 = pysimulavr.Net()
    self.__net1.Add(self.a2pin)
    self.__net1.Add(self.dev.GetPin("B4")) # connect to ADC2
    # create analog pin as injector and connect to ADC3
    self.a3pin = pysimulavr.Pin(1.0) # set to 1V level
    self.__net2 = pysimulavr.Net()
    self.__net2.Add(self.a3pin)
    self.__net2.Add(self.dev.GetPin("B3")) # connect to ADC3

  def tearDown(self):
    del self.__net2
    del self.a3pin
    del self.__net1
    del self.a2pin
    SimTestCase.tearDown(self)

  def assertLoop(self, inLoop):
    if inLoop:
      # check, that we are now in idle loop ...
      self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 1, "in idle loop")
    else:
      # check, that we are not in idle loop ...
      self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 0, "not in idle loop")
      
  def assertComplete(self, complete = True):
     c = self.sim.getByteByName(self.dev, "complete")
     if complete:
       self.assertEqual(c, 1, "conversion is completed")
     else:
       self.assertEqual(c, 0, "conversion is not completed")

  def assertValue(self, pValue, nValue, refValue, unipolar = True):
    if unipolar:
      rng = 1024
    else:
      rng = 512
    v = self.sim.getWordByName(self.dev, "adc_value")
    e = int(((pValue - nValue) / refValue) * rng) & 0x3ff
    self.assertEqual(v, e, "expected adc value is 0x%x, got 0x%x" % (e, v))

  def test_00(self):
    """check adc conversion, differential channel, bipolar mode"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertLoop(False)
    # run to come in idle loop
    self.sim.doRun(self.sim.getCurrentTime() + self.ADC_CLOCK)
    # check, that we are in idle loop now...
    self.assertLoop(True)
    # start first conversion
    self.sim.setByteByName(self.dev, "complete", 2)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    self.assertComplete(False)
    # run and end first conversion, channel A2 / A3
    self.sim.doRun(self.sim.getCurrentTime() + (12 * self.ADC_CLOCK))
    self.assertComplete()
    # check ADC value
    self.assertValue(1.5, 1.0, 2.56, False)
    # start second conversion
    self.sim.setByteByName(self.dev, "complete", 2)
    self.a2pin.SetAnalogValue(0.8)
    # run and end second conversion, channel A2 / A3
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    self.assertComplete()
    # check ADC value
    self.assertValue(0.8, 1.0, 2.56, False)

  def test_01(self):
    """check adc conversion, differential channel, unipolar mode"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertLoop(False)
    # run to come in idle loop
    self.sim.doRun(self.sim.getCurrentTime() + self.ADC_CLOCK)
    # check, that we are in idle loop now...
    self.assertLoop(True)
    # start first conversion
    self.sim.setByteByName(self.dev, "complete", 3)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    self.assertComplete(False)
    # run and end first conversion, channel A2 / A3
    self.sim.doRun(self.sim.getCurrentTime() + (12 * self.ADC_CLOCK))
    self.assertComplete()
    # check ADC value
    self.assertValue(1.5, 1.0, 2.56)
    # start second conversion
    self.sim.setByteByName(self.dev, "complete", 3)
    self.a2pin.SetAnalogValue(0.8)
    # run and end second conversion, channel A2 / A3
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    self.assertComplete()
    # check ADC value
    v = self.sim.getWordByName(self.dev, "adc_value")
    self.assertEqual(v, 0, "expected adc value is 0x0, got 0x%x" % v)

  def test_02(self):
    """check adc conversion, differential channel, unipolar mode with IPR"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertLoop(False)
    # run to come in idle loop
    self.sim.doRun(self.sim.getCurrentTime() + self.ADC_CLOCK)
    # check, that we are in idle loop now...
    self.assertLoop(True)
    # start first conversion
    self.sim.setByteByName(self.dev, "complete", 4)
    # run
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    self.assertComplete(False)
    # run and end first conversion, channel A2 / A3
    self.sim.doRun(self.sim.getCurrentTime() + (12 * self.ADC_CLOCK))
    self.assertComplete()
    # check ADC value
    v = self.sim.getWordByName(self.dev, "adc_value")
    self.assertEqual(v, 0, "expected adc value is 0x0, got 0x%x" % v)
    # start second conversion
    self.sim.setByteByName(self.dev, "complete", 4)
    self.a2pin.SetAnalogValue(0.8)
    # run and end second conversion, channel A2 / A3
    self.sim.doRun(self.sim.getCurrentTime() + (15 * self.ADC_CLOCK))
    self.assertComplete()
    # check ADC value
    self.assertValue(1.0, 0.8, 2.56)

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("adc_diff_t25_attiny25.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
