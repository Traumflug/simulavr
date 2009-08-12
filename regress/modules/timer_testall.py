from unittest import TextTestRunner, TestSuite, TestLoader
from sys import argv
from sys import stderr

from vcdtestutil import VCDTestLoader

def parseVCDName(name):
  l = name.split(".")[0].split("_")
  return "_".join(l[:-1])
  
def getTests(vcdfiles):
  l = list()
  for name in vcdfiles:
    try:
      m = __import__(parseVCDName(name))
      l.append(VCDTestLoader(name).loadTestsFromModule(m))
    except Exception, e:
      print >> stderr, "error: %s" % str(e)
  return TestSuite(l)
  
if __name__ == '__main__':

  TextTestRunner(verbosity=2).run(getTests(argv[1:]))

# EOF
