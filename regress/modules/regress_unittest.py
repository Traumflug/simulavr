from unittest import TextTestRunner, TestSuite
from sys import argv, stderr

from vcdtestutil import VCDTestLoader
from simtestutil import SimTestLoader

def parseTargetName(name):
  l = name.split(".")[0].split("_")
  return "_".join(l[:-1])
  
def parseTargetType(name):
  return name.split(".")[-1].lower()
  
targetLoader = {
  "vcd": VCDTestLoader,
  "elf": SimTestLoader,
}

def getTests(targets):
  l = list()
  for name in targets:
    try:
      m = __import__(parseTargetName(name))
      l.append(targetLoader[parseTargetType(name)](name).loadTestsFromModule(m))
    except Exception, e:
      print >> stderr, "error: %s" % str(e)
  return TestSuite(l)
  
if __name__ == '__main__':

  TextTestRunner(verbosity=2).run(getTests(argv[1:]))

# EOF
