Graphic User Interface with TCL
===============================

To adjust reader's expectations about simulavr let's start with some
design goals. The main design goals are:

* Create a framework instead of an all-purpose simulator
* Keep the simulator well structured
* Make it easy to extend this simulator
* Develop it for the needs of the developer rather than everybody
  future needs

To find a framework instead of an all-purpose simulator might be confusing
but is the good old habit of Unix programs.  Keep it simple and easy to
extend. That's what can be found over here.

Next let's define what a GUI is necessary for. Showing the source
code, variables and so on is done by avr-gdb and that comes with a GUI
e.g. ddd. There is no need to provide an alternative. Within the examples
provided together with simulavr the following graphical components
are provided by the script gui.tcl:

* Digital-IO Display of the status of an port pin output as well
  as a mechanism to set an input value to an input pin @item Analog Input
  Set an analog value to a port pin
* LCD Have a 4*20 character LCD with a 4 bit data interface
* PC Keyboard Have a PC serial keyboard
* Scope This item is only mentioned here because it is available. The
  function is a development forecast.
* SerialRx / SerialTx Have distinct serial input and output devices

To use any of these a program providing the graphical representation
of these components must run and take / provide contents via the socket
7777. Additionally each currently used instance of these components have
to be registered with the simulation kernel to be updated. The current
implementation adds a new graphic representation of a GUI-component
whenever a new instance of the corresponding component is registered. For
more details see below.

Details of the example GUI
--------------------------

In the following sections all currently available components defined in
the script ``gui.tcl`` are described.  The reader should be aware
that ``gui.tcl`` is an **example**. If you don't like it feel
free to change it accordingly.

UpdateControl
+++++++++++++

While processing the general registration of the GUI (-u parameter or TCL:
``set UI [new_UserInterface 7777 ]``) a button is created. Pressing
this button makes the button's background color change from red to green
vice versa.  While pressing this button values changed by the simulation
are exchanged between the simulation and the GUI. Until this button
pressed, any updates are ignored.

Net
+++

Commonly spoken a Net connects a digital IO-pin of the simulated CPU
with another pin like a copper wire.  In the context of the GUI a Net
provides the possibility to enter a value for an input pin and also
shows the status of an output pin.  Valid values for this GUI element are:

* H representing a "hard" high value - tied the pin directly to the
  supply voltage (TCL: $Pin_HIGH)
* h representing a pulled-up high - here the input is tied by a
  resistor to the supply (TCL: $Pin_PULLUP)
* t Tri-state this input is left open (TCL: $Pin_TRISTATE)
* l like "h" but pulled to GND (TCL: $Pin_PULLDOWN)
* L like "H" but connected to GND (TCL: $Pin_LOW)

Additionally the value "S" might appear, if there is a short circuit
(TCL: $Pin_SHORTED).

For the input direction the values are selected by a radio button.
The following snippet from the TCL example anacomp shows the usage of
the Net component::

  ExtPin epb $Pin_TRISTATE $ui "->BO" ".x"
  Net portb
  portb Add epb
  portb Add [AvrDevice_GetPin $dev1 "B0"]

First there is an endpoint for the Net created with the instance name "epb".

* "epb" is created by calling the class ExtPin (via swig) within
  the simulator (see net.cpp).
* "$Pin_TRISTATE" define the level to be tri-state (no pull-up,
  no pull-down).
* "$ui" is the reference to the wanted GUI.
* "->B0" is the object headline / description.
* ".x" is the window reference.

Next an instance of a digital Net is created named "portb".  The next two
statement wire the Net, one end of the cable is connected to the graphic
while the other end is connected to pin "B0" of the device "$dev1".

Each instance-name and string in the TCL script is case sensitive.
CPU-Pins (e.g. "B0") always begin with a capital character.  Pins names
of external devices (e.g. Clock-Pin of the Keyboard) are always written
in lower-case charcters ("clk").  TCL itself has some ideas of the
components names. If you use lowercase characters it is mostly fine.

AnalogNet
+++++++++

Net and AnalogNet are at least the same.  Digital Nets have potentially
distinct input and output values that represent a smll number of digital
states.  An AnalogNet has a "continuum" of values represented by numbers
in the range from 0..MAX_INT.  Based on the absence of a simulated ADC
this simplified analog model is sufficient but might change in the future.
After entering a analog value into the AnalogNet input field a click on
the update button of this graphic object forwards the analog value to
the simulation::

  ExtAnalogPin pain0 0 $ui "ain0" ".x"
  Net ain0
  ain0 Add pain0
  ain0 Add [AvrDevice_GetPin $dev1 "D6"]

The parameter of ExtAnalogPin are identical to ExtPin, with the difference
of the default value.  Here "0" is the default value. The rest including
the "Net" and "Add" commands are described above.

LCD
+++

The LCD component simulates a simplified character LCD with a HD
44780 compatible controller.  The LCD simulation is simplified for the
following reasons:

* only a 4 * 20 LCD layout is available (no others like 1 * 16, ...).
* the graphic representation is character based. Display of of
  characters follows the rules of your display, not of the LCD character
  generator.
* loadable characters are not supported.
* reading of display is not supported.
* reading of busy flag does not give the current address in the lower bits.
* scrolling not supported.
* shift right / left of the display content is not supported.
* only one character set is supported - based on your diplay font.
* only the 4 bit interface is supported. At start-up the commands
  are interpreted as if an eight bit interface is available (one write
  cycle per command). After finishing the initialization switching to the
  four bit interface is permitted at any time.

With these limitations, one might wonder what actually is supported:

  A simple display of characters with a simplified HD 44780 interface plus some
  easy to implement LCD-controller commands.

The timing as described by the HD 44780 datasheet is used to set the
BusyFlag. Problems detected by the LCD (such as invalid initialization,
command not supported, command to early,...) are output to the standard
error device.  More details of the LCD specifc commands are described
at the LCD example.

Keyboard
++++++++

The Keyboard component simulates a simplified PC keyboard. It generates
Make-Codes and Break-Codes for pressing and releasing a button of the
PC's keyboard. After selecting the keyboard icon in the simulator window
(gui.tcl) keys pressed and released on the PC keyboard are redirected
to Keyboard simulation component. There they are transformed into
a serial stream and sent synchronous with a clock signal to the AVR
application.  The simulation of the keyboard is simplified too. There is
no communication **to** the keyboard supported. Neither reading the
status nor re-/setting of the keyboard LEDs is supported.  More details
of the Keyboard specifc commands are described at the Keyboard example.

SerialRx / SerialTx
+++++++++++++++++++

The SerialRx component as well as the SerialTx component simulates
a serial receiver / transmitter and display. The transfer format is
fixed set to 8n1 (8 Databits, No Parity, 1 Stopbit) The baud rate can
be set to any "unsigned long long" value - not only to the common baud
rates 9600, 19200,... By default the baud rate is set to 115.200. The
graphic representation shows a display field that contains the received
/ entered characters.  The following display translations are made for
the SerialRx component: " " is displayed by "_". Characters which are
not marked by the function ``isprint`` as printable are displayed
in hex-format (e.g. 0x0d for "\n").

The additional three hashed lines in the GUI shall be used for "status",
"pin", "baudrate" in a future release of simulavr. The necessary data
is currently not forwarded by the simulation to the GUI.

The SerialRx component provides a Pin named "rx" that has to be wired as
usual.  The SerialTx component provides a Pin named "tx" that has to be
wired as usual.  For more details of how to use the SerialRx component
see the Keyboard example. A combined SerialRx / SerialTx example is
added to LCD example.

Scope
+++++

The Scope does not yet have a real functioning back-end in the
simulator. Before this feature was implemented completely the development
was halted.

Command Line Parameter -u vs. Interpreter
-----------------------------------------

Coming into touch with simulavr it might be confusing why there is
a simulavr program providing a command-line switch -u and all the
swig story and a interpreter program.  Lets start with a closer look
to the example anacomp/checkdebug.*.  It's a personal preference of
the reader if you look at the python or the TCL source.  There is no
difference in function between them.  Simulavr is able to simulate
the AVR silicon device as well as some external components which will
be called Environment further on.  Each Environment component needs a
graphical representation, a registration in the simulator and a connection
to one or more pins of the simulated CPU (see chapter above).  To keep
these tasks simple and clearly separate the graphical representation
is done by the script examples/gui.tcl.  This script is able only to
display components and forward inputs to the simulator via socket 7777
(and currently only on the local host).

Now we should compare main.cpp of simulavr and anacomp/checkdebug.*.
Both files are the "main" routines (spoken in C-language).  They share
major parts while other's are different.  The simulator core can be
understood as a library that is linked to the main to have a simulator
either with the result of a command line program or with the result of
an extension to an interpreter language

From the beginning of the TCL-script up to ``set sc
[GetSystemClock]`` the script is functional identical to main.cpp with
the corresponding command-line parameters set.  The following line
``$sc AddAsyncMember $ui`` is graphic specific and registers an
update button of the graphic.

The important part for understanding is, defining a NET within the
simulator registers this component.  Only registered components are
updated by the simulator.  The current implementation provides no network
interface to register graphical components.  Instead the swig-I/F is
able to access any function of the simulator core.  Here the framework
character of simulavr becomes visible.  Each specific simulation needs
a specific main-program to display the necessary graphical components.
Within a script file it is much simpler to create a case specific
simulation GUI.

If there is anyone looking for a task to create an all-purpose GUI feel
free to start.

