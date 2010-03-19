The VPI interface to Verilog
============================

Verilog, as a language designed for **veri**\ fying **log**\ ic 
allows to describe a hardware setup in a very general way. Simulators,
such as Icarus Verilog can then be used to simulate this hardware
setup. Tools such as ``gtkwave`` can be used to verify the
output of a circuit by looking at the waveforms the simulation
generates.

Simulavr comes with an interface to (Icarus) Verilog. If the
``./configure`` script finds the necessary header file for the
interface, the so called VPI (Verilog Procedural Interface) to Icarus
Verilog will be build. The result of this is a file called
``avr.vpi``. This file, in essence a shared library, can then be
used as an externally loaded module after compilation::
  
  $ iverilog [...]          # compile verilog .v into .vvp
  
  $ vvp -M<path-to-avr.vpi> -mavr [...] # run compiled verilog 
                                        # with additional
                                        # avr.vpi module

In principle, it would also be possible to implement the AVR
completely in verilog (and there are several existing models, see e.g.
opencores.org), but this would result in decreased performance
and duplicated effort, as not only the core needs to be implemented,
but also the complex on-board periphery.

Usage
-----

The Verilog interface comes with glue code on the verilog side, for
which the main file is ``avr.v`` in ``src/verilog``. This is a
thin wrapper in Verilog around the exported methods from the core of
Simulavr, consisting of the ``AVRCORE`` module encapsulating one
AVR core and ``avr_pin`` for I/O through any AVR pin. On top of
this, files named ``avr_*.v`` exist in the same directory which
contain verilog modules reflecting particular AVR models from
Simulavr. The modules in these files are meant to be the interface to
be used to connect to simulavr by the user, they have a very simple signature::

  module AVRxyz(CLK, port1, port2, ...);

where ``port1``, ``port2``, ... are simple arrays of
``inout`` wires representing the various ports of the selected
AVR. Note that the width of the arrays as visible from the Verilog 
side is always eight; this does not mean that all bits are connected
on the simulavr side!

Clock generation and distribution to the AVR cores is done from the
verilog side. Simply connect a clock source with the preferred
frequency to the CLK input of the AVR code.

The more complete, low level interface to simulavr in ``avr.vpi``
can be accessed directly. For documentation of the available
functions, see either ``src/vpi.cpp`` or look into the
implementation of the high level modules in ``avr_*.v``.

Example iverilog command line
-----------------------------

A simple run with the ``avr.vpi`` interface could look like this::

  $ iverilog -s test -v -I. $(AVRS)/avr.v $(AVRS)/avr_ATtiny15.v \
      $(AVRS)/avr_ATtiny2313.v -o test.vvp
      
Here for a model having both an ATtiny15 and an ATtiny2313 in the
simulation, and the top module ``test`` and the environment
variable ``$AVRS`` pointing to the right directory.

A set of a few simple examples has been put into the
``verilog/examples`` subdirectory of the Simulavr source
distribution. This directory also contains a ``Makefile`` which can
be used as an example of command sequences for compiling
verilog, running it and producing ``.vcd`` output files to be
viewed with ``gtkwave``.

Bugs and particularities
------------------------

* No problems have been found when instantiating multiple AVR instances
  inside verilog.
* Analog pins have not been tested and will probably need some changes
  in the verilog-side wrapper code.

