# This is not ready for prime time yet...Please post your experiences
# to the mailing list

import sys, os
sys.path.append("../../src/python/")

traceFile = "trace"

#we use some itcl :-)
# import Itcl???

#load the avr-simulator packag
#
# for now you need to have _simulavr.so and simulavr.py from the src/python subdir in this
# directory
from simulavr import *

print "simulavr loaded"

#now start external generic gui server 
os.system('wish ../gui.tcl &')

#start the trace output to given filename
StartTrace(traceFile)

#start the user interface client 
ui = UserInterface( 7777 )

#create new device
dev1 = AvrDevice_at90s4433()

#load elf file to the device 
dev1.Load("./anacomp" )

#set the clock cycle time [ns]
dev1.SetClockFreq( 250 )

#systemclock must know that this device will be stepped from application
sc = GetSystemClock()

#also the gui updates after each cycle
sc.AddAsyncMember(ui)


#create some external pins
pain0=ExtAnalogPin( 0, ui, "ain0", ".x" )
pain1=ExtAnalogPin( 0, ui, "ain1", ".x" )
epb  =ExtPin( Pin.TRISTATE, ui, "->BO", ".x")


#create some nets which connect the pins 
ain0 = Net()
ain0.Add(pain0)
ain0.Add( dev1.GetPin("D6") )

ain1=Net()
ain1.Add(pain1)
ain1.Add( dev1.GetPin("D7") )


portb = Net()
portb.Add(epb)
portb.Add( dev1.GetPin("B0") )
#tcl syntax: exec xterm -e tail -f $traceFile &

print "Simulation runs endless, please press CTRL-C to abort"

gdb1 = GdbServer( dev1, 1212, 0 )
sc.Add( gdb1 )

os.system("exec ddd --debugger avr-gdb --command checkdebug.gdb &")

#now run simulation
sc.Endless()


