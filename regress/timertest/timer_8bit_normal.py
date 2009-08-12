from vcdtestutil import VCDTestCase, VCDTestLoader, mSec

class TestCase(VCDTestCase):
  
  def setUp(self):
    self.getVCD()
    self.setClock(4000000)
    
  def test_00(self):
    """simulation time [0..5ms]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertEqual(self.vcd.endtime, 5 * mSec)
    
if __name__ == '__main__':
  
  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("example3_atmega128.vcd").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
