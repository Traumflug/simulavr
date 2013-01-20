# -*- coding: UTF-8 -*-
from simtestutil import SimTestCase, SimTestLoader

class TestCase(SimTestCase):
  
  def runAndCheckTicks(self, runtime, label, value):
    self.sim.doRun(self.sim.getCurrentTime() + runtime)
    self.assertWordValue(label, value)

  def assertPin(self, name, value):
    self.assertEqual(self.dev.GetPin(name).toChar(), value)

  _state_0 = "L" # active driven, LOW
  _state_1 = "H" # active driven, HIGH
  _state_t = "t" # tristate
  
  # time sheet:
  # ~52µs: main function arrived
  # ~70µs: left function init_timer1
  # ~133µs: OCRB compare event (1)
  # ~170µs: count timer_ticks_compare_b
  # ... OCRB event (1) repeated after 128µs = 261µs and so one ...
  def test_00(self):
    """check counting"""
    self.assertDevice()
    self.dev.SetClockFreq(1000) # clock with factory default is 1MHz
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone(limit=55000) # about 66us till init_timer1 done
    # check timer_ticks_overflow and timer_ticks_compare_b after initialisation
    self.assertWordValue("timer_ticks_overflow", 0)
    self.assertWordValue("timer_ticks_compare_b", 0)
    self.assertPin("B4", self._state_t)
    # check timer compare channel B and if pin is set
    self.runAndCheckTicks(120000, "timer_ticks_compare_b", 1)
    self.assertPin("B4", self._state_1)
    # next compare event after 128µs, check ticks and toggle pin 
    self.runAndCheckTicks(128000, "timer_ticks_compare_b", 2)
    self.assertPin("B4", self._state_0)
    # and next compare event after 128µs, check ticks and toggle pin 
    self.runAndCheckTicks(128000, "timer_ticks_compare_b", 3)
    self.assertPin("B4", self._state_1)
    # overflow tick should not count
    self.assertWordValue("timer_ticks_overflow", 0)
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("timer_tX5_8bit_ctc_attiny25.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
