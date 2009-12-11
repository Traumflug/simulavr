from simtestutil import SimTestLoader

import ext_int0

class TestCase(ext_int0.TestCase):
  
  extpin = {
    "atmega128":  "D1",
    "atmega16":   "D3",
    "atmega48":   "D3",
    "attiny2313": "D3",
    "at90s8515":  "D3",
    "at90s4433":  "D3",
  }
  
  ctrlshift = 2
  
if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("ext_int1_atmega16.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF