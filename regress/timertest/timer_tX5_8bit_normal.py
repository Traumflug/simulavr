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
  # ~51µs: main function arrived
  # ~66µs: left function init_timer1
  # ~129µs: OCRB compare event (1)
  # ~160µs: count timer_ticks_compare_b
  # ~322µs: timer overflow event
  # ~352µs: count timer_ticks_overflow and toggle pin
  # ... OCRB event (1) repeated after 256µs = 385µs and so one ...
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
    # overflow should not happen till here
    self.runAndCheckTicks(170000, "timer_ticks_overflow", 0)
    self.assertPin("B4", self._state_1)
    # but should happen till here
    self.runAndCheckTicks(20000, "timer_ticks_overflow", 1)
    self.assertPin("B4", self._state_0)
    # next cycle, start with compare event
    self.runAndCheckTicks(60000, "timer_ticks_compare_b", 2)
    self.assertPin("B4", self._state_1)
    # no overflow till here
    self.runAndCheckTicks(180000, "timer_ticks_overflow", 1)
    self.assertPin("B4", self._state_1)
    # but till here
    self.runAndCheckTicks(20000, "timer_ticks_overflow", 2)
    self.assertPin("B4", self._state_0)
    
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("timer_tX5_8bit_normal_attiny25.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
