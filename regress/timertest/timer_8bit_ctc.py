from vcdtestutil import VCDTestCase, VCDTestLoader, mSec

class TestCase(VCDTestCase):
  
  p2irq = {
    "atmega128": "IRQ.VECTOR9",
    "atmega48":  "IRQ.VECTOR7",
  }
  
  def setUp(self):
    self.getVCD()
    self.setClock(4000000)
    self.processor = self.getProcessorType()
    self.tov2 = self.p2irq[self.processor]
    
  def test_00(self):
    """simulation time [0..8ms]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertEqual(self.vcd.endtime, 8 * mSec)
    
if __name__ == '__main__':
  
  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("timer_8bit_ctc_atmega128.vcd").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
