from simtestutil import SimTestCase, SimTestLoader
import pysimulavr

class PortConnector(object):

  def __init__(self, tc, pchar, inet):
    self.tc = tc
    self.pchar = pchar
    # connect digital pin as injector to port pin 0 as input
    inet.Add(tc.dev.GetPin(pchar + "0")) # connect to pin 0
    # get a output pin for port pin 1
    self.opin = tc.dev.GetPin(pchar + "1")

  def assertState(self, state):
    # check output state
    o = self.opin.toChar()
    self.tc.assertEqual(o, state, "output value %s1 wrong: got=%s, exp=%s" % (self.pchar, o, state))

class TestCase(SimTestCase):
  
  DELAY = 12000 # run 12 microseconds

  not_port_a = ("at90s4433", "atmega8", "atmega48", "attiny25")
  not_port_c = ("attiny2313", "attiny25")
  not_port_d = ("attiny25", )
  port_e_g =   ("atmega64", "atmega128", "at90can32")

  def create_connections(self, inet):
    ports = list()
    p = self.processorName
    if not p in self.not_port_a: ports.append(PortConnector(self, "A", inet))
    ports.append(PortConnector(self, "B", inet))
    if not p in self.not_port_c: ports.append(PortConnector(self, "C", inet))
    if not p in self.not_port_d: ports.append(PortConnector(self, "D", inet))
    if p in self.port_e_g:
      ports.append(PortConnector(self, "E", inet))
      ports.append(PortConnector(self, "F", inet))
      ports.append(PortConnector(self, "G", inet))
    return ports

  def test_00(self):
    """check port function (I/O)"""
    self.assertDevice()
    self.assertStartTime()
    # create digital pin as injector
    bpin = pysimulavr.Pin()
    bpin.SetPin("L") # set to L state
    bnet = pysimulavr.Net()
    bnet.Add(bpin)
    # connect to ports
    ports = self.create_connections(bnet)
    # skip initialisation
    self.assertInitDone()
    # check, that we are not in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 0, "not in idle loop")
    # run till in idle loop
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    # check, that we are now in idle loop ...
    self.assertEqual(self.sim.getByteByName(self.dev, "in_loop"), 1, "in idle loop")
    # and 1 time processed the complete loop
    c = self.sim.getByteByName(self.dev, "loop_count")
    self.assertTrue(c > 0, "loop one time processed")
    # check output state
    for p in ports: p.assertState("L")
    # set input pin to H
    bpin.SetPin("H") # set to H state
    # run
    self.sim.doRun(self.sim.getCurrentTime() + self.DELAY)
    c2 = self.sim.getByteByName(self.dev, "loop_count")
    self.assertTrue(c2 > c, "loop count incremented")
    # check output state
    for p in ports: p.assertState("H")

if __name__ == '__main__':
  
  from unittest import TextTestRunner
  tests = SimTestLoader("port_at90s4433.elf").loadTestsFromTestCase(TestCase)
  TextTestRunner(verbosity = 2).run(tests)

# EOF
