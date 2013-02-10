from types import IntType, FloatType

from simtestutil import PyTestCase, PyTestLoader
import pysimulavr

class XPin(pysimulavr.Pin):
  
  def SetInState(self, pin):
    pysimulavr.Pin.SetInState(self, pin)
    self.__inState = pin.toChar()
    
  @property
  def inState(self): return self.__inState

class TestCase(PyTestCase):
  
  """
  This tests should check (and also show, how to make) the behaviour of
  2 Pin instances (aka pins with a output stage and a input line) connected
  together. (by a Net instance)
  """

  def setUp(self):
    # Net instance
    self.net = pysimulavr.Net()

  def tearDown(self):
    # destroy net: disconnect pins
    del self.net

  def createPin(self, init = None):
    if init is None:
      p = XPin()
    elif type(init) in (IntType, FloatType):
      p = XPin(init)
    else:
      raise Exception, "Pin: parameter type not allowed"
    self.net.Add(p)
    return p

  def test_00(self):
    """check pin state after creation without parameter"""
    pin1 = self.createPin()
    pin2 = self.createPin()
    self.assertTrue(pin1.toChar() == "t", "pin1 is tristate")
    self.assertTrue(pin1.inState == "t", "pin1 input is tristate")
    self.assertTrue(pin2.toChar() == "t", "pin2 is tristate")
    self.assertTrue(pin2.inState == "t", "pin2 input is tristate")
    self.assertTrue(pin2.GetAnalogValue(5.0) == 2.75, "pin2: analog value is 2.75V (floating)")
        
  def test_01(self):
    """check pin state after creation one with digital out parameter"""
    pin1 = self.createPin(pysimulavr.Pin.HIGH)
    pin2 = self.createPin()
    self.assertTrue(pin1.toChar() == "H", "pin1 is digital output = high")
    self.assertTrue(pin1.inState == "H", "pin1 input is digital high")
    self.assertTrue(pin2.toChar() == "t", "pin2 is tristate")
    self.assertTrue(pin2.inState == "H", "pin2 input is digital high")
    self.assertTrue(pin2.GetAnalogValue(5.0) == 5.0, "pin2: analog value is 5.0V")
        
  def test_02(self):
    """check pin state after creation one with analog out parameter"""
    pin1 = self.createPin(3.0)
    pin2 = self.createPin()
    self.assertTrue(pin1.toChar() == "a", "pin1 is analog output")
    self.assertTrue(pin1.inState == "a", "pin1 input is analog")
    self.assertTrue(pin2.toChar() == "t", "pin2 is tristate")
    self.assertTrue(pin2.inState == "a", "pin2 input is analog")
    self.assertTrue(pin2.GetAnalogValue(5.0) == 3.0, "pin2: analog value is 3.0V")
        
  def test_03(self):
    """check transmission of digital states"""
    pin1 = self.createPin(pysimulavr.Pin.HIGH)
    pin2 = self.createPin()
    self.assertTrue(pin2.inState == "H", "pin2 input is digital = high")
    pin1.SetPin("L")
    self.assertTrue(pin2.inState == "L", "pin2 input is digital = low")
    pin1.SetPin("h")
    self.assertTrue(pin2.inState == "h", "pin2 input is digital = weak high (pullup)")
    pin1.SetPin("l")
    self.assertTrue(pin2.inState == "l", "pin2 input is digital = weak low (pulldown)")

  def test_04(self):
    """check special digital cases (because 2 active outputs together)"""
    pin1 = self.createPin(pysimulavr.Pin.HIGH)
    pin2 = self.createPin(pysimulavr.Pin.HIGH)
    self.assertTrue(pin2.inState == "H", "pin2 input is digital = high")
    pin1.SetPin("L")
    self.assertTrue(pin2.inState == "S", "pin2 input is short circuit")
    pin1.SetPin("l")
    self.assertTrue(pin2.inState == "H", "pin2 input is digital = high")
    pin1.SetPin("h")
    self.assertTrue(pin2.inState == "H", "pin2 input is digital = high")
    pin2.SetPin("l")
    self.assertTrue(pin2.inState == "t", "pin2 input is on tristate level")

  def test_05(self):
    """check special digital/analog cases (because 2 active outputs together)"""
    pin1 = self.createPin(pysimulavr.Pin.HIGH)
    pin2 = self.createPin(1.0)
    self.assertTrue(pin2.inState == "A", "pin2 input is analog short circuit")
    pin1.SetPin("l")
    self.assertTrue(pin2.inState == "A", "pin2 input is analog short circuit")
    pin1.SetPin("a")
    self.assertTrue(pin2.inState == "A", "pin2 input is analog short circuit")

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = PyTestLoader("pin").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
