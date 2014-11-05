from vcdtestutil import VCDTestCase, VCDTestLoader, uSec

class TestCase(VCDTestCase):

  def setUp(self):
    self.getVCD()

  def test_00(self):
    """simulation time [0..100us]"""
    self.assertVCD()
    self.assertEqual(self.vcd.starttime, 0)
    self.assertTrue(self.vcd.endtime >= 100 * uSec)

  def test_01(self):
    """check clock is 4MHz"""
    self.assertVCD()
    p = self.getVariable("test.clk")
    e = p.getNextEdge(p.getNextEdge(p.firstedge))
    r = e.analyseWire(0)
    self.assertEqual(round(r.frequency, 0), 4000000)

  def test_02(self):
    """check B0 out toggles in 1MHz"""
    self.assertVCD()
    p = self.getVariable("test.pb0")
    e = p.getNextEdge(p.getNextEdge(p.firstedge))
    r = e.analyseWire(0)
    self.assertEqual(round(r.frequency, 0), 1000000)

if __name__ == '__main__':

  from unittest import TestLoader, TextTestRunner
  tests = VCDTestLoader("baretest.vcd").loadTestsFromTestCase(TestCase)
  res = TextTestRunner(verbosity = 2).run(tests)
  if res.wasSuccessful():
    exit(0)
  else:
    exit(1)

# EOF
