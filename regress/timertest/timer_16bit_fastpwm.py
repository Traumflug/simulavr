from vcdtestutil import VCDTestCase, VCDTestLoader, mSec

class TestCase(VCDTestCase):
  
  ocra_value = 0x63f
  ocrb_value = 0x27f
  prescaler = 1
  max_irq_delay = 6
  
  def setUp(self):
    self.getVCD()
    self.setClock(4000000)
    self.processor = self.getProcessorType()
    
  def test_00(self):
    """simulation time [0..4ms]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertEqual(self.vcd.endtime, 4 * mSec)
    
  def test_01(self):
    """init port and counter"""
    self.assertVCD()
    p = self.getVariable("TIMER3.TCNTH")
    self.assertEqual(p.firstedge.intValue, 0)
    p = self.getVariable("TIMER3.TCNTL")
    self.assertEqual(p.firstedge.intValue, 0)
    p = self.getVariable("TIMER3.OCRAH")
    self.assertEqual(p.firstedge.intValue, self.ocra_value >> 8)
    p = self.getVariable("TIMER3.OCRAL")
    self.assertEqual(p.firstedge.intValue, self.ocra_value & 0xff)
    p = self.getVariable("TIMER3.OCRBH")
    self.assertEqual(p.firstedge.intValue, self.ocrb_value >> 8)
    p = self.getVariable("TIMER3.OCRBL")
    self.assertEqual(p.firstedge.intValue, self.ocrb_value & 0xff)
    p = self.getVariable("PORTE.E4-Out")
    self.assertEqual(p.firstedge.intValue, 0)

  def test_02(self):
    """counter period = 0.25us"""
    self.assertVCD()
    c = self.getVariable("TIMER3.Counter")
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
    c = self.getVariable("TIMER3.Counter")
    c1 = c.firstedge
    tp = self.tClock * self.prescaler
    t0 = c1.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    c2 = c.getNextEdge(t0 + dtc)
    self.assertEqual(c2.intValue, 0)
        
  def test_04(self):
    """check occurence of TOV3 interrupt"""
    self.assertVCD()
    ctr = self.getVariable("TIMER3.Counter")
    tp = self.tClock * self.prescaler
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    idelay = self.max_irq_delay * self.tClock
    irq = self.getVariable("IRQ.VECTOR29")
    # first overflow
    t = t0 + dtc
    ce = ctr.getNextEdge(t)
    self.assertEqual(ce.internalTime, t)
    self.assertEqual(ce.intValue, 0)
    # check, when interrupt occurs
    ie = irq.getNextEdge(t)
    self.assertEqual(ie.intValue, 1)
    self.assertTrue(ie.internalTime <= (t + idelay), "TOV3 occured to late")
    # seek next TOV3
    ie = irq.getNextEdge(irq.getNextEdge(ie))
    self.assertTrue(ie.internalTime <= (t + dtc + idelay), "second TOV3 occured to late")
    
  def test_05(self):
    """check occurence of OCIE3B interrupt"""
    self.assertVCD()
    ctr = self.getVariable("TIMER3.Counter")
    tp = self.tClock * self.prescaler
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    idelay = self.max_irq_delay * self.tClock
    irq = self.getVariable("IRQ.VECTOR27")
    # first compare match
    t = t0 + (tp * (self.ocrb_value + 1))
    ce = ctr.getNextEdge(t)
    self.assertEqual(ce.internalTime, t)
    self.assertEqual(ce.intValue, self.ocrb_value + 1)
    # check, when interrupt occurs
    ie = irq.getNextEdge(t)
    self.assertEqual(ie.intValue, 1)
    self.assertTrue(ie.internalTime <= (t + idelay), "OCIE3B occured to late")
    # seek next OCIE3B
    ie = irq.getNextEdge(irq.getNextEdge(ie))
    self.assertTrue(ie.internalTime <= (t + dtc + idelay), "second OCIE3B occured to late")
    
  def test_06(self):
    """check pwm signal on port pin"""
    self.assertVCD()
    ctr = self.getVariable("TIMER3.Counter")
    tp = self.tClock * self.prescaler
    t0 = ctr.firstedge.internalTime - tp
    dtc = tp * (self.ocra_value + 1)
    dtcmp = tp * (self.ocrb_value + 1)
    p = self.getVariable("PORTE.E4-Out")
    # check occurence of set pin at compare match
    pe = p.getNextEdge(p.firstedge)
    self.assertEqual(pe.intValue, 1)
    self.assertEqual(pe.internalTime, t0 + dtcmp)
    # check occurence of reset pin at timer reset
    pe = p.getNextEdge(pe)
    self.assertEqual(pe.intValue, 0)
    self.assertEqual(pe.internalTime, t0 + dtc)
    # check next period
    pe = p.getNextEdge(pe)
    self.assertEqual(pe.intValue, 1)
    self.assertEqual(pe.internalTime, t0 + dtc + dtcmp)
    pe = p.getNextEdge(pe)
    self.assertEqual(pe.intValue, 0)
    self.assertEqual(pe.internalTime, t0 + (2 * dtc))
    
if __name__ == '__main__':
  
  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("timer_16bit_fastpwm_atmega128.vcd").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
