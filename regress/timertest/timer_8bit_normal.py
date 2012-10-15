from vcdtestutil import VCDTestCase, VCDTestLoader, mSec

class TestCase(VCDTestCase):
  
  p2irq = {
    "atmega128":  "IRQ.VECTOR16",
    "at90s4433":  "IRQ.VECTOR6",
    "at90s8515":  "IRQ.VECTOR7",
    "atmega48":   "IRQ.VECTOR16",
    "attiny2313": "IRQ.VECTOR6",
    "attiny25":   "IRQ.VECTOR5",
  }
  
  def setUp(self):
    self.getVCD()
    self.setClock(4000000)
    self.processor = self.getProcessorType()
    self.tov2 = self.p2irq[self.processor]
    
  def test_00(self):
    """simulation time [0..5ms]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertEqual(self.vcd.endtime, 5 * mSec)
    
  def test_01(self):
    """init port and counter"""
    self.assertVCD()
    p = self.getVariable("TIMER0.TCNT")
    self.assertEqual(p.firstedge.intValue, 0)

  def test_02(self):
    """counter period = 2us"""
    self.assertVCD()
    c = self.getVariable("TIMER0.Counter")
    c1 = c.firstedge
    tp = self.tClock * 8
    t0 = c1.internalTime - tp
    dtc = tp * 256
    self.assertEqual(c1.intValue, 1)
    c2 = c.getNextEdge(c1)
    self.assertEqual(c2.intValue, 2)
    self.assertEqual(c2.internalTime - c1.internalTime, tp)
      
  def test_03(self):
    """counter mode: count 0xff, then 0"""
    self.assertVCD()
    c = self.getVariable("TIMER0.Counter")
    c1 = c.firstedge
    tp = self.tClock * 8
    t0 = c1.internalTime - tp
    dtc = tp * 256
    c2 = c.getNextEdge(t0 + dtc)
    self.assertEqual(c2.intValue, 0)
        
  def test_04(self):
    """check occurence of TOV0 interrupt"""
    self.assertVCD()
    ctr = self.getVariable("TIMER0.Counter")
    tp = self.tClock * 8
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * 256
    idelay = 6 * self.tClock
    irq = self.getVariable(self.tov2)
    # first overflow
    t = t0 + dtc
    ce = ctr.getNextEdge(t)
    self.assertEqual(ce.internalTime, t)
    self.assertEqual(ce.intValue, 0)
    # check, when interrupt occurs
    ie = irq.getNextEdge(t)
    self.assertEqual(ie.intValue, 1)
    self.assertTrue(ie.internalTime <= (t + idelay), "TOV0 occured to late")
    # seek next TOV0
    ie = irq.getNextEdge(irq.getNextEdge(ie))
    self.assertTrue(ie.internalTime <= (t + dtc + idelay), "second TOV0 occured to late")
    
if __name__ == '__main__':
  
  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("timer_8bit_normal_atmega128.vcd").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
