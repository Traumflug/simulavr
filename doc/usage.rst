Usage
=====

Invoke simulavr::
  
  > simulavr {options}
  
Common options
--------------

``-V, --version``
  show the software version of simulavr
  
``-v, --verbose``
  output some hints to console
  
``-h, --help``
  show commandline help for simulavr and what devices are supported
  
Simulation options
------------------

``-d <device name>, --device <device name>``
  tell simulavr, what type of device it has to simulate. The following devices
  are supported for version 1.0: (to find out, which devices are supported with
  your current installation, use the ``--help`` option)

  - at90can128
  - at90can32
  - at90can64
  - at90s4433
  - at90s8515
  - atmega128
  - atmega1284a
  - atmega16
  - atmega164a
  - atmega168
  - atmega32
  - atmega324a
  - atmega328
  - atmega48
  - atmega644a
  - atmega8
  - atmega88
  - attiny2313
  - attiny25
  - attiny45
  - attiny85

  This option is mandatory, if an elf file isn't given or elf file dosn't contain device
  signature. To put device signature to elf file you can insert the following line to
  your source code (but only once!)::
  
    #include <avr/signature.h>
  
  If this option is given and device signature will be found in elf file, then the given
  signature by device name is compared to signature in elf file. If this isn't equal,
  then simulavr stops with an error message.
  
  **Attention:** some devices doesn't support all peripheral parts of controller. (for
  example CAN peripheral in at90can... devices) Ports and timer are mostly implemented.

``-f <name>, --file <name>``
  load ELF-file <name> for simulation in simulated target.
  
``-F <value>, --cpufrequency <value>``
  set the CPU frequence to <Hz>. Default is 4MHz.
  
``-t <file name>, --trace <file name>``
  enable trace outputs into <file name>
  
``-s, --irqstatistic``
  Writes IRQ statistic to stdout at the end of simulation.

``-C <name>, --core-dump <name>``
  write a core dump to file <name> at simulation exit.
  
GDB options
-----------

.. note::

   Do not run simulavr with `-p`-option unattended and also not with admin rights. This
   could be a security hole for your system!

``-g, --gdbserver``
  running as avr-gdb-server
  
``-G``
  running as avr-gdb-server and write debug info for avr-gdb-connection to stdout.
  Use it alternative to option ``-g``. **This is only useful, if you want to see,
  what data is sent from gdb to simulavr and back!**
  
``-n, --nogdbwait``
  do not wait for avr-gdb connection. Default is to wait for gdb connection, if
  option ``-g`` or ``-G`` is given.
  
``-p <port>``
  change <port> for avr-gdb server to port. Default is port 1212.
  
``--gdb-stdin``
  for use with GDB as ``target remote | ./simulavr``
  
Control options
---------------

``-m  <nanoseconds>``
  maximum run time of <nanoseconds>
  
``-R <offset>,<file>, --readfrompipe <offset>,<file>``
  add a special pipe register to device at IO-offset and opens <file>
  for reading
  
``-T <label or address>, --terminate <label or address>``
  stops simulation if PC runs on <label> or <address>. If this parameter
  is omitted, simulavr has to be terminated manually.
  For <label> you can use any label listed in the map-file of the linker -
  no matter if it is ever reached or not.

``-B <label> or <address>, --breakpoint <label> or <address>``
  same as -T for backward compatibility
  
``-W <offset>,<file>, --writetopipe <offset>,<file>``
  add a special pipe register to device at IO-Offset and opens <file> for writing
  
``-a <offset>, --writetoabort <offset>``
  add a special register to device at IO-Offset which aborts simulation
  
``-e <offset>, --writetoexit <offset>``
  add a special register to device at IO-Offset which exits simulation (if you
  write to this IO-Offset, then the written value will be given back as exit value
  of the simulator!)

``-M``
  disable messages for bad I/O and memory references
  
``-l <number> --linestotrace <number>``
  maximum number of lines in each trace file. 0 means endless. **Attention:** if
  you use gdb & trace, please use always 0!
  
``-M``
  disable messages for bad I/O and memory references
  
The commands -R / -W / -a / -e are not AVR-hardware related. Here you can link
an address within the address space of the AVR to an input or output
pipe. This is a simple way to create a "printf"- debugger, e.g. after
leaving the debugging phase and running the AVR-Software in the simulator or to
abort/exit a simulation on a specified situation inside of your program.
For more details see the example in the directory :file:`examples/simple_ex1` or
:ref:`here <intro-simple-ex>`.

VCD trace options
-----------------

``-o <filename|->``
  Writes all available VCD trace sources for a device to <filename> or to stdout,
  if <-> is given.
  
``-c <trace-params>``
  Enable a trace dump, for valid <trace-params> see below.
  
Special options
---------------

``-u``
  run with user interface for external pin handling at port 7777. This
  does not open any graphics but activates the interface to communicate
  with the TCL environment simulation.
  
Examples
--------

Using the simulator with avr-gdb is very simple. Start simulavr with::

  simulavr -g

Now simulavr opens a socket on port 1212. If you need another port
give the port number with::

  simulavr -p5566

which will start simulavr with avr-gdb socket at port 5566.

After that you can start avr-gdb or ddd with avr-gdb::

  avr-gdb
  
or::

  ddd --debugger avr-gdb

In the comandline of ddd or avr-gdb you can now enter your debug commands::

  file a.out
  target remote localhost:1212
  load
  step
  step
  ....
  quit

**Attention:** In the actual implementation there is a known bug: If you
start in avr-gdb mode and give no file to execute ``-f filename``
you will run into an ``"Illegal Instruction"``.  The reason
is that simulavr runs immediately with an empty flash. But avr-gdb
is not connected and could stop the core. Solution: Please start with
``simulavr -g -f <filename>``. The problem will be fixed later.
It doesn't matter whether the filename of the simulavr command line
is identical to the filename of avr-gdb file command.  The avr-gdb
downloads the file itself to the simulator. And after downloading the
core of simulavr will be reset complete, so there is not a real problem.

Tracing
-------

One of the core features is tracing one or multiple AVR cores in the
simulator.  To enable the trace feature you have simply to add the
``-t`` option to the command line.  If the ELF-file you load into
the simulator has debug information the trace output will also contain
the label information of the ELF-file. This information is printed for
all variables in flash, RAM, ext-RAM and also for all known hardware
registers. Also all code labels will be written to the trace output.

What is written to trace output::
  
  2000 a.out 0x0026: __do_copy_data                 LDI R17, 0x00 R17=0x00
  2250 a.out 0x0028: __do_copy_data+0x1             LDI R26, 0x60 R26=0x60 X=0x0060
  2500 a.out 0x002a: __do_copy_data+0x2             LDI R27, 0x00 R27=0x00 X=0x0060
  2750 a.out 0x002c: __do_copy_data+0x3             LDI R30, 0x22 R30=0x22 Z=0x0022
  3000 a.out 0x002e: __do_copy_data+0x4             LDI R31, 0x01 R31=0x01 Z=0x0122
  3250 a.out 0x0030: __do_copy_data+0x5             RJMP 38
  3500 a.out 0x0038: .do_copy_data_start            CPU-waitstate
  3750 a.out 0x0038: .do_copy_data_start            CPI R26, 0x60 SREG=[------Z-]
  4000 a.out 0x003a: .do_copy_data_start+0x1        CPC R27, R17 SREG=[------Z-]
  4250 a.out 0x003c: __SP_L__                       BRNE ->0x0032 .do_copy_data_loop
  4500 a.out 0x003e: __SREG__,__SP_H__,__do_clear_bss LDI R17, 0x00 R17=0x00
  4750 a.out 0x0040: __SREG__,__SP_H__,__do_clear_bss+0x1 LDI R26, 0x60 R26=0x60 X=0x0060
  5000 a.out 0x0042: __SREG__,__SP_H__,__do_clear_bss+0x2 LDI R27, 0x00 R27=0x00 X=0x0060
  5250 a.out 0x0044: __SREG__,__SP_H__,__do_clear_bss+0x3 RJMP 48
  5500 a.out 0x0048: .do_clear_bss_start            CPU-waitstate

What the columns mean:

* absolute time value, it is measured in nanoseconds (ns)
* the code you simulate, normally shown as the file name of the loaded executable
  file.  If your simulation runs multiple cores with multiple files you can see
  which core is stepping with which instruction.
* actual PC, meaning bytes not instructions! The original AVR
  documentation often writes in instructions, but here we write number of
  flash bytes.
* label corresponding to the address. The label is shown for all
  known labels from the loaded ELF-file.  If multiple labels are located
  to one address all labels are printed. In future releases it is maybe
  possible to give some flags for the labels which would be printed. This
  is dependent on the ELF-file and BFD-library.
* after the label a potential offset to that label is printed. For
  example ``main+0x6`` which means 6 instructions after the
  ``main`` label is defined.
* The decoded AVR instruction. Keep in mind pseudo-opcodes. If
  you wonder why you write an assembler instruction one way and get
  another assembler instruction here you have to think about the Atmel
  AVR instruction set. Some instructions are not really available in
  the AVR-core. These instructions are only supported for convenience
  (i.e. are pseudo-ops) not actual opcodes for the hardware. For example,
  ``CLR R16`` is in the real world on the AVR-core ``EOR R16,R16`` which means
  exclusive or with itself which results also in zero.
* operands for the instruction. If the operands access memory or registers the
  actual values of the operands will also be shown. 

  * If the operands access memory (Flash, RAM) also the labels of the accessed
    addresses will be written for convenience.
  * If a register is able to build a special value with 16 bits range (X,Y,Z)
    also the new value for this pseudo register is printed.
  * If a branch/jump instruction is decoded the branch or jump target is also
    decoded with the label name and absolute address also if the branch
    or jump is relative.
  * A special instruction @command{CPU-waitstate} will be written to
    the output if the core needs more then one cycle for the instruction.
    Sometimes a lot of wait states will be generated e.g. for eeprom access.

* if the status register is affected also the ``SREG=[------Z-]`` is shown.

**Attention:** If you want to run the simulator in connection to the
avr-gdb interface and run the trace in parallel you have to keep in mind
that you MUST load the file in avr-gdb and also in the simulator from
command-line or script. It is not possible to transfer the symbols from
the ELF-file through the avr-gdb interface. For that reason you always
must give the same ELF-file for avr-gdb and for simulavr. If you load
another ELF-file via the avr-gdb interface to the simulator the symbols
for tracing could not be updated which means that the label information
in the trace output is wrong. That is not a bug, this is related to the
possibilities of the avr-gdb interface.

