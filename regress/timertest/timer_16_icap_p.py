from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class XPin(pysimulavr.Pin):
  
  def __init__(self, dev, sim, name, state = None):
    pysimulavr.Pin.__init__(self)
    self.ct = sim.getCurrentTime
    self.name = name
    if state is not None: self.SetPin(state)
    # hold the connecting net here, it have not be destroyed, if we leave this method
    self.__net = pysimulavr.Net()
    self.__net.Add(self)
    self.__net.Add(dev.GetPin(name))
    
class TestCase(SimTestCase):
  
  _state_0 = "L"
  _state_1 = "H"
  
  def setPinAndGetValue(self, pin, state, runtime, label):
    pin.SetPin(state)
    self.sim.doRun(self.sim.getCurrentTime() + runtime)
    return self.sim.getWordByName(self.dev, label)
    
  def test_00(self):
    """check input capture"""
    self.assertDevice()
    self.assertStartTime()
    ct = self.dev.GetClockFreq()
    rt = 15000
    # skip initialisation
    self.assertInitDone()
    # check input_capture after initialisation
    self.assertWordValue("input_capture", 0)
    # connect pin, set to state before edge
    x = XPin(self.dev, self.sim, "D4", self._state_0)
    self.sim.doRun(self.sim.getCurrentTime() + 10000)
    # set pin to state after edge
    t0 = self.sim.getCurrentTime()
    v0 = self.setPinAndGetValue(x, self._state_1, rt, "input_capture")
    self.assertTrue(v0 > 0, "value input capture is 0")
    # set pin to state before edge
    self.setPinAndGetValue(x, self._state_0, rt, "input_capture")
    # set pin to state after edge (second edge)
    t1 = self.sim.getCurrentTime()
    v1 = self.setPinAndGetValue(x, self._state_1, rt, "input_capture")
    self.assertTrue(v1 > 0, "value input capture is 0 (2. edge)")
    # calculate difference on captured values
    self.assertEqual(t1 - t0, (v1 - v0) * ct)
    # delete external pin
    del x
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("timer_16_icap_p_atmega128.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF