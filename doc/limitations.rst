Limitations
===========

Please be aware, that this chapter is version dependent so compare
document version and software version to ensure both fit together.

Overall Limitations
-------------------

This chapters describes an overview of system wide limitations for
simulavr. Specific limitations see below.

* The documentation of the simulator provides a wide field of
  activities to be carried out.
* Currently not all AVR-CPUs are simulated. There are many ATMega
  and some ATTiny CPU's implemented. If your CPU is not available
  recompile your project and use (for example) a Mega128 CPU for
  simulation. This works only if your destination CPU and
  the Mega128 share **identical** components. Comparing of the names
  e.g. "Timer0" is not sufficient - you need to compare each component
  for identical function!
* simulavr simulates an AVR-CPU and a small amount of environment,
  like IO-network, some analogue components as well as SPI, ...
  There is neither a fully description for the environment available nor
  comprehensive examples around.
* simulavr does not verify if the current instruction
  is available for the selected CPU (e.g. MUL for Tiny,...)
* The current version of simulavr is not validated against the
  avr-gcc regression tests.
* AVR XMEGA are completely **not yet** simulated by simulavr.

CPU Limitations
---------------

This chapters describes an overview of limitations for simulavr. Specific 
limitations see below.  This chapter focuses only on the Mega128 CPU.

The following hardware is **not** simulated by simulavr:

* TWI/I2C Serial Interface
* Analog to Digital Converter Subsystem (Really? What about src/hwad.h file?)
* Analog comparator in some devices (ATmega16/ATmega32)
* Boot Loader Support (incl. Fuses)
* Timer 1 external crystal support (for Real Time Clock)
* Watchdog Timer
* Sleep-command
* Reset-pin is not available
* With activating the Tx-Pin of an UART the DDR-Register is not
  set properly to output. Workaround: Set the Pin's default value to
  PULLUP. While the Pin behaves as Open Colletor (pulls down only) the
  pull-up "resistor" lets the system run as it should.

There are 64kByte of external memory automatically attached to the
Mega128.

While Atmel changed some function details of the EEPROM, Watchdog Timer,
Timer Subsystem, ADC, and USART / USI these subsystems have identical
names but different functions.  Therefore adding a new CPU to simulavr
might end in reprogramming a subsystem!

