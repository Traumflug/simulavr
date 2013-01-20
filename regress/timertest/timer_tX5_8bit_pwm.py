# -*- coding: UTF-8 -*-
from simtestutil import SimTestCase, SimTestLoader

class TestCase(SimTestCase):
  
  def assertPin(self, name, value):
    self.assertEqual(self.dev.GetPin(name).toChar(), value)

  def assertPinChange(self, pin, invpin, delay = 0):
    pState = self.dev.GetPin(pin).toChar()
    nState = self.dev.GetPin(invpin).toChar()
    if pState == self._state_0:
      pName = invpin
      nName = pin
    else:
      pName = pin
      nName = invpin
    t0 = self.sim.getCurrentTime()
    if delay > 3000:
      self.sim.doRun(self.sim.getCurrentTime() + (delay - 3000))
    while True:
      self.sim.doStep()
      if self.dev.GetPin(pName).toChar() == self._state_0: break
    t1 = self.sim.getCurrentTime() # time, where pName pin changed
    if delay > 0:
      self.assertTrue((t1 - t0) == delay, "wrong delay (%d != %d)" % (t1 - t0, delay))
    # check dead time delay
    self.assertPin(pName, self._state_0)
    self.assertPin(nName, self._state_0)
    self.sim.doRun(self.sim.getCurrentTime() + 1000) # +>1µs
    self.assertPin(pName, self._state_0)
    self.assertPin(nName, self._state_1)
    return t1

  _state_0 = "L" # active driven, LOW
  _state_1 = "H" # active driven, HIGH
  
  # time sheet:
  # ~51µs: main function arrived
  # ~67µs: left function init_timer1
  # ~170µs: pll is locked, switch timer1 clock to 64MHz
  # ~298µs: overflow event, pin B3 goes to low, B4 after 1µs to high
  # +64µs: compare event, pin B4 goes to low, B3 after 1µs to high (1)
  # +64µs: overflow event, pin B3 goes to low, B4 after 1µs to high
  # ... and so one ... [repeated from (1)]
  def test_00(self):
    """check counting"""
    self.assertDevice()
    self.dev.SetClockFreq(1000) # clock with factory default is 1MHz
    self.assertStartTime()
    # skip initialisation
    self.assertInitDone(limit=55000) # about 66us till init_timer1 done
    self.sim.doRun(self.sim.getCurrentTime() + 16000) # +16µs
    # check timer_ticks_overflow and timer_ticks_compare_b after initialisation
    self.assertWordValue("timer_ticks_overflow", 0)
    self.assertWordValue("timer_ticks_compare_b", 0)
    self.assertPin("B3", self._state_1)
    self.assertPin("B4", self._state_0)
    # wait, till pll is locked and clock for timer1 switched to pll clock
    self.sim.doRun(self.sim.getCurrentTime() + 100000) # +100µs
    self.assertEqual(self.dev.getRWMem(0x50), 0) # TCCR1 = 0
    self.sim.doRun(self.sim.getCurrentTime() + 10000) # +10µs
    self.assertEqual(self.dev.getRWMem(0x50), 7) # TCCR1 = 7
    # find first overflow event, about 130µs later
    self.sim.doRun(self.sim.getCurrentTime() + 100000) # +100µs
    self.assertPin("B3", self._state_1)
    self.assertPin("B4", self._state_0)
    t = self.assertPinChange("B3", "B4")
    self.sim.doRun(self.sim.getCurrentTime() + 30000) # +30µs
    self.assertWordValue("timer_ticks_overflow", 1)
    self.assertWordValue("timer_ticks_compare_b", 0)
    # next compare event, 64µs later
    t = self.assertPinChange("B3", "B4", 64000 - (self.sim.getCurrentTime() - t))
    self.sim.doRun(self.sim.getCurrentTime() + 30000) # +30µs
    self.assertWordValue("timer_ticks_overflow", 1)
    self.assertWordValue("timer_ticks_compare_b", 1)
    # next overflow event, 64µs later
    t = self.assertPinChange("B3", "B4", 64000 - (self.sim.getCurrentTime() - t))
    self.sim.doRun(self.sim.getCurrentTime() + 30000) # +30µs
    self.assertWordValue("timer_ticks_overflow", 2)
    self.assertWordValue("timer_ticks_compare_b", 1)
    # next compare event, 64µs later
    t = self.assertPinChange("B3", "B4", 64000 - (self.sim.getCurrentTime() - t))
    self.sim.doRun(self.sim.getCurrentTime() + 30000) # +30µs
    self.assertWordValue("timer_ticks_overflow", 2)
    self.assertWordValue("timer_ticks_compare_b", 2)

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("timer_tX5_8bit_pwm_attiny25.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
