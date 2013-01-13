from vcdtestutil import VCDTestCase, VCDTestLoader, mSec

class TestCase(VCDTestCase):
  
  p2irq = {
    "atmega128": "IRQ.VECTOR9",
    "atmega48":  "IRQ.VECTOR7",
  }
  p2pin = {
    "atmega128": "PORTB.B7-Out",
    "atmega48":  "PORTB.B3-Out",
  }
  ocra_value = 124
  prescaler = 64
  max_irq_delay = 6
  
  def setUp(self):
    self.getVCD()
    self.setClock(4000000)
    self.processor = self.getProcessorType()
    self.tcmp = self.p2irq[self.processor]
    self.pinName = self.p2pin[self.processor]
    
  def test_00(self):
    """simulation time [0..8ms]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertEqual(self.vcd.endtime, 8 * mSec)
    
  def test_01(self):
    """init port and counter"""
    self.assertVCD()
    p = self.getVariable("TIMER2.TCNT")
    self.assertEqual(p.firstedge.intValue, 0)
    p = self.getVariable("TIMER2.OCRA")
    self.assertEqual(p.firstedge.intValue, self.ocra_value)
    p = self.getVariable(self.pinName)
    self.assertEqual(p.firstedge.intValue, 0)

  def test_02(self):
    """counter period = 2ms"""
    self.assertVCD()
    c = self.getVariable("TIMER2.Counter")
    c1 = c.firstedge
    tp = self.tClock * self.prescaler
    t0 = c1.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    self.assertEqual(c1.intValue, 1)
    c2 = c.getNextEdge(c1)
    self.assertEqual(c2.intValue, 2)
    self.assertEqual(c2.internalTime - c1.internalTime, tp)
      
  def test_03(self):
    """counter mode: count OCRA, then 0"""
    self.assertVCD()
    c = self.getVariable("TIMER2.Counter")
    c1 = c.firstedge
    tp = self.tClock * self.prescaler
    t0 = c1.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    c2 = c.getNextEdge(t0 + dtc)
    self.assertEqual(c2.intValue, 0)
        
  def test_04(self):
    """check occurence of OCIE2 interrupt"""
    self.assertVCD()
    ctr = self.getVariable("TIMER2.Counter")
    tp = self.tClock * self.prescaler
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    idelay = self.max_irq_delay * self.tClock
    irq = self.getVariable(self.tcmp)
    # first overflow
    t = t0 + dtc
    ce = ctr.getNextEdge(t)
    self.assertEqual(ce.internalTime, t)
    self.assertEqual(ce.intValue, 0)
    # check, when interrupt occurs
    ie = irq.getNextEdge(t)
    self.assertEqual(ie.intValue, 1)
    self.assertTrue(ie.internalTime <= (t + idelay), "OCIE2 occured to late")
    # seek next OCIE2
    ie = irq.getNextEdge(irq.getNextEdge(ie))
    self.assertTrue(ie.internalTime <= (t + dtc + idelay), "second OCIE2 occured to late")
    
  def test_05(self):
    """check toggle port pin"""
    self.assertVCD()
    ctr = self.getVariable("TIMER2.Counter")
    tp = self.tClock * self.prescaler
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    p = self.getVariable(self.pinName)
    # check occurence and value of first toggle change
    pe = p.getNextEdge(p.firstedge)
    self.assertEqual(pe.intValue, 1)
    self.assertEqual(pe.internalTime, t0 + dtc)
    # check occurence and value of following toggle changes
    pe = p.getNextEdge(pe)
    self.assertEqual(pe.intValue, 0)
    self.assertEqual(pe.internalTime, t0 + (2 * dtc))
    pe = p.getNextEdge(pe)
    self.assertEqual(pe.intValue, 1)
    self.assertEqual(pe.internalTime, t0 + (3 * dtc))
    
if __name__ == '__main__':
  
  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("timer_8bit_ctc_atmega128.vcd").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
