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
  
  def setPinAndCheck(self, pin, state, runtime, label, value):
    pin.SetPin(state)
    self.sim.doRun(self.sim.getCurrentTime() + runtime)
    self.assertWordValue(label, value)
    
  _state_0 = "L"
  _state_1 = "H"
  
  def test_00(self):
    """check counting"""
    self.assertDevice()
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone()
    # check timer_ticks after initialisation
    self.assertWordValue("timer_ticks", 0)
    # connect pin, set to state before edge
    x = XPin(self.dev, self.sim, "D6", self._state_0)
    self.setPinAndCheck(x, self._state_0, 10000, "timer_ticks", 0)
    # set to state after edge
    self.setPinAndCheck(x, self._state_1, 10000, "timer_ticks", 1)
    # set to state before edge (second clock pulse)
    self.setPinAndCheck(x, self._state_0, 10000, "timer_ticks", 1)
    # set to state after edge (second clock pulse)
    self.setPinAndCheck(x, self._state_1, 10000, "timer_ticks", 2)
    # delete external pin
    del x
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("timer_16_extp_atmega128.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF