from vcdtestutil import VCDTestCase, VCDTestLoader, mSec

class TestCase(VCDTestCase):
  
  p2irq = {
    "atmega128": "IRQ.VECTOR14",
    "at90s4433": "IRQ.VECTOR5",
    "at90s8515": "IRQ.VECTOR6",
    "atmega48":  "IRQ.VECTOR13",
  }
  
  def setUp(self):
    self.getVCD()
    self.setClock(4000000)
    self.processor = self.getProcessorType()
    self.tov1 = self.p2irq[self.processor]
    
  def test_00(self):
    """simulation time [0..40ms]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertEqual(self.vcd.endtime, 40 * mSec)
    
  def test_01(self):
    """init counter"""
    self.assertVCD()
    p = self.getVariable("TIMER1.TCNTH")
    self.assertEqual(p.firstedge.intValue, 0)
    p = self.getVariable("TIMER1.TCNTL")
    self.assertEqual(p.firstedge.intValue, 0)

  def test_02(self):
    """counter period = 0,25us"""
    self.assertVCD()
    c = self.getVariable("TIMER1.Counter")
    c1 = c.firstedge
    tp = self.tClock
    t0 = c1.internalTime - tp
    dtc = tp * 65536
    self.assertEqual(c1.intValue, 1)
    c2 = c.getNextEdge(c1)
    self.assertEqual(c2.intValue, 2)
    self.assertEqual(c2.internalTime - c1.internalTime, tp)
      
  def test_03(self):
    """counter mode: count 0xffff, then 0"""
    self.assertVCD()
    c = self.getVariable("TIMER1.Counter")
    c1 = c.firstedge
    tp = self.tClock
    t0 = c1.internalTime - tp
    dtc = tp * 65536
    c2 = c.getNextEdge(t0 + dtc)
    self.assertEqual(c2.intValue, 0)
        
  def test_04(self):
    """check occurence of TOV1 interrupt"""
    self.assertVCD()
    ctr = self.getVariable("TIMER1.Counter")
    tp = self.tClock
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * 65536
    idelay = 6 * self.tClock
    irq = self.getVariable(self.tov1)
    # first overflow
    t = t0 + dtc
    ce = ctr.getNextEdge(t)
    self.assertEqual(ce.internalTime, t)
    self.assertEqual(ce.intValue, 0)
    # check, when interrupt occurs
    ie = irq.getNextEdge(t)
    self.assertEqual(ie.intValue, 1)
    self.assertTrue(ie.internalTime <= (t + idelay), "TOV1 occured to late")
    # seek next TOV1
    ie = irq.getNextEdge(irq.getNextEdge(ie))
    self.assertTrue(ie.internalTime <= (t + dtc + idelay), "second TOV1 occured to late")
    
if __name__ == '__main__':
  
  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("timer_16bit_normal_atmega128.vcd").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
