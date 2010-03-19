Introduction
============

The SimulAVR program is a simulator for the Atmel AVR family of
microcontrollers.  SimulAVR can be used either standalone or as a
remote target for avr-gdb.  When used in gdbserver mode, the simulator is
used as a back-end so that avr-gdb can be used as a source level debugger for
AVR programs.

SimulAVR started out as a C based project written by Theodore Roth. The hardware
simulation part has since been completely re-written in C++. Only the
instruction decoder and the avr-gdb interface are mostly copied from the 
original simulavr sources. This C++ based version was known as simulavrxx until
it became feature compatibile with the old simulavr code, then it renamed back
to simulavr.

What features are new:

* Run multiple AVR devices in one simulation. (only with interpreter
  interfaces or special application linked against simulavr library) Multiple
  cores can run where each has a different clock frequency.
* Connect multiple AVR core pins to other devices like LCD, LED and
  others. (environment)
* Connect multiple AVR cores to multiple avr-gdb instances. (each on its
  own socket/port number, but see first point for running multiple avr cores)
* Write simulation scripts in Tcl/Tk or Python, other languages could be
  added by simply adding swig scripts!
* Tracing the execution of the program, these traces support all debugging
  information directly from the ELF-file.
* The traces run step by step for each device so you see all actions
  in the multiple devices in time-correct order.
* Every interrupt call is visible.
* Interrupt statistics with latency, longest and shortest execution
  time and some more.
* There is a simple text based UI interface to add LCD, switches,
  LEDs or other components and can modify it during simulation, so there
  is no longer a need to enter a pin value during execution. (Tcl/Tk based)
* Execution timing should be nearly accurate, different access
  times for internal RAM / external RAM / EEPROM and other hardware
  components are simulated.
* A pseudo core hardware component is introduced to do "printf"
  debugging. This "device" is connected to a normal named UNIX socket so
  you do not have to waste a UART or other hardware in your test environment.
* ELF-file loading is supported, no objcopy needed anymore.
* Execution speed is tuned a lot, most hardware simulations are now
  only done if needed.
* External IO pins which are not ports are also available.
* External I/O and some internal states of hardware units (link prescaler
  counter and interrupt states) can be dumped ot into a VCD trace to analyse I/O
  behaviour and timing. Or you can use it for tests.

The core of SimulAVR is functionally a library. This library is linked together
with a command-line interface to create a command-line program. It is also
linked together with a interpreter interface to create a library that can
be use by a graphical interpreter language (currently Python / TCL). In
the examples directory there are examples of simulations with a graphical
environment (with the Tcl/Tk interface) or writing unit tests by using Python
interface. The graphic components do not show any hardware / registers
of the simulated CPU. It shows only external components attached to the
IO-pins of the simulated CPU.

