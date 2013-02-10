from unittest import TextTestRunner, TestSuite
from sys import argv, stderr, exit

from vcdtestutil import VCDTestLoader
from simtestutil import SimTestLoader, PyTestLoader

def parseTargetName(name):
  n = name.split(".")
  l = n[0].split("_")
  if len(l) == 1 and n[1].lower() == "py":
    return l[0] # just a python file
  return "_".join(l[:-1])
  
def parseTargetType(name):
  return name.split(".")[-1].lower()
  
targetLoader = {
  "vcd": VCDTestLoader,
  "elf": SimTestLoader,
  "py":  PyTestLoader,
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

  res = TextTestRunner(verbosity=2).run(getTests(argv[1:]))
  if res.wasSuccessful():
    exit(0)
  else:
    exit(1)
    
# EOF
