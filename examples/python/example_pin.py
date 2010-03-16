# Python Script
import pyavrs

class XPin(pyavrs.Pin):
  
  def SetInState(self, pin):
    pyavrs.Pin.SetInState(self, pin)
    print "<pin: id=%s, in=%s, out=%s>" % (id(self), pin.toChar(), self.toChar()),
    
if __name__ == "__main__":

  print "create 2 pins ..."
  p1 = XPin()
  p1.outState = pyavrs.Pin.LOW
  print "  pin 1: id=%s, out='%s'" % (id(p1), p1.toChar())
  
  p2 = XPin()
  p2.outState = pyavrs.Pin.TRISTATE
  print "  pin 2: id=%s, out='%s'" % (id(p2), p2.toChar())
  
  print "\ncreate net ..."
  n = pyavrs.Net()
  print "  add pin 1",
  n.Add(p1)
  print ""
  print "  add pin 2",
  n.Add(p2)
  print ""
  
  print "  pin 1: out='%s'" % p1.toChar()
  print "  pin 2: out='%s'" % p2.toChar()

  print "\nset pin 1 output to HIGH:",
  p1.SetPin("H")
  print ""
  print "  pin 1: out='%s'" % p1.toChar()
  print "  pin 2: out='%s'" % p2.toChar()

  print "\nset pin 1 output to TRISTATE:",
  p1.SetPin("t")
  print ""
  print "  pin 1: out='%s'" % p1.toChar()
  print "  pin 2: out='%s'" % p2.toChar()

  print "\nset pin 2 output to LOW:",
  p2.SetPin("L")
  print ""
  print "  pin 1: out='%s'" % p1.toChar()
  print "  pin 2: out='%s'" % p2.toChar()
  
# EOF
