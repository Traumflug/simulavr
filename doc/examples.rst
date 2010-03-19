Examples
========

Simulavr is designed to interact in a few different ways. These
examples briefly explain the examples that can be found in the source
distribution's ``examples`` directory.

There are examples, which use Tcl/Tk. **For that you must also install Itcl
package for your Tcl.** It will be used in all examples with Tcl and a Tk GUI!
Over that you can find also examples for python interface and for the verilog
module.

The anacomp example is all we have started with. Anacomp brings up an
Itcl based GUI which shows two analog input simulations, a comparison
output value, and a toggle button on bottom. After changing the inputs,
hit the corresponding update to clock the simulation to respond to the
changed inputs.

The avr-gdb session for me requires a "load" before hitting "continue",
which actually starts the simulation.

It is strongly recommended to implement own simulation scripts very
closely to the examples. Usage of a different name than ``.x`` for
the grahic frame need changes of gui.tcl as well as some simulavr
sources. So stay better close to the example.

TCL Anacomp Example
-------------------

This is Klaus' very nice original example simulation.

After performing the build, go to the ``examples/anacomp`` directory
and try ``make do`` (without gdb) or ``make dogdb``.

Python Example
--------------

There is a file ``README`` in ``examples/python`` path, which describes
examples there. You can try it with ``make run_example``, this will run all
available examples together. Or try ``make example1`` till ``make example4``
to run each example alone.

Simple Example
--------------

This sample uses only simulavr to execute a hacked AVR program. I say
"hacked" because is shows using 3 simulator features that provide input,
output and simulation termination based on "magic" port access and
reaching a particular symbol.  It is only really useful for getting
your feet wet with simulavr, it is not a great example of how to use
simulavr. It is thought to be useful enough to the absolute newbie to
get you started though.

After performing the build, go to the ``examples/simple_ex1`` directory
and try ``make run_sim``. Notice the use of -W, -R and -T flags.

And again you can try ``make do``, which uses Tcl interface and a Tcl script
to make the simulation. Results are the same as in ``make run_sim``!

LCD and SerialRx, SerialTx Example
----------------------------------

This example is based on Klaus' Anacomp Example and uses the avr-libc
example stdiodemo to display characters on the LCD.

After performing the build, go to the ``examples/stdiodemo`` directory
and try ``./checkdebug.tcl``. The following commands are taken
from the LCD-specific ``examples/stdiodemo/checkdebug.tcl`` script::

  Lcd mylcd $ui "lcd0" ".x"
  sc AddAsyncMember  mylcd

The first command creates a LCD instance ``mylcd`` with the name
``lcd0`` The second command adds the LCD instance to the simulavr
timer subsystem as an asynchronous member.  Asynchronous Timer objects
are updated every 1ns - which means every iteration in the simulavr
main-loop.  All timing is done internally in the ``lcd.cpp``. The
rest of this simulation script is the normal business create Nets for
each LCD pin, wire the Nets to the CPU pins.  The stdiodemo application
contains a serial receiver and transmitter part to receive commands and
interprete it and if possible prints it on the LCD or sends a response to
the serial receiver. Transmitter and receiver application are implemented
by polling opposite to the Keyboard example. The components used for
the SerialRx/Tx are described below. Together with the comments in the
script you should be able to understand what happens. Please mind the
different names for the functions SetBaudRate and GetPin for SerialRx
and SerialTx! Not optimal but that's it at the moment...

And you can try ``make do`` or ``make dogdb``.

Keyboard and SerialRx Example
-----------------------------

This example is based on Klaus' Anacomp Example and uses the Atmel
application note AVR313 to convert the incomming data from the
keyboard into a serial ASCII stream and sends this stream via the serial
interface. Atmel's C-Code is ported to a current avr-gcc (4.x) and a
Mega128. For this example only the serial transmitter is used. Atmel
implemented the serial transmitter as interrupt controlled application,
opposite to the serial transmitter / receiver of the LCD example. Here
a polled solution is implemented.

After performing the build, go to the ``examples/atmel-key`` directory
and try ``./checkdebug.tcl``.  This example by itself is good to
show how the GUI needs to be setup to make the Keyboard component work.
The output of the keyboard is displayed into SerialRx component.
Let's look into the simulation script to point out some details:

**Keyboard:**::

  Keyboard kbd $ui "kbd1" ".x"
  Keyboard_SetClockFreq kbd 40000
  sc Add kbd

These three commands create a Keyboard instance ``kbd`` with
the name ``"kbd1"``. For this instance the clock timing is
set to 40000ns. simulavr internal timing for any asynchronous
activity are multiples of 1ns. The third command adds the keyboard
instance to the simulavr timer. The rest of the commands in
``examples/atmel-key/checkdebug.tcl`` is the normal for this
simmulation. Create a CPU AtMega128 with 4MHz clock. Create indicators
for the digital pins (not necessary but good looking). Create a Net for
each signal - here Clock(key_clk), Data(key_data), Run-LED(key_runLED),
Test-Pin(key_TestPin), and Serial Output(key_txD0). Wire the pins
Net specific. Run-LED and Test-Pin are specific to the Atmel AP-Note
AVR313. The output of the keyboard converter is send to the serial
interface. Based on an "implementation speciality" of simulavr a serial
output must be either set by the AVR program to output or a Pin with a
Pull-Up activated has to be wired.

**SerialRx:**::
  
  SerialRx mysrx $ui "serialRx0" ".x"
  SerialRxBasic_SetBaudRate mysrx 19200

These two commands create a SerialRx instance ``mysrx`` with the name
``"serialRx0"``. For this instance the baud rate is set to 19200. This SerialRx
is wired to the controller pin, a display pin by the following commands::

  ExtPin exttxD0 $Pin_PULLUP $ui "txD0" ".x"
  key_txD0 Add [AvrDevice_GetPin $dev1 "E1"]
  key_txD0 Add exttxD0
  key_txD0 Add [SerialRxBasic_GetPin mysrx "rx"]

The last command ExtPin shows an alternative default value for
txD0-Pin. Here it is pulled high - what is identical of adding any pull-up
resistor to the device pin - no matter which resistor value is used.

While creating this example, simulavr helped to find the bugs left in
the AP-Note.

